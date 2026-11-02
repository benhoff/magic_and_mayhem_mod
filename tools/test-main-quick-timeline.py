#!/usr/bin/env python3
"""Compile actual session/witness controls and retain durable SQLite results."""
import argparse,hashlib,json,os,shlex,subprocess,sys
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();assert a.report.startswith('research/runtime/main-quick-timeline-results-20261101/') and not (ROOT/a.report).exists()
 out=ROOT/'working/main-quick-timeline'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Diagnostic witness and SQLite crash controls.\n')
 tmp=Path('/tmp/mnm-main-quick-timeline-20261101/controls');tmp.mkdir(exist_ok=False);env=dict(os.environ,TMPDIR=str(tmp),MNM_TIMELINE_TEST_INPUTS=str(out/'optimized'))
 names=['tests/menu-transition-witness-test.cpp','tests/test-menu-transition-timeline.py','tools/menu_transition_timeline.py','tools/test-main-quick-timeline.py','apps/qt-shell/live_menu_session.cpp','apps/qt-shell/live_menu_session.hpp','apps/qt-shell/menu_transition_witness.hpp','apps/qt-shell/menu_bridge.cpp','apps/qt-shell/menu_bridge.hpp','apps/qt-shell/engine_preferences_store.cpp','apps/qt-shell/engine_preferences_store.hpp','apps/qt-shell/game_process.hpp','protocols/include/mnm/menu_timeline_v1.h']
 source={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names};result=dict(schema=1,success=False,kind='main_quick_timeline_controls',original_execution=False,sources=source)
 def run(label,cmd):
  with (out/(label+'.log')).open('xb') as log:subprocess.run([str(x) for x in cmd],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=120,check=True)
 try:
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True));src=[ROOT/n for n in ['tests/menu-transition-witness-test.cpp','apps/qt-shell/live_menu_session.cpp','apps/qt-shell/menu_bridge.cpp','apps/qt-shell/engine_preferences_store.cpp']]
  for mode,extra in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   exe=tmp/mode;run(mode+'-build',['g++','-std=c++17','-fPIC','-Wall','-Wextra','-Werror','-I',ROOT/'apps/qt-shell',*extra,*src,'-o',exe,*flags]);run(mode,[exe,out/mode])
  run('python',[sys.executable,'-B',ROOT/'tests/test-menu-transition-timeline.py']);result['success']=True
 except Exception as e:result['error']=repr(e)
 result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in source.items());result['success'] &= result['sources_stable'];result['artifacts']=[]
 for f in sorted(out.rglob('*')):
  if not f.is_file():continue
  raw=f.read_bytes();assert len(raw)<=8*1024**2;n=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(out)).encode()).hexdigest()[:16]+'.bin';result['artifacts'].append(dict(record=n,sha256=write_report(ROOT,n,raw),bytes=len(raw)))
 write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps({k:result.get(k) for k in ['success','error']}),flush=True);return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
