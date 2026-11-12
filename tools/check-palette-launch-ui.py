#!/usr/bin/env python3
"""Verify the exact public launch UI/accessibility path without starting a game."""
import argparse,json,os,signal,subprocess,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from palette_accessibility import AccessibilityBus
from palette_cycle_journey import PaletteJourney,menus
from native_only_contract import configure
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/palette-cycling-results-20261108/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned UI control required')
 out=ROOT/'working/palette-cycling'/Path(a.report).stem;out.mkdir();(out/'.retain').write_text('Actual no-game launch UI controls and failures. Preserve.\n')
 names=('tools/check-palette-launch-ui.py','tools/palette_accessibility.py','tools/run-palette-cycle.py','tools/palette_cycle_journey.py','tools/scene_attachment_journey.py','tools/rendering_menu_accessibility.py','tools/run-rendering-menu-acceptance.py')
 with Reader(ROOT)as reader:sources={n:reader.digest(n)for n in names}
 q=dict(schema=1,success=False,kind='palette_public_launch_ui_controls',original_game_runs=0,native_player_runs=0,qt_host_runs=1,sources=sources);server=legacy=child=bus=journey=None
 try:
  server,display,_=menus.module('run-native-owned.py').start_display(out,allow_glx=True)
  owner=out/'owner';owner.mkdir();legacy,legacy_display,cookie=menus.module('run-native-owned.py').start_display(owner)
  env=configure({k:v for k,v in os.environ.items()if not k.startswith(('MNM_','WINE'))and k not in('DISPLAY','XAUTHORITY','AT_SPI_BUS_ADDRESS')},[])
  env.update(DISPLAY=display['display'],XAUTHORITY=display['authority'],QT_QPA_PLATFORM='xcb',QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1',QT_ACCESSIBILITY='1',QT_SCALE_FACTOR='1',LP_NUM_THREADS='2',XDG_CONFIG_HOME=str(out/'config'))
  env.update(MNM_NATIVE_OUTPUT_OWNER=str(owner/'owner.json'),MNM_LEGACY_DISPLAY=legacy_display['display'],MNM_LEGACY_XCOOKIE=cookie.hex())
  bus=AccessibilityBus(out,env);env=bus.environment;q['accessibility_bus']=bus.record
  binary=ROOT/'working/build/qt-shell/mnm-qt-shell';q['binary_sha256']=menus.sha(binary)
  command=[str(binary),'--repo',str(ROOT),'--renderer','opengl','--live-menus','--native-commands','--resident-world','--native-only'];q['command']=command
  with(out/'qt.log').open('xb')as log:
   child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
   os.environ.update(DISPLAY=env['DISPLAY'],XAUTHORITY=env['XAUTHORITY'],AT_SPI_BUS_ADDRESS=env['AT_SPI_BUS_ADDRESS'])
   journey=PaletteJourney(child,out,env,(800,600));deadline=time.monotonic()+20
   while True:
    nodes=[v for n,v in journey.access.nodes()]
    if any(v['name']=='Launch game'for v in nodes)and journey.input.find_window():break
    require(child.poll()is None and time.monotonic()<deadline,'No-game public launch UI did not become ready');time.sleep(.1)
   journey.prepare_ui();q['public_controls']=journey.proof;q['launch_control']=next(v for v in nodes if v['name']=='Launch game');q['actions']=journey.actions
   require(not journey.session and all(v.get('object_name')!='launchGameButton'for v in journey.actions),'Unexpected launch action')
   q['success']=True
 except Exception as e:q['error']=str(e)
 finally:
  if journey:journey.input.close()
  if child and child.poll()is None:os.killpg(child.pid,signal.SIGTERM);child.wait(timeout=10)
  if bus:bus.close();q['accessibility_bus']['stopped']=True
  for display_server in (legacy,server):
   if display_server and display_server.poll()is None:display_server.terminate();display_server.wait(timeout=10)
  q['sources_stable']=all(menus.sha(ROOT/n)==h for n,h in sources.items());q['success']&=q['sources_stable'];q['artifacts']=[]
  for i,f in enumerate(sorted(out.rglob('*'))):
   if f.is_file()and not f.is_symlink()and f.name!='Xauthority':
    n=a.report.removesuffix('.json')+'-artifact-'+str(i)+'.bin';q['artifacts'].append(dict(path=str(f.relative_to(out)),record=n,sha256=write_report(ROOT,n,f.read_bytes())))
  h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h)))
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
