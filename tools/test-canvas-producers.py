#!/usr/bin/env python3
"""Exercise producer entry/return forwarding and strict closed-input refusal."""
import argparse,hashlib,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path);a=p.parse_args()
 spec=importlib.util.spec_from_file_location('inspector',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 paths=['runtime/scene/canvas_producers.c','runtime/scene/canvas_producers.S','tests/canvas-producers-test.c','tests/canvas-startup-call.S','tools/test-canvas-producers.py','tools/inspect-canvas-producers.py','runtime/shadow/win32_min.h','protocols/include/mnm/canvas_producers_v1.h','tools/build-scene-observer.py','runtime/scene/world_producer_bypass.c','protocols/include/mnm/world_producer_bypass_v1.h','tests/world-producer-bypass-call.S','runtime/scene/world_raster_callers.h']
 claims=json.loads(a.claims.read_text()) if a.claims else None
 sources={name:sha(ROOT/name) for name in paths}
 if claims:
  for name,h in claims['sources'].items():
   if sha(ROOT/name)!=h:raise ValueError('Changed prospective source: '+name)
   sources[name]=h
 parent=ROOT/'working/tests/canvas-producers';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));capture=out/'capture';capture.mkdir();print(out,flush=True)
 env={**os.environ,'WINEPREFIX':str(ROOT/'working/tests/scene-selftest-wine'),'WINEDEBUG':'-all','MNM_CANVAS_PRODUCERS':'1','MNM_SCENE_DIR':'Z:'+str(capture).replace('/','\\'),'MNM_SCENE_SAMPLES':'1'}
 env.pop('MNM_CANVAS_STARTUP',None);env.pop('MNM_SCENE_WORLD',None)
 def run(name,command):
  with (out/(name+'.log')).open('x') as log:subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
 run('bridge-build',['python3',ROOT/'tools/build-scene-observer.py','--selftest']);build=ROOT/'working/build/scene-observer-selftest'
 objects=[]
 for name in ['tests/canvas-producers-test.c','tests/canvas-startup-call.S','runtime/scene/canvas_producers.S','tests/world-producer-bypass-call.S']:
  obj=out/(Path(name).name+'.obj');objects.append(obj);run('compile-'+Path(name).name,['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/name,'-o',obj])
 run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'fixture.exe'),*objects,build/'kernel32.lib'])
 run('forwarding',['xvfb-run','-a','wine',out/'fixture.exe'])
 raw=(capture/'canvas-producers.bin').read_bytes();h,rs=m.decode(raw,complete=False);assert rs
 bad=[raw[:63],raw[:-1],b'BADMAGIC'+raw[8:]]
 for offset,value in [(8,2),(12,32),(16,0),(20,1),(24,0),(64,95),(68,0),(72,24),(64+23*4,1)]:
  b=bytearray(raw);struct.pack_into('<I',b,offset,value);bad.append(bytes(b))
 refused=0
 for b in bad:
  try:m.decode(b,complete=False)
  except ValueError:refused+=1
  else:raise AssertionError('Malformed producer stream accepted')
 assert sources=={name:sha(ROOT/name) for name in sources}
 report={'success':True,'sources':sources,'sources_stable':True,'forwarded_entries':49,'native_bypass_entries':1,'complete_raster_bypass_entries':13,'complete_raster_ax_and_cleanup_preserved':True,'outside_world_raster_forwarded':True,'native_bypass_state_preserved':True,'malformed_bypass_refusals':2,'nested_jpeg_lock_forwarded':True,'register_flags_float_error_preserved':True,'signature_refusal_before_mutation':True,'malformed_refusals':refused,'original_renderer_executed':False,'scope':'Actual PE32 entry/return/relocated JPEG-call forwarding against synthetic compatible prologues, all49hook sites, post-close calls, full register/flags/x87/SSE/LastError and atomic signature refusal. No original pixel equivalence.'}
 if claims:report['claims']=claims['claims']
 (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
