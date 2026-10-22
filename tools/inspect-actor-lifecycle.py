#!/usr/bin/env python3
"""Read-only instruction and provider-ABI evidence for pinned actor lifecycles."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ENTRIES = [0x506290, 0x5065e0, 0x506d30, 0x508b00, 0x509230, 0x5093e0,
           0x51f5e0, 0x52a640, 0x52a670, 0x52a710, 0x505db0, 0x505f70,
           0x473100, 0x52a5c0, 0x534520, 0x411e50, 0x40e8a0, 0x40e290, 0x414890, 0x506080, 0x416530]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require new retained output under working/tests')
    out.mkdir(parents=True)
    reg = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    inventory_path = ROOT/reg['builds'][0]['inventory']
    inv = json.loads(inventory_path.read_text())
    functions = {f['entry']: f for f in inv['functions']}
    sources = {p: sha(ROOT/p) for p in ['tools/inspect-actor-lifecycle.py', 'tools/original_x86.py']}
    report = dict(schema=1, success=False, static_only=True, sources=sources,
                  original_sha256=PIN, inputs={str(inventory_path.relative_to(ROOT)): sha(inventory_path)},
                  semantic_or_indirect_closure_proved=False)
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')
    verified = False
    decoder = None
    try:
        with (out/'original-before.log').open('x') as log:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], stdout=log, stderr=subprocess.STDOUT, check=True)
        verified = True
        image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if image.sha256 != PIN:
            raise ValueError('Unreviewed original build')
        decoder = Decoder()
        # This separately documented compiler entry is not necessarily a
        # discovered function start. Preserve the inventory; do not invent a
        # semantic function mapping from its absence.
        if 0x416530 not in functions:
            start,end=0x416530,0x4165b0
            functions[start]={'entry':start,'ranges':[[start,end]],
                'sha256':hashlib.sha256(image.read(start,end-start)).hexdigest(),
                'documented_body_only':True,'document':'research/runtime/entity-lifetimes.md',
                'discovered_owners':[f['entry'] for f in inv['functions']
                    if any(a<=start<b for a,b in f['ranges'])]}
        rows, helpers = {}, set()
        def decode(entry):
            f = functions[entry]
            if f.get('documented_body_only'):
                pending=[entry];seen={};first,end=f['ranges'][0]
                while pending:
                    at=pending.pop()
                    while at not in seen:
                        if not first<=at<end:
                            raise ValueError('Documented entry branches outside reviewed extent')
                        i=decoder.decode(image.read(at,min(15,end-at)),at);seen[at]=i
                        if i['mnemonic'].startswith('ret'):break
                        if i['mnemonic'].startswith('j'):
                            if not i['operand'].startswith('0x'):
                                raise ValueError('Unresolved documented-entry branch')
                            pending.append(int(i['operand'],16))
                            if i['mnemonic']=='jmp':break
                        at+=i['size']
                result=[seen[k] for k in sorted(seen)]
                f['ranges']=[[result[0]['address'],result[-1]['address']+result[-1]['size']]]
                f['sha256']=hashlib.sha256(image.read(f['ranges'][0][0],f['ranges'][0][1]-f['ranges'][0][0])).hexdigest()
                return result
            result = []
            for first, end in f['ranges']:
                at = first
                while at < end:
                    i = decoder.decode(image.read(at, min(15, end-at)), at)
                    result.append(i)
                    at += i['size']
            return result
        for entry in ENTRIES:
            rows[hex(entry)] = decode(entry)
            for i in rows[hex(entry)]:
                if i['mnemonic'] == 'call' and i['operand'].startswith('0x'):
                    helpers.add(int(i['operand'], 16))
        abi = {}
        for entry in sorted(helpers-set(ENTRIES)):
            if entry not in functions:
                abi[hex(entry)] = {'unresolved': True}
                continue
            code = decode(entry)
            returns = sorted({int(i['operand'], 0) if i['operand'] else 0
                              for i in code if i['mnemonic'] in {'ret', 'retf'}})
            abi[hex(entry)] = {'entry_bytes': image.read(entry, 8).hex(),
                              'return_pop_bytes': returns, 'ranges': functions[entry]['ranges']}
        (out/'instructions.json').write_text(json.dumps(rows, indent=2)+'\n')
        (out/'instructions.txt').write_text('\n'.join(
            f"{i['address']:08x} {i['bytes']:30} {i['mnemonic']:8} {i['operand']}"
            for code in rows.values() for i in code)+'\n')
        (out/'provider-abi.json').write_text(json.dumps(abi, indent=2)+'\n')
        report.update(success=True, decoder_version=decoder.version,
            entries={hex(e): functions[e] for e in ENTRIES},
            artifacts={name: sha(out/name) for name in ['instructions.json', 'instructions.txt', 'provider-abi.json']})
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if decoder:
            decoder.close()
        if verified:
            with (out/'original-after.log').open('x') as log:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], stdout=log, stderr=subprocess.STDOUT, check=True)
            report['original_manifest_verified_before_after'] = True
        report['sources_stable'] = all(sha(ROOT/p)==h for p, h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(out/'report.json')
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
