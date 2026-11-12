#!/usr/bin/env python3
"""Compare shared World WORD entry state/pixels with original code; retain failures."""
from collections import Counter
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import random
import re
import shlex
import struct
import subprocess
import tempfile

from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', '--git-common-dir'], text=True).strip()).resolve().parent
ORIGINAL = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PREFIX = 'research/runtime/caller-state-results-20261108/'
NAMES = ['reconstruction/rendering/word_backend_state.c', 'renderer/sprites/word_raster.c',
         'renderer/sprites/word_clipped.c', 'runtime/scene/canvas_producers.S',
         'tests/word-backend-probe.S']


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def corpus():
    fixtures = load('world_word_fixtures', 'tools/test-word-sprites.py')
    oracle = load('world_word_pixels', 'tools/test-word-backend-state.py')
    records, cases = [], []
    for seed in range(24):
        fw, fh = [1,2,3,4,7,16,129,255][seed%8], [1,2,3][seed//8]
        frame = fixtures.fixture(fw, fh, seed)
        for cl, ct in ((0,0),(2,2)):
            w,h,stride = fw+5,fh+5,fw+8
            positions = [(cl+1,ct+1),(cl,ct),(w-fw,ct+1),(cl+1,h-fh),
                (cl-1,ct+1),(cl+1,ct-1),(w-fw+1,h-fh+1),
                (cl-fw,ct+1),(cl+1,ct-fh),(w,ct+1),(cl+1,h)]
            for position,(left,top) in enumerate(positions):
                for variant in range(12):
                    rng = random.Random(seed*7919+position*113+variant)
                    before = rng.randbytes(stride*h*2)
                    after = oracle.pixels(frame,w,h,stride,left,top,before,cl,ct)
                    workspace = struct.pack('<16I', *[0xabc00000+i*7919+seed for i in range(16)])
                    for backend in (0x197086,0x196cb8):
                        raw = bytearray(fixtures.sample(frame,w,h,stride,left-3,top+2,backend,before,after,(seed%2)*2))
                        struct.pack_into('<III',raw,64,cl,ct,0)
                        struct.pack_into('<I',raw,76,variant)
                        raw[80:144] = workspace
                        records.append(bytes(raw))
                        cases.append(dict(seed=seed,position=position,backend=backend,variant=variant,empty=False))
    for width,height in ((0,0),(0,2),(3,0)):
        frame=struct.pack('<IIIii8sIII',40,width,height,-3,2,b'empty\0\0\0',0xffffffff,0,0)
        for variant in range(12):
            for backend in (0x197086,0x196cb8):
                before=bytes(range(256))
                raw=bytearray(fixtures.sample(frame,8,8,16,3,4,backend,before,before))
                struct.pack_into('<I',raw,76,variant)
                records.append(bytes(raw))
                cases.append(dict(seed=0,position=0,backend=backend,variant=variant,empty=True))
    return records,cases


def analyze(raw, cases):
    if len(raw)!=16+256*len(cases) or raw[:16]!=b'MNMWST01'+struct.pack('<II',1,256):
        raise ValueError('Incomplete original/native state result')
    counts=Counter();failures=[]
    for index,case in enumerate(cases):
        row=struct.unpack_from('<64I',raw,16+256*index)
        if row[1]!=case['backend'] or bool(row[63])!=case['empty']:
            raise ValueError('State result does not bind the planned case')
        counts['cases']+=1
        counts['original_model_matches']+=row[0]==1023
        counts['complete_native_matches']+=row[60]==1023
        for bit,name in enumerate(('workspace','arguments','gpr','stack','flags','x87_status_control',
                                    'x87_values','xmm','mxcsr','pixels_guards_source')):
            counts[name+'_matches']+=bool(row[60]&(1<<bit))
        if (row[0]!=1023 or row[60]!=1023) and len(failures)<12:
            failures.append(dict(case=index,input=case,original_mask=row[0],native_mask=row[60],
                expected_words=list(row[8:24]),actual_arguments=list(row[48:52]),
                fp_status=list(row[54:58]),original_flags=row[5]))
    return dict(counts=dict(counts),examples=failures,
                equivalent=counts['complete_native_matches']==len(cases) and counts['original_model_matches']==len(cases))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name',required=True)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args()
    if not re.fullmatch('[a-z0-9-]+',args.name):parser.error('Require a unique lowercase acquisition name')
    destination=ROOT/(PREFIX+args.name+'.json')
    if destination.exists():raise ValueError('Preserve immutable previous results')
    declaration=json.loads(args.claims.read_text())
    files=set(declaration['sources'])|set(NAMES)|{
        'tools/test-world-word-caller-state.py','tools/test-word-sprites.py','tools/test-word-backend-state.py',
        'tools/build-scene-observer.py','tools/original-manifest.sh',
        'tests/world-word-caller-state-reference.cpp','tests/word-backend-state-reference.cpp',
        'runtime/scene/world_word_state.h','reconstruction/rendering/word_backend_state.h',
        'renderer/sprites/word_raster.h','renderer/sprites/word_clipped.h'}
    frozen={p:(ROOT/p).read_bytes() for p in files}
    sources={p:sha(b) for p,b in frozen.items()}
    if any(sources[p]!=h for p,h in declaration['sources'].items()):raise ValueError('Prospective sources changed')
    report=dict(schema=1,kind='world_word_caller_state_comparison',success=False,claims=declaration['claims'],
        sources=sources,original_sha256=ORIGINAL,original_game_runs=0,newly_passed_requirements=0,
        scope='Actual shared production tags22/36 entry assembly and common prepared state transaction; supplied owned CPU-plane pixel provider. Unchanged original597086/596cb8 versus independent Python pixels, state model, all GPR/ESP, argument mutation, workspace, defined integer flags, selected x87/FX and guards. Twelve masked precision/rounding modes; no whole canvas_producers callback, World batch transport, installed provider/game/profile or all-operation claim.',commands=[])
    with tempfile.TemporaryDirectory(prefix='mnm-world-word-state-',dir='/tmp') as temporary:
        out=Path(temporary)
        def retain(label,raw):
            path=PREFIX+args.name+'-'+label
            write_report(ROOT,path,raw)
            return dict(path=path,sha256=sha(raw),bytes=len(raw))
        def run(label,argv,timeout=300):
            result=subprocess.run(list(map(str,argv)),cwd=ROOT,env=dict(os.environ,TMPDIR='/tmp'),capture_output=True,timeout=timeout)
            log=retain(label+'.log.bin',result.stdout+result.stderr)
            report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=result.returncode,log=log))
            if result.returncode:raise RuntimeError(label+': exit '+str(result.returncode))
            return result.stdout
        try:
            report['source_preimages']={}
            for p,raw in frozen.items():
                old=subprocess.run(['git','show','HEAD:'+p],cwd=ROOT,capture_output=True)
                if old.returncode or old.stdout!=raw:
                    report['source_preimages'][p]=retain('source-'+sha(p.encode())[:16]+'.bin',raw)
            run('manifest-before',[MAIN/'tools/original-manifest.sh','verify'])
            original=MAIN/'original/Arcane_Nocd/Chaos.exe'
            if sha(original.read_bytes())!=ORIGINAL:raise ValueError('Original executable mismatch')
            records,cases=corpus()
            report['corpus']=retain('corpus.bin',b'MNMWSCP1'+struct.pack('<I',len(records))+b''.join(struct.pack('<I',len(r))+r for r in records))
            report['case_index']=retain('case-index.bin',(json.dumps(cases,sort_keys=True)+'\n').encode())
            paths=[]
            for index,raw in enumerate(records):
                path=out/('case-%05d.bin'%index);path.write_bytes(raw);paths.append(str(path))
            manifest=out/'cases.txt';manifest.write_text('\n'.join(paths)+'\n')
            objects=[]
            for index,name in enumerate(NAMES):
                obj=out/(str(index)+'.o')
                run('compile-'+str(index),['gcc','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
                    '-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj])
                objects.append(obj)
            for mode in ('baseline','native'):
                exe=out/(mode+'-reference')
                run(mode+'-link',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',
                    *(['-DMNM_WORLD_WORD_STATE_BASELINE'] if mode=='baseline' else []),
                    '-MMD','-MF',out/(mode+'.d'),ROOT/'tests/world-word-caller-state-reference.cpp',*objects,'-o',exe])
                report[mode+'_executable']=retain(mode+'-reference.bin',exe.read_bytes())
                closure=set()
                for dep in out.glob('*.d'):
                    for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                        path=Path(token).resolve()
                        if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):closure.add(path.relative_to(ROOT).as_posix())
                if closure-files:raise ValueError('Unfrozen compiler dependencies: '+repr(sorted(closure-files)))
                report['compiled_sources']=sorted(closure)
                output=out/(mode+'.bin')
                run(mode+'-original-native',[exe,original,manifest,output])
                raw=output.read_bytes();report[mode+'_raw']=retain(mode+'-states.bin',raw)
                report[mode]=analyze(raw,cases)
                if mode=='baseline':
                    c=report[mode]['counts']
                    if report[mode]['equivalent'] or c['original_model_matches']!=len(cases) or c['pixels_guards_source_matches']!=len(cases):
                        raise ValueError('Pixel-only negative control did not isolate a state discrepancy')
                elif not report[mode]['equivalent']:raise ValueError('Native shared World entry differs from original')
            report['success']=True
        except (ValueError,OSError,RuntimeError,subprocess.SubprocessError) as error:
            report['error']=str(error)
        finally:
            try:run('manifest-after',[MAIN/'tools/original-manifest.sh','verify'])
            except (OSError,RuntimeError,subprocess.SubprocessError) as error:report.update(success=False,manifest_error=str(error))
            report['sources_stable']=all((ROOT/p).read_bytes()==b for p,b in frozen.items())
            report['inputs_stable']=sha((MAIN/'original/Arcane_Nocd/Chaos.exe').read_bytes())==ORIGINAL
            report['success']=report['success']and report['sources_stable']and report['inputs_stable']
            write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'],error=report.get('error'),native=report.get('native',{}).get('counts'),baseline=report.get('baseline',{}).get('counts'))))
    return 0 if report['success']else 1


if __name__=='__main__':raise SystemExit(main())
