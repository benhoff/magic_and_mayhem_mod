#!/usr/bin/env python3
"""Compare native source-only DIB/BMP canvases with frozen independent DC pixels."""
import argparse
import base64
import gzip
import hashlib
import json
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();claims=json.loads(args.claims.read_text())
    for name,digest in claims['sources'].items():
        if sha(ROOT/name)!=digest:
            raise ValueError('Prospective source changed: '+name)
    parent=ROOT/'working/tests/canvas-image-producers';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report={'success':False,'sources':claims['sources'],'claims':claims['claims'],
            'scope':'Source-only RGB565 memory-DIB and positioned BMP canvas production. Memory-DIB output compared with frozen independent Wine DirectDraw DC RGB565 captures; positioned BMP with independent placement of the captured source plane. No new driver/original execution, original wrapper ABI or live bypass.',
            'original_pixels_used_as_native_inputs':False,'live_replacement':False}

    def run(label,command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(v) for v in command],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=240,check=True)

    try:
        with gzip.open(ROOT/'tests/fixtures/surfaces/original-sentinel.json.gz','rt') as source:
            assets=json.load(source)['assets']
        with gzip.open(ROOT/'tests/fixtures/surfaces/surface-dib-ddraw.json.gz','rt') as source:
            rows=json.load(source)['rows']
        captures=[r for r in rows if r['input']['bits']==16]
        if len(captures)!=36:
            raise ValueError('Expected 36 frozen RGB565 driver outputs')
        records=[];manifest=[];expected=bytearray()

        def case(kind,raw,header_size,w,h,x,y,pixels,label):
            fields=[32+len(raw),kind,w,h,x,y,len(raw),header_size]
            records.append(struct.pack('<8I',*(v&0xffffffff for v in fields))+raw)
            expected.extend(struct.pack('<'+'I'*len(pixels),*pixels))
            manifest.append({'kind':kind,'source':label,'destination':[w,h],'placement':[x,y],
                             'words':len(pixels),'expected_sha256':hashlib.sha256(struct.pack('<'+'I'*len(pixels),*pixels)).hexdigest()})

        for row in captures:
            c=row['input'];asset=base64.b64decode(assets[c['asset']],validate=True)
            bits=struct.unpack_from('<H',asset,28)[0];palette=0 if bits==24 else (1<<bits)*4
            sw,sh=struct.unpack_from('<2I',asset,18);pitch=((sw*bits+31)//32)*4
            # Retained original loader corpus submits these sequential bytes to
            # GDI; the DIB packet owns that exact source, without file seeking.
            dib=asset[14:54+palette+pitch*sh]
            tw,th=c['width'],c['height']
            case(19,dib,40+palette,tw,th,0,0,row['pixels'],c['asset'])
            source_row=next(r for r in captures if r['input']['asset']==c['asset'] and r['input']['shape']=='equal')
            plane=source_row['pixels']
            # Canonicalize the file wrapper input so its declared file offset
            # names the same known source plane. No captured destination bytes
            # enter the native test fixture.
            bitmap=bytearray(asset[:14]+dib)
            struct.pack_into('<I',bitmap,2,len(bitmap));struct.pack_into('<I',bitmap,10,54+palette)
            for x,y in ((0,0),(-2,-1),(2,1),(tw,0),(-int(sw)+1,-int(sh)+1),(2147483647,0),(-2147483648,0)):
                pixels=[plane[(dy-y)*sw+dx-x] if 0<=dx-x<sw and 0<=dy-y<sh else 0x2bab
                        for dy in range(th) for dx in range(tw)]
                case(20,bytes(bitmap),0,tw,th,x,y,pixels,c['asset'])
        corpus=out/'cases.bin';corpus.write_bytes(b'MNMIMG01'+struct.pack('<I',len(records))+b''.join(records))
        (out/'cases.json').write_text(json.dumps(manifest,indent=2)+'\n')
        (out/'expected.bin').write_bytes(expected)
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'tests/canvas-image-producers','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','canvas-image-producer-test','-j4'])
        dependencies={'tools/test-canvas-image-producers.py','assets/CMakeLists.txt','tests/canvas-image-producers/CMakeLists.txt'}
        for path in build.rglob('*.o.d'):
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                p=Path(name).resolve() if Path(name).is_absolute() else Path(name)
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    dependencies.add(str(p.relative_to(ROOT)))
        if dependencies-set(report['sources']):
            raise ValueError('Undeclared dependencies: '+repr(sorted(dependencies-set(report['sources']))))
        run('native',[build/'canvas-image-producer-test',corpus,out/'native.bin'])
        result=json.loads((out/'native.log').read_text())
        actual=(out/'native.bin').read_bytes()
        if actual!=expected:
            first=next((i for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),min(len(actual),len(expected)))
            raise ValueError('Image pixel mismatch at byte '+str(first))
        if result['atomic_refusals']!=len(records):
            raise ValueError('Missing atomic image refusals')
        report.update(cases=len(records),driver_cases=len(captures),positioned_cases=len(records)-len(captures),
                      words_compared=sum(c['words'] for c in manifest),atomic_refusals=result['atomic_refusals'],
                      corpus_sha256=sha(corpus),expected_sha256=sha(out/'expected.bin'),native_sha256=sha(out/'native.bin'),
                      native_binary_sha256=sha(build/'canvas-image-producer-test'),success=True)
    except Exception as exc:
        report['error']=str(exc)
    finally:
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['success']=report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
