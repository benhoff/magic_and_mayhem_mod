#!/usr/bin/env python3
"""Bounded native Main Load/Cancel live validation with independent callback trace."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
PROSPECTIVE={}
CLAIMS=[]
def module(name,path):
    s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def validate(root,args):
    report=json.loads((root/'report.json').read_text());lines=(root/'shell.log').read_text().splitlines()
    if not report['success'] or [(s['screen'],s['ack']) for s in report['states']]!=[(3,0),(12,1),(3,2),(12,3),(3,4)]:raise ValueError('Incomplete native Load round trip')
    if len({s['thread'] for s in report['states']})!=1:raise ValueError('Engine thread changed')
    if not any(l.startswith('Menu launcher exited with status 0.') for l in lines):raise ValueError('Missing normal original Quit')
    roots=[Path(l[20:]) for l in lines if l.startswith('Evidence directory: ')];
    if len(roots)!=1:raise ValueError('Missing unique experiment')
    experiment=roots[0];metadata=json.loads((experiment/'manifest.json').read_text());rows=module('storage_decoder','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes())
    actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
    if actions!=[(3,1),(12,1),(3,1),(12,1),(3,4)]:raise ValueError('Original callback trace mismatch: '+str(actions))
    raw=(experiment/'channel.bin').read_bytes()
    if len(raw)!=131072 or raw[:8]!=b'MNMMCM13':raise ValueError('Wrong V13 channel')
    for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
        if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError('Staged input changed')
    sources=['tools/test-live-storage-menus.py','apps/qt-shell/live_storage_menu_test.cpp','apps/qt-shell/live_storage_menu_controller.cpp','apps/qt-shell/live_storage_menu_controller.hpp','apps/qt-shell/live_menu_test.cpp','apps/qt-shell/live_menu_test.hpp','apps/qt-shell/menu_bridge.cpp','apps/qt-shell/menu_bridge.hpp','apps/qt-shell/live_menu_session.cpp','apps/qt-shell/live_menu_session.hpp','apps/qt-shell/main.cpp','apps/qt-shell/CMakeLists.txt','runtime/menu/storage.h','runtime/menu/channel.h','runtime/menu/observer.c','runtime/menu/mini.h','tools/build-menu-observer.py','tools/run-menu-observer.py','protocols/include/mnm/menu_v13.h']
    current={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in PROSPECTIVE}
    if current!=PROSPECTIVE:raise ValueError('Source changed during the live run')
    built=json.loads((experiment/'bridge-build.json').read_text())
    for p,h in current.items():
        if (p.startswith('runtime/menu/') or (p.startswith('protocols/') and Path(p).name.startswith('menu_v'))) and built['sources'].get(Path(p).name)!=h:raise ValueError('Menu bridge was built from different source: '+p)
    report.update(source_sha256=metadata['source_sha256'],sources=current,claims=CLAIMS,source_stable=True,shell_sha256=hashlib.sha256(args.shell.resolve().read_bytes()).hexdigest(),experiment=str(experiment),original_callback_trace=actions,normal_quit=True,live_validated=True,original_drawing_retained=True)
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Native Main Load/Cancel twice and normal Quit passed')
def main():
    global PROSPECTIVE,CLAIMS
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behavior=next(b for b in register['behaviors'] if b['id']=='NP.storage-menu-ownership');scenario=next(s for s in register['scenarios'] if s['id']=='UI.storage-main-roundtrip')
    accounting=module('storage_claims','tools/coverage_claims.py');builds={b['id']:b for b in register['builds']}
    CLAIMS=[dict(behavior=behavior['id'],contract_sha256=accounting.behavior_contract(behavior,builds),scenarios={scenario['id']:accounting.scenario_contract(scenario)})]
    paths=set(behavior['implementation'])|set(behavior['tests'])|set(behavior.get('validation_dependencies',[]))
    paths.update(module('storage_compiler','tools/test-native-campaign.py').linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT))
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/menu').iterdir() if p.is_file())
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols/include/mnm').glob('menu_v*.h'))
    PROSPECTIVE={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}
    parent=ROOT/'working/tests/storage-menu-prospective';parent.mkdir(parents=True,exist_ok=True)
    snapshot=Path(tempfile.mkdtemp(prefix='run-',dir=parent))/'prospective.json'
    snapshot.write_text(json.dumps({'sources':PROSPECTIVE,'claims':CLAIMS},indent=2)+'\n');print('Prospective source/claim snapshot: '+str(snapshot),flush=True)
    h=module('storage_harness','tools/test-live-menus.py');h.validate=validate;os.environ['MNM_LIVE_MENU_TEST_STORAGE']='1';sys.argv=[sys.argv[0],'--exit-from','main',*sys.argv[1:]];h.main()
if __name__=='__main__':main()
