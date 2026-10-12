#!/usr/bin/env python3
"""Source-bound mapped-menu progress and retirement controls."""
import hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parent=ROOT/"working/tests/render-progress-heartbeat";parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix="run-",dir=parent));print(out,flush=True)
 subprocess.run(["python3",str(ROOT/"tools/draft-coverage-claims.py"),"--behavior","HOST.menu-render-progress-heartbeat","--behavior","HOST.resident-report-publication","--scenario","HOST.menu-render-progress-heartbeat.fixture","--scenario","HOST.resident-report-publication.fixture","--output",str(out/"claims.json")],cwd=ROOT,check=True)
 d=json.loads((out/"claims.json").read_text());r=dict(success=False,claims=d["claims"],sources=d["sources"],scope=__doc__)
 env=dict(os.environ,TMPDIR=str(ROOT/"working/coverage-hook-tmp"),QT_QPA_PLATFORM="xcb",LIBGL_ALWAYS_SOFTWARE="1",GALLIUM_DRIVER="llvmpipe",LP_NUM_THREADS="4")
 try:
  build=ROOT/"working/build/qt-shell"
  with (out/"build.log").open("x") as log:subprocess.run(["cmake","--build",str(build),"--target","menu-resident-quit-test","--parallel","4"],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
  binary=build/"menu-resident-quit-test"
  with (out/"fixture.log").open("x") as log:subprocess.run(["xvfb-run","-a","-e",str(out/"xvfb.log"),str(binary)],env=env,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
  r["controls"]=json.loads((out/"fixture.log").read_text().splitlines()[-1]);r["binary_sha256"]=sha(binary);r["sources_unchanged"]=all(sha(ROOT/p)==h for p,h in r["sources"].items());r["success"]=r["sources_unchanged"]
 except (OSError,ValueError,IndexError,subprocess.SubprocessError) as error:r["error"]=str(error)
 (out/"report.json").write_text(json.dumps(r,indent=2)+"\n");print("Report:",out/"report.json",flush=True);return 0 if r["success"] else 1
if __name__=="__main__":raise SystemExit(main())
