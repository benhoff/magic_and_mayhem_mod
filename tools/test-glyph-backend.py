#!/usr/bin/env python3
"""Compare actual guarded glyph route/entry and native x87 kernel with unchanged PE32."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
ORIGINAL='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--corpus',type=Path,required=True)
    parser.add_argument('--index',type=Path,required=True)
    args=parser.parse_args();declaration=json.loads(args.claims.read_text())
    sources={p:sha(ROOT/p) for p in declaration['sources']}
    if sources!=declaration['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/glyph-backend';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    inputs={str(p.resolve().relative_to(ROOT)):sha(p) for p in (args.corpus,args.index)}
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,inputs=inputs,
        original_sha256=ORIGINAL,experiment=str(out.relative_to(ROOT)),live_replacement=False,
        original_work_bypassed=False,scope='Actual guarded RET16 entry and explicit owned-context route, native bounded RGB565 RLE pixels, integer/stack/defined flags and selected x87 control/status/TOP/tag/active values plus XMM/MXCSR versus unchanged pinned original. Cold/initialized tables, 24/53/64-bit precision/four rounding controls, contiguous0/1 finite active values and automatic original-once fallback for two active values. Instruction/data pointer history and empty FP register contents excluded. No installed live hook, process-memory admission/lifetime, GPU glyph takeover, higher text layout/lifecycle or arbitrary FP/unmasked exception claim.')
    commands=[]
    def run(label,argv):
        result=subprocess.run([str(a) for a in argv],capture_output=True,text=True,timeout=180)
        log=out/(label+'.log');log.write_text(result.stdout+result.stderr)
        commands.append(dict(label=label,argv=[str(a) for a in argv],returncode=result.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
        result.check_returncode();return result.stdout
    executable=ROOT/'working/game-nocd/Chaos.exe'
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);assert sha(executable)==ORIGINAL
        report['atomic_result']=run('unit-build',['gcc','-O1','-g','-std=c11','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=address,undefined','-fno-omit-frame-pointer',ROOT/'reconstruction/rendering/glyph_backend.c',ROOT/'reconstruction/rendering/glyph_backend_state.c',ROOT/'tests/glyph-backend-test.c','-o',out/'admission'])
        report['atomic_result']=run('atomic-sanitized',[out/'admission']).strip()
        objects=[];closure={'tools/test-glyph-backend.py','tools/original-manifest.sh','tests/glyph-backend-state-probe.S','tests/glyph-backend-entry-probe.S'}
        names=['reconstruction/rendering/glyph_backend.c','reconstruction/rendering/glyph_backend_state.c','reconstruction/rendering/glyph_backend_x87.S','runtime/scene/glyph_route.c','runtime/scene/glyph_entry.S','tests/glyph-backend-state-probe.S','tests/glyph-backend-entry-probe.S']
        pe_objects=[]
        for name in names:
            obj=out/(Path(name).stem+'.o');run(obj.stem,['gcc','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj]);objects.append(obj)
            if name.startswith(('reconstruction/','runtime/')):
                pe=out/(Path(name).stem+'.obj');run(pe.stem+'-pe32',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MMD','-MF',pe.with_suffix('.pe.d'),'-c',ROOT/name,'-o',pe]);pe_objects.append(pe)
        # Resolve native references into one relocatable PE32 object: only the
        # embedder-supplied original trampoline may remain undefined, no CRT.
        run('pe32-combine',['ld','-r','-m','i386pe',*pe_objects,'-o',out/'glyph-combined.obj'])
        report['pe32_undefined']=run('pe32-imports',['nm','-u',out/'glyph-combined.obj']).strip()
        if report['pe32_undefined']!='U _mnm_glyph_original':raise ValueError('Unexpected native runtime helper: '+report['pe32_undefined'])
        reference=out/'glyph-reference'
        run('reference',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',ROOT/'tests/glyph-backend-reference.cpp',*objects,'-o',reference])
        for dependency in out.glob('*.d'):
            for p in dependency.read_text().replace('\\\n',' ').split(':',1)[1].split():
                if p.startswith(str(ROOT)+'/'):closure.add(str(Path(p).resolve().relative_to(ROOT)))
        closure.add('tests/glyph-backend-test.c')
        if closure-sources.keys():raise ValueError('Unbound compiler/source dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources']=sorted(closure)
        cases=json.loads(args.index.read_text());data=args.corpus.read_bytes()
        if data[:8]!=b'MNMGLS01' or struct.unpack_from('<I',data,8)[0]!=len(cases):raise ValueError('Corpus/index count')
        result=out/'results.bin';run('original-native-entry',[reference,executable,args.corpus,result])
        data=result.read_bytes();assert data[:8]==b'MNMGLBK1' and struct.unpack_from('<I',data,8)[0]==64 and len(data)==12+64*(len(cases)+168)
        counts=Counter();failures=[];placements=Counter();controls=Counter()
        executions=(len(data)-12)//64
        for index in range(executions):
            row=struct.unpack_from('<16I',data,12+64*index)
            c=cases[row[15]];mode=row[14]
            counts['complete_matches']+=row[0]==255;counts['native_handled']+=row[1];counts['original_once_fallbacks']+=row[2];counts['original_body_calls']+=row[3];counts['compared_canvas_words']+=row[6]
            placements[c['placement']]+=1;controls[str(c['fp_control_seed'])]+=1
            counts['forced_forward_matches']+=bool(mode) and row[0]==255
            counts['unbound_context_original_once']+=mode==1 and row[3]==1
            expected=(0,0,1) if mode==1 else ((0,1,1) if mode or c['active_x87']==2 else (1,0,0))
            if row[0]!=255 or tuple(row[1:4])!=expected:failures.append(dict(case=index,seed=c,record=list(row)))
        report.update(case_count=len(cases),execution_count=executions,counts=dict(counts),placements=dict(placements),controls=dict(controls),failures=failures,atomic_refusals=int(report['atomic_result'].split()[0]),
            input_provenance=dict(corpus_format='MNMGLS01',case_index_format='glyph-backend-state case manifest',generator='tools/test-glyph-backend-state.py',generator_sha256=sha(ROOT/'tools/test-glyph-backend-state.py')),
            original_entries={'glyph':dict(address=0x581ec0,bytes='83ec288b442430'),'truncation':dict(address=0x59bee0,bytes='558bec83c4f4')})
        if failures:raise ValueError(str(len(failures))+' original/native mismatches; first '+repr(failures[0]))
        report['success']=True
    except Exception as error:report['error']=repr(error)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error:report.update(success=False,manifest_error=repr(error))
        finally:
            report['commands']=commands;report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items());report['inputs_stable']=all(sha(ROOT/p)==h for p,h in inputs.items()) and sha(executable)==ORIGINAL
            report['success']=report['success'] and report['sources_stable'] and report['inputs_stable'] and all(c['returncode']==0 for c in commands)
            report['artifacts']={str(p.relative_to(ROOT)):dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file()}
            (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
            print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable','inputs_stable']}))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
