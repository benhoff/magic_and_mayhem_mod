#!/usr/bin/env python3
"""Independently replay clipped shadow samples with original and native bodies."""
import argparse
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
    parser.add_argument('--directory',type=Path,required=True)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();directory=args.directory.resolve()
    if not directory.is_relative_to(ROOT/'working'):parser.error('Samples must be under working/')
    declaration=json.loads(args.claims.read_text())
    sources={p:sha(ROOT/p) for p in declaration['sources']}
    if sources!=declaration['sources']:raise RuntimeError('Prospective sources changed')
    parent=ROOT/'working/tests/word-clip-replay';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
        original_sha256=ORIGINAL,live_replacement=False,original_work_bypassed=False,
        scope='Eight diverse captured shadow samples (two clipped, six interior) replayed independently against unchanged original and native CPU/state composition. Complete canvas/guards/source and all16 workspace words; frame-pointer writes alone normalized. No live bypass/full-session/Win32 ABI exception claim.',experiment=str(run.relative_to(ROOT)))
    def command(argv,name):
        result=subprocess.run([str(a) for a in argv],capture_output=True,text=True,timeout=180)
        log=run/(name+'.log');log.write_text(result.stdout+result.stderr)
        report.setdefault('commands',[]).append(dict(argv=[str(a) for a in argv],returncode=result.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
        result.check_returncode()
    try:
        command([ROOT/'tools/original-manifest.sh','verify'],'manifest-before')
        exe=ROOT/'working/game-nocd/Chaos.exe';assert sha(exe)==ORIGINAL
        objects=[]
        for p in ['renderer/sprites/word_clipped.c','reconstruction/rendering/word_backend_state.c']:
            obj=run/(Path(p).stem+'.o');objects.append(obj)
            command(['gcc','-m32','-O2','-std=c11','-Wall','-Wextra','-Werror','-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/p,'-o',obj],obj.stem)
        binary=run/'clip-replay'
        command(['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror','-MMD','-MF',run/'reference.d',ROOT/'tests/word-clip-shadow-replay.cpp',*objects,'-o',binary],'reference-build')
        compiled=set()
        for dep in run.glob('*.d'):
            for p in dep.read_text().replace('\\\n',' ').split(':',1)[1].split():
                if p.startswith(str(ROOT)+'/'):compiled.add(str(Path(p).resolve().relative_to(ROOT)))
        assert compiled<=sources.keys(),sorted(compiled-sources.keys())
        report['compiled_project_sources']=sorted(compiled)
        capture_modes=set();records=[];paths=sorted(directory.glob('word-*.bin'));assert len(paths)==8
        for index,path in enumerate(paths):
            b=path.read_bytes();capture_modes.add(struct.unpack_from('<I',b,16)[0]);frame,canvas,size,bytes_=struct.unpack_from('<4I',b,24)
            expected_workspace=list(struct.unpack_from('<16I',b,144))
            expected_workspace[7]=(expected_workspace[7]-frame)&0xffffffff
            expected_workspace[10]=(expected_workspace[10]-frame)&0xffffffff
            expected=b[208+size+bytes_:];assert len(expected)==bytes_
            for mode in ['original','native']:
                output=run/(path.stem+'-'+mode+'.bin')
                command([binary,exe,path,output,mode],path.stem+'-'+mode)
                actual=output.read_bytes()
                assert struct.unpack_from('<I',actual)[0]==0 and actual[4:68]==struct.pack('<16I',*expected_workspace) and actual[68:]==expected,(path,mode)
            fw,fh,ox,oy=struct.unpack_from('<IIii',b,212);ax,ay=struct.unpack_from('<ii',b,52)
            right,bottom=struct.unpack_from('<II',b,40);left,top=struct.unpack_from('<ii',b,64)
            clipped=ax-ox<left or ay-oy<top or ax-ox+fw>=right or ay-oy+fh>=bottom
            records.append(dict(clipped=clipped,frame_sha256=hashlib.sha256(b[208:208+size]).hexdigest(),backend=hex(struct.unpack_from('<I',b,60)[0]+0x400000),auxiliary_offsets=list(struct.unpack_from('<II',b,240)),width=struct.unpack_from('<I',b,212)[0],height=struct.unpack_from('<I',b,216)[0],sample=str(path.relative_to(ROOT)),sha256=sha(path),pixels=bytes_//2,pixels_match=True,workspace_match=True,canvas_alignment=canvas&3))
        assert len(capture_modes)==1 and capture_modes <= {3,4}
        report['capture_mode']=next(iter(capture_modes))
        report['scope']='Eight diverse mode3 shadow or mode4 native bypass samples independently replayed against unchanged original and native CPU/state composition. Complete canvas, guards, source and all16 workspace words; frame-pointer writes normalized. Replay alone does not establish live bypass or full-session equivalence.'
        assert len({r['frame_sha256'] for r in records})>=4 and sum(r['clipped'] for r in records)==2
        report.update(success=True,clipped_sample_count=sum(r['clipped'] for r in records),distinct_frames=len({r['frame_sha256'] for r in records}),samples=records,sample_count=len(records),compared_pixels=sum(r['pixels'] for r in records),binary_sha256=sha(binary),original_pixels_match=True,native_pixels_match=True,workspace_match=True)
    except Exception as error:report['error']=str(error)
    finally:
        try:command([ROOT/'tools/original-manifest.sh','verify'],'manifest-after')
        except Exception as error:report.update(success=False,manifest_error=str(error))
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items())
        report['success']=report['success'] and report['sources_stable']
        (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
        print(json.dumps({k:report.get(k) for k in ['success','sample_count','compared_pixels','error']}),flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
