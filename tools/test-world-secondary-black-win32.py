#!/usr/bin/env python3
"""Compare exact secondary black queue continuations and pixels on a PE32 host."""
import argparse,hashlib,json,os,struct,subprocess
from pathlib import Path
from coverage_evidence import write_report
from kind8_sprite_fixture import BUILD,original_fixture
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--wine-prefix',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve();prefix=a.wine_prefix.resolve()
 if out.exists()or(ROOT/a.report).exists()or not out.is_relative_to(ROOT/'working')or not prefix.is_relative_to(ROOT/'working'):p.error('Fresh owned output/report and owned fixture prefix required')
 out.mkdir(parents=True);names=['tests/world-secondary-black-win32.c','tests/world-secondary-black-call.S','tools/test-world-secondary-black-win32.py','tools/kind8_sprite_fixture.py','runtime/scene/world_raster_callers.h','runtime/shadow/win32_min.h','reconstruction/rendering/raster_pixels.c','reconstruction/rendering/raster_pixels.h','reconstruction/rendering/pixel_kernel.c','reconstruction/rendering/pixel_kernel.h']
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in names}
 report=dict(schema=1,success=False,sources=sources,original_sha256=BUILD,original_game_runs=0,scope='Private PE32 original secondary black pushes/CALL/normalization/clipped fallback/node stores. Exhaustive AX routing and original black/clipped pixel bodies versus native CPU model; no complete queue traversal, GPU execution, full volatile ABI or natural game claim.',commands=[])
 env={k:v for k,v in os.environ.items()if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_SECONDARY_ORIGINAL='Z:'+str(ROOT/'original/Arcane_Nocd/Chaos.exe').replace('/','\\'),MNM_SECONDARY_PROOF='Z:'+str(out/'proof.bin').replace('/','\\'))
 def run(label,cmd):
  with(out/(label+'.log')).open('xb')as f:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=180)
  report['commands'].append(dict(label=label,command=list(map(str,cmd)),returncode=r.returncode))
  if r.returncode:raise ValueError(label+' exited '+str(r.returncode))
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);assert hashlib.sha256((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()).hexdigest()==BUILD
  report['fixture']=original_fixture(ROOT,out)
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-omit-frame-pointer','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-I'+str(out)]
  objects=[]
  for n in ['tests/world-secondary-black-win32.c','tests/world-secondary-black-call.S','reconstruction/rendering/pixel_kernel.c','reconstruction/rendering/raster_pixels.c']:
   obj=out/(Path(n).name+'.obj');run('compile-'+Path(n).name,[*flags,'-c',ROOT/n,'-o',obj]);objects.append(obj)
  run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'reference.exe'),*objects,ROOT/'working/build/scene-observer-selftest/kernel32.lib'])
  run('original-and-native',['xvfb-run','-a','wine',out/'reference.exe'])
  proof=struct.unpack('<10I',(out/'proof.bin').read_bytes());assert proof[0]==0x53424c32 and proof[1:4]==(393216,6804,6804*304)and proof[7:]==(1,1,1),proof
  report.update(success=True,ax_cases=proof[1],pixel_cases=proof[2],compared_words_including_canaries=proof[3],guarded_callbacks=proof[4],black_callbacks=proof[5],clipped_callbacks=proof[6],selected_fragment_bytes_unchanged=True,original_raster_bodies_executed=True,source_and_node_canaries_preserved=True)
 except Exception as e:report['error']=repr(e)
 finally:
  try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except Exception as e:report.update(success=False,manifest_error=repr(e))
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());report['success']&=report['sources_stable'];report['artifacts']={}
  for f in sorted(out.iterdir()):
   if f.is_file()and f.suffix!='.obj':
    name=a.report.removesuffix('.json')+'-'+f.name+'.bin';report['artifacts'][f.name]=dict(record=name,sha256=write_report(ROOT,name,f.read_bytes()))
  for n in names:write_report(ROOT,a.report.removesuffix('.json')+'-source-'+hashlib.sha256(n.encode()).hexdigest()[:16]+'.bin',(ROOT/n).read_bytes())
  write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k)for k in ['success','error','ax_cases','pixel_cases','compared_words_including_canaries']}),flush=True)
 return not report['success']
if __name__=='__main__':raise SystemExit(main())
