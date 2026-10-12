#!/usr/bin/env python3
"""Private TITAN owned-snapshot gameplay and independent native rendering proof."""
import argparse,hashlib,json,signal,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--target-fps',type=float,default=59);a=p.parse_args();d=json.loads(a.claims.read_text());assert all(sha(ROOT/s)==h for s,h in d['sources'].items())
 parent=ROOT/'working/tests/independent-native-world';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Independent native evidence:',out,flush=True);r=dict(success=False,claims=d['claims'],sources=d['sources'],scope=__doc__,target_native_fps=a.target_fps);child=None
 try:
  cmd=['python3',str(ROOT/'tools/test-native-campaign.py'),'--resident-world','--independent-world','--hardware','--difficulty','1','--require-casting-combat','--spell-cases','--portrait-stress-seconds','10','--stress-seconds','60','--min-fps','19','--normal-quit','--timeout','600']
  with (out/'campaign.log').open('x') as log:child=subprocess.Popen(cmd,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT);exit_code=child.wait(timeout=720)
  paths=[Path(l.partition(': ')[2]) for l in (out/'campaign.log').read_text().splitlines() if l.startswith('Report: ')];assert len(paths)==1;game=json.loads(paths[0].read_text());r.update(campaign_report=str(paths[0]),campaign_report_sha256=sha(paths[0]),campaign_success=game['success'],campaign_exit_code=exit_code,campaign_error=game.get('error'),resident_world=game.get('resident_world'),hardware_driver=game.get('hardware_driver'),experiment=game.get('experiment'),sources_unchanged=all(sha(ROOT/s)==h for s,h in d['sources'].items()),private_wine_stopped=game.get('private_wine_stopped') is True,original_manifest_verified_after=game.get('original_manifest_verified_after') is True,gameplay_checks={k:game.get(k) for k in ['casting_verified','combat_verified','movement_verified','normal_quit_verified']})
  if exit_code or not game['success']:raise ValueError('Full independent native gameplay failed')
  w=game['resident_world']
  if w.get('delivery_mode')!='owned-v5' or int(w['source_accepted'])!=int(w['source_returned']) or int(w['source_returned'])!=int(w['completed']) or int(w['imports'])!=int(w['completed']) or w['source_outstanding'] or w['source_high_water']>2:raise ValueError('Owned snapshot/import accounting failed')
  if not all(r[k] for k in ['sources_unchanged','private_wine_stopped','original_manifest_verified_after']):raise ValueError('Independent source/integrity/cleanup failed')
  if not 19.5<=w['engine_fps']<=21:raise ValueError('Original source cadence did not remain near20Hz')
  if w['native_extra_world_frames']!=w['native_extra_presented_frames']:raise ValueError('Extra native World and presented frame counts disagree')
  if not int(w['native_extra_world_frames']) or w['native_render_fps']<a.target_fps:raise ValueError('Actual native render cadence missed target; no60FPS claim')
  r['success']=True
 except (OSError,AssertionError,KeyError,ValueError,subprocess.SubprocessError) as e:r['error']=str(e)
 finally:
  if child and child.poll() is None:
   child.send_signal(signal.SIGINT)
   try:child.wait(timeout=60)
   except subprocess.TimeoutExpired:child.kill();child.wait();r['cleanup_unconfirmed']=True
  (out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print('Report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
