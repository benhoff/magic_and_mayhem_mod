#!/usr/bin/env python3
"""Real optimized/sanitized epoch naming, writer and producer-binding controls."""
import argparse,hashlib,json,os,shlex,subprocess
from pathlib import Path
from coverage_evidence import write_report
from rendering_capture import capture
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-transition-recovery-results-20261107/'
SOURCES=['tools/test-rendering-epoch-capture.py','tests/resident-epoch-capture-test.cpp','apps/qt-shell/acceptance_capture.hpp','apps/qt-shell/host_timing_trace.hpp','apps/qt-shell/resident_capture_path.hpp','apps/qt-shell/resident_profile_capture.hpp','apps/qt-shell/resident_profile_capture.cpp','apps/qt-shell/profile_capture_codec.hpp','tools/rendering_capture.py','tools/rendering_pilot.py']
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args();out=a.output.resolve()
 if not out.is_relative_to(ROOT/'working') or out.exists() or not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('Fresh owned output and SQLite report required')
 out.mkdir(parents=True);(out/'.retain').write_text('Epoch recorder controls and native artifacts; preserve refusals.\n');sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in SOURCES}
 q=dict(schema=1,kind='native_epoch_capture_controls',success=False,sources=sources,checks=[],original_game_runs=0,scope=__doc__)
 try:
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True))
  for mode,options in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   binary=out/mode;cmd=['g++','-std=c++17','-Wall','-Wextra','-Werror','-fPIC','-pthread',*options,str(ROOT/'tests/resident-epoch-capture-test.cpp'),str(ROOT/'apps/qt-shell/resident_profile_capture.cpp'),*flags,'-lz','-o',str(binary)]
   for stage,command in [('build',cmd),('execute',[str(binary),str(out/(mode+'-data'))])]:
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env['ASAN_OPTIONS']='detect_leaks=1'
    r=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,text=True,timeout=120);row=dict(mode=mode,stage=stage,command=command,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr);q['checks'].append(row)
    assert r.returncode==0,(mode,stage,r.stderr)
    if stage=='execute':
     row['result']=json.loads(r.stdout);assert row['result']['success'] and len(row['result']['checks'])==9
     accepted=refused=0
     for f in (out/(mode+'-data')/'captures').glob('*.bin'):
      try:capture(f.read_bytes());accepted+=1
      except ValueError as e:assert 'Capture incomplete or truncated' in str(e);refused+=1
     assert (accepted,refused)==(4,2),(accepted,refused)
     bindings=[json.loads(f.read_bytes()) for f in (out/(mode+'-data')/'captures').glob('*.producer.json')]
     assert sorted(v['producer'] for v in bindings)==[101,202,303]
     assert all(v['schema']==1 and v['kind']=='command-producer-binding' and v['transport_version']==3 and Path(v['channel']).is_absolute() and Path(v['capture']).is_file() for v in bindings)
     row['reader']=dict(complete=accepted,refused=refused,producer_ids=[101,202,303])
  q['success']=True;q['native_cases']=18
 except Exception as e:q['error']=str(e)
 finally:
  q['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());q['success']&=q['sources_stable'];q['artifacts']=[]
  for f in sorted(out.rglob('*')):
   if f.is_file():
    raw=f.read_bytes();name=a.report.removesuffix('.json')+'-raw-'+hashlib.sha256(str(f.relative_to(out)).encode()).hexdigest()[:20]+'.bin';q['artifacts'].append(dict(path=str(f.relative_to(out)),record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw)))
  sha=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=sha)))
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
