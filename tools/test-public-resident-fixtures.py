#!/usr/bin/env python3
"""Prove bounded resident import, ordered effects, capture ownership and leaf ABI.

Native reference comparison only; public gameplay, original pixel equivalence
and hardware throughput remain separate execution milestones.
"""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('channel','assets','prefix'):p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();parent=ROOT/'working/tests/public-resident-fixtures';parent.mkdir(parents=True,exist_ok=True)
 out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));report=dict(success=False,scope=__doc__)
 env=dict(os.environ,TMPDIR=str(ROOT/'working/compiler-tmp'));Path(env['TMPDIR']).mkdir(exist_ok=True)
 def run(command,graphics=False):
  current=env.copy()
  if graphics:current.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='8')
  result=subprocess.run(command,cwd=ROOT,env=current,capture_output=True,text=True,timeout=180)
  with (out/'execution.log').open('a') as log:log.write(result.stdout+result.stderr)
  if result.returncode:raise RuntimeError('Fixture command failed: '+repr(command))
  return result.stdout
 try:
  for path in (a.channel,a.assets,a.prefix):
   if not path.resolve().is_relative_to(ROOT/'working'):raise ValueError('Require owned retained test inputs')
  run([str(ROOT/'tools/original-manifest.sh'),'verify'])
  run(['python3','tools/draft-coverage-claims.py','--behavior','NR.public-resident-world','--scenario','NR.public-resident-world.owned-fixtures','--output',str(out/'claims.json')])
  claims=json.loads((out/'claims.json').read_text());report.update(claims=claims['claims'],sources=claims['sources'])
  for directory in ('apps','assets','audio','compat','reconstruction','renderer','runtime','protocols'):
   for source in (ROOT/directory).rglob('*'):
    if source.is_file() and (source.suffix in ('.cpp','.hpp','.h','.c','.S','.py') or source.name=='CMakeLists.txt'):report['sources'][source.relative_to(ROOT).as_posix()]=sha(source)
  report['inputs']={str(a.channel.resolve()):sha(a.channel)}
  # Pin the actual asset tree before replay, excluding generated or runtime data.
  for source in a.assets.rglob('*'):
   if source.is_file() and source.suffix.lower() in ('.spr','.pcx','.bmp','.jpg','.jpeg','.fp','.sft'):report['inputs'][str(source.resolve())]=sha(source)
  leaf=run(['python3','tools/test-resident-leaf-abi.py','--prefix',str(a.prefix.resolve())]);leafpath=Path(next(line[8:] for line in leaf.splitlines() if line.startswith('Report: ')))
  report['leaf_abi']=json.loads(leafpath.read_text());assert report['leaf_abi']['success'] and report['leaf_abi']['sources_unchanged']
  run(['cmake','-S','apps/qt-shell','-B','working/build/qt-shell','-DCMAKE_BUILD_TYPE=RelWithDebInfo'])
  targets=['resident-public-block-replay','render-resident-world-test','world-deferred-test','world-raster-batch-test','minimap-producer-test']
  run(['cmake','--build','working/build/qt-shell','--target',*targets,'-j','8'])
  build=ROOT/'working/build/qt-shell';sub=build/'world-rolling/canvas-producers'
  run(['xvfb-run','-a',str(build/'world-rolling/resident-public-block-replay'),str(a.channel.resolve()),str(a.assets.resolve()),str(out/'mixed.json')],True)
  report['mixed_world']=json.loads((out/'mixed.json').read_text())
  output=run(['xvfb-run','-a',str(build/'renderer/render-resident-world-test')],True);report['import_hud']=json.loads(next(line for line in output.splitlines() if line.startswith('{')))
  report['deferred']=json.loads(run([str(build/'world-rolling/world-deferred-test')]))
  report['strict_batch']=run([str(sub/'world-raster-batch-test')]).strip()
  report['minimap']=run([str(sub/'minimap-producer-test')]).strip()
  report['executables']={str(binary.relative_to(ROOT)):sha(binary) for binary in [build/'world-rolling/resident-public-block-replay',build/'renderer/render-resident-world-test',build/'world-rolling/world-deferred-test',sub/'world-raster-batch-test',sub/'minimap-producer-test']}
  report['success']=True
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError,AssertionError) as e:report['error']=str(e)
 finally:
  if 'sources' in report:
   report['sources_unchanged']=all(sha(ROOT/path)==digest for path,digest in report['sources'].items())
   report['inputs_unchanged']=all(sha(Path(path))==digest for path,digest in report['inputs'].items())
   if not report['sources_unchanged'] or not report['inputs_unchanged']:report.update(success=False,error='Fixture sources or inputs changed during execution')
  try:run([str(ROOT/'tools/original-manifest.sh'),'verify']);report['original_manifest_verified_after']=True
  except (OSError,RuntimeError,subprocess.SubprocessError) as e:report.update(success=False,cleanup_error=str(e))
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
