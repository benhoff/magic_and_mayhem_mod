#!/usr/bin/env python3
"""Refuse malformed captured lighting inputs without producing a reply."""
import argparse,json,struct,subprocess
from pathlib import Path
def main():
 p=argparse.ArgumentParser();p.add_argument('--binary',type=Path,required=True);p.add_argument('--input',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve();out.mkdir(exist_ok=False);source=a.input.read_bytes();cases=[]
 def changed(name,offset,value):
  b=bytearray(source);struct.pack_into('<I',b,offset,value);cases.append((name,b))
 for size in [0,7,127,299,len(source)-1]:cases.append(('truncated-'+str(size),bytearray(source[:size])))
 for name,offset,value in [('magic',0,0),('version',8,2),('header-size',12,124),('total',16,len(source)+1),('width-zero',32,0),('width-large',32,129),('height-zero',36,0),('layers-zero',40,0),('layers-large',40,33),('count',44,1),('creatures-large',76,65537),('scan-large',80,65537),('objects-large',84,65537),('object-scan',88,65537),('creature-enabled',68,2),('object-enabled',72,2),('mode',112,2),('reserved',116,1),('reserved-tail',124,1),('creature-phase',256,8),('creature-cursor',268,65537),('published',272,2),('static-phase',276,2),('static-cursor',288,65537),('static-active',292,2),('static-request',296,2),('source-active',300,2),('source-enabled',304,2),('source-reserved',324,1)]:changed(name,offset,value)
 cc=struct.unpack_from('<I',source,76)[0];obj=300+cc*28
 changed('object-active',obj,2);changed('object-reserved',obj+28,1)
 for i,(name,b) in enumerate(cases):
  if len(b)>=128:
   h=2166136261
   for j,v in enumerate(b):h=((h^(0 if 120<=j<124 else v))*16777619)&0xffffffff
   struct.pack_into('<I',b,120,h)
  path=out/(str(i)+'.input');reply=out/(str(i)+'.reply');path.write_bytes(b)
  r=subprocess.run([str(a.binary.resolve()),str(path),str(reply)],capture_output=True,text=True,timeout=10)
  if r.returncode!=1 or reply.exists():raise RuntimeError('Malformed input accepted: '+name)
  (out/(str(i)+'.log')).write_text(r.stdout+r.stderr)
 print(json.dumps(dict(success=True,refused_inputs=len(cases))))
if __name__=='__main__':main()
