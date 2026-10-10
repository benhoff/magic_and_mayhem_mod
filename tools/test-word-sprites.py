#!/usr/bin/env python3
"""Compare scoped native word rasterization and live samples with original code."""
import argparse
import hashlib
import importlib.util
import json
import random
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES=['renderer/sprites/word_raster.h','renderer/sprites/word_raster.c',
         'runtime/scene/word_route.c','runtime/scene/word_entry.S',
         'runtime/scene/word_workspace.h',
         'runtime/shadow/win32_min.h','tests/word-sprite-reference.cpp',
         'renderer/sprites/word-raster/CMakeLists.txt','tests/word-raster-test.c',
         'tools/build-word-sprites.py','tools/test-word-sprites.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def fixture(width,height,seed):
    rng=random.Random(seed);runs=bytearray();pixels=bytearray();rows=[];table=40+height*8
    for y in range(height):
        rows.append((table+len(runs),len(pixels)))
        # Independent expanded mask/word oracle, including opaque zero.
        mask=[int(rng.randrange(4)!=0) for _ in range(width)]
        if seed%5==0:mask=[0]*width
        if seed%5==1:mask=[1]*width
        x=0;opaque=0
        while x<width:
            n=0
            while x+n<width and mask[x+n]==opaque:n+=1
            runs.append(n)
            if opaque:
                for _ in range(n):pixels+=struct.pack('<H',rng.choice([0,0xffff,0xf800,0x7e0,rng.randrange(65536)]))
            x+=n;opaque=1-opaque
    pixel_base=table+len(runs)
    header=struct.pack('<IIIii8sIII',pixel_base+len(pixels),width,height,-3,2,b'fixture\0',0xffffffff,0,0)
    return header+b''.join(struct.pack('<II',d,pixel_base+p) for d,p in rows)+runs+pixels
def expected(frame,width,height,stride,ax,ay,before):
    out=bytearray(before);w,h,ox,oy=struct.unpack_from('<IIii',frame,4)
    for y in range(h):
        delta,pixel=struct.unpack_from('<II',frame,40+y*8);x=0;opaque=0
        while x<w:
            n=frame[delta];delta+=1
            if opaque:
                at=((ay-oy+y)*stride+ax-ox+x)*2
                out[at:at+n*2]=frame[pixel:pixel+n*2];pixel+=n*2
            x+=n;opaque=1-opaque
    return bytes(out)
def sample(frame,w,h,stride,ax,ay,backend,before,after,parity=0):
    header=struct.pack('<8s18I',b'MNMWRC01',1,208+len(frame)+len(before)*2,2,1,
        0x21000000,0x20000000+parity,len(frame),len(before),w,h,stride,ax&0xffffffff,ay&0xffffffff,backend,0,0,0,0)
    scratch=struct.pack('<16I',*[0xabc000+i for i in range(16)])
    return header+scratch+scratch+frame+before+after
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--live-directory',type=Path,help='Compare native takeover samples and workspace against the unmodified original backend')
    p.add_argument('--report',type=Path,help='New immutable report path')
    args=p.parse_args()
    if args.report and args.report.exists():p.error('Refusing to overwrite evidence')
    if args.live_directory:
        args.live_directory=args.live_directory.resolve()
        if not args.live_directory.is_relative_to(ROOT/'working'):p.error('Live samples must be under working/')
    parent=ROOT/'working/tests/word-sprites';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    sources={name:sha(ROOT/name) for name in SOURCES};exe=ROOT/'working/game-nocd/Chaos.exe'
    report={'schema':1,'success':False,'sources':sources,'original_sha256':HASH,'experiment':str(run.relative_to(ROOT)),
        'live_replacement':False,'scope':'Fully in-bounds direct-word contiguous SPR rows only; selected backend entries. Clipped/empty/indexed/unknown routes excluded; original auxiliary passes and other rendering remain active.'}
    def verify(phase):
        r=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True)
        (run/('original-'+phase+'.log')).write_text(r.stdout+r.stderr);r.check_returncode()
    verify('before')
    try:
        assert sha(exe)==HASH
        build=ROOT/'working/build/word-raster'
        subprocess.run(['cmake','-S',str(ROOT/'renderer/sprites/word-raster'),'-B',str(build)],check=True)
        subprocess.run(['cmake','--build',str(build),'--parallel','4'],check=True)
        subprocess.run(['ctest','--test-dir',str(build),'--output-on-failure'],check=True)
        binary=run/'word-reference';obj=run/'word-raster.o'
        subprocess.run(['gcc','-m32','-O2','-std=c11','-Wall','-Wextra','-Werror','-c',str(ROOT/'renderer/sprites/word_raster.c'),'-o',str(obj)],check=True)
        subprocess.run(['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',
            str(ROOT/'tests/word-sprite-reference.cpp'),str(obj),'-o',str(binary)],check=True)
        spec=importlib.util.spec_from_file_location('word_build',ROOT/'tools/build-word-sprites.py')
        builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder);dll=builder.build()
        def execute(path,mode):
            output=run/(path.stem+'-'+mode+'.bin')
            subprocess.run([str(binary),str(exe),str(path),str(output),mode],check=True,timeout=5)
            raw=output.read_bytes();return struct.unpack_from('<I',raw)[0],raw[4:68],raw[68:]
        matches=0;refusals=0;pixels=0
        for i in range(128):
            fw=[1,2,3,4,7,16,129,255][i%8];fh=[1,2,3,5][i//8%4];f=fixture(fw,fh,i)
            w,h,stride=fw+5,fh+5,fw+8;ax,ay=-2,3
            before=struct.pack('<H',0x1234)*(stride*h);oracle=expected(f,w,h,stride,ax,ay,before)
            for backend in [0x197086,0x196cb8]:
                path=run/f'fixture-{i}-{backend:x}.bin';path.write_bytes(sample(f,w,h,stride,ax,ay,backend,before,oracle,(i%2)*2))
                a=execute(path,'original');n=execute(path,'native')
                assert a[0]==n[0]==0 and a[2]==n[2]==oracle,path
                matches+=1;pixels+=stride*h
            # Out-of-bounds and exact right/bottom-edge requests must preserve all
            # pixels for fallback, rather than attempting partial native output.
            for j,(cx,cy) in enumerate([(-4,ay),(ax,1),(w-fw-3,ay),(ax,h-fh+2),(2**31-1,ay)]):
                path=run/f'refusal-{i}-{j}.bin';path.write_bytes(sample(f,w,h,stride,cx,cy,0x197086,before,before))
                n=execute(path,'native');assert n[0]==2 and n[2]==before;refusals+=1
        # Malformed run/offset/plane inputs refuse before even an early row write.
        valid=fixture(7,3,11);w,h,stride=12,8,16;before=b'\x34\x12'*(stride*h)
        mutations=[(0,0),(28,0),(40,0),(48,0),(36,41),(4,0),(8,2049)]
        for i,(at,value) in enumerate(mutations):
            f=bytearray(valid);struct.pack_into('<I',f,at,value)
            path=run/f'malformed-{i}.bin';path.write_bytes(sample(f,w,h,stride,-2,3,0x197086,before,before))
            n=execute(path,'native');assert n[0]==1 and n[2]==before;refusals+=1
        live=[]
        if args.live_directory:
            paths=sorted(args.live_directory.glob('word-*.bin'));assert paths,'No takeover samples'
            for path in paths:
                b=path.read_bytes();a=execute(path,'original');size,bytes_=struct.unpack_from('<II',b,32)
                after=b[208+size+bytes_:];scratch=list(struct.unpack_from('<16I',b,144));frame=struct.unpack_from('<I',b,24)[0]
                scratch[7]=(scratch[7]-frame)&0xffffffff;scratch[10]=(scratch[10]-frame)&0xffffffff
                assert a[0]==0 and a[2]==after and a[1]==struct.pack('<16I',*scratch),path
                live.append({'sample':str(path.relative_to(ROOT)),'sha256':sha(path),'pixels_match':True,'workspace_match':True,'pixels':bytes_//2})
            raw=(args.live_directory/'stats.bin').read_bytes();assert len(raw)%64==0
            stats=struct.unpack_from('<16I',raw,len(raw)-64)
            assert stats[:6]==(0x574d4e4d,0x31304452,1,64,2,1) and stats[8]>=len(paths) and stats[13]==len(paths) and stats[14]==stats[15]==0,stats
            report.update(live_replacement=True,live_original_pixels_match=True,original_work_bypassed=True,
                live_samples=live,live_stats=list(stats),stats_sha256=sha(args.live_directory/'stats.bin'))
        assert sha(exe)==HASH and all(sha(ROOT/name)==digest for name,digest in sources.items())
        report.update(success=True,synthetic_original_matches=matches,compared_pixels=pixels,
            native_refusals=refusals,sources_stable=True,binary_sha256=sha(binary),dll_sha256=sha(dll),
            host_ctest_passed=True,host_test_sha256=sha(build/'word-raster-test'))
    except Exception as error:
        report['error']=str(error)
        raise
    finally:
        verify('after');target=args.report or run/'report.json';target.parent.mkdir(parents=True,exist_ok=True)
        with target.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
        print(target,flush=True)
if __name__=='__main__':main()
