#!/usr/bin/env python3
"""Replay independently captured format draws through owned signed byte rows, offline."""
import argparse,importlib.util,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('formats',ROOT/'tools/check-surface-formats.py');oracle=importlib.util.module_from_spec(s);s.loader.exec_module(oracle)
SOURCES=['renderer/blit.cpp','renderer/blit.hpp','renderer/pixel_rows.cpp','renderer/pixel_rows.hpp','renderer/surface_backend.cpp','renderer/surface_backend.hpp','renderer/surface_access.hpp','renderer/bitmap_dc.hpp','renderer/surface_copy.cpp','renderer/surface_copy.hpp','renderer/known_pixels.hpp','renderer/CMakeLists.txt','tests/render-surface-formats-test.cpp','tools/check-native-surface-formats.py','tools/check-surface-formats.py','tests/test-surface-formats.py','research/runtime/surface-formats-final-capture-20261006.json','tests/fixtures/surfaces/surface-formats-final.json.gz']
def pitch(f,l):
 tight=7*(oracle.DEPTHS[f]//8);padded=((tight+3)&~3)+4
 return [0,padded,-padded,tight,-tight][l]
def packed(pixels,f,p):
 size=oracle.DEPTHS[f]//8;data=bytearray([0xcd]*256);first=16+(4*-p if p<0 else 0)
 for i,v in enumerate(pixels):at=first+(i//7)*p+(i%7)*size;data[at:at+size]=v.to_bytes(size,'little')
 return data.hex()
def rgba(pixels,f):
 out=[]
 for v in pixels:
  if f==0:
   n=0x81 if v==0x82 else v;channels=[n*17&255,(n*29+3)&255,(n*43+9)&255]
  else:
   channels=[]
   for i,mask in enumerate(oracle.MASKS[f]):
    low=mask&-mask;value=(v&mask)//low;maximum=mask//low
    shift=2 if f==2 and i==1 else 3
    channels.append((value<<shift)|(value>>(6-shift if shift==2 else 2)) if f==2 else value*255//maximum)
  out.append(channels[0]|channels[1]<<8|channels[2]<<16|0xff000000)
 return out
def compare(actual,inputs,rows):
 byid={r['input']['id']:r for r in rows};seen=set();words=0;driver=0;imports=0
 oracle.require(actual['refusals']>=50,'Missing guards')
 for out,c in zip(actual['rows'],inputs,strict=True):
  tag=(c['id'],c['variant']);oracle.require(tag not in seen and (out['id'],out['variant'])==tag,'Native identity');seen.add(tag);r=byid[c['id']];f=c['format'];p=c['pitch']
  oracle.require(out['hresult']==r['hresult'] and out['source']==r['source_after']['pixels'] and out['destination']==r['destination_after']['pixels'],'Native draw mismatch '+str(tag));words+=70
  for side in ['source','destination']:
   descriptor=r[side+'_after']['descriptor'][:];descriptor[4]=p&0xffffffff
   oracle.require(out[side+'_descriptor']==descriptor,'Native descriptor '+str(tag));words+=27
   expected=packed(r[side+'_after']['pixels'],f,p);oracle.require(out[side+'_bytes']==expected,'Native signed rows/padding/guards '+str(tag))
   if c['variant']==0 and c['layout']:oracle.require(out[side+'_bytes']==r[side+'_storage'],'Independent caller storage '+str(tag))
  oracle.require(out['rgba']==rgba(out['destination'],f),'Native presentation policy '+str(tag));words+=35
  if c['variant']==0:driver+=1
  else:imports+=1
 return dict(driver_draws=driver,signed_row_draws=imports,native_checks=actual['checks'],native_refusals=actual['refusals'],native_word_checks=words,packed_byte_checks=len(inputs)*512,presentation_pixels=len(inputs)*35)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/renderer');p.add_argument('--executable',type=Path);p.add_argument('--report',type=Path);a=p.parse_args();capture,rows=oracle.load();counts=oracle.validate(capture,rows);parent=ROOT/'working/tests/native-surface-formats';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));report=a.report or run/'report.json';fingerprints={n:oracle.sha(ROOT/n) for n in SOURCES};inputs=[]
 for r in rows:
  if any(r['create']):continue
  c=r['input'];f=c['format'];default=r['source_before']['descriptor'][4];default=default if default<0x80000000 else default-0x100000000
  for variant in ([0,1,2,3,4] if c['layout']==0 else [0]):inputs.append(dict(c,variant=variant,pitch=default if variant==0 else pitch(f,variant),**{'import':c['layout']!=0 or variant!=0}))
 inp=run/'inputs.json';inp.write_text(json.dumps(inputs,separators=(',',':')));output=run/'outputs.json';binary=a.executable.resolve() if a.executable else a.build.resolve()/'render-surface-formats-test';library=a.build.resolve()/'libmnm-renderer.a'
 proc=subprocess.run([str(binary),str(inp),str(output)],capture_output=True,text=True,env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),timeout=160);(run/'native.log').write_text(proc.stdout+proc.stderr);oracle.require(proc.returncode==0,proc.stdout+proc.stderr);actual=json.loads(output.read_text());results=compare(actual,inputs,rows);oracle.require(all(oracle.sha(ROOT/n)==h for n,h in fingerprints.items()),'Sources changed during replay')
 r=dict(schema=1,success=True,sources=fingerprints,**counts,**results,binary_sha256=oracle.sha(binary),library_sha256=oracle.sha(library),inputs_sha256=oracle.sha(inp),outputs_sha256=oracle.sha(output),artifacts=str(run.relative_to(ROOT)),scope='Owned five-format fill/key/opaque/keyed/overlap/clip results and complete native words compare independent standalone DirectDraw2/Surface2 captures. Four additional tight/padded signed byte imports use the same recorded logical outputs; driver negative-pitch rejection retained separately. Active RGB key masking, high installed key bits and fill masking measured; copies retain unused native bits. Caller bytes are owned copies, no borrowed pointers. Presentation conversion and signed import/guard/budget policy are native. No original game multi-format reachability, cross-format/DC conversion, Windows or live replacement claim.')
 with report.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
 print(json.dumps(dict(success=True,**counts,**results)))
if __name__=='__main__':main()
