#!/usr/bin/env python3
"""Compare retained installed glyph sources/tables/tints with unmodified581ec0."""
import argparse,hashlib,json,shlex,struct,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from resident_capture import records
from scene_effects import module
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/font-dc-lifetime-results-20261108/'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--acquisition',action='append',required=True);p.add_argument('--report',required=True);p.add_argument('--failed-prefix',action='store_true');a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned glyph result required')
 out=ROOT/'working/font-dc-glyph-kernels'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Retained original/native glyph comparison, input corpus, exact tested binaries and source. Preserve.\n')
 build=Path('/dev/shm/mnm-font-dc-20261108/glyph-kernel');reference=build.parent/'glyph-reference';native=build/'glyph-raster-native'
 q=dict(schema=1,kind='retained_installed_glyph_kernel_comparison',success=False,original_game_runs=0,new_original_game_runs=0,original_body='NoCD40209ca7:581ec0',original_pixels_used_as_native_inputs=False,sources={},acquisitions=[],artifacts=[],scope='Every distinct retained installed glyph/frame, RGB tint and initialized coverage table in these actual cases, in three bounded whole/partly clipped/invisible initialized destinations. This is isolated glyph-body comparison, not another game, original font layout or DC behavior.')
 def run(name,cmd):
  with(out/(name+'.log')).open('xb')as log:subprocess.run([str(v)for v in cmd],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=180,check=True)
 before=False
 try:
  dependencies={'tools/check-installed-glyph-kernels.py','tools/resident_capture.py','tools/inspect-canvas-producers.py','tools/scene_effects.py','tests/glyph-raster-reference.cpp','tests/glyph-raster-native.cpp','tests/glyph-raster-test.cpp','tests/glyph-raster/CMakeLists.txt','assets/CMakeLists.txt'}
  for dep in [ROOT/'working/font-dc-glyph-reference.d',*build.rglob('*.o.d')]:
   for name in shlex.split(dep.read_text().replace('\\\n',' ')):
    path=Path(name).resolve()
    if path.is_file()and path.is_relative_to(ROOT)and not path.is_relative_to(ROOT/'working'):dependencies.add(path.relative_to(ROOT).as_posix())
  q['sources']={n:sha(ROOT/n)for n in sorted(dependencies)}
  unique={};cps=module('inspect-canvas-producers')
  with Reader(ROOT)as d:
   for record in a.acquisition:
    aq=d.json(record);require((aq['success']or a.failed_prefix)and aq['sources_stable']and aq.get('private_wine_stopped')is True,'Stopped source acquisition required');q['acquisitions'].append(dict(record=record,sha256=d.digest(record)))
    source=Path(aq['temporary_root'])/'worlds.bin';matches=[v for v in aq['artifacts']if v['path']==source.relative_to(ROOT).as_posix()];require(len(matches)==1 and sha(source)==matches[0]['sha256']==d.digest(matches[0]['record']),'Changed retained glyph source archive')
    for generation,chunk,stream,raster in records(source):
     _,rows=cps.decode(stream,complete=False)
     for r,body in rows:
      if r[2]!=8:continue
      frame,table=body[:r[19]],body[r[19]+16:];require(len(table)==256,'Initialized font table extent')
      key=hashlib.sha256(frame+table+struct.pack('<3I',*r[15:18])).hexdigest();unique.setdefault(key,(frame,table,r[15:18]))
  q['failed_prefix_only']=a.failed_prefix;q['closes_acceptance']=False
  require(0<len(unique)<=8192,'Bounded nonempty actual glyph domain required');corpus=[];words=0
  for frame,table,tint in unique.values():
   fw,fh,ox,oy=struct.unpack_from('<4i',frame,4);w,h=max(32,fw+8),max(24,fh+8);require(w<=256 and h<=256,'Glyph destination bound')
   for x,y in ((4,4),(4,-3),(4,h)):
    values=[316+len(frame),len(frame),w,h,w+2,x+ox,y+oy,0,0,w,h,*tint,113];corpus.append(struct.pack('<15I',*(v&0xffffffff for v in values))+table+frame);words+=w*h
  raw=b'MNMGLY01'+struct.pack('<I',len(corpus))+b''.join(corpus);(out/'cases.bin').write_bytes(raw);q.update(unique_actual_sources=len(unique),cases=len(corpus),words_compared=words,corpus_sha256=hashlib.sha256(raw).hexdigest())
  pe=ROOT/'original/Arcane_Nocd/Chaos.exe';require(sha(pe)=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Original build mismatch')
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
  run('admission',[build/'glyph-raster-test']);run('original',[reference,pe,out/'cases.bin',out/'original.565']);run('native',[native,out/'cases.bin',out/'native.565'])
  require((out/'original.565').stat().st_size==words*2 and(out/'native.565').stat().st_size==words*2,'Glyph comparison denominator')
  q.update(original_sha256=sha(out/'original.565'),native_sha256=sha(out/'native.565'));require(q['original_sha256']==q['native_sha256'],'Original/native installed glyph output differs');q['success']=True
 except Exception as e:q['error']=str(e)
 finally:
  if before:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);q['original_manifest_verified_before_after']=True
  q['sources_stable']=all(sha(ROOT/n)==h for n,h in q['sources'].items());q['success']&=q['sources_stable']
  for path in [*sorted(out.glob('*')),*[ROOT/n for n in q['sources']],native,reference,build/'glyph-raster-test']:
   if not path.is_file()or path.name=='.retain':continue
   name=a.report.removesuffix('.json')+'-artifact-'+str(len(q['artifacts']))+'.bin';raw=path.read_bytes();h=write_report(ROOT,name,raw);q['artifacts'].append(dict(path=str(path),record=name,sha256=h,bytes=len(raw)))
  h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps({k:q.get(k)for k in ['success','error','cases','words_compared']}|dict(record=a.report,sha256=h)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
