#!/usr/bin/env python3
"""Owned backend lifetimes, flipping, driver Restore and recovered scheduling."""
import argparse,importlib.util,json,os,shlex,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,n):
 s=importlib.util.spec_from_file_location(name,ROOT/n);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
own=module('ownership','tools/check-surface-ownership.py');pal=module('palette_restore','tools/check-surface-palette-restore.py');retry=module('retry','tools/check-original-surface-retry.py')
def compare(actual,observed,palette_states):
 own.require(actual['checks']>100 and actual['refusals']>10,'Missing native guards');a=actual['ownership'];frames=observed['frames'] if observed else {};tables=observed['tables'] if observed else {};count=0
 def words(values,expected):
  nonlocal count
  own.require(values==expected,'Native independent output mismatch');count+=len(values)
 if observed is not None:
  for v,k in zip(a['pixels'],[(1,0),(3,0),(3,1),(6,1)],strict=True):words(v,frames[k])
  for v,k in zip(a['palettes'],[(3,0),(3,1),(4,1),(6,1)],strict=True):words(v,tables[k])
  own.require(a['detached_status']==observed['detached'],'Native detach result')
  for v,phase in zip(a['flips'],range(10,14),strict=True):
   own.require(v['hresult']==observed['flips'][phase] and [v['front_key'],v['back_key']]==[0x61,0x72],'Flip result/keys');words(v['front'],frames[(phase+10,3)]);words(v['back'],frames[(phase+10,4)])
  for v,phase in zip(a['held'],range(30,34),strict=True):
   own.require(v['hresult']==observed['flips'][phase] and [v[k] for k in ['same_release','other_release','undo_flip','returned']]==observed['cleanups'][phase],'Held flip partial effect/lease return');words(v['front'],frames[(phase,3)]);words(v['back'],frames[(phase,4)])
  for field,k in [('reacquired',(40,4)),('final_front',(42,3)),('final_back',(42,4))]:words(a[field],frames[k])
 state_map={(s['phase'],s['surface']):s for s in palette_states};seen=set();native=actual['driver_recovery'];palette_count=0;pixels=0;statuses=0
 for s in native:
  key=(s['phase'],s['surface']);own.require(key not in seen,'Duplicate driver-recovery state');seen.add(key);lost=0x887601c2 if s['surface'] and (s['phase'] in (1,3) or (s['surface']==3 and s['phase']==2)) else 0
  own.require(s['lost']==s['get_palette']==lost and s['key']==0x35,'Native recovered loss/palette/key state');statuses+=2
  if not lost:
   expected=state_map[key];words(s['table'],expected['colors']);palette_count+=256
   if 'indices' in s:words(s['indices'],expected['indices']);pixels+=48
   else:own.require(s['phase'] in (2,4,5),'Missing defined bytes')
  else:own.require('table' not in s and 'indices' not in s,'Invented lost outputs')
 expected_keys={(phase,n) for phase in [0,1,2,3,4,5,6,8] for n in range(4)}|{(9,1)};own.require(seen==expected_keys,'Incomplete recovery matrix')
 own.require(actual['recovery']['cases']==16 and actual['recovery']['cursor_loads']==1,'Missing wrapper/cursor recovery composition')
 return dict(ownership_word_checks=count-palette_count-pixels,driver_palette_entries=palette_count,driver_index_checks=pixels,driver_status_checks=statuses,flip_results=11 if observed is not None else 0,owned_recovery_cases=16,owned_recovery_input_pixel_checks=actual['recovery']['pixels'],native_checks=actual['checks'],native_refusals=actual['refusals'])
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/renderer');p.add_argument('--executable',type=Path);p.add_argument('--report',type=Path);p.add_argument('--recovery-only',action='store_true');a=p.parse_args();observed=None
 if not a.recovery_only:capture,rows=own.load();observed=own.validate(capture,rows)
 pr,prows=pal.load();states=pal.validate(pr,prows)
 parent=ROOT/'working/tests/native-surface-ownership';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));a.report=a.report or run/'report.json'
 names=['tests/surface-retry-model.cpp','tools/check-original-surface-retry.py','tests/fixtures/surfaces/original-retry.json.gz','tests/fixtures/surfaces/original-retry-corpus.json','renderer/surface_backend.hpp','renderer/surface_backend.cpp','renderer/surface_access.hpp','renderer/bitmap_dc.hpp','renderer/blit.cpp','renderer/blit.hpp','renderer/known_pixels.hpp','renderer/CMakeLists.txt','reconstruction/rendering/surface_retry.hpp','reconstruction/rendering/owned_surface_retry.hpp','tests/render-surface-ownership-test.cpp','tools/check-native-surface-ownership.py','tools/check-surface-ownership.py','tests/test-surface-ownership.py','research/runtime/surface-ownership-final-capture-20261006.json','tests/fixtures/surfaces/surface-ownership-final.json.gz','tools/check-surface-palette-restore.py','research/runtime/surface-palette-restore-capture-20261006.json','tests/fixtures/surfaces/surface-palette-restore.json.gz']
 if a.recovery_only:names=[n for n in names if n not in ['research/runtime/surface-ownership-final-capture-20261006.json','tests/fixtures/surfaces/surface-ownership-final.json.gz']]
 fingerprints={n:own.sha(ROOT/n) for n in names};library=a.build.resolve()/'libmnm-renderer.a';binary=a.executable.resolve() if a.executable else run/'native'
 if not a.executable:
  qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True));subprocess.run(['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),str(ROOT/'tests/render-surface-ownership-test.cpp'),str(library),*qt,'-o',str(binary)],check=True)
 output=run/'outputs.json';proc=subprocess.run([str(binary),str(output)],capture_output=True,text=True,env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),timeout=120);(run/'native.log').write_text(proc.stdout+proc.stderr);own.require(proc.returncode==0,proc.stdout+proc.stderr);actual=json.loads(output.read_text());counts=compare(actual,observed,states);own.require(all(own.sha(ROOT/n)==h for n,h in fingerprints.items()),'Sources changed during replay')
 rows,m=retry.load();model=run/'retry-model';subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-pedantic',str(ROOT/'tests/surface-retry-model.cpp'),'-o',str(model)],check=True)
 cases=run/'retry-inputs.txt';cases.write_text(''.join(' '.join(str(v) for v in [*(c[k] for k in ['id','entry','fast','no_wait','key_enabled','reload_bits','source_restore','destination_restore','key_result','color']),len(c['draw_results']),*c['draw_results']])+'\n' for c in (row['input'] for row in rows)))
 recovered=subprocess.run([str(model),str(cases)],capture_output=True,text=True,check=True,timeout=30);(run/'retry-outputs.json').write_text(recovered.stdout);traces=json.loads(recovered.stdout)
 for row,out in zip(rows,traces,strict=True):own.require(out['returned'] and not out['budget_exhausted'] and {k:out[k] for k in ['id','draws_consumed','events']}==row['original'],'Recovered original trace mismatch')
 bounded=json.loads(subprocess.check_output([str(model),str(cases),'2'],text=True,timeout=30));stops=0
 for row,out in zip(rows,bounded,strict=True):
  own.require(out['draws_consumed']<=2 and out['events']==row['original']['events'][:len(out['events'])],'Bounded original prefix')
  if out['budget_exhausted']:own.require(not out['returned'],'Bounded stop is not original success');stops+=1
 own.require(all(own.sha(ROOT/n)==h for n,h in fingerprints.items()),'Sources changed during final replay')
 report=dict(schema=1,success=True,ownership_driver_comparison=not a.recovery_only,original_sha256=retry.BUILD_HASH,retry_traces=len(traces),bounded_retry_stops=stops,sources=fingerprints,**counts,binary_sha256=own.sha(binary),library_sha256=own.sha(library),outputs_sha256=own.sha(output),artifacts=str(run.relative_to(ROOT)),scope=('Recovery-only intermediate native checks; missing standalone ownership/flip comparison remains explicitly pending. ' if a.recovery_only else '')+('Owned aliases/shared palettes and two-buffer storage/lease flips compare independent standalone Wine outputs. ' if observed is not None else 'Owned aliases/shared palettes and flips have native guard checks only in this intermediate report. ')+'Shared update/detach/rebind/DC snapshots retain distinct state. Real recorded driver loss/palette/key/Restore results and defined index/table states reused with explicit reloads; unspecified restored pixels remain unknown. Fresh 4480recovered unchanged-original fault traces /1536 bounded stops remain separate from 16 native wrapper/reloader compositions and global cursor binding use supplied bytes; physical wrapper-lost-draw/COM pointer normalization/flagged palettes/long chains/live/Windows remain pending. Canonical native reference-count returns and lost wrapper admission are native policies, not COM count or Wine COLORFILL equivalence.')
 a.report.parent.mkdir(parents=True,exist_ok=True)
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(success=True,**counts)))
if __name__=='__main__':main()
