#!/usr/bin/env python3
"""Retain hash-pinned terrain/light producer disassembly directly in SQLite."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def main():
 p=argparse.ArgumentParser();p.add_argument('--main-root',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args();m=a.main_root.resolve();target=a.report.absolute()
 if target.exists() or not target.is_relative_to(ROOT/'research/runtime'):p.error('Require new research report')
 result=dict(schema=1,kind='static',success=False,original_sha256=BUILD)
 try:
  result['manifest_before']=subprocess.check_output([m/'tools/original-manifest.sh','verify'],text=True)
  pe=m/'original/Arcane_Nocd/Chaos.exe';assert hashlib.sha256(pe.read_bytes()).hexdigest()==BUILD
  ranges={'map-load':(0x501e00,0x502409),'main-loop':(0x46afc0,0x46bcab),'state-handler':(0x51e890,0x51ec76),'sharing-script':(0x56a8d0,0x56b200),'owner-setters':(0x465440,0x465650),'wrapper-init':(0x4ecdd0,0x4ecf1c),'boxed-coordinates':(0x40e290,0x40e2c0),'boxed-y':(0x40e8a0,0x40e8e0),'boxed-z':(0x40eeb0,0x40eef0)}
  result['ranges']={n:dict(start=lo,end=hi,disassembly=subprocess.check_output(['objdump','-d','-Mintel','--start-address='+hex(lo),'--stop-address='+hex(hi),pe],text=True)) for n,(lo,hi) in ranges.items()};result['success']=True
 except Exception as e:result['error']=str(e)
 finally:
  result['manifest_after']=subprocess.check_output([m/'tools/original-manifest.sh','verify'],text=True)
  print(write_report(ROOT,target,(json.dumps(result,indent=2)+'\n').encode()),flush=True)
 return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
