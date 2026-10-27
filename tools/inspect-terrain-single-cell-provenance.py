#!/usr/bin/env python3
"""Record original static-map storage and candidate construction/write references."""
import argparse,hashlib,json,struct,subprocess
from pathlib import Path
from coverage_evidence import write_report
from original_x86 import PE32
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--main-root',type=Path,required=True);parser.add_argument('--report',required=True);args=parser.parse_args();main=args.main_root.resolve()
 assert not (ROOT/args.report).exists(),'Preserve prior reports'
 report=dict(schema=1,success=False,original_sha256=BUILD,sources={str(Path(__file__).relative_to(ROOT)):hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},scope='Static pinned PE storage and byte-pattern candidates. Candidate references require instruction/receiver confirmation; no installed runtime or blanket writer completeness claim.')
 try:
  report['manifest_before']=subprocess.check_output([main/'tools/original-manifest.sh','verify'],text=True)
  exe=main/'original/Arcane_Nocd/Chaos.exe';pe=PE32(exe.read_bytes());assert pe.sha256==BUILD
  address=0x6c5490+0x6380;section=pe.section(address)
  report['storage']=dict(map=0x6c5490,offset=0x6380,address=address,section=section,file_backed=address-section['address']<section['stored'],image_initial_byte=pe.read(address,1,zero_fill=True)[0],preceding_pointer_slot_address=0x6c5490+0x637d)
  report['candidates']={}
  for label,value in [('global-map',0x6c5490),('absolute-kernel-byte',address),('relative-kernel-byte',0x6380),('preceding-slot',0x637d)]:
   found=[];needle=struct.pack('<I',value)
   for s in pe.sections:
    if not s['executable']:continue
    raw=pe.read(s['address'],s['stored']);at=raw.find(needle)
    while at>=0:found.append(s['address']+at);at=raw.find(needle,at+1)
   report['candidates'][label]=found
  ranges={'map-construction':(0x4ec800,0x4ecbe0),'map-clear-init':(0x4ecbe0,0x4ecf20),'kernel-builder':(0x4f1890,0x4f1b60),'type-lookup':(0x49cf60,0x49cf90)}
  report['ranges']={k:dict(start=a,end=b,disassembly=subprocess.check_output(['objdump','-d','-Mintel','--start-address='+hex(a),'--stop-address='+hex(b),exe],text=True)) for k,(a,b) in ranges.items()}
  report['success']=True
 except Exception as exc:report['error']=repr(exc)
 finally:
  report['manifest_after']=subprocess.check_output([main/'tools/original-manifest.sh','verify'],text=True)
  digest=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],sha256=digest,storage=report.get('storage'),candidates=report.get('candidates'),error=report.get('error'))),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
