#!/usr/bin/env python3
"""Exercise bounded capture/history storage and actual producer checkpoint policy."""
import argparse,hashlib,importlib.util,json,os,shlex,subprocess,sys
from pathlib import Path
from coverage_evidence import read_json,write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',required=True,type=Path);p.add_argument('--report',required=True);p.add_argument('--prefix-template',required=True,type=Path);a=p.parse_args()
 out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require fresh owned output')
 out.mkdir(parents=True);(out/'.retain').write_text('SAVECHUNK component outputs,claims,compiler and producer proof.\n')
 spec=importlib.util.spec_from_file_location('save_claims',ROOT/'tools/draft-coverage-claims.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());declaration=module.declaration(ROOT,reg,['NP.resident-source-chunk-capture','HOST.native-render-save-load'],['SAVECHUNK.capture-controls','SAVECHUNK.menu-opening'])
 for directory in ('apps/qt-shell','assets','audio','renderer','compat','reconstruction','protocols','runtime','tests','tools'):
  for f in (ROOT/directory).rglob('*'):
   if f.is_file() and (f.suffix in ('.cpp','.hpp','.c','.h','.S','.py','.inc') or f.name=='CMakeLists.txt'):declaration['sources'][f.relative_to(ROOT).as_posix()]=hashlib.sha256(f.read_bytes()).hexdigest()
 claims=out/'claims.json';claims.write_text(json.dumps(declaration,indent=2)+'\n')
 report=dict(success=False,claims=declaration['claims'],sources=declaration['sources'],commands=[],scope=__doc__+';no original game entrypoint or full journey completion.')
 def run(label,cmd):
  with (out/(label+'.log')).open('x') as f:x=subprocess.run(list(map(str,cmd)),cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=600)
  report['commands'].append(dict(label=label,argv=list(map(str,cmd)),exit_code=x.returncode,sha256=hashlib.sha256((out/(label+'.log')).read_bytes()).hexdigest()))
  if x.returncode:raise ValueError(label+' failed: '+(out/(label+'.log')).read_text(errors='replace')[-2000:])
 try:
  build=ROOT/'working/build/save-controls'
  run('build',['cmake','--build',build,'--target','low-resident-capture','source-prefix-test','resident-storage-replay','resident-canvas-transition-test','--parallel','4'])
  menu=ROOT/'working/build/save-menu-controls'
  run('menu-configure',['cmake','-S',ROOT/'apps/qt-shell','-B',menu,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'])
  run('menu-build',['cmake','--build',menu,'--target','menu-storage-bridge-test','--parallel','4'])
  run('menu-opening',['env','QT_QPA_PLATFORM=offscreen',menu/'menu-storage-bridge-test'])
  closure=set()
  receipts=list(build.rglob('*.o.d'))+list(menu.rglob('*.o.d'))
  if not receipts:raise ValueError('Missing component compiler dependency receipts')
  for receipt in receipts:
   for token in shlex.split(receipt.read_text().replace('\\\n',' ')):
    source=Path(token).resolve()
    if source.is_absolute() and source.is_file() and source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):closure.add(source.relative_to(ROOT).as_posix())
  missing=closure-set(report['sources'])
  if missing:raise ValueError('Undeclared component compiler dependencies: '+repr(sorted(missing)))
  report['compiled_sources']={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(closure)}
  run('capture',[build/'low-resident-capture',out/'capture'])
  run('archive-reader',[sys.executable,'-B',ROOT/'tests/resident-profile-capture-test.py',out/'capture'])
  for label in ('source-prefix-test','resident-canvas-transition-test'):run(label,['xvfb-run','-a',build/'world/world-rolling'/label])
  run('producer',[sys.executable,'-B',ROOT/'tools/test-canvas-producers.py','--claims',claims,'--prefix-template',a.prefix_template,'--report',out/'producer.json'])
  producer=read_json(out/'producer.json',root=ROOT)
  if not producer['metadata_checkpoint_ignores_file_oracle_limit'] or not producer['metadata_checkpoint_unreadable_pixels_untouched']:raise ValueError('Missing actual checkpoint guard regression')
  report.update(producer=producer,archive_controls=json.loads((out/'archive-reader.log').read_text().splitlines()[-1]),source_prefix=json.loads((out/'source-prefix-test.log').read_text().splitlines()[-1]),success=True)
 except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
 finally:
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in report['sources'].items());report['success']&=report['sources_stable'];write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps(dict(success=report['success'],error=report.get('error'))),flush=True);return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
