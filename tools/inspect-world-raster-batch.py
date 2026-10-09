#!/usr/bin/env python3
"""Retain pinned World consumer/caller disassembly before queue batching."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ORIGINAL_SHA='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Preserve prior reports; select a new path')
 subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
 try:
  exe=ROOT/'original/Arcane_Nocd/Chaos.exe';assert hashlib.sha256(exe.read_bytes()).hexdigest()==ORIGINAL_SHA
  ranges=[]
  for name,start,end in [('consumer',0x5002a0,0x5008b0),('fallback-wrapper',0x57e8e0,0x57eb00)]:
   asm=subprocess.check_output(['objdump','-d','-Mintel','--start-address='+hex(start),'--stop-address='+hex(end),str(exe)],text=True)
   ranges.append({'name':name,'start':hex(start),'end':hex(end),'disassembly':asm,'call_lines':[line.strip() for line in asm.splitlines() if re.search(r'\bcall\s',line)]})
  result={'success':True,'source_executable_sha256':ORIGINAL_SHA,'original_manifest_verified_before_after':True,'scope':'Pinned static ranges and retained unresolved calls only. Review node/traversal and low16AX paths alongside runtime guard evidence; static inventory alone does not prove absence of pixel access or whole-function recovery.','ranges':ranges,'sources':{Path(__file__).relative_to(ROOT).as_posix():hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}}
 finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
 a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(result,indent=2)+'\n');print(a.report)
if __name__=='__main__':main()
