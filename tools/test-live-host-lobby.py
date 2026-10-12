#!/usr/bin/env python3
"""Bounded native multiplayer forms round trip on original Wine graphics/audio."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[1]
IDS=['NP.host-lobby-view-ownership','HOST.native-host-lobby-view']
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def fingerprints():
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());claims=module('intro_claims','tools/coverage_claims.py');builds={b['id']:b for b in register['builds']}
 scenario=next((s for s in register['scenarios'] if s['id']=='UI.host-lobby-view-native'),None)
 if scenario is None:scenario=json.loads((ROOT/'research/runtime/coverage/planned-host-lobby-scenario-20261012.json').read_text())
 paths=set(['tools/test-live-host-lobby.py','tools/test-live-menus.py','tools/coverage_claims.py','tools/build-menu-observer.py','tools/prepare-menu-observer.py','tools/run-menu-observer.py','tools/menu-game-runner.py','tools/run-game.sh','runtime/shadow/win32_min.h','apps/qt-shell/multiplayer_setup_widget.cpp','apps/qt-shell/multiplayer_setup_widget.hpp'])
 bindings=[]
 for b in register['behaviors']:
  if b['id'] in IDS:
   paths.update(claims.required_sources(b));bindings.append(dict(behavior=b['id'],contract_sha256=claims.behavior_contract(b,builds),scenarios={scenario['id']:claims.scenario_contract(scenario)}))
 paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/menu').iterdir() if p.is_file())
 paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols/include/mnm').glob('menu_v*.h'))
 return dict(sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)},claims=bindings)
def validate(root,args,before):
 report=json.loads((root/'report.json').read_text());screens=[3,22,15,24,22,15,24]
 if not report['success'] or report.get('handoffs')!=2 or [(s['screen'],s['ack']) for s in report['states']]!=list(zip(screens,range(7))):raise RuntimeError('Incomplete native multiplayer form transitions')
 if len({s['thread'] for s in report['states']})!=1:raise RuntimeError('Original thread changed')
 lines=(root/'shell.log').read_text().splitlines()
 if not any(l.startswith('Menu launcher exited with status 0.') for l in lines):raise RuntimeError('Normal original Quit missing')
 experiments=[l[20:] for l in lines if l.startswith('Evidence directory: ')]
 if len(experiments)!=1:raise RuntimeError('Missing experiment provenance')
 exp=Path(experiments[0]);meta=json.loads((exp/'manifest.json').read_text())
 if meta.get('native_render'):raise RuntimeError('Multiplayer forms evidence requires original graphics')
 events=module('intro_decode','tools/test-menu-observer.py').decode((exp/'events.bin').read_bytes());actions=[(r['menu_id'],r['argument']) for r in events if r['event']==3]
 expected=[(3,2),(22,0),(15,0),(24,0),(22,0),(15,0),(24,0),(22,3),(3,4)]
 if actions!=expected:raise RuntimeError('Original callback trace differs: '+str(actions))
 raw=(exp/'channel.bin').read_bytes()
 if len(raw)!=196608 or raw[:8]!=b'MNMMCM17' or struct.unpack_from('<I',raw,20)[0] or struct.unpack_from('<I',raw,144)[0]!=9:raise RuntimeError('V17 channel did not retire after final Quit')
 for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
  if hashlib.sha256((exp/'game'/name).read_bytes()).hexdigest()!=meta[key]:raise RuntimeError('Staged input changed')
 if fingerprints()!=before:raise RuntimeError('Sources/claims changed during native validation')
 # Retain both pre-launch and post-run fingerprints; never infer a scope binding after execution.
 if json.loads((root/'prospective.json').read_text())!=before:raise RuntimeError('Pre-launch source claims differ')
 report.update(before,source_sha256=meta['source_sha256'],experiment=str(exp),original_callback_trace=actions,normal_quit=True,source_stable=True,original_work_bypassed=False,native_host_view=True,live_submission=True,networking_executed=True,peer_join=False,original_graphics_audio=True)
 (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Native one-player TCP host view round trips passed',flush=True)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-shell/mnm-qt-shell');args=parser.parse_args()
 before=fingerprints();harness=module('intro_harness','tools/test-live-menus.py');harness.validate=lambda root,unused:validate(root,args,before)
 original_mkdtemp=harness.tempfile.mkdtemp
 def capture(**kwargs):
  path=original_mkdtemp(**kwargs);(Path(path)/'prospective.json').write_text(json.dumps(before,indent=2)+'\n');return path
 harness.tempfile=SimpleNamespace(mkdtemp=capture)
 os.environ.update(MNM_LIVE_MENU_TEST_HOST_LOBBY='1')
 sys.argv=[sys.argv[0],'--exit-from','main','--legacy-audio','--shell',str(args.shell)];harness.main()
if __name__=='__main__':main()
