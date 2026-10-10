#!/usr/bin/env python3
"""Capture and compare the bounded original-active owned minimap journal."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--motion',action='store_true',help='Paced private-Xvfb rotation/pan schedule');args=parser.parse_args()
 declaration=json.loads(args.claims.read_text());sources=declaration['sources']
 if not all(sha(ROOT/n)==h for n,h in sources.items()):raise ValueError('Prospective sources changed')
 parent=ROOT/'working/tests/minimap-live';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 report={'success':False,'sources':sources,'claims':declaration['claims'],'scope':'Bounded first16 original-active World startup prefix, owned minimap journal V2 and complete source-only native producer replay. Live states are enumerated explicitly; no unobserved rotation/camera/visibility coverage or drawing bypass.','original_pixels_used_as_native_inputs':False,'original_work_bypassed':False,'live_replacement':False}
 def run(name,cmd):
  with (out/(name+'.log')).open('x') as log:subprocess.run([str(x) for x in cmd],cwd=ROOT,env={**os.environ,'LIBGL_ALWAYS_SOFTWARE':'1'},stdout=log,stderr=subprocess.STDOUT,check=True,timeout=900)
 def result(name):
  paths=[Path(line.strip()) for line in (out/(name+'.log')).read_text().splitlines() if line.strip().endswith('/report.json')]
  if not paths:raise ValueError('No complete '+name+' report')
  p=paths[-1];r=json.loads(p.read_text())
  if not r['success'] or not r.get('sources_stable',False):raise ValueError('Failed/stale '+name+' execution')
  return p,r
 try:
  run('hook',['python3',ROOT/'tools/test-canvas-producers.py','--claims',args.claims.resolve()]);hookpath,hook=result('hook')
  capture_extra=[]
  if args.motion:
   run('input-configure',['cmake','-S',ROOT/'tests/minimap-live-input','-B',out/'input-build'])
   run('input-build',['cmake','--build',out/'input-build','-j','4'])
   capture_extra=['--minimap-input-fixture',out/'input-build/mnm-minimap-live-input']
  run('capture',['xvfb-run','-a','-s','-screen 0 1280x1024x24','python3',ROOT/'tools/capture-scene-game.py','--canvas-producers','--minimap-owned','--samples','16','--magic-items','0','--skip-window-screenshot','--claims',args.claims.resolve(),*capture_extra]);capturepath,capture=result('capture')
  run('replay',['python3',ROOT/'tools/test-native-canvas-producers.py','--capture-report',capturepath,'--claims',args.claims.resolve()]);replaypath,replay=result('replay')
  observed=capture['canvas_producers']['minimap_owned']
  if not observed['terrain_calls'] or any(not observed['kinds'].get(str(k)) for k in range(3)):raise ValueError('Missing observed terrain or overlay kind')
  if args.motion:
   if observed['orientations']!=[0,1,2,3] or len(observed['centers'])<2:raise ValueError('Rotation/pan state coverage incomplete')
   timeline=observed['timeline']
   if not any(a['view']==b['view'] and a['center']!=b['center'] for a,b in zip(timeline,timeline[1:])):raise ValueError('No observed same-orientation camera pan')
   report['motion']=capture['minimap_input_fixture']
   report['motion']['same_orientation_pan_observed']=True
   report['scope']='Original-active first16 World calls with private-Xvfb comma/period rotation and arrow panning; 250ms fixture pacing. Only enumerated observed states, no original input/camera generation replacement.'
  if replay['equal']!=len(capture['canvas_producers']['checkpoints']) or replay['mismatches']:raise ValueError('Incomplete exact native producer comparison')
  native_root=Path(replaypath).parent
  # Actual compiled repository dependency closure, beyond hand-written source lists.
  compiled=set()
  for dep in (native_root/'build').rglob('*.o.d'):
   for raw in dep.read_text().replace('\\\n',' ').split():
    p=Path(raw).resolve()
    try:n=p.relative_to((native_root/'source').resolve()).as_posix()
    except ValueError:continue
    if (native_root/'source'/n).is_file():compiled.add(n)
  for n in compiled:
   frozen=sha(native_root/'source'/n)
   if n not in sources or sources[n]!=frozen:raise ValueError('Undeclared compiled dependency: '+n)
  if args.motion:
   for dep in (out/'input-build').rglob('*.o.d'):
    for raw in dep.read_text().replace('\\\n',' ').split():
     p=Path(raw).resolve()
     try:n=p.relative_to(ROOT).as_posix()
     except ValueError:continue
     if p.is_file() and (n not in sources or sources[n]!=sha(p)):raise ValueError('Undeclared input fixture dependency: '+n)
     if p.is_file():compiled.add(n)
  report.update(success=True,source_executable_sha256=capture['source_executable_sha256'],observed=observed,
    forwarded_entries=hook['forwarded_entries'],state_preservation=hook['register_flags_float_error_preserved'],
    signature_refusal_before_mutation=hook['signature_refusal_before_mutation'],fixture_configuration_refusals=hook['minimap_fixture_refused_before_hook_mutation'],compiled_dependencies=sorted(compiled),
    checkpoints=replay['completed'],equal=replay['equal'],pixels_compared=replay['pixels_compared'],mismatches=replay['mismatches'],
    native_synthetic_tests=True,original_manifest_verified_before_after=capture['original_manifest_verified_before_after'] and replay['original_manifest_verified_before_after'],
    inputs={str(p.relative_to(ROOT)):sha(p) for p in (hookpath,capturepath,replaypath)},execution_records={name:str(p.relative_to(ROOT)) for name,p in [('hook',hookpath),('capture',capturepath),('replay',replaypath)]})
 except Exception as error:report['error']=str(error)
 finally:
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable']
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
 if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
