#!/usr/bin/env python3
"""Require native campaign Save/Load, resumed World, original Quit and owned save merge."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import secrets
ROOT=Path(__file__).resolve().parents[1]
PROSPECTIVE={}
CLAIMS=[]
RESTART=False
def module(name,path):
    s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def validate(root,args):
    report=json.loads((root/'report.json').read_text());lines=(root/'shell.log').read_text().splitlines()
    if not report['success'] or report['stage']!=12 or not report['saved_sha256']:raise ValueError('Incomplete native campaign storage journey')
    if not any(l.startswith('Menu launcher exited with status 0.') for l in lines):raise ValueError('Missing normal Quit')
    experiment=Path(next(l[20:] for l in lines if l.startswith('Evidence directory: ')));metadata=json.loads((experiment/'manifest.json').read_text());decoded=module('storage_decoder','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes())
    actions=[(r['menu_id'],r['argument']) for r in decoded if r['event']==3]
    expected=[(3,1),(12,0),(17,3),(6,21),(3,4)] if RESTART else [(3,0),(18,0),(17,1),(13,0),(17,0),(12,0),(17,3),(6,21),(3,4)]
    if actions!=expected:raise ValueError('Original callback sequence mismatch: '+str(actions))
    if len({r['thread'] for r in decoded if r['event'] in (2,3)})!=1:raise ValueError('Original engine thread changed')
    saved=experiment/'game/Save'/(report['saved_name']+'.sav')
    if hashlib.sha256(saved.read_bytes()).hexdigest()!=report['saved_sha256']:raise ValueError('Saved bytes changed')
    merged=json.loads((experiment/'save-store-result.json').read_text());owned=Path(metadata['save_store']['store'])/(report['saved_name']+'.sav')
    if not merged.get('merged') or hashlib.sha256(owned.read_bytes()).hexdigest()!=report['saved_sha256']:raise ValueError('Completed save did not persist in owned store')
    current={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in PROSPECTIVE}
    if current!=PROSPECTIVE:raise ValueError('Sources changed during test')
    built=json.loads((experiment/'bridge-build.json').read_text())
    compiled=built.get('compiled_sources',{})
    if not compiled or any(current.get(p)!=h for p,h in compiled.items()):raise ValueError('Missing or different actual menu compiler input')
    for p,h in current.items():
        if (p.startswith('runtime/menu/') or (p.startswith('protocols/') and Path(p).name.startswith('menu_v'))) and built['sources'].get(Path(p).name)!=h:raise ValueError('Different menu compiler source: '+p)
    report.update(restarted_process=RESTART,source_sha256=metadata['source_sha256'],sources=current,claims=CLAIMS,source_stable=True,menu_compiler_sources=compiled,normal_quit=True,owned_save_persisted=True,original_callbacks=actions,experiment=str(experiment),save_bytes=saved.stat().st_size,native_render=bool(metadata.get('native_render')),scope='Fresh native Qt campaign menus, original Save/new named file/original row Load/resumed World/Mini/normal Quit and owned-store merge; full restored-state, overwrite/failure and cross-process Load remain separate')
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Native campaign Save/Load and owned save persistence passed')
def main():
    global PROSPECTIVE,CLAIMS,RESTART
    if '--restore' in sys.argv:RESTART=True;sys.argv.remove('--restore');os.environ['MNM_CAMPAIGN_STORAGE_RESTART']='1'
    if not RESTART:os.environ['MNM_CAMPAIGN_STORAGE_NAME']='native-menu-'+secrets.token_hex(4)
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='HOST.native-campaign-save-load');scenario=next(s for s in r['scenarios'] if s['id']=='UI.campaign-save-load')
    claims=module('storage_claims','tools/coverage_claims.py');CLAIMS=[dict(behavior=b['id'],contract_sha256=claims.behavior_contract(b,{v['id']:v for v in r['builds']}),scenarios={scenario['id']:claims.scenario_contract(scenario)})]
    paths=set(b['implementation'])|set(b['tests'])|set(b.get('validation_dependencies',[]));paths.update(module('storage_compiler','tools/test-native-campaign.py').linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT));paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/menu').iterdir() if p.is_file());paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols/include/mnm').glob('menu_v*.h'))
    PROSPECTIVE={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)};parent=ROOT/'working/tests/campaign-storage-prospective';parent.mkdir(parents=True,exist_ok=True);snapshot=Path(tempfile.mkdtemp(prefix='run-',dir=parent))/'prospective.json';snapshot.write_text(json.dumps(dict(sources=PROSPECTIVE,claims=CLAIMS),indent=2)+'\n');print(snapshot,flush=True)
    h=module('storage_harness','tools/test-live-menus.py');h.validate=validate;os.environ.update(MNM_CAMPAIGN_SMOKE_TEST='1',MNM_CAMPAIGN_STORAGE='1',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4');sys.argv=[sys.argv[0],'--exit-from','main',*(['--native-commands','--legacy-audio'] if os.environ.get('MNM_CAMPAIGN_STORAGE_NATIVE')=='1' else []),*sys.argv[1:]];h.main()
if __name__=='__main__':main()
