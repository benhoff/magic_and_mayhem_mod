#!/usr/bin/env python3
"""Compare overhanging Save clips with unchanged original pixels/caller/FP state."""
import argparse,hashlib,json,os,struct,subprocess
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 (out/'.retain').write_text('Original/native Save clip regressions and atomic admission controls; preserve.\n')
 names=['reconstruction/rendering/glyph_backend.c','reconstruction/rendering/glyph_backend.h','reconstruction/rendering/glyph_backend_state.c','reconstruction/rendering/glyph_backend_state.h','reconstruction/rendering/glyph_backend_x87.S','runtime/scene/glyph_route.c','runtime/scene/glyph_route.h','runtime/scene/glyph_entry.S','tests/glyph-backend-state-probe.S','tests/glyph-backend-entry-probe.S','tests/glyph-backend-reference.cpp','tests/sprite-binary-reference.cpp','tests/glyph-backend-test.c','tools/test-glyph-save-clip.py']
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names};report=dict(schema=1,success=False,sources=sources)
 def run(label,command):
  r=subprocess.run(list(map(str,command)),capture_output=True,timeout=180);(out/(label+'.log')).write_bytes(r.stdout+r.stderr);r.check_returncode();return r.stdout.decode()
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);original=ROOT/'original/Arcane_Nocd/Chaos.exe';assert hashlib.sha256(original.read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
  frame=bytearray(52);struct.pack_into('<3I',frame,0,52,2,1);struct.pack_into('<2I',frame,40,48,50);frame[48:]=bytes([0,2,0,63]);rows=[]
  for width in (8,800):
   for cold in (0,1):
    for control in range(12):
     for active in (0,1):
      for x in (2,width-2,width,width+1):
       header=bytearray(328);struct.pack_into('<18I',header,0,380,52,width,8,width,x,2,0,0,width+1,8,173,91,237,0,cold,control,active)
       for i in range(64):struct.pack_into('<f',header,72+i*4,i/63)
       rows.append(bytes(header)+frame)
  (out/'cases.bin').write_bytes(b'MNMGLS01'+struct.pack('<I',len(rows))+b''.join(rows))
  objects=[]
  for name in names:
   if name.endswith(('.c','.S')) and name!='tests/glyph-backend-test.c':
    obj=out/(Path(name).name+'.o');run(Path(name).name,['gcc','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/name,'-o',obj]);objects.append(obj)
  run('reference-build',['g++','-std=c++17','-m32','-O2','-fno-pie','-no-pie','-DMNM_GLYPH_FORMATS_TEST','-Wall','-Wextra','-Werror',ROOT/'tests/glyph-backend-reference.cpp',*objects,'-o',out/'reference'])
  report['original_comparison']=run('original-native',[out/'reference',original,out/'cases.bin',out/'results.bin']).strip();data=(out/'results.bin').read_bytes();assert data[:8]==b'MNMGLBK1' and len(data[12:])%64==0
  records=[struct.unpack_from('<16I',data,at) for at in range(12,len(data),64)];assert all(r[0]==255 for r in records);assert all(r[1:4]==(1,0,0) for r in records if r[14]==0)
  run('atomic-build',['gcc','-std=c11','-O1','-g','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie',ROOT/'reconstruction/rendering/glyph_backend.c',ROOT/'reconstruction/rendering/glyph_backend_state.c',ROOT/'tests/glyph-backend-test.c','-o',out/'atomic'])
  report['atomic_controls']=run('atomic-controls',[out/'atomic']).strip();report.update(success=True,cases=len(rows),complete_original_matches=len(records),native_without_original=sum(r[14]==0 for r in records),formats=['RGB565','RGB555'],clip_right=[9,801],stride=[8,800])
 except Exception as e:report['error']=repr(e)
 finally:
  try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except Exception as e:report.update(success=False,manifest_error=repr(e))
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());report['success'] &= report['sources_stable'];report['artifacts']={}
  for path in out.iterdir():
   if path.is_file():
    name='research/runtime/terrain-light-qt-results-20261026/'+out.name+'-'+path.name+'.bin';digest=write_report(ROOT,name,path.read_bytes());report['artifacts'][name]=digest
  print(write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode()),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
