#!/usr/bin/env python3
"""Replay the retained timeout and native cohort through the persistent worker.

Every cohort reply is compared with unchanged PE32 and sanitized one-shot code;
4096 persistent requests also exercise kernel cache reuse and invalidation.
"""
import argparse,ctypes,hashlib,importlib.util,json,mmap,os,struct,subprocess,sys,time
from pathlib import Path
from coverage_evidence import read_bytes,read_json,write_report
import terrain_light_session as session
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('build','output','report'):p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 (out/'.retain').write_text('Timeout replay, unchanged-original and sanitizer comparisons; preserve.\n')
 spec=importlib.util.spec_from_file_location('session_controls',ROOT/'tests/test-terrain-light-session.py');controls=importlib.util.module_from_spec(spec);spec.loader.exec_module(controls)
 build=json.loads((a.build/'manifest.json').read_text());sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in set(build['sources'])|{'tests/test-terrain-light-session.py','tools/test-terrain-light-worker-replay.py'}}
 report=dict(schema=1,success=False,sources=sources,results=[],requests=0)
 def run(command):
  r=subprocess.run(list(map(str,command)),capture_output=True,timeout=180)
  with (out/'commands.log').open('ab') as f:f.write(r.stdout+r.stderr)
  r.check_returncode()
 try:
  run([ROOT/'tools/original-manifest.sh','verify'])
  models=[ROOT/'reconstruction/rendering'/n for n in ('terrain_lighting.cpp','terrain_creature_lighting.cpp','terrain_static_lighting.cpp','terrain_lighting_cycle.cpp')]
  for name,flags in [('original',['-O2','-DORIGINAL_REFERENCE','-m32','-fno-pie','-no-pie']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
   run(['g++','-std=c++17',*flags,ROOT/'tools/terrain-light-replay.cpp',*models,'-o',out/name])
  analysis=read_json(ROOT/'research/runtime/terrain-light-qt-results-20261026/native-timeout-analysis01.json',root=ROOT)
  samples=[]
  for i,row in enumerate(analysis['results']):
   raw=read_bytes(ROOT/('research/runtime/terrain-light-qt-results-20261026/native-timeout-sample-%02d.bin'%i),root=ROOT)
   n=row['input_bytes'];r=row['reply_bytes'];samples.append((raw[:n],raw[n+32:n+r]))
  channel=read_bytes(ROOT/'research/runtime/terrain-light-qt-results-20261026/native-timeout-final-channel01.bin',root=ROOT)
  n=struct.unpack_from('<I',channel,20)[0];late=channel[128:128+n];samples.append((late,None))
  # Configuration changes invalidate derived kernels, even with equal geometry.
  for offset in (48,52,56):
   data=bytearray(late);value=struct.unpack_from('<i',data,offset)[0];struct.pack_into('<i',data,offset,value+1 if value<126 else value-1);struct.pack_into('<I',data,120,controls.checksum(data));samples.append((bytes(data),None))
  golden=[]
  for i,(data,expected) in enumerate(samples):
   source=out/('sample-%02d.in'%i);source.write_bytes(data)
   run([out/'original',source,out/'reference.reply',ROOT/'original/Arcane_Nocd/Chaos.exe']);run([out/'sanitized',source,out/'sanitized.reply'])
   original=(out/'reference.reply').read_bytes()[32:];assert (out/'sanitized.reply').read_bytes()[32:]==original
   assert expected is None or original==expected
   golden.append((data,original));report['results'].append(dict(input_sha256=hashlib.sha256(data).hexdigest(),result_sha256=hashlib.sha256(original).hexdigest(),original_match=True,sanitized_match=True))
  # FNV implementation used only to renew sequence/nonce in test inputs.
  checksum_source=out/'checksum.c';checksum_source.write_text('#include <stdint.h>\nuint32_t checksum(const unsigned char*p,uint32_t n){uint32_t h=2166136261u;for(uint32_t i=0;i<n;++i)h=(h^(i>=120&&i<124?0:p[i]))*16777619u;return h;}\n')
  run(['gcc','-O2','-shared','-fPIC',checksum_source,'-o',out/'checksum.so']);fnv=ctypes.CDLL(str(out/'checksum.so')).checksum;fnv.argtypes=[ctypes.c_void_p,ctypes.c_uint32];fnv.restype=ctypes.c_uint32
  controls.BUILD=a.build.resolve();controls.OUT=out;controls.GOLD=golden[0]
  root,metadata,env=controls.fixture(capture='none');maximum=0;start=time.monotonic()
  with session.Session(root,metadata,env) as peer:
   mm=peer.mapping
   for seq in range(1,4097):
    source,expected=golden[(seq-1)%len(golden)];data=bytearray(source);data[24:32]=golden[0][0][24:32];struct.pack_into('<II',data,20,seq,struct.unpack_from('<I',data,24)[0]);struct.pack_into('<I',data,112,1)
    array=(ctypes.c_ubyte*len(data)).from_buffer(data);struct.pack_into('<I',data,120,fnv(array,len(data)))
    mm[128:128+len(data)]=data;struct.pack_into('<I',mm,20,len(data));struct.pack_into('<I',mm,32,seq)
    tick=time.monotonic();controls.publish(mm,1);controls.until(lambda:controls.phase(mm)==2);maximum=max(maximum,time.monotonic()-tick)
    n=struct.unpack_from('<I',mm,24)[0];reply=bytes(mm[128+session.CAPACITY:128+session.CAPACITY+n]);assert reply[32:]==expected;assert struct.unpack_from('<I',reply,16)[0]==seq
    controls.publish(mm,0);report['requests']=seq
   report['statistics']=dict(zip(session.STATS,struct.unpack_from('<17I',mm,56)))
  assert peer.result['success'];report.update(success=True,elapsed_seconds=time.monotonic()-start,max_round_trip_ms=1000*maximum,worker_reaped=True)
 except Exception as e:report['error']=repr(e)
 finally:
  try:run([ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except Exception as e:report.update(success=False,manifest_error=repr(e))
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());report['success'] &= report['sources_stable']
  for path in out.glob('*'):
   if path.is_file():write_report(ROOT,'research/runtime/terrain-light-qt-results-20261026/'+out.name+'-'+path.name+'.bin',path.read_bytes())
  print(write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode()),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
