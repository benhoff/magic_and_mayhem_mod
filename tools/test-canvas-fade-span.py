#!/usr/bin/env python3
"""Compare NoCD contiguous fade words and visible canvas projection."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();claims=json.loads(args.claims.read_text())
    for n,h in claims['sources'].items():
        if sha(ROOT/n)!=h:
            raise ValueError('Prospective source changed: '+n)
    parent=ROOT/'working/tests/canvas-fade-span';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report={'success':False,'sources':claims['sources'],'claims':claims['claims'],
            'scope':'Positive bounded original 58ed80 contiguous packed-WORD loop with private lock/unlock adapters. Full physical padding and logical canvas projection, RGB565/555 masks. No real driver, wrapper ABI or live replacement equivalence.',
            'source_executable_sha256':HASH,'live_replacement':False,'original_pixels_used_as_native_inputs':False}
    def run(label,command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(v) for v in command],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=240,check=True)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(pe)!=HASH:
            raise ValueError('Pinned executable changed')
        image=pe.read_bytes();offset=struct.unpack_from('<I',image,60)[0];sections=offset+24+struct.unpack_from('<H',image,offset+20)[0]
        anchors={}
        for address in (0x58ed80,0x58b660):
            for k in range(struct.unpack_from('<H',image,offset+6)[0]):
                rva,size,raw=struct.unpack_from('<3I',image,sections+k*40+12)
                if rva<=address-0x400000<rva+size-16:
                    at=raw+address-0x400000-rva;anchors[hex(address)]=image[at:at+16].hex();break
        report['anchors']=anchors
        cases=[];manifest=[]
        def case(w,h,stride,format,seed):
            plane=[(i*1273+seed)&65535 for i in range(stride*h)]
            cases.append(struct.pack('<4I',w,h,stride,format&0xffffffff)+struct.pack('<'+'H'*len(plane),*plane))
            manifest.append({'width':w,'height':h,'stride':stride,'format':format,'seed':seed,'physical_words':len(plane),'visible_words':w*h})
        for w in (1,2,3,5,7,8,15,16,31,32,63,64):
            for h in (1,2,3,5,8):
                for stride in (w,w+1,w+3,w+8):
                    for format in (0,1,2,-1):
                        case(w,h,stride,format,113)
        # Odd/even large planes include every possible WORD and cross-row boundaries.
        for w in (255,256,257):
            for stride in (w,w+3):
                for format in (0,1):
                    case(w,256,stride,format,0)
        corpus=out/'cases.bin';corpus.write_bytes(b'MNMFADE1'+struct.pack('<I',len(cases))+b''.join(cases))
        (out/'cases.json').write_text(json.dumps(manifest,indent=2)+'\n')
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'tests/canvas-fade','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'-j4'])
        run('reference-build',['g++','-std=c++17','-m32','-fno-pie','-no-pie','-O0','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',ROOT/'tests/canvas-fade-reference.cpp','-o',out/'reference'])
        deps={'tools/test-canvas-fade-span.py','tests/canvas-fade/CMakeLists.txt'}
        for path in [out/'reference.d',*build.rglob('*.o.d')]:
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                p=Path(name)
                if p.is_absolute():
                    p=p.resolve()
                    if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                        deps.add(str(p.relative_to(ROOT)))
        if deps-set(report['sources']):
            raise ValueError('Undeclared dependencies: '+repr(sorted(deps-set(report['sources']))))
        run('original',[out/'reference',pe,corpus,out/'original.bin'])
        run('native',[build/'canvas-fade-native',corpus,out/'native.bin'])
        a=(out/'original.bin').read_bytes();b=(out/'native.bin').read_bytes()
        if a!=b:
            at=next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
            raise ValueError('Fade mismatch at byte '+str(at))
        run('refusals',[build/'canvas-fade-test'])
        refusals=json.loads((out/'refusals.log').read_text())['atomic_refusals']
        san=out/'sanitize'
        run('sanitize-configure',['cmake','-S',ROOT/'tests/canvas-fade','-B',san,'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'])
        run('sanitize-build',['cmake','--build',san,'--target','canvas-fade-test','-j4'])
        run('sanitize',[san/'canvas-fade-test'])
        report.update(cases=len(cases),words_compared=len(a)//2,physical_words_compared=sum(v['physical_words'] for v in manifest),
                      logical_words_compared=sum(v['visible_words'] for v in manifest),atomic_refusals=refusals,
                      sanitizer_success=True,original_sha256=sha(out/'original.bin'),native_sha256=sha(out/'native.bin'),
                      corpus_sha256=sha(corpus),success=True)
    except Exception as exc:
        report['error']=str(exc)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['success']=report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':
    raise SystemExit(main())
