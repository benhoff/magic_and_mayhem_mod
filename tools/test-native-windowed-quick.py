#!/usr/bin/env python3
"""Natural original windowed Quick Battle copy callers; not campaign acceptance."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import struct
import subprocess
import sys
import time
from coverage_evidence import write_report
from native_prefix_template import clone
from native_draw_retirement import require_retired
from native_pixel_counters import read_counters,validate_builds
from native_copy_routes import decode
from native_copy_output import validate as validate_output
from native_windowed_copy import validate as validate_natural
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def load(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ['claims','output','report','shell','prefix-template','source-game']:p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args()
 if a.output.exists() or a.report.exists():p.error('Fresh output/report required')
 d=json.loads(a.claims.read_text());sources=d['sources'];stable=lambda:all(sha(ROOT/n)==h for n,h in sources.items())
 if not stable():p.error('Prospective sources changed')
 out=a.output.resolve();out.mkdir(parents=True);(out/'.retain').write_text('Natural windowed Quick Battle caller evidence and attempts.\n')
 report=dict(schema=1,success=False,sources=sources,claims=d['claims'],scope=__doc__,original_game_executed=False,complete_replacement=False,artifacts=str(out))
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
 env.update(PYTHONDONTWRITEBYTECODE='1',TMPDIR=str(out),XDG_CONFIG_HOME=str(out/'config'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',WINEDEBUG='-all',MNM_LIVE_MENU_TEST_QT_JOURNEY='1',MNM_ORIGINAL_WINDOWED_DIAGNOSTIC='1',MNM_MENU_GAME_SOURCE=str(a.source_game.resolve()),MNM_OTHER_PIXEL_PRODUCERS='1',MNM_NATIVE_PIXEL_NO_FORWARD='1',MNM_GLYPH_BACKEND='takeover',MNM_NATIVE_CPU_GUARD='3',MNM_NATIVE_COPY_DIAGNOSTICS=str(out/'copy-diagnostics'),MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all',MNM_VERIFY_ASSETS='1',MNM_VERIFY_ORIGINALS='1')
 server=child=None;verified=False
 def original(label):
  with (out/(label+'.log')).open('xb') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=log,stderr=subprocess.STDOUT,check=True)
 try:
  original('original-before');verified=True
  with (out/'display').open('w+') as display,(out/'xvfb.log').open('xb') as log:
   server=subprocess.Popen(['Xvfb','-displayfd',str(display.fileno()),'-screen','0','1280x1024x16','-nolisten','tcp'],pass_fds=(display.fileno(),),stdout=log,stderr=subprocess.STDOUT)
   deadline=time.monotonic()+10
   while time.monotonic()<deadline:
    display.seek(0);number=display.read().strip()
    if number:env['DISPLAY']=':'+number;break
    if server.poll() is not None:raise ValueError('Private16-bit display exited')
    time.sleep(.1)
   else:raise ValueError('Private display deadline')
  report['prefix_template']=clone(a.prefix_template,out/'wineprefix',env)
  campaign=load('quick_caller_campaign','tools/test-native-campaign.py')
  compiled=campaign.linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT)
  if not compiled<=set(sources):raise ValueError('Undeclared actual shell compiler closure')
  report['compiled_source_count']=len(compiled);report['shell_sha256']=sha(a.shell)
  argv=[str(a.shell.resolve()),'--repo',str(ROOT),'--live-menus','--native-commands','--resident-world','--legacy-audio','--skip-movies','--no-native-music','--live-menu-test',str(out/'flow.json')]
  report['command']=argv
  with (out/'shell.log').open('xb') as log:
   child=subprocess.Popen(argv,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
   code=child.wait(timeout=240)
  report['shell_exit']=code;flow=json.loads((out/'flow.json').read_text());report['flow']=flow
  # Menu state acknowledgements already witness original execution even if
  # the subsequent World input/lifecycle acceptance fails.
  report['original_game_executed']=bool(flow.get('states'))
  if code or not flow.get('success'):raise ValueError('Quick Battle lifecycle failed: '+str(flow.get('error')))
  if [s['screen'] for s in flow['states']]!=[3,22,14,25,14,7,26,26,22,3] or flow['world_visits']!=2:raise ValueError('Incomplete existing Quick Battle lifecycle')
  if not all(flow[k] for k in ['focus_release','resize','escape_results','input_via_qt_viewport']) or flow['native_fallback'] or flow['native_error'] or flow['recoveries']:raise ValueError('Input/native presentation boundary failed')
  lines=(out/'shell.log').read_text().splitlines();paths=[Path(line[20:]) for line in lines if line.startswith('Evidence directory: ')]
  if len(paths)!=1 or not any(line.startswith('Menu launcher exited with status 0.') for line in lines) or any(line.startswith('Smoke test passed:') for line in lines):raise ValueError('Independent normal launcher exit missing')
  e=paths[0];report['experiment']=str(e);report['graphics']=campaign.validate_native_draw(e)
  menu=load('quick_caller_menu','tools/test-menu-observer.py').decode((e/'events.bin').read_bytes())
  actions=[(r['menu_id'],r['argument']) for r in menu if r['event']==3]
  expected=[(3,2),(22,2),(14,2),(25,0),(14,1),(7,12),(26,1),(26,2),(22,3),(3,4)]
  if actions!=expected:raise ValueError('Original callback sequence changed: '+repr(actions))
  report['original_callbacks']=actions
  resident=json.loads((e/'resident-world-report.json').read_text());report['resident']=resident;report['original_game_executed']=int(resident['completed'])>0
  if resident['error'] or not resident['normal_quit_retired'] or resident['pending'] or not (64<=int(resident['completed'])==int(resident['imports'])==int(resident['verified_returns'])):raise ValueError('World ownership/balance failed')
  report['graphics_retirement']=require_retired(e/'native-draw-retirement.bin')
  report['producer_builds']=validate_builds(e,ROOT,sources);report['producers']=read_counters(e/'game')
  if report['producers']['forwarded'] or report['producers']['history_refused']:raise ValueError('Original pixel forward/history refusal')
  forbidden=['native-cpu-guard-fault.bin','native-cpu-guard-refusal.bin','native-cpu-history-refusal.bin','native-cpu-transfer-refusal.bin']
  if any((e/'game'/name).exists() for name in forbidden):raise ValueError('Protected reader boundary refused')
  report['copy_output']=validate_output(e,out/'copy-diagnostics')
  header,rows=decode(e/'lock-capture/copy-routes-v2.bin');report['natural_copy']=validate_natural(rows,report['copy_output'],module_base=header[15],module_extent=header[16])
  report['normal_exit']=True;report['success']=True
 except Exception as error:report['error']=str(error)
 finally:
  if child is not None and child.poll() is None:
   os.killpg(child.pid,signal.SIGTERM)
   try:child.wait(timeout=10)
   except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
  if (out/'wineprefix').is_dir():
   subprocess.run(['wineserver','-k'],env=env|dict(WINEPREFIX=str(out/'wineprefix')),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10)
   subprocess.run(['wineserver','-w'],env=env|dict(WINEPREFIX=str(out/'wineprefix')),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10,check=True)
  if server is not None and server.poll() is None:server.terminate();server.wait(timeout=10)
  if verified:
   try:original('original-after');report['original_verified_after']=True
   except Exception as error:report.update(success=False,original_manifest_error=str(error))
  report['sources_stable']=stable();report['success'] &= report['sources_stable']
  write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps({k:report.get(k) for k in ['success','error','natural_copy','normal_exit','sources_stable']}),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
