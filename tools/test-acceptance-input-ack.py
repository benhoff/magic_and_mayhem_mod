#!/usr/bin/env python3
"""Verify bounded receipt-driven acceptance input without original artifacts."""
import argparse,hashlib,json,os,shlex,subprocess,sys
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
 if not a.report.startswith('research/runtime/rendering-watchdog-results-20261101/') or (ROOT/a.report).exists():p.error('New immutable report required')
 out=ROOT/'working/watchdog-sampler'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Input receipt controls and negative producer cases.\n')
 names=['apps/qt-shell/acceptance_input_ack.hpp','tests/acceptance-input-ack-test.cpp','tools/test-acceptance-input-ack.py']
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names};r=dict(schema=1,kind='acceptance_input_receipt_controls',success=False,sources=sources,original_artifacts_consumed=False,checks=[])
 try:
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True))
  for mode,extra in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   binary=out/mode;commands=[['g++','-std=c++17','-Wall','-Wextra','-Werror','-fPIC',*extra,'-I',ROOT/'apps/qt-shell',ROOT/'tests/acceptance-input-ack-test.cpp',*flags,'-o',binary],[binary,out/(mode+'-wire')]]
   for command in commands:
    c=subprocess.run([str(v) for v in command],cwd=ROOT,capture_output=True,timeout=120,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'))
    row=dict(command=[str(v) for v in command],exit_code=c.returncode,stdout=c.stdout.decode(errors='replace'),stderr=c.stderr.decode(errors='replace'));r['checks'].append(row)
    if c.returncode:raise ValueError('Input control build/execution failed')
   r['checks'][-1]['result']=json.loads(c.stdout);assert r['checks'][-1]['result']['success']
  r['success']=True
 except Exception as error:r['error']=repr(error)
 r['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==sha for n,sha in sources.items());r['success'] &= r['sources_stable'];r['artifacts']=[]
 for f in sorted(out.rglob('*')):
  if not f.is_file():continue
  raw=f.read_bytes();name=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(out)).encode()).hexdigest()[:20]+'.bin';r['artifacts'].append(dict(path=str(f.relative_to(out)),record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw)))
 sha=write_report(ROOT,a.report,(json.dumps(r,indent=2)+'\n').encode());print(json.dumps(dict(success=r['success'],record=a.report,sha256=sha,error=r.get('error'))),flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
