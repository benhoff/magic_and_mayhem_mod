#!/usr/bin/env python3
"""Compare bounded native panel geometry and atomic refusals with pinned PE32."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text());sources={p:sha(ROOT/p) for p in declaration['sources']}
    if sources!=declaration['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/panel-domain';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,original_sha256=HASH,experiment=str(out.relative_to(ROOT)),live_replacement=False,original_work_bypassed=False,
      scope='RGB565 bevel/halo/whitewash original physical pixels, padded rows, corner/pass ordering, degenerate rectangles, per-channel extremes and percent0..1000 with both pressed/reverse booleans. Native complete-footprint refusal before writes. Synthetic external Lock/Unlock adapters, no actual producer suppression, arbitrary caller state or live panel lifetime claim.')
    commands=[]
    def run(label,argv):
        result=subprocess.run([str(a) for a in argv],capture_output=True,text=True,timeout=180);log=out/(label+'.log');log.write_text(result.stdout+result.stderr);commands.append(dict(label=label,argv=[str(a) for a in argv],returncode=result.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)));result.check_returncode();return result.stdout
    executable=ROOT/'working/game-nocd/Chaos.exe'
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);assert sha(executable)==HASH
        corpus=bytearray();index=[]
        for w,h in [(16,16),(17,13),(31,9)]:
            for stride in [w,w+3]:
                for mode in range(15,18):
                    rects=[(0,0,0,0),(0,0,1,1),(3,4,4,5),(1,1,w-3,h-3),(0,0,w-2,h-2)]
                    if mode!=15:rects += [(0,0,w-1,h-1),(w-1,h-1,w-1,h-1)]
                    if mode==17:rects += [(0,0,w,h),(w,h,w,h)]
                    for rect in rects:
                        settings=[(p,pressed,reverse) for p in [0,1,40,99,100,101,1000] for pressed in [0,1] for reverse in [0,1]] if mode==15 else [(0,0,0)]
                        for percent,pressed,reverse in settings:
                            for pattern in range(8):
                                pixels=[([0,0xffff,0xf800,0x7e0,0x1f,0x39e7][pattern] if pattern<6 else (i*1273+113 if pattern==6 else i*7+len(index)*271))&65535 for i in range(stride*h)]
                                corpus+=struct.pack('<11I',w,h,stride,mode,*rect,percent,pressed,reverse)+struct.pack('<'+str(len(pixels))+'H',*pixels)
                                index.append(dict(width=w,height=h,stride=stride,mode=mode,rect=rect,percent=percent,pressed=pressed,reverse=reverse,pattern=pattern))
        # Exhaustive initial word values across a full half-open whitewash.
        for repeat in range(2):
            corpus+=struct.pack('<11I',256,256,256,17,0,0,256,256,0,0,0)+struct.pack('<65536H',*[(i*1273+repeat*31)&65535 for i in range(65536)])
            index.append(dict(width=256,height=256,stride=256,mode=17,rect=[0,0,256,256],percent=0,pressed=0,reverse=0,pattern='exhaustive'))
        corpus=b'MNMPAN01'+struct.pack('<I',len(index))+corpus;(out/'cases.bin').write_bytes(corpus);(out/'cases.json').write_text(json.dumps(index,indent=2)+'\n')
        report['inputs']={str((out/name).relative_to(ROOT)):sha(out/name) for name in ['cases.bin','cases.json']}
        qtflags=shlex.split(run('qt-flags',['pkg-config','--cflags','--libs','Qt6Gui']))
        report['qt_version']=run('qt-version',['pkg-config','--modversion','Qt6Gui']).strip()
        run('reference-build',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',ROOT/'tests/panel-domain-reference.cpp','-o',out/'reference'])
        names=['tests/panel-domain-native.cpp','renderer/canvas_sequence.cpp','renderer/minimap/minimap.cpp','renderer/minimap/overlays.cpp'];objects=[]
        for name in names:
            obj=out/(Path(name).stem+'.o');run(obj.stem,['g++','-O1','-g','-std=c++17','-fPIC','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=address,undefined','-fno-omit-frame-pointer','-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,*qtflags,'-o',obj]);objects.append(obj)
        run('native-link',['g++','-fsanitize=address,undefined',*objects,*qtflags,'-o',out/'native'])
        closure={'tools/test-panel-domain.py','tools/original-manifest.sh'}
        for dep in out.glob('*.d'):
            for p in dep.read_text().replace('\\\n',' ').split(':',1)[1].split():
                if p.startswith(str(ROOT)+'/'):closure.add(str(Path(p).resolve().relative_to(ROOT)))
        if closure-sources.keys():raise ValueError('Unbound compiler dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources']=sorted(closure)
        run('unchanged-original',[out/'reference',executable,out/'cases.bin',out/'original.bin']);report['atomic_result']=run('native-sanitized',[out/'native',out/'cases.bin',out/'native.bin']).strip()
        original=(out/'original.bin').read_bytes();native=(out/'native.bin').read_bytes();at=0;failures=[];counts=Counter()
        for c,row in enumerate(index):
            size=row['stride']*row['height']*2
            if original[at:at+size]!=native[at:at+size]:failures.append(dict(case=c,input=row,offset=at))
            at+=size;counts[str(row['mode'])]+=1
        if len(original)!=at or len(native)!=at or failures:raise ValueError('Panel mismatches: '+repr(failures[:3]))
        report.update(success=True,case_count=len(index),compared_words=at//2,modes=dict(counts),failures=failures,atomic_refusals=int(report['atomic_result'].split()[0]),original_entries={'bevel':dict(address=0x58ddf0,bytes='83ec40a1881f6e00'),'halo':dict(address=0x58d8c0,bytes='83ec38a1881f6e00'),'wash':dict(address=0x58e2c0,bytes='83ec24a1881f6e00')})
    except Exception as error:report['error']=repr(error)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error:report.update(success=False,manifest_error=repr(error))
        report.update(commands=commands,sources_stable=all(sha(ROOT/p)==h for p,h in sources.items()),source_input_stable=sha(executable)==HASH)
        report['success']=report['success'] and report['sources_stable'] and report['source_input_stable']
        report['artifacts']={str(p.relative_to(ROOT)):dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable']}))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
