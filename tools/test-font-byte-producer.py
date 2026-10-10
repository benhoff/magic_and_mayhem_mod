#!/usr/bin/env python3
"""Compare owned byte text state and canvases with unchanged pinned PE32 code."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    for name, digest in claims['sources'].items():
        if sha(ROOT/name) != digest:
            raise ValueError('Prospective source changed: '+name)
    parent = ROOT/'working/tests/font-byte-producer'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = {'success': False, 'sources': claims['sources'], 'claims': claims['claims'],
              'source_executable_sha256': HASH, 'live_replacement': False,
              'original_pixels_used_as_native_inputs': False,
              'scope': 'NoCD byte advance, admitted single-byte draw cursor/contours and RGB565 canvases. Explicit initialized state, no higher text layout/lifecycle or live bypass.'}
    inputs = {}

    def run(label, command, env=None):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command], cwd=ROOT, env=env, stdout=log,
                           stderr=subprocess.STDOUT, timeout=180, check=True)

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        pe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Original executable changed')
        image=pe.read_bytes();pe_offset=struct.unpack_from('<I',image,60)[0]
        sections=pe_offset+24+struct.unpack_from('<H',image,pe_offset+20)[0]
        anchors={}
        for address in (0x4a58e0,0x4a59e0):
            for index in range(struct.unpack_from('<H',image,pe_offset+6)[0]):
                section=sections+index*40
                rva,size,offset=struct.unpack_from('<3I',image,section+12)
                if rva<=address-0x400000<rva+size-16:
                    at=offset+address-0x400000-rva
                    anchors[hex(address)]=image[at:at+16].hex()
                    break
        if set(anchors)!={'0x4a58e0','0x4a59e0'}:
            raise ValueError('Text entry anchors unavailable')
        report['original_entry_anchors']=anchors
        inputs[str(pe.relative_to(ROOT))] = sha(pe)
        records, manifest = [], []

        def case(raw, label, operations, draw, update, mode, tracking, trailing, x=22, y=26, top=0):
            rows = struct.unpack_from('<I',raw,16)[0]
            if len(trailing) != rows:
                raise ValueError('Contour fixture rows')
            fields = [56+len(raw)+4*(rows+len(operations)),len(raw),len(operations),draw,
                      update,mode,tracking,11,x,y,0,top,256,64]
            records.append(struct.pack('<14I',*(v & 0xffffffff for v in fields))+raw+
                           struct.pack('<'+'I'*rows,*(v & 0xffffffff for v in trailing))+
                           struct.pack('<'+'I'*len(operations),*operations))
            manifest.append({'source':label,'draw':bool(draw),'operations':len(operations),
                             'rows':rows,'words':256*64 if draw else 0,
                             'output_bytes':4*(3+rows)*len(operations)+(256*64*2 if draw else 0),
                             'update':update,'mode':mode,'tracking':tracking,'cursor':[x,y],'top':top})

        fonts = sorted((ROOT/'working/game-clean/Sprites').rglob('*.sft'))
        if len(fonts) != 6:
            raise ValueError('Expected six installed fonts')
        for path in fonts:
            inputs[str(path.relative_to(ROOT))] = sha(path)
            raw = path.read_bytes()
            _,_,version,glyphs,rows = struct.unpack_from('<5I',raw)
            if version != 3 or glyphs > 223 or rows > 128:
                raise ValueError('Installed font domain')
            for mode in (-1,0,1,2):
                for tracking in (-3,0,2):
                    for update in (0,1):
                        case(raw,path.name,list(range(256)),0,update,mode,tracking,
                             [7+r%5 for r in range(rows)])
            # Every available draw byte followed by contour-changing, suppressed
            # bytes and a second glyph; reset cursor per bounded short chain.
            for byte in range(33,33+glyphs):
                for mode,tracking,top,y in ((0,0,0,26),(1,2,23,26),(1,-1,0,1)):
                    case(raw,path.name,[byte,32,95,9,10,ord('A'),ord('!')],1,1,
                         mode,tracking,[0]*rows,y=y,top=top)
            # Long mixed byte sequence exercises retained contours and rejection
            # beyond the right canvas edge without resetting text state.
            sequence=list(b"Magic & Mayhem!?:; ' text\t_\n")*4
            for x in (-10,220):
                case(raw,path.name,sequence,1,1,1,1,[0]*rows,x=x)
        # Signed subtraction/addition wrap is a recovered x86 behavior; keep
        # malformed/overflow frame lookup separate from valid isolated advance.
        raw = bytearray(fonts[0].read_bytes())
        rows = struct.unpack_from('<I',raw,16)[0]
        glyphs = struct.unpack_from('<I',raw,12)[0]
        for i in range(rows*glyphs):
            struct.pack_into('<2I',raw,808+i*8,0x80000000 if i%2 else 0x7fffffff,
                             (i*0x1234567)&0xffffffff)
        for tracking in (-2147483648,2147483647):
            for update in (0,1):
                case(bytes(raw),'synthetic-wrap',list(range(256)),0,update,1,tracking,
                     [2147483647 if r%2 else -2147483648 for r in range(rows)])
        # Zero profile rows still apply tracking and punctuation adjustments.
        raw = fonts[0].read_bytes();rows=struct.unpack_from('<I',raw,16)[0]
        glyphs=struct.unpack_from('<I',raw,12)[0]
        zero=bytearray(raw[:808]+raw[808+rows*glyphs*8:])
        struct.pack_into('<I',zero,4,len(zero));struct.pack_into('<I',zero,16,0)
        case(bytes(zero),'synthetic-zero-rows',list(range(256)),0,1,1,2,[])
        corpus = out/'cases.bin'
        corpus.write_bytes(b'MNMTXT01'+struct.pack('<I',len(records))+b''.join(records))
        (out/'cases.json').write_text(json.dumps(manifest,indent=2)+'\n')
        report.update(cases=len(records),operations=sum(c['operations'] for c in manifest),
                      draw_cases=sum(c['draw'] for c in manifest),
                      words_compared=sum(c['words'] for c in manifest),corpus_sha256=sha(corpus))
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'tests/font-byte','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','font-byte-native','font-byte-test','-j4'])
        run('admission',['ctest','--test-dir',build,'-R','^font-byte-atomic-admission$','--output-on-failure'])
        reference=out/'font-reference'
        run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',
            '-MMD','-MF',out/'reference.d',ROOT/'tests/font-byte-reference.cpp','-o',reference])
        dependencies={'tools/test-font-byte-producer.py','assets/CMakeLists.txt','tests/font-byte/CMakeLists.txt'}
        for path in [*build.rglob('*.o.d'),out/'reference.d']:
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                p=Path(name).resolve() if Path(name).is_absolute() else Path(name)
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    dependencies.add(str(p.relative_to(ROOT)))
        if dependencies-set(report['sources']):
            raise ValueError('Unbound compiler dependencies: '+repr(sorted(dependencies-set(report['sources']))))
        qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui'],text=True))
        sanitized=out/'font-admission-sanitized'
        run('sanitizer-build',['c++','-std=c++17','-O1','-g','-fPIC','-fno-omit-frame-pointer',
            '-fsanitize=address,undefined',ROOT/'tests/font-byte-test.cpp',ROOT/'compat/legacy/font_text.cpp',
            ROOT/'reconstruction/rendering/font_byte.cpp',ROOT/'renderer/canvas_sequence.cpp',
            ROOT/'renderer/minimap/minimap.cpp',ROOT/'renderer/minimap/overlays.cpp',*qt,'-o',sanitized])
        run('sanitizer',[sanitized],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                                       'UBSAN_OPTIONS':'halt_on_error=1'})
        report['sanitizer_passed']=True
        run('original',[reference,pe,corpus,out/'original.bin'])
        run('native',[build/'font-byte-native',corpus,out/'native.bin'])
        old,new=(out/'original.bin').read_bytes(),(out/'native.bin').read_bytes()
        if len(old)!=len(new) or len(old)!=sum(c['output_bytes'] for c in manifest):
            raise ValueError('Text output extent mismatch')
        at=0;unequal=[]
        for i,c in enumerate(manifest):
            end=at+c['output_bytes']
            if old[at:end]!=new[at:end]:
                first=next(k for k,(a,b) in enumerate(zip(old[at:end],new[at:end])) if a!=b)
                unequal.append({'case':i,'first_byte':first,'fixture':c})
            at=end
        report.update(equal=len(records)-len(unequal),mismatches=unequal,
                      output_bytes=len(old),source_and_padding_guards_passed=True,
                      atomic_refusals=6,original_sha256=sha(out/'original.bin'),native_sha256=sha(out/'native.bin'),
                      original_binary_sha256=sha(reference),native_binary_sha256=sha(build/'font-byte-native'))
        report['success']=not unequal
    except Exception as exc:
        report['error']=str(exc)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['inputs']=inputs
        report['inputs_stable']=all(sha(ROOT/n)==h for n,h in inputs.items())
        report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
