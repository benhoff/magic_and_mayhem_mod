#!/usr/bin/env python3
"""Source-bound headless menu resource admission and movie-readiness controls."""
import argparse,hashlib,json,os,shlex,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args();output=a.output.resolve()
 if output.exists() or not output.is_relative_to(ROOT/'working'):raise ValueError('Require a new working result')
 parent=ROOT/'working/tests/native-startup-resources';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Source-bound startup asset admission and movie readiness controls.\n');print(run,flush=True)
 claims=run/'claims.json';subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','HOST.native-startup-resources','--scenario','HOST.native-startup-resources.headless','--output',str(claims)],check=True,stdout=subprocess.DEVNULL);declaration=json.loads(claims.read_text());sources=declaration['sources'].copy()
 for directory in ('apps/qt-shell','protocols/include'):
  for f in (ROOT/directory).rglob('*'):
   if f.is_file() and f.suffix in ('.cpp','.hpp','.h'):sources[f.relative_to(ROOT).as_posix()]=sha(f)
 build=ROOT/'working/build/native-startup-resources';env=dict(os.environ,QT_QPA_PLATFORM='offscreen')
 def execute(label,command):
  with (run/(label+'.log')).open('x') as log:subprocess.run([str(v) for v in command],cwd=ROOT,env=env,stdout=log,stderr=log,check=True,timeout=300)
 execute('configure',['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DBUILD_TESTING=ON','-DCMAKE_BUILD_TYPE=RelWithDebInfo']);execute('build',['cmake','--build',build,'--target','menu-startup-resources-test','--parallel','4']);binary=build/'menu-startup-resources-test'
 closure=set()
 for f in build.rglob('*.o.d'):
  for word in shlex.split(f.read_text().replace('\\\n',' ')):
   dep=Path(word);dep=(build/dep).resolve() if not dep.is_absolute() else dep.resolve()
   if dep.is_file() and dep.is_relative_to(ROOT) and not dep.is_relative_to(ROOT/'working'):closure.add(dep.relative_to(ROOT).as_posix())
 missing=sorted(closure-set(sources))
 if missing:raise ValueError('Undeclared compiler inputs: '+repr(missing))
 execute('controls',[binary]);result=json.loads((run/'controls.log').read_text().splitlines()[-1]);changed=[name for name,digest in sources.items() if sha(ROOT/name)!=digest]
 if changed:raise ValueError('Sources changed during controls: '+repr(changed))
 report=dict(schema=1,success=result['success'],sources=sources,claims=declaration['claims'],sources_stable=True,controls=result,compiled_sources={name:sources[name] for name in sorted(closure)},binary_sha256=sha(binary),artifacts=str(run.relative_to(ROOT)),original_execution=False,scope='Actual LiveMenuSession staged-resource callback precedes channels/process launch; refusal launches neither. During a movie retain heartbeat and defer original menu readiness without consuming sequence; after completion deliver latest state and dispatch once. Fake preparation/original process, no original game/asset/media/GL equivalence.')
 output.parent.mkdir(parents=True,exist_ok=True)
 with output.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(result),flush=True)
if __name__=='__main__':main()
