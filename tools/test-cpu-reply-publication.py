#!/usr/bin/env python3
"""Reproduce the CPU-reply deadline failure under delayed durable writeback.

The control uses the former QSaveFile publisher; the candidate uses the current
publisher. Both publish the same exact RGB565 reply. This is a host IPC fixture,
not original-game comparison or general scheduling acceptance.
"""
import argparse
from array import array
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import time

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    (out/'.retain').write_text('CPU reply writeback regression, including expected failing control.\n')
    paths=['apps/qt-shell/cpu_reply_file.hpp','apps/qt-shell/live_command_renderer.cpp',
           'tests/cpu-reply-sync-delay.c','tests/native-cpu-reply-publication-test.cpp',
           'tools/test-cpu-reply-publication.py','protocols/include/mnm/native_cpu_read_v1.h']
    declaration=json.loads(args.claims.read_text())
    sources=dict(declaration['sources'])
    if any(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=h for p,h in sources.items()):
        raise RuntimeError('Prospective publication sources changed')
    sources.update({p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths})
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],scope=__doc__,cases=[])
    child=None
    try:
        flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True))
        commands=[['c++','-std=c++17','-fPIC','-O2','-Wall','-Wextra','-Werror',
                   str(ROOT/'tests/native-cpu-reply-publication-test.cpp'),'-o',str(out/'publisher'),*flags],
                  ['cc','-shared','-fPIC','-O2','-Wall','-Wextra','-Werror',
                   str(ROOT/'tests/cpu-reply-sync-delay.c'),'-ldl','-o',str(out/'sync-delay.so')]]
        with (out/'build.log').open('x') as log:
            for command in commands:subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,check=True)
        header=struct.pack('<16I',0x434d4e4d,0x44414552,1,64,501,1,0,2,800,600,960000,7,0,5,6,1)
        pixels=array('H',((i*71+19)&65535 for i in range(480000)))
        if struct.pack('=I',1)!=struct.pack('<I',1):pixels.byteswap()
        expected=header+pixels.tobytes();previous=b'previous reply retained until atomic replacement'
        for mode in ('durable','volatile'):
            dest=out/(mode+'.cpu-reply');dest.write_bytes(previous)
            marker=out/(mode+'-sync-entered')
            env=dict(os.environ,LD_PRELOAD=str(out/'sync-delay.so'),MNM_TEST_CPU_SYNC_MARKER=str(marker),
                     MNM_TEST_CPU_SYNC_DIRECTORY=str(out))
            started=time.monotonic();published=None;reads=0
            child=subprocess.Popen([str(out/'publisher'),mode,str(dest)],env=env)
            while time.monotonic()-started<15:
                value=dest.read_bytes();reads+=1
                if value not in (previous,expected):raise RuntimeError('Partial or corrupt reply became visible')
                if value==expected and published is None:published=time.monotonic()-started
                if child.poll() is not None:break
                time.sleep(.005)
            code=child.wait(timeout=2);child=None
            assert code==0 and dest.read_bytes()==expected,(mode,code)
            assert published is not None
            if mode=='durable':assert marker.exists() and published>=10,(mode,published)
            else:assert not marker.exists() and published<3,(mode,published)
            report['cases'].append(dict(mode=mode,publication_seconds=published,
                published_before_reader_deadline=published<10,durable_sync_entered=marker.exists(),
                atomic_observations=reads,exact_rgb565_words=480000,success=True))
        # Failed rename must preserve its destination and remove its temporary.
        blocked=out/'blocked.cpu-reply';blocked.mkdir();pin=blocked/'keep';pin.write_text('keep')
        result=subprocess.run([str(out/'publisher'),'volatile',str(blocked)],check=False)
        assert result.returncode==1 and pin.read_text()=='keep'
        assert not list(out.glob('blocked.cpu-reply.*'))
        report['failed_publication_preserves_destination']=True
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        if child is not None and child.poll() is None:child.kill();child.wait()
        report['sources_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report));return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
