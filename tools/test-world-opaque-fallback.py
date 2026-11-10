#!/usr/bin/env python3
"""Retain exact original/native primary opaque fallback controls, including failures."""
import argparse,hashlib,json,subprocess,sys
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import write_report,Reader
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BID='NP.world-opaque-fallback-caller'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--expect-rejected',action='store_true');a=p.parse_args()
 out=a.output.resolve();target=ROOT/a.report
 if out.exists() or target.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh owned output and immutable report required')
 out.mkdir(parents=True);(out/'.retain').write_text('Retain exact tested executables, sources and failures; compiler .o files are temporary with commands/tool versions in the report.\n')
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(v for v in reg['behaviors'] if v['id']==BID)
 sources={n:sha(ROOT/n) for n in sorted(required_sources(b))}
 prefix=Path(a.report).with_suffix('').as_posix()+'-artifacts/'
 report=dict(schema=1,kind='primary_opaque_fallback_regression',success=False,mode='expect-rejected' if a.expect_rejected else 'expect-fixed',original_sha256=BUILD,original_game_executed=False,original_game_starts=0,source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT).decode().strip(),sources=sources,claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={})],commands=[],artifacts=[],scope=b['scope'])
 def retain(path,name=None):
  name=name or path.relative_to(out).as_posix();record=prefix+name+'.bin';data=path.read_bytes();h=write_report(ROOT,record,data)
  report['artifacts'].append(dict(record=record,sha256=h,size=len(data)));return h
 def run(label,cmd,timeout=600):
  cmd=list(map(str,cmd));log=out/(label+'.log')
  try:
   x=subprocess.run(cmd,cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=timeout);data=x.stdout;code=x.returncode
  except subprocess.TimeoutExpired as e:data=(e.stdout or b'')+b'\nTIMEOUT\n';code=124
  log.write_bytes(data);retain(log);report['commands'].append(dict(command=cmd,returncode=code,log=prefix+label+'.log.bin'))
  if code:raise ValueError(label+': '+data.decode(errors='replace')[-3000:])
  return data.decode()
 try:
  for name in sources:retain(ROOT/name,'sources/'+name)
  run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
  image=ROOT/'original/Arcane_Nocd/Chaos.exe';assert sha(image)==BUILD,'Original image hash'
  run('cc-version',['cc','--version']);run('cxx-version',['g++','--version'])
  objects=[]
  for name in ['pixel_kernel','raster_pixels']:
   obj=out/(name+'.o');run('compile-'+name,['cc','-m32','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-c',ROOT/('reconstruction/rendering/'+name+'.c'),'-o',obj]);objects.append(obj)
  binary=out/'opaque-wrapper';run('compile-wrapper',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-pie','-no-pie',ROOT/'tests/world-opaque-fallback-test.cpp',*objects,'-o',binary]);retain(binary)
  report['wrapper']=json.loads(run('wrapper',[binary,image,report['mode']]))
  if not a.expect_rejected:
   run('caller-controls',[sys.executable,ROOT/'tools/test-world-raster-callers.py','--output',out/'caller-controls'])
   report['caller_controls']=json.loads((out/'caller-controls/report.json').read_text())
   assert report['caller_controls']['callers']==54 and report['caller_controls']['admitted_pairs']==64
   assert report['caller_controls']['production_and_selftest_hooks_compile']
   # The subtool uses bounded physical interchange files. Preserve every output
   # and tested executable in SQLite; no raw objects are lost on a later failure.
   report['guard_generator_and_hook_build_verified']=True
  report.update(success=True,defect_reproduced=a.expect_rejected,native_equivalence=not a.expect_rejected)
 except Exception as e:report['error']=str(e)
 finally:
  for item in sorted(out.rglob('*')):
   if item.is_file() and item.suffix!='.o' and item.name!='.retain' and not any(v['record']==prefix+item.relative_to(out).as_posix()+'.bin' for v in report['artifacts']):retain(item)
  for directory in ['working/build/scene-observer','working/build/scene-observer-selftest']:
   for item in sorted((ROOT/directory).iterdir() if (ROOT/directory).is_dir() else []):
    if not item.is_file() or item.suffix not in {'.dll','.exe','.json'}:continue
    retain(item,'tested-hooks/'+Path(directory).name+'/'+item.name)
  try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except Exception as e:report.update(success=False,original_manifest_error=str(e))
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps({k:report.get(k) for k in ['success','mode','error','wrapper','sources_stable']}),flush=True)
 return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
