#!/usr/bin/env python3
"""Exercise source callbacks with an opposite-lock scene producer in real PE32."""
import argparse
import importlib.util
import json
import os
import subprocess
import tempfile
from pathlib import Path
from drawing_api_session import sha
from drawing_fixture_prefix import prepare
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('prefix-template','claims','report'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_bytes());sources=claims['sources']
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    stable();parent=ROOT/'working/tests/drawing-source-lock';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Production source callback lock ordering and watchdog negative.\n')
    spec=importlib.util.spec_from_file_location('builder',ROOT/'tools/build-drawing-session.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder);dll=builder.build(True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-source-lock-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(dll.parent/'packet.obj'),str(dll.parent/'kernel32.lib')],check=True)
    prefix,lock=prepare(ROOT,run,a.prefix_template,None);env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all')
    results=[]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        for name,expected in (('callbacks-outside-journal',0),('inverted-lock-negative',211)):
            target=run/name;target.mkdir();trial=dict(env,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
            if expected:trial['MNM_TEST_INVERSE_LOCK']='1'
            with (target/'wine.log').open('x') as out:r=subprocess.run(['wine',str(binary)],env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=20)
            if r.returncode!=expected:raise ValueError(f'{name}: {r.returncode}, expected {expected}')
            results.append(dict(case=name,exit_code=r.returncode,expected=expected))
        stable();a.report.write_text(json.dumps(dict(schema=1,success=True,sources_stable=True,sources=sources,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),cases=results,binary_sha256=sha(binary.read_bytes()),original_game_executed=False,scope='Production source lease and held snapshot getters complete four Scene-to-journal interleavings. Deliberately reintroducing the inverse order triggers the independent watchdog.'),indent=2)+'\n')
        print(json.dumps(dict(success=True,cases=len(results))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if lock is not None:lock.close()


if __name__=='__main__':main()
