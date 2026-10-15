#!/usr/bin/env python3
"""Re-export pinned read-only discovery after identifying the navigation comparator."""
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
                 '-scriptPath',str(ROOT/'tools/ghidra'),'-postScript','ExportRegionalGraphInventory.java',str(out/'discovery.json'),'-max-cpu','2']
        with (out/'ghidra.log').open('w') as log:subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600)
        module=SourceFileLoader('regional_inventory',str(ROOT/'tools/inventory-binary.py')).load_module()
        inventory=module.normalize(pe.read_bytes(),json.loads((out/'discovery.json').read_text()))
        sources={path:sha(ROOT/path) for path in ['tools/export-regional-graph-inventory.py','tools/ghidra/ExportRegionalGraphInventory.java','tools/ghidra/ExportBinaryInventory.java','tools/inventory-binary.py','tools/audit-file-apis.py']}
        inventory['provenance']={'command':command,'discovery_sha256':sha(out/'discovery.json'),'sources':sources,'scope':'Read-only whole-image discovery re-export; comparator53d6f0 explicitly seeded. No behavior validation follows from discovery.'}
        (out/'inventory.json').write_text(json.dumps(inventory,separators=(',',':'))+'\n')
        old=json.loads((ROOT/'research/runtime/coverage/binary-nocd.json').read_text());before={f['entry']:f for f in old['functions']};after={f['entry']:f for f in inventory['functions']}
        report={'success':True,'original_sha256':PIN,'sources':sources,'inventory_sha256':sha(out/'inventory.json'),'functions_before':len(before),'functions_after':len(after),
                'added':[after[a] for a in sorted(after.keys()-before.keys())], 'removed':[before[a] for a in sorted(before.keys()-after.keys())],
                'changed':[{'before':before[a],'after':after[a]} for a in sorted(before.keys()&after.keys()) if before[a]!=after[a]],
                'scope':'Discovery boundaries only; unknown functions, unassigned ranges and indirect-flow gaps retained. Review required before central inventory publication.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        if sha(pe)!=PIN:raise ValueError('Original changed')
        print(json.dumps({k:report[k] for k in ['success','functions_before','functions_after']}))
    finally:subprocess.run(manifest,check=True)
if __name__=='__main__':main()
