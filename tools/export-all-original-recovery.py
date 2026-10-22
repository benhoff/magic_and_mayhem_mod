#!/usr/bin/env python3
"""Read-only original decompilation and discovery-boundary re-export for all five areas."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def sha(path):return hashlib.sha256(evidence_bytes(path)).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--flow',type=Path,required=True);p.add_argument('--focused-roots',nargs='+',type=lambda v:int(v,0));a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output')
    out.mkdir(parents=True);(out/'tmp').mkdir()
    paths=['tools/export-all-original-recovery.py','tools/ghidra/ExportAllOriginalRecovery.java',
        'tools/ghidra/ExportBinaryInventory.java','tools/inventory-binary.py','tools/audit-file-apis.py','tools/decompile-game.py']
    sources={n:sha(ROOT/n) for n in paths};flow=json.loads(evidence_text(a.flow))
    if not flow['success'] or flow['original_sha256']!=PIN:raise ValueError('Unadmitted flow input')
    report=dict(schema=1,success=False,static_only=True,original_sha256=PIN,sources=sources,
        flow_input_sha256=sha(a.flow),scope=__doc__,semantic_closure_proved=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    roots=set(flow['control_flow']['seed_entries'])|{r['target'] for r in flow['control_flow']['potential_code_pointer_roots']}
    roots|={c['target'] for c in flow['control_flow']['direct_calls']}
    roots={a for a in roots if 0x545500<=a<0x54ad40}
    roots|={0x5002a0,0x4d8ab0,0x4bd960,0x4a2f90,0x57de00,0x57e1b0,0x581730,0x581ae0,0x4fd280,
        0x57e8e0,0x57fe10,0x580260,0x581200,0x580a90,0x5806f0,0x57e540,0x57dc20,0x57dc60,
        0x5947b2,0x59521a,0x595b47,0x59603e,0x595677,0x596490,0x5968a4,0x597086,0x596cb8,
        0x548860,0x547890,0x4e10e0,0x545990,0x4bec00,0x4e4270}
    callers={c['site'] for c in flow['kind8_rng_seed_callers']}
    callers|={x['address'] for x in flow['alternate_nearby_address_constructions']}
    callers|={c['site'] for c in flow['control_flow']['direct_calls']
        if 0x545500<=c['target']<0x54ad40 or 0x58a500<=c['target']<0x58f500}
    if a.focused_roots:
        roots=set(a.focused_roots);callers=set()
    listing=''.join('root '+format(a,'x')+'\n' for a in sorted(roots))+''.join('caller '+format(a,'x')+'\n' for a in sorted(callers))
    (out/'roots.txt').write_text(listing)
    before=False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
        executable=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(executable)!=PIN:raise ValueError('Original changed')
        headless=module('recovery_decompile','tools/decompile-game.py').find_headless(None)
        if headless is None:raise ValueError('No installed headless decompiler')
        env=dict(os.environ,TMPDIR=str(out/'tmp'),XDG_CONFIG_HOME=str(out/'config'),XDG_CACHE_HOME=str(out/'cache'),
            GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={out}/cache -Dapplication.tempdir={out}/tmp -Djava.io.tmpdir={out}/tmp')
        command=[headless,ROOT/'working/decompiled/nocd-nn0_667n/project','MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis',
            '-scriptPath',ROOT/'tools/ghidra','-postScript','ExportAllOriginalRecovery.java',out,out/'roots.txt','-max-cpu','2']
        if a.focused_roots:
            command.insert(command.index('-max-cpu'),'selected-only')
        with (out/'ghidra.log').open('x') as log:subprocess.run([str(x) for x in command],env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=1200)
        raw=json.loads(evidence_text(out/'discovery.json'));inventory=module('recovery_inventory','tools/inventory-binary.py').normalize(evidence_bytes(executable),raw)
        (out/'inventory.json').write_text(json.dumps(inventory,separators=(',',':'))+'\n')
        old=json.loads(evidence_text(ROOT/'research/runtime/coverage/binary-nocd.json'));oldfs={f['entry']:f for f in old['functions']};newfs={f['entry']:f for f in inventory['functions']}
        report.update(success=True,command=[str(x) for x in command],decompiled_functions=len(list((out/'c').glob('*.c'))),
            added_functions=[newfs[k] for k in sorted(newfs.keys()-oldfs.keys())],
            removed_functions=[oldfs[k] for k in sorted(oldfs.keys()-newfs.keys())],
            changed_functions=[dict(before=oldfs[k],after=newfs[k]) for k in sorted(newfs.keys()&oldfs.keys()) if newfs[k]!=oldfs[k]],
            artifacts={p.relative_to(out).as_posix():sha(p) for p in [out/'inventory.json',out/'discovery.json',out/'discovery-notes.txt',out/'global-references.txt',*sorted((out/'c').glob('*.c'))]})
    except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,error=str(error))
    finally:
        if before:
            try:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
