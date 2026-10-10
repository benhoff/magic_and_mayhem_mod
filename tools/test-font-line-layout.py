#!/usr/bin/env python3
"""Compare bounded higher text state and pixels with unchanged pinned PE32."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args()
    claims=json.loads(args.claims.read_text())
    for name,digest in claims['sources'].items():
        if sha(ROOT/name)!=digest:
            raise ValueError('Prospective source changed: '+name)
    parent=ROOT/'working/tests/font-line-layout';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report={'success':False,'sources':claims['sources'],'claims':claims['claims'],
            'source_executable_sha256':HASH,'live_replacement':False,
            'original_pixels_used_as_native_inputs':False,
            'scope':'Bounded NoCD rectangle text, height-limited text and measurement, full cursor/contours, line triples, consumed offset and RGB565 canvases. Explicit initialized raster state; no allocation/lifetime, machine ABI or live bypass.'}
    inputs={}

    def run(label,command,env=None):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,
                           stderr=subprocess.STDOUT,timeout=240,check=True)

    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(pe)!=HASH:
            raise ValueError('Original executable changed')
        inputs[str(pe.relative_to(ROOT))]=sha(pe)
        image=pe.read_bytes();pe_offset=struct.unpack_from('<I',image,60)[0]
        sections=pe_offset+24+struct.unpack_from('<H',image,pe_offset+20)[0]
        anchors={}
        for address in (0x4a5ce0,0x4a6190,0x4a6630):
            for index in range(struct.unpack_from('<H',image,pe_offset+6)[0]):
                section=sections+index*40;rva,size,offset=struct.unpack_from('<3I',image,section+12)
                if rva<=address-0x400000<rva+size-16:
                    pos=offset+address-0x400000-rva;anchors[hex(address)]=image[pos:pos+16].hex();break
        if len(anchors)!=3:
            raise ValueError('Missing text entry anchors')
        report['original_entry_anchors']=anchors
        records=[];manifest=[];font_data=[]

        def case(raw,label,text,kind,flags,mode,tracking,right=220,bottom=128,first_x=25,top=4,line_height=None,left=8):
            rows,ascent,descent=struct.unpack_from('<3i',raw,16)
            if line_height is None:
                line_height=ascent+descent
            if kind==1 and (bottom-top)&0xffffffff < (line_height&0xffffffff):
                return # Original first-iteration look-behind is unsafe here.
            trailing=[7+r%5 for r in range(rows)]
            fields=[72+rows*4+len(text)+1,len(font_data)-1,len(text)+1,kind,mode,tracking,
                    11,77,29,line_height,first_x,flags,left,top,right,bottom,128,64]
            records.append(struct.pack('<18I',*(v&0xffffffff for v in fields))+
                           struct.pack('<'+'I'*rows,*trailing)+text+b'\0')
            manifest.append({'font':label,'text_hex':text.hex(),'kind':kind,'flags':flags,
                             'mode':mode,'tracking':tracking,'rows':rows,'bounds':[left,top,right,bottom],
                             'first_x':first_x,'line_height':line_height,'words':128*64})

        fonts=sorted((ROOT/'working/game-clean/Sprites').rglob('*.sft'))
        if len(fonts)!=6:
            raise ValueError('Expected six installed fonts')
        texts=[b'A',b'Magic & Mayhem!',b"!?:; ' ` \x91\x92\xb4",b'one\ttwo_three',
               b'A\nB',b'AA\nBB\nC',b'\nA',b'A\n',b'  A  B  ',
               b'A\x01B\x1fC\xffD',b'longer words wrap across lines ',b'AAA BBB CCC DDD EEE FFF ',
               b'AAA BBBB CCCCC DDDDDD EEEEEEE ']
        for path in fonts:
            raw=path.read_bytes();font_data.append(raw);inputs[str(path.relative_to(ROOT))]=sha(path)
            for kind in range(3):
                for flags in (0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30):
                    for text in texts:
                        for mode,tracking in ((0,0),(1,2),(2,-1)):
                            case(raw,path.name,text,kind,flags,mode,tracking)
                # Height equality, first-line offset, Y clipping, retained row
                # state, and all byte values safe for these original consumers.
                for bottom in (12,16,24,32,48):
                    for flags in (0,2,4,8):
                        case(raw,path.name,b'A BB CCC D ',kind,flags,1,1,bottom=bottom)
                for byte in range(1,256):
                    case(raw,path.name,bytes([byte]),kind,0,1,1)
                for left,right,top in ((-5,40,-5),(240,250,4),(8,65,4)):
                    for flags in (0,2,4,16,18,20):
                        if flags&16 and (left<0 or right-left<40):
                            continue # Native refusal, never an original fixture.
                        case(raw,path.name,b'AA BB CC DD EE FF ',kind,flags,1,0,right=right,left=left,top=top)
            case(raw,path.name,b'',0,0,0,0)
            # 255 line-break limit admits the final 256th line without reading
            # farther. Measurement checks table/state; drawing clips at canvas.
            for kind in (0,2):
                case(raw,path.name,b'A\n'*257+b'B',kind,0,0,0)
        # Original line resets clear exactly32 rows, including when active
        # profile rows extend farther. Frame data remains independently owned.
        raw=fonts[0].read_bytes();old_rows=struct.unpack_from('<I',raw,16)[0]
        metrics=struct.unpack_from('<I',raw,36)[0]
        metric_start=40+(768 if struct.unpack_from('<I',raw,32)[0] else 0)
        for rows in (0,33,64,128):
            pairs=b''.join(struct.pack('<2i',-2+(row%5),2+(row%7))
                           for glyph in range(metrics) for row in range(rows))
            synthetic=bytearray(raw[:metric_start]+pairs+raw[metric_start+old_rows*metrics*8:])
            struct.pack_into('<I',synthetic,4,len(synthetic));struct.pack_into('<I',synthetic,16,rows)
            font_data.append(bytes(synthetic))
            for kind in range(3):
                for flags in (0,2,4,8,16,20,30,255):
                    case(bytes(synthetic),'synthetic-profile-'+str(rows),b'AA BB\nCC DD ',kind,flags,1,2)
            for tracking in (-2147483648,2147483647):
                for flags in (0,2,4,6):
                    case(bytes(synthetic),'synthetic-wrap32-'+str(rows),b"A!?:;\t_",2,flags,1,tracking)
        corpus=out/'cases.bin';corpus.write_bytes(b'MNMLAY02'+struct.pack('<2I',len(records),len(font_data))+
            b''.join(struct.pack('<I',len(f))+f for f in font_data)+b''.join(records))
        (out/'cases.json').write_text(json.dumps(manifest,indent=2)+'\n')
        report.update(cases=len(records),words_compared=sum(c['words'] for c in manifest),corpus_sha256=sha(corpus))
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'tests/font-layout','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','font-layout-native','font-layout-test','-j4'])
        run('admission',['ctest','--test-dir',build,'-R','^font-layout-atomic-admission$','--output-on-failure'])
        reference=out/'font-layout-reference'
        run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',
            '-MMD','-MF',out/'reference.d',ROOT/'tests/font-layout-reference.cpp','-o',reference])
        dependencies={'tools/test-font-line-layout.py','assets/CMakeLists.txt','tests/font-layout/CMakeLists.txt'}
        for path in [*build.rglob('*.o.d'),out/'reference.d']:
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                p=Path(name).resolve() if Path(name).is_absolute() else Path(name)
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    dependencies.add(str(p.relative_to(ROOT)))
        if dependencies-set(report['sources']):
            raise ValueError('Unbound compiler dependencies: '+repr(sorted(dependencies-set(report['sources']))))
        qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui'],text=True))
        sanitized=out/'admission-sanitized'
        run('sanitizer-build',['c++','-std=c++17','-O1','-g','-fPIC','-fno-omit-frame-pointer',
            '-fsanitize=address,undefined',ROOT/'tests/font-layout-test.cpp',ROOT/'compat/legacy/font_layout.cpp',
            ROOT/'compat/legacy/font_text.cpp',ROOT/'reconstruction/rendering/font_layout.cpp',
            ROOT/'reconstruction/rendering/font_byte.cpp',ROOT/'renderer/canvas_sequence.cpp',
            ROOT/'renderer/minimap/minimap.cpp',ROOT/'renderer/minimap/overlays.cpp',*qt,'-o',sanitized])
        run('sanitizer',[sanitized],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
        report['sanitizer_passed']=True
        run('native',[build/'font-layout-native',corpus,out/'native.bin'])
        run('original',[reference,pe,corpus,out/'original.bin'])
        old,new=(out/'original.bin').read_bytes(),(out/'native.bin').read_bytes()

        def segments(data):
            at=0;parts=[]
            for c in manifest:
                lines=struct.unpack_from('<I',data,at+16+c['rows']*4)[0]
                if lines>256:
                    raise ValueError('Compared line extent')
                end=at+20+c['rows']*4+lines*12+c['words']*2
                parts.append(data[at:end]);at=end
            if at!=len(data):
                raise ValueError('Compared output extent')
            return parts

        unequal=[]
        for i,(a,b) in enumerate(zip(segments(old),segments(new))):
            if a!=b:
                first=next((k for k,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
                unequal.append({'case':i,'first_byte':first,'fixture':manifest[i],
                                'original_prefix':a[:min(len(a),200)].hex(),'native_prefix':b[:min(len(b),200)].hex()})
        report.update(equal=len(records)-len(unequal),mismatches=unequal,output_bytes=len(old),
                      source_object_and_padding_guards_passed=True,atomic_refusals=9,
                      original_sha256=sha(out/'original.bin'),native_sha256=sha(out/'native.bin'),
                      original_binary_sha256=sha(reference),native_binary_sha256=sha(build/'font-layout-native'))
        report['success']=not unequal
    except Exception as exc:
        report['error']=str(exc)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['inputs']=inputs;report['inputs_stable']=all(sha(ROOT/n)==h for n,h in inputs.items())
        report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
