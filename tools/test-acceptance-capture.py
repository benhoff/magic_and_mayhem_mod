#!/usr/bin/env python3
"""Bounded real compression-stage regression, ownership/order and failure controls."""
import argparse,hashlib,json,os,shlex,subprocess
from pathlib import Path
from coverage_evidence import write_report
from rendering_pilot import capture
from rendering_capture import Plane
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--baseline-header',type=Path);a=p.parse_args()
 out=a.output.resolve()
 if not out.is_relative_to(ROOT/'working') or out.exists() or (ROOT/a.report).exists():p.error('Fresh owned working output and immutable report required')
 out.mkdir(parents=True);(out/'.retain').write_text('Acceptance capture controls, including refused old source and wire bytes.\n')
 header=(a.baseline_header or ROOT/'apps/qt-shell/acceptance_capture.hpp').resolve()
 if a.baseline_header and not header.is_relative_to(ROOT/'working'):p.error('Use an owned working baseline preimage')
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in ['tools/test-acceptance-capture.py','tests/acceptance-capture-test.cpp','apps/qt-shell/acceptance_capture.hpp','apps/qt-shell/host_timing_trace.hpp','tools/rendering_pilot.py','tools/rendering_capture.py']}
 report=dict(schema=1,kind='acceptance_capture_worker_controls',success=False,baseline=bool(a.baseline_header),sources=sources,checks=[],scope=__doc__,original_game_executed=False)
 def run(command,**kwargs):
  r=subprocess.run([str(v) for v in command],cwd=ROOT,capture_output=True,timeout=120,**kwargs)
  report['checks'].append(dict(command=[str(v) for v in command],exit_code=r.returncode,stdout=r.stdout.decode(errors='replace'),stderr=r.stderr.decode(errors='replace')))
  return r
 try:
  report['executed_header']=dict(record=str(Path(a.report).with_suffix(''))+'-executed-header.bin',sha256=hashlib.sha256(header.read_bytes()).hexdigest())
  write_report(ROOT,report['executed_header']['record'],header.read_bytes())
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True))
  for mode,options in [('optimized',['-O2']),*([] if a.baseline_header else [('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])])]:
   binary=out/mode
   r=run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-fPIC','-pthread',*options,'-I',header.parent,ROOT/'tests/acceptance-capture-test.cpp',*flags,'-lz','-o',binary]);assert r.returncode==0,'Control build failed'
   command=[binary,out/(mode+'-wire')]+(['--baseline'] if a.baseline_header else ['--campaign-long'] if mode=='optimized' else [])
   r=run(command,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'))
   result=json.loads(r.stdout);report['checks'][-1]['result']=result
   if a.baseline_header:
    assert r.returncode==1 and result['success'] is False and result['cases'][0]['append_returned_while_compression_blocked'] is False,'Expected synchronous old-stage failure absent'
    report['old_stage_regression_observed']=True
   else:
    assert r.returncode==0 and result['success'] is True,'Worker control failed'
    accepted=rejected=0
    for wire in (out/(mode+'-wire')).rglob('*.bin'):
     try:
      decoded=capture(wire.read_bytes());accepted+=1
      campaign=[c for c in result['cases'] if c.get('campaign') and Path(c['directory'])==wire.parent]
      if campaign:
       paints=[body for metadata,body in decoded if metadata['kind']=='paint']
       assert all(isinstance(body,Plane) for body in paints),'Campaign reader expanded retained planes'
       assert len(paints)==campaign[0]['paints'] and hashlib.sha256(paints[-1]()).hexdigest()==campaign[0]['last_plane_sha256'],'Lazy campaign replay differs from source'
       report['checks'][-1]['campaign_reader']=dict(paints=len(paints),decoded_bytes=sum(map(len,paints)),lazy_planes=True,last_plane_matches_source=True)
     except ValueError as e:
      assert 'Capture incomplete or truncated' in str(e),str(e)
      rejected+=1
    assert (accepted,rejected)==(2,3),'Production reader did not distinguish complete and refused proofs'
    report['checks'][-1]['production_reader']=dict(complete_accepted=accepted,incomplete_rejected=rejected)
  report['success']=not bool(a.baseline_header)
 except Exception as e:report['error']=str(e)
 finally:
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());report['success'] &= report['sources_stable']
  report['artifacts']=[]
  for f in sorted(out.rglob('*')):
   if f.is_file():
    name=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(out)).encode()).hexdigest()[:20]+'.bin'
    raw=f.read_bytes();report['artifacts'].append(dict(path=str(f.relative_to(out)),record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw)))
  h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','old_stage_regression_observed','sources_stable']}|dict(record=a.report,sha256=h)),flush=True)
 return 0 if report['sources_stable'] and (report['success'] or (a.baseline_header and report.get('old_stage_regression_observed'))) else 1
if __name__=='__main__':raise SystemExit(main())
