#!/usr/bin/env python3
"""Source-bound isolated diagnostic lifetime/record/error checks; no game media."""
import hashlib,json,subprocess,tempfile,importlib.util
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 spec=importlib.util.spec_from_file_location('status_claims',ROOT/'tools/draft-coverage-claims.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());d=m.declaration(ROOT,reg,['NR.native-draw-status-session-file'],['native-draw-status-session-file-20261011'])
 parent=ROOT/'working/tests/native-draw-status';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'claims.json').write_text(json.dumps(d,indent=2)+'\n')
 r=dict(success=False,scope='Isolated stubbed Win32 status sink and failure contract only; no real Wine/API latency or original engine equivalence.',sources=d['sources'],claims=d['claims'],executions=[])
 try:
  for mode,flags in [('assertions',['-O2']),('asan-ubsan',['-O1','-g','-fsanitize=address,undefined'])]:
   exe=run/mode;dep=run/(mode+'.d');cmd=['clang++','-std=c++17','-UNDEBUG','-Wall','-Wextra','-Werror',*flags,'-MD','-MF',str(dep),str(ROOT/'tests/native-draw-status-test.cpp'),'-o',str(exe)];subprocess.run(cmd,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
   closure={p.resolve().relative_to(ROOT).as_posix():sha(p) for token in dep.read_text().replace('\\\n',' ').split() if (p:=Path(token)).is_file() and p.resolve().is_relative_to(ROOT) and not p.resolve().is_relative_to(ROOT/'working')};assert all(d['sources'].get(p)==v for p,v in closure.items()),'Undeclared actual compiler input'
   out=subprocess.run([str(exe)],check=True,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);r['executions'].append(dict(mode=mode,command=cmd,compiler_inputs=closure,executable_sha256=sha(exe),output=out.stdout))
  r['sources_stable']=all(sha(ROOT/p)==v for p,v in d['sources'].items());assert r['sources_stable'];r['success']=True
 finally:(run/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print(run/'report.json')
if __name__=='__main__':main()
