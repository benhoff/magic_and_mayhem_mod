#!/usr/bin/env python3
"""Compare complete consumer/finalizer/font pixels and state from owned inputs."""
import argparse
import gzip
import hashlib
import itertools
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def validate(path):
    count=draws=closed=0
    with gzip.open(path,'rb') as f:
        for font in range(6):
            for flags in itertools.product(range(2),repeat=5):
                enabled,ready,blue,large,format=flags
                for rows in (3,5):
                    for line_height in (8,20):
                        for value in (0,1,-1,1234,-2147483648,2147483647):
                            for composed in (0,1,2,3):
                                raw=f.read(56)
                                if len(raw)!=56:raise ValueError('Incomplete finalizer denominator')
                                h=struct.unpack('<14I',raw)
                                expected=(count,font,*flags,rows,line_height,value&0xffffffff,composed)
                                if h[:11]!=expected or h[11]>128 or h[12]>256 or h[13]!=408*96:
                                    raise ValueError('Finalizer record identity or extent')
                                state=f.read(48*4)
                                if len(state)!=48*4:raise ValueError('Truncated metadata')
                                state=struct.unpack('<48I',state)
                                actual=state[42:46] # clip and height/stride precede fill/Lock counts
                                if actual!=(int(enabled and ready),int(enabled and ready),
                                           int(enabled and ready)*(1+bool(composed&2)),int(enabled and ready)*bool(composed&2)):
                                    raise ValueError('Finalizer fill/lease completeness')
                                length=h[11]*4+h[12]*12+h[13]*2
                                if len(f.read(length))!=length:raise ValueError('Truncated finalizer pixels/state')
                                draws+=int(enabled and ready);closed+=int(enabled and ready)*bool(composed&2);count+=1
        if count!=18432 or f.read(1):raise ValueError('Finalizer denominator/trailing bytes')
    return dict(cases=count,draw_cases=draws,post_unlock_font_intervals=closed,
                pixel_words_compared=count*408*96)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output')
    out.mkdir(parents=True);(out/'tmp').mkdir();env=dict(os.environ,TMPDIR=str(out/'tmp'))
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    ids=['RS.world-finalizer-composition']
    paths=set().union(*(required_sources(bs[i]) for i in ids))|{'tools/coverage_claims.py'}
    sources={n:sha(ROOT/n) for n in sorted(paths)}
    report=dict(schema=1,success=False,scope=bs[ids[0]]['scope'],sources=sources,
        claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],builds)) for i in ids],
        original_sha256=BUILD,software_com_boundary=True,actual_driver_or_physical_ownership=False,
        original_pixels_used_as_native_inputs=False,live_replacement=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,command,stream=False,timeout=240):
        print(label,flush=True)
        with (out/(label+'.log')).open('x') as log:
            if stream:
                process=subprocess.Popen([str(x) for x in command],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=log)
                digest=hashlib.sha256();size=0
                try:
                    with (out/(label+'.bin.gz')).open('xb') as destination,gzip.GzipFile(filename='',mode='wb',fileobj=destination,compresslevel=1,mtime=0) as compressed:
                        while True:
                            raw=process.stdout.read(65536)
                            if not raw:break
                            compressed.write(raw);digest.update(raw);size+=len(raw)
                            if size>2*1024**3:raise ValueError('Finalizer output budget exceeded')
                    if process.wait(timeout=timeout):raise ValueError(label+' failed')
                finally:
                    if process.poll() is None:process.kill();process.wait()
                return dict(bytes=size,sha256=digest.hexdigest(),compressed_sha256=sha(out/(label+'.bin.gz')))
            subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=timeout,check=True)
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        executable=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(executable)!=BUILD:raise ValueError('Pinned original changed')
        fonts=sorted((ROOT/'working/game-clean/Sprites').rglob('*.sft'))
        if len(fonts)!=6:raise ValueError('Require six installed SFT fonts')
        report['inputs']={f.relative_to(ROOT).as_posix():sha(f) for f in fonts}
        qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui'],text=True));report['qt_version']=subprocess.check_output(['pkg-config','--modversion','Qt6Gui'],text=True).strip()
        translation_units=['assets/sft.cpp','assets/sprite_loader.cpp','assets/path_resolver.cpp','assets/asset_file.cpp',
            'compat/legacy/font_text.cpp','compat/legacy/font_layout.cpp','reconstruction/rendering/font_byte.cpp',
            'reconstruction/rendering/font_layout.cpp',
            'renderer/canvas_sequence.cpp','renderer/minimap/minimap.cpp','renderer/minimap/overlays.cpp']
        if set(translation_units)-paths:raise ValueError('Undeclared compiler units')
        fixture=ROOT/'tests/world-finalizer-composition.cpp'
        run('native-coverage-build',['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-c',ROOT/'reconstruction/rendering/glyph_backend_state.c','-o',out/'coverage.o'])
        run('native-build',['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror',fixture,*[ROOT/n for n in translation_units],out/'coverage.o',*qt,'-o',out/'native'])
        run('original-build',['g++','-std=c++17','-O2','-m32','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE','-Wall','-Wextra','-Werror',fixture,'-o',out/'original'])
        native=run('native',[out/'native',*fonts],True);original=run('original',[out/'original',executable,*fonts],True)
        left=validate(out/'native.bin.gz');right=validate(out/'original.bin.gz')
        if left!=right:raise ValueError('Finalizer denominators differ')
        if native['sha256']!=original['sha256'] or native['bytes']!=original['bytes']:
            with gzip.open(out/'native.bin.gz','rb') as n,gzip.open(out/'original.bin.gz','rb') as o:
                offset=0
                while True:
                    x=n.read(65536);y=o.read(65536)
                    if x!=y:
                        at=next(i for i,(a,b) in enumerate(zip(x,y)) if a!=b)
                        report['mismatch_byte']=offset+at;report['mismatch_native']=x[max(0,at-16):at+32].hex();report['mismatch_original']=y[max(0,at-16):at+32].hex();break
                    if not x:break
                    offset+=len(x)
            raise ValueError('Original/native finalizer composition differs')
        report.update(success=True,**left,outputs=dict(native=native,original=original),
            original_consumer_finalizer_wrapper_font_glyph_bodies_unchanged=True,
            binary_sha256={n:sha(out/n) for n in ['native','original']})
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if before:
            try:
                run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/n)==v for n,v in sources.items());report['inputs_stable']=all(sha(ROOT/n)==v for n,v in report.get('inputs',{}).items())
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
