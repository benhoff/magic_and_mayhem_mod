#!/usr/bin/env python3
"""Inventory pinned direct cursor resource references and setup/release instructions."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--original-root',type=Path,default=ROOT)
    p.add_argument('--report',required=True)
    a=p.parse_args()
    if (ROOT/a.report).exists():p.error('Require a new immutable report')
    original_root=a.original_root.resolve()
    manifest=original_root/'tools/original-manifest.sh'
    image=original_root/'original/Arcane_Nocd/Chaos.exe'
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in ['tools/inspect-cursor-resources.py','tools/coverage_evidence.py']}
    report=dict(schema=1,success=False,sources=sources,original_sha256=BUILD,
        claims=[dict(behavior='DRAW.cursor-resource-setup-static',stage='understanding')],
        scope='Linear direct-operand inventory and exact instruction windows only. Data mistaken for code and indirect/derived references require separate review. No full alias/writer closure, original execution or live replacement.')
    verified=False
    try:
        subprocess.run([str(manifest),'verify'],check=True,stdout=subprocess.DEVNULL);verified=True
        if hashlib.sha256(image.read_bytes()).hexdigest()!=BUILD:raise ValueError('Unsupported original executable')
        targets=[0x641fc8,0x642000,0x642c58,0x6e1ec0,0x6a49e0,0x6a4fe0,0x6f6b50]
        disassembly=subprocess.check_output(['objdump','-d','-Mintel','--wide',str(image)],text=True)
        references=[]
        for line in disassembly.splitlines():
            match=re.match(r'\s*([0-9a-f]+):\s+(?:[0-9a-f]{2}\s+)+\s*(.*)',line)
            if not match:continue
            operands=match.group(2)
            found=[target for target in targets if re.search(r'0x'+format(target,'x')+r'(?![0-9a-f])',operands)]
            if found:references.append(dict(address=int(match.group(1),16),instruction=operands,targets=found))
        report['references']=references
        report['windows']={}
        for label,begin,end in [('static-constructors-and-reload',0x4a2ec0,0x4a30b0),('setup',0x4a30b0,0x4a371b),('dispose',0x4a37d0,0x4a37fe),('wrapper-create-release',0x58ad90,0x58b5a0)]:
            text=subprocess.check_output(['objdump','-d','-Mintel','--wide','--start-address='+hex(begin),'--stop-address='+hex(end),str(image)],text=True)
            report['windows'][label]=dict(begin=begin,end=end,disassembly=text)
        report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        if verified:
            try:subprocess.run([str(manifest),'verify'],check=True,stdout=subprocess.DEVNULL);report['manifest_before_after']=True
            except (OSError,subprocess.SubprocessError) as exc:report.update(success=False,manifest_error=str(exc))
        report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],references=len(report.get('references',[])),error=report.get('error'))))
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
