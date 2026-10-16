#!/usr/bin/env python3
"""Read-only owner risk tolerance data and cross-reference discovery."""
import argparse
import hashlib
from importlib.machinery import SourceFileLoader
import json
import os
from pathlib import Path
import subprocess
ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--project',type=Path,required=True)
    p.add_argument('--headless',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    manifest=[ROOT/'tools/original-manifest.sh','verify']
    subprocess.run(manifest,check=True)
    try:
        pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(pe)!=PIN:raise ValueError('Unsupported original')
        env={**os.environ,'XDG_CONFIG_HOME':str(out/'config'),'XDG_CACHE_HOME':str(out/'cache'),
             'GHIDRA_HEADLESS_JAVA_OPTIONS':f'-Dapplication.cachedir={out}/cache -Dapplication.tempdir={out}/tmp'}
        command=[str(a.headless),str(a.project),'MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis',
                 '-scriptPath',str(ROOT/'tools/ghidra'),'-postScript','ExportOwnerRisk.java',str(out/'risk'),'-max-cpu','2']
        with (out/'ghidra.log').open('w') as log:subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600)
        sources={p:sha(ROOT/p) for p in ['tools/export-owner-risk.py','tools/ghidra/ExportOwnerRisk.java']}
        report=dict(success=True,original_sha256=PIN,sources=sources,scope='Read-only static owner risk data/write-reference discovery; no execution validation.',artifacts={str(p.relative_to(out)):sha(p)for p in (out/'risk').iterdir()})
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        if sha(pe)!=PIN:raise ValueError('Original changed')
        print(report['success'])
    finally:subprocess.run(manifest,check=True)
if __name__=='__main__':main()
