#!/usr/bin/env python3
"""Run a bounded Quick Battle through native Qt menus, viewport input and Quit."""
import argparse,hashlib,importlib.util,json,os,struct,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def load(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for k in ('shell','claims','report'):p.add_argument('--'+k,type=Path,required=True)
 a=p.parse_args()
 if a.report.exists():p.error('Refusing report overwrite')
 claims=json.loads(a.claims.read_text());sources=claims['sources']
 if any(sha(ROOT/n)!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
 binary=sha(a.shell);harness=load('qt_app_harness','tools/test-live-menus.py');created=[];original_temp=tempfile.mkdtemp
 def new_run(*args,**kwargs):
  result=original_temp(*args,**kwargs);root=Path(result)
  if root.parent==ROOT/'working/tests/live-menus':created.append(root);(root/'.retain').write_text('Native Qt Quick Battle app journey; preserve failed and passing inputs/evidence.\n')
  return result
 harness.tempfile.mkdtemp=new_run
 result=dict(schema=1,success=False,sources=sources,claims=claims['claims'],qt_binary_sha256=binary,original_game_executed=False,independent_pixel_comparison_performed=False,complete_drawing_replacement=False,original_cpu_raster_retained=True,scope='Actual Qt Main/setup/World/Results/Main/Quit via default owned native graphics and native commands. Qt viewport key input, focus release, resize, active paints and normal child exit. Original CPU surface writers remain active; this does not claim source-owned whole-frame replacement or new pixel parity.')
 def validate(root,unused):
  report=json.loads((root/'report.json').read_text());lines=(root/'shell.log').read_text().splitlines()
  if not report['success'] or [(s['screen'],s['ack']) for s in report['states']]!=list(zip((3,22,14,25,14,7,26,22,3),range(9))):raise ValueError('Qt journey incomplete')
  if len({s['thread'] for s in report['states']})!=1:raise ValueError('Menu engine thread changed')
  if not all(report[k] for k in ('focus_release','resize','escape_results','input_via_qt_viewport')) or report['native_fallback'] or report['native_error'] or report['recoveries']:raise ValueError('Native input/presentation boundary failed')
  if not report['native_frames'] or not report['native_paints'] or not report['world_presentation']:raise ValueError('No real native viewport paint')
  if not any(line.startswith('Menu launcher exited with status 0.') for line in lines) or any(line.startswith('Smoke test passed:') for line in lines):raise ValueError('Normal original Quit missing')
  paths=[Path(line.removeprefix('Evidence directory: ')) for line in lines if line.startswith('Evidence directory: ')]
  if len(paths)!=1:raise ValueError('Staged experiment provenance missing')
  experiment=paths[0];metadata=json.loads((experiment/'manifest.json').read_text())
  graphics=load('qt_app_graphics','tools/test-native-campaign.py').validate_native_draw(experiment)
  rows=load('qt_app_menu','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes())
  actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
  expected=[(3,2),(22,2),(14,2),(25,0),(14,1),(7,12),(26,2),(22,3),(3,4)]
  if actions!=expected:raise ValueError('Original semantic callback sequence changed: '+str(actions))
  channel=(experiment/'channel.bin').read_bytes();words=struct.unpack_from('<48I',channel)
  if words[5] or words[36]!=9:raise ValueError('Menu channel did not retire final acknowledged action')
  input_raw=(experiment/'render-input.bin').read_bytes();input_words=struct.unpack('<272I',input_raw)
  if input_raw[:8]!=b'MNMINK01' or input_words[5] or any(w&0x80000000 for w in input_words[16:]):raise ValueError('Input remained active or held after Quit')
  if sha(experiment/'game/Chaos.exe')!=metadata['staged_sha256'] or sha(experiment/'game/MnmMenu.dll')!=metadata['dll_sha256']:raise ValueError('Staged executable/menu changed')
  result.update(success=True,original_game_executed=True,artifacts=str(root.relative_to(ROOT)),experiment=str(experiment.relative_to(ROOT)),qt_report_sha256=sha(root/'report.json'),qt=report,native_graphics=graphics,callback_trace=actions,normal_exit=True,channel_retired=True,input_cleared_after_quit=True,original_manifest_verified_before_after=True)
 harness.validate=validate
 os.environ.update(MNM_LIVE_MENU_TEST_QT_JOURNEY='1',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',MNM_GLYPH_BACKEND='takeover')
 os.environ.pop('MNM_NATIVE_DRAW',None)
 sys.argv=[sys.argv[0],'--shell',str(a.shell.resolve()),'--native-commands','--battle','results']
 try:harness.main()
 except (Exception,SystemExit) as exc:
  result['error']=str(exc)
  if created:
   result['artifacts']=str(created[0].relative_to(ROOT))
   log=created[0]/'shell.log'
   if log.exists():
    experiments=[Path(line.removeprefix('Evidence directory: ')) for line in log.read_text().splitlines() if line.startswith('Evidence directory: ')]
    if len(experiments)==1 and (experiments[0]/'events.bin').exists():
     result['original_game_executed']=bool(load('qt_app_failed_menu','tools/test-menu-observer.py').decode((experiments[0]/'events.bin').read_bytes()))
     result['experiment']=str(experiments[0].relative_to(ROOT))
   qt=created[0]/'report.json'
   if qt.exists():result['qt']=json.loads(qt.read_text());result['qt_report_sha256']=sha(qt)
 finally:
  tempfile.mkdtemp=original_temp
  if sha(a.shell)!=binary or any(sha(ROOT/n)!=h for n,h in sources.items()):result.update(success=False,sources_stable=False,error='Sources or binary changed')
  else:result['sources_stable']=True
  with a.report.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
 print(json.dumps({k:result.get(k) for k in ('success','error','artifacts','normal_exit','channel_retired','input_cleared_after_quit')}))
 if not result['success']:raise SystemExit(1)
if __name__=='__main__':main()
