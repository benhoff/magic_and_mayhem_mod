#!/usr/bin/env python3
"""Offline Surface2 full-surface lock/DC admission and output-mutation comparison."""
import argparse,gzip,hashlib,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-access-capture-20261006.json'
spec=importlib.util.spec_from_file_location('capture',ROOT/'tools/capture-surface-access.py');capture=importlib.util.module_from_spec(spec);spec.loader.exec_module(capture)
def require(ok,message):
 if not ok:raise ValueError(message)
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def load():
 cap=json.loads(DEFAULT.read_text());require(cap['schema']==1 and cap['success'] is True,'Invalid access capture');path=cap['corpus']['path'];require(path=='tests/fixtures/surfaces/surface-access.json.gz','Unsafe fixture path');require(sha(ROOT/path)==cap['corpus']['sha256'],'Access fixture hash differs')
 with gzip.open(ROOT/path,'rb') as f:raw=f.read(8*1024*1024+1)
 require(len(raw)<=8*1024*1024,'Expanded access limit');data=json.loads(raw);require(data['schema']==1 and len(data['rows'])==cap['cases']==96,'Access case count');return cap,data

def descriptor(bits,caps,busy):
 d=[0]*27;d[0]=108
 # Failure clears all meaningful fields, including lpSurface; trailing caps
 # is unstable and retained verbatim in capture but excluded from equivalence.
 if not busy:
  d[1]=0x100f;d[2]=6;d[3]=8;d[4]=bits;d[9]=2;d[18]=32;d[19]=0x60 if bits==8 else 0x40;d[21]=bits
  d[22:25]={8:[0,0,0],16:[0xf800,0x7e0,0x1f],32:[0xff0000,0xff00,0xff]}[bits]
 else:d[9]=1
 d[26]=0x840 if caps==0x840 else 0x10004040
 return d

def compare(data):
 require([r['input'] for r in data['rows']]==capture.inputs(),'Access case matrix differs');counts=dict(cases=0,steps=0,descriptor_fields=0,unstable_caps_words=0,pixels=0,terminal_busy_cases=0);states=[]
 for row in data['rows']:
  c=row['input'];require(len(row['steps'])==len(c['operations']),'Access step extent differs');balance=0;dc=False;expected=[]
  for op,actual in zip(c['operations'],row['steps']):
   result=0;out=0xffffffff;desc=[0xffffffff]*27
   if op==0:
    busy=balance!=0 or dc;result=0x887601ae if busy else 0;desc=descriptor(c['bits'],c['caps'],busy)
    if not busy:balance+=1
   elif op==1:
    result=0x88760248 if balance==0 else 0
    if balance!=0:balance-=1
   elif op==2:
    result=0x8876026c if dc else 0;out=0 if dc else 2
    if not dc:balance+=1;dc=True
   else:
    result=0x8876024a if not dc else 0x8876086c if op in [4,5] else 0
    if result==0:balance-=1;dc=False
   require(actual['result']==result,'Access HRESULT differs');require(actual['dc_out_state']==out,'DC output mutation differs');require(len(actual['descriptor'])==27,'Descriptor extent differs')
   fields=26 if op==0 and result else 27;require(actual['descriptor'][:fields]==desc[:fields],'Lock descriptor/output clearing differs');counts['descriptor_fields']+=fields if op==0 else 0;counts['unstable_caps_words']+=int(fields==26);counts['steps']+=1;expected.append(dict(result=result,dc_out_state=out,descriptor=desc,balance=balance,dc=dc))
  busy=balance!=0 or dc;final=row['final_lock'];require(final['result']==(0x887601ae if busy else 0),'Terminal Lock result differs');fields=26 if busy else 27;require(len(final['descriptor'])==27 and final['descriptor'][:fields]==descriptor(c['bits'],c['caps'],busy)[:fields],'Terminal descriptor differs');counts['descriptor_fields']+=fields;counts['unstable_caps_words']+=int(busy)
  if busy:require(row['pixels'] is None,'Busy terminal cannot supply pixels');counts['terminal_busy_cases']+=1
  else:require(row['pixels']==[(i*17+3)&((1<<c['bits'])-1) for i in range(48)],'Post-transition pixels changed');counts['pixels']+=48
  states.append(expected);counts['cases']+=1
 return counts,states

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path);a=p.parse_args();cap,data=load();counts,_=compare(data);names=['tools/check-surface-access.py','tools/capture-surface-access.py','tests/test-surface-access.py',str(DEFAULT.relative_to(ROOT)),cap['corpus']['path']];out=dict(schema=1,success=True,sources={n:sha(ROOT/n) for n in names},**counts,scope='Input-derived signed mapping-balance/DC admission model compares independently captured HRESULTs, output mutation,26 stable busy-Lock descriptor words/all27 successful words and readable final pixels. Unstable failure caps word retained but excluded; terminal over-unlock debt represented explicitly. No underlying Wine counter implementation proof, game/COM alias/cross-thread/loss/Windows/live equivalence.')
 if a.report:
  with a.report.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
