#!/usr/bin/env python3
"""Compare all three sprite wrappers using original Win32 primary fragments."""
import argparse,hashlib,json,os,struct,subprocess
from pathlib import Path
from kind8_sprite_fixture import original_fixture
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args();out=a.output.resolve()
 if out.exists()or not out.is_relative_to(ROOT/'working'):p.error('Fresh owned working output required')
 out.mkdir(parents=True);report=dict(schema=1,success=False,scope='Three original method bodies and primary virtual call/store;216000complete traces versus owned native model. Actual protected batch class/caller predicates see original call stacks. RNG/pixel callouts synthetic; no complete game, natural reachability, original pixel or lifetime claim.',original_game_runs=0)
 names=['tests/kind8-sprite-primary-win32.c','tests/kind8-sprite-primary-call.S','tests/kind8-sprite-method-test.cpp','tools/test-kind8-sprite-primary-win32.py','tools/kind8_sprite_fixture.py','runtime/shadow/win32_min.h','runtime/scene/world_raster_callers.h','runtime/scene/world_producer_bypass.c','runtime/scene/world_trace.c','runtime/scene/kind8_sprite_admission.h','runtime/scene/fading_particle_admission.h','reconstruction/rendering/kind8_class.h','reconstruction/rendering/kind8_sprite_class.h','reconstruction/rendering/kind8_sprite_method.hpp','reconstruction/rendering/world_dispatch.hpp']
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in names};report['sources']=sources
 env={k:v for k,v in os.environ.items()if not k.startswith('MNM_')};env.update(WINEPREFIX=str(ROOT/'working/tests/scene-selftest-wine'),WINEDEBUG='-all',MNM_SPRITE_TRACE='Z:'+str(out/'original.trace').replace('/','\\'))
 def run(label,cmd):
  with(out/(label+'.stdout')).open('xb')as f,(out/(label+'.stderr')).open('xb')as e:subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,stdout=f,stderr=e,check=True,timeout=180)
 try:
  report['original']=original_fixture(ROOT,out)
  src=(ROOT/'runtime/scene/world_producer_bypass.c').read_text();(out/'sprite-batch-caller.inc').write_text(src[src.index('static int raster_caller('):src.index('static u32 bypass_hash')]+src[src.index('static int raster_batch_caller('):])
  src=(ROOT/'runtime/scene/world_trace.c').read_text();(out/'sprite-observer-caller.inc').write_text(src[src.index('static int kind8_supported('):src.index('void world_begin(')])
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-omit-frame-pointer','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I'+str(out)]
  for n in ['tests/kind8-sprite-primary-win32.c','tests/kind8-sprite-primary-call.S']:run(Path(n).name+'-build',[*flags,'-c',ROOT/n,'-o',out/(Path(n).name+'.obj')])
  run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'reference.exe'),out/'kind8-sprite-primary-win32.c.obj',out/'kind8-sprite-primary-call.S.obj',ROOT/'working/build/scene-observer-selftest/kernel32.lib'])
  run('native-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-DMNM_SPRITE_PRIMARY_FRAGMENT',ROOT/'tests/kind8-sprite-method-test.cpp','-o',out/'native'])
  run('native',[out/'native']);subprocess.run(['wineserver','-w'],env=env,check=True,timeout=5)
  run('original',['xvfb-run','-a','wine',out/'reference.exe'])
  native=(out/'native.stdout').read_bytes();original=(out/'original.trace').read_bytes();assert len(native)==216000*140 and native==original,(len(native),len(original),next((i for i,(a,b)in enumerate(zip(native,original))if a!=b),None))
  hits=sum(r[1]for r in struct.iter_unpack('<35I',native));proof=struct.unpack('<5I',(out/'original.trace.proof').read_bytes());assert proof[:4]==(0x53505233,216000,hits,1)and proof[4]>1500,proof
  assert (out/'native.stderr').read_text()=='native_controls=10\n'
  report.update(success=True,cases=216000,actual_guarded_callbacks=hits,trace_sha256=hashlib.sha256(native).hexdigest(),native_controls=10,guard_controls=proof[4],original_primary_and_methods_unchanged=True)
 except Exception as error:report['error']=repr(error);raise
 finally:
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());report['success']&=report['sources_stable'];report['artifacts']={}
  for f in out.iterdir():
   if f.is_file():name=a.report.removesuffix('.json')+'-'+f.name+'.bin';report['artifacts'][f.name]=dict(record=name,sha256=write_report(ROOT,name,f.read_bytes()))
  for n in names:write_report(ROOT,a.report.removesuffix('.json')+'-source-'+hashlib.sha256(n.encode()).hexdigest()[:16]+'.bin',(ROOT/n).read_bytes())
  sha=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],sha256=sha,cases=report.get('cases'),callbacks=report.get('actual_guarded_callbacks'),error=report.get('error'))))
 return not report['success']
if __name__=='__main__':raise SystemExit(main())
