#!/usr/bin/env python3
"""Retain byte-pinned format-setter and concrete screen publication frontiers.

Decoded calls are candidates. No dynamic class, allocation ownership, alias
closure, natural reachability or native equivalence follows from these bytes.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from coverage_evidence import Reader, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
TARGETS = {0x58d540: 'global RGB mask setter', 0x58b660: 'surface Lock and format provider',
           0x58aa90: 'windowed display setup', 0x58a9b0: 'exclusive display setup',
           0x4a6a60: 'Main concrete vptr publication caller', 0x4a6b00: 'Main concrete vptr reset caller',
           0x4a7030: 'Main resource release', 0x4739d0: 'Report concrete vptr publication caller',
           0x473b00: 'Report concrete vptr reset caller', 0x46a460: 'World entry'}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    args = p.parse_args()
    if (ROOT/args.report).exists() or not args.report.startswith('research/runtime/'):
        p.error('Require a new research/runtime result')
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        name = register['builds'][0]['inventory']
        inventory = reader.json(name)
        sources = {n:reader.digest(n) for n in [name, 'tools/research-rendering-reference-contracts.py',
            'tools/original_x86.py', 'tools/coverage_evidence.py', 'tools/original-manifest.sh',
            'research/original-files.tsv', 'runtime/native-draw/surfaces.h', 'runtime/scene/canvas_producers.c',
            'reconstruction/rendering/screen_services.hpp', 'research/runtime/screen-resource-ownership-20261025.md']}
    report = dict(schema=1, kind='original_rendering_reference_frontier', success=False, sources=sources,
                  original_code_executed=False, native_equivalence=False, natural_reachability_proved=False,
                  physical_driver_equivalence=False, status_promotions=[], scope=__doc__)
    before = False
    decoder = None
    try:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)
        before = True
        image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if image.sha256 != BUILD or inventory['source_sha256'] != BUILD:
            raise ValueError('Build mismatch')
        decoder = Decoder()
        functions = {f['entry']:f for f in inventory['functions']}
        rows = []
        for entry, role in TARGETS.items():
            if entry not in functions:
                rows.append(dict(entry=entry, role=role, discovered_function=False, decoded=False))
                continue
            function = functions[entry]
            instructions = []
            bodies = []
            for start, end in function['ranges']:
                bodies.append(image.read(start, end-start))
                at = start
                while at < end:
                    row = decoder.decode(image.read(at, min(15, end-at)), at)
                    instructions.append(row)
                    at += row['size']
                if at != end:
                    raise ValueError('Instruction crosses discovered extent')
            digest = hashlib.sha256(b''.join(bodies)).hexdigest()
            if digest != function['sha256']:
                raise ValueError('Function byte hash differs')
            callers = [f for f in inventory['flows'] if entry in f['targets']]
            callees = [f for f in inventory['flows'] if f['owner'] == entry]
            rows.append(dict(entry=entry, role=role, discovered_function=True, decoded=True,
                ranges=function['ranges'], sha256=digest, instructions=instructions,
                retained_incoming_candidates=callers, retained_outgoing_candidates=callees,
                complete_semantics_recovered=False))
        report.update(original_sha256=BUILD, functions=rows, success=True)
    except Exception as error:
        report.update(success=False, error=str(error))
    finally:
        if decoder:
            decoder.close()
        if before:
            check = subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'])
            report['original_manifest_verified_before_after'] = check.returncode == 0
            report['success'] &= check.returncode == 0
        with Reader(ROOT) as reader:
            report['sources_stable'] = all(reader.digest(n) == h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        digest = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'], sha256=digest, error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
