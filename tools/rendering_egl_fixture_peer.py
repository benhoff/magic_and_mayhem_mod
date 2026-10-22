#!/usr/bin/env python3
"""Owned Xvfb post-EGL viewport protocol fixture; no original game or scanout."""
import json,os,subprocess,sys,time
from pathlib import Path
from drawing_api_session import sha
from drawing_egl_physical import decode_request,analyze_egl_physical
from coverage_evidence import write_report


def main():
    root,library,binary,peer,target,report=map(Path,sys.argv[1:]);target.mkdir()
    env=dict(os.environ,LD_PRELOAD=str(library),MNM_DRAW_X11_DIRECTORY=str(target),MNM_DRAW_EGL_PHYSICAL='1')
    frames=[];ordinal=1;deadline=time.monotonic()+60
    with (target/'fixture.log').open('xb') as log:
        process=subprocess.Popen([str(binary)],env=env,stdout=log,stderr=subprocess.STDOUT)
        try:
            while process.poll() is None:
                if time.monotonic()>deadline:raise ValueError('EGL viewport fixture deadline')
                request=target/f'egl-physical-request-{ordinal:06d}.bin'
                if request.exists() and request.stat().st_size==128:
                    raw=request.read_bytes();r=decode_request(raw);captures=[];planes=[]
                    for sample in (1,2):
                        path=target/f'root-{ordinal}-{sample}.rgba';begin=time.monotonic_ns()
                        subprocess.run([str(peer),str(path)],check=True,timeout=5)
                        end=time.monotonic_ns();captures.append(dict(begin_ns=str(begin),end_ns=str(end)));planes.append(path.read_bytes())
                    if planes[0]!=planes[1] or len(planes[0])!=1920000:raise ValueError('Owned root viewport did not stabilize')
                    name=f'egl-physical-oracle-{ordinal:06d}.rgba';(target/name).write_bytes(planes[0])
                    frames.append(dict(ordinal=ordinal,pid=r[5],thread=r[6],entry=r[7],method=11,display=r[9],surface=r[10],alias=r[11],width=800,height=600,clock_domain='linux_CLOCK_MONOTONIC',first_capture=captures[0],second_capture=captures[1],path=name,sha256=sha(planes[0]),request_sha256=sha(raw),stable_double_capture=True,settling_samples=[]))
                    if ordinal==4:(target/'egl-physical-close.flag').write_bytes(b'closed\n')
                    ack=target/f'egl-physical-ack-{ordinal:06d}.bin';tmp=ack.with_suffix('.tmp');tmp.write_bytes(b'MNMEACK1'+r[4].to_bytes(4,'little')+r[7].to_bytes(4,'little'));tmp.rename(ack);ordinal+=1
                time.sleep(.001)
            if process.wait()!=0 or len(frames)!=4:raise ValueError('Owned viewport swap fixture failed')
        finally:
            if process.poll() is None:process.kill();process.wait()
    # The fixture's fifth swap observes the close flag and seals the preceding four.
    payload=dict(success=True,original_egl_caller_fenced=True,native_rendering=False,boundary='observed_post_egl_return_physical',all_global_presentation_routes=False,source_inputs_bound=False,all_physical_changes_captured=False,scanout_established=False,frames=frames,transient_bytes=0,transient_byte_budget=0)
    write_report(root,report,(json.dumps(payload)+'\n').encode())
    result=analyze_egl_physical(target,report)
    print(json.dumps(dict(success=True,frames=len(result['frames']),pixels=result['pixels'],actual_owned_root_capture=True,scanout_established=False)))


if __name__=='__main__':main()
