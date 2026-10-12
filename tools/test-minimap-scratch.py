#!/usr/bin/env python3
"""Compare recycled minimap preparation with independent fresh backgrounds."""
import hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 out=Path(tempfile.mkdtemp(prefix="run-",dir=ROOT/"working/tests/minimap-scratch"));print(out,flush=True)
 subprocess.run(["python3",str(ROOT/"tools/draft-coverage-claims.py"),"--behavior","NR.minimap-write-candidates","--scenario","NR.minimap-write-candidates.scratch-fixture","--output",str(out/"claims.json")],cwd=ROOT,check=True)
 d=json.loads((out/"claims.json").read_text());r=dict(success=False,claims=d["claims"],sources=d["sources"],scope=__doc__,commands=[])
 try:
  build=ROOT/"working/build/qt-shell"
  command=["cmake","--build",str(build),"--target","minimap-candidates-test","minimap-producer-test","--parallel","4"];r["commands"].append(command)
  with (out/"build.log").open("x") as log:subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
  directory=build/"world-rolling/canvas-producers"
  r["binaries"]={}
  for name in ["minimap-candidates-test","minimap-producer-test"]:
   binary=directory/name;r["binaries"][name]=sha(binary);command=[str(binary)];r["commands"].append(command)
   with (out/(name+".log")).open("x") as log:subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
  r["candidate_controls"]=json.loads((out/"minimap-candidates-test.log").read_text().splitlines()[-1]);r["producer_controls"]=(out/"minimap-producer-test.log").read_text().strip()
  r["sources_unchanged"]=all(sha(ROOT/p)==h for p,h in r["sources"].items());r["success"]=r["sources_unchanged"]
 except (OSError,ValueError,subprocess.SubprocessError) as error:r["error"]=str(error)
 (out/"report.json").write_text(json.dumps(r,indent=2)+"\n");print("Report:",out/"report.json",flush=True);return 0 if r["success"] else 1
if __name__=="__main__":
 (ROOT/"working/tests/minimap-scratch").mkdir(parents=True,exist_ok=True)
 raise SystemExit(main())
