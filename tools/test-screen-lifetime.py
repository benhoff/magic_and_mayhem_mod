#!/usr/bin/env python3
"""Compare class11 screen lifetimes and map shared-tick tables to direct stores."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import shlex
import subprocess
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import evidence_digest, read_json, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RS.screen-class-ownership-static', 'RS.screen-class11-lifetime']
RANGES = [(0x4a8430, 0x4a847e), (0x4a8480, 0x4a848e),
          (0x4a84e0, 0x4a84fa), (0x4a8500, 0x4a8540),
          (0x4cab50, 0x4cab69), (0x4cab70, 0x4cabcc),
          (0x4bc790, 0x4bc7a0), (0x4bc980, 0x4bc99e), (0x4bcc00, 0x4bcc07)]
NOOPS = [0x4a8490, 0x4a84a0, 0x4a84b0, 0x4a84c0, 0x4a8540, 0x475c40]


def flow(pe, start, end):
    decoder = Decoder()
    todo, rows, tails = [start], {}, []
    try:
        while todo:
            at = todo.pop()
            if at in rows:
                continue
            if not start <= at < end:
                raise ValueError('Local flow escapes ' + hex(at))
            row = decoder.decode(pe.read(at, min(15, end-at)), at)
            rows[at] = row
            mnemonic, operand = row['mnemonic'], row['operand']
            if mnemonic.startswith('ret'):
                continue
            if mnemonic.startswith('j'):
                if not operand.startswith('0x'):
                    raise ValueError('Unresolved local branch')
                target = int(operand, 16)
                if at == 0x4a8489 and target == 0x4cab70:
                    tails.append(target)
                else:
                    todo.append(target)
            if mnemonic != 'jmp':
                todo.append(at + row['size'])
        occupied = {a for row in rows.values()
                    for a in range(row['address'], row['address']+row['size'])}
        if occupied != set(range(start, end)) or len(occupied) != sum(r['size'] for r in rows.values()):
            raise ValueError('Local byte census mismatch')
        return dict(entry=start, end=end, sha256=hashlib.sha256(pe.read(start, end-start)).hexdigest(),
                    instructions=sorted(rows.values(), key=lambda r: r['address']),
                    explicit_tail_suppliers=tails, closed_local_cfg=True)
    finally:
        decoder.close()


def ownership_tables(pe, disassembly, inventory):
    """Direct immediate stores are witnesses, not an exhaustive alias census."""
    tables = []
    for section in pe.sections:
        if section['executable']:
            continue
        raw = pe.read(section['address'], section['stored'])
        for offset in range(16, len(raw)-32, 4):
            if raw[offset:offset+4] != bytes.fromhex('d0955500'):
                continue
            address = section['address'] + offset - 16
            tables.append(dict(address=address,
                               slots=[pe.word(address+i*4) for i in range(12)],
                               direct_store_witnesses=[]))
    by_address = {t['address']: t for t in tables}
    for line in disassembly.splitlines():
        match = re.match(r'\s*([0-9a-f]+):\s+(?:[0-9a-f]{2}\s+)+\s*(mov\s+DWORD PTR .*,0x([0-9a-f]+))\s*$', line)
        if not match or int(match[3], 16) not in by_address:
            continue
        at = int(match[1], 16)
        functions = [f['entry'] for f in inventory['functions']
                     if any(a <= at < b for a, b in f['ranges'])]
        by_address[int(match[3], 16)]['direct_store_witnesses'].append(
            dict(address=at, instruction=match[2], inventory_functions=functions))
    if len(tables) != 22 or any(not t['direct_store_witnesses'] for t in tables):
        raise ValueError('Shared-tick table/store denominator changed')
    expected = [0x4a8490, 0x4a84a0, 0x4a84c0, 0x4a84b0, 0x5595d0,
                0x559390, 0x4732d0, 0x4a84d0, 0x4a84e0, 0x4a8500,
                0x4a8540, 0x475c40]
    if by_address[0x5c6454]['slots'] != expected:
        raise ValueError('Concrete class11 dispatch changed')
    return tables


def main():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/args.report).exists():
        parser.error('Require new owned output/report')
    out.mkdir(parents=True)
    (out/'tmp').mkdir()
    (out/'.retain').write_text('Original screen lifetime comparisons, exact source and compiler provenance.\n')
    reg = read_json(ROOT/'research/runtime/coverage/register.json', root=ROOT)
    behaviors = {b['id']: b for b in reg['behaviors']}
    builds = {b['id']: b for b in reg['builds']}
    sources = {name: evidence_digest(ROOT/name, root=ROOT)
               for name in sorted(set().union(*(required_sources(behaviors[i]) for i in IDS)))}
    report = dict(schema=1, success=False, original_sha256=BUILD, sources=sources,
                  claims=[dict(behavior=i, contract_sha256=behavior_contract(behaviors[i], builds)) for i in IDS],
                  scope='Complete local class11 construction, vector retirement and callback publication/disposal against authored allocation/free/virtual endpoints; static table/store witnesses. No whole-class registry, physical allocator, asynchronous alias closure, live integration or replacement.',
                  commands=[], live_replacement=False, allocator_and_virtual_endpoints_authored=True)

    def run(label, command):
        print(label, flush=True)
        result = subprocess.run(list(map(str, command)), cwd=ROOT,
                                env=os.environ | dict(TMPDIR=str(out/'tmp'),
                                    ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
                                    UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'),
                                capture_output=True, timeout=180)
        payload = result.stdout + b'\n' + result.stderr
        (out/(label+'.log')).write_bytes(payload)
        report['commands'].append(dict(label=label, exit=result.returncode,
                                       sha256=hashlib.sha256(payload).hexdigest()))
        if result.returncode:
            raise ValueError(label+': '+payload.decode(errors='replace')[-3500:])
        return result.stdout.decode()

    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        image = ROOT/'original/Arcane_Nocd/Chaos.exe'
        pe = PE32(image.read_bytes())
        if pe.sha256 != BUILD:
            raise ValueError('Unsupported original build')
        report['flow'] = [flow(pe, a, b) for a, b in RANGES]
        for at in NOOPS:
            if pe.read(at, 1) != b'\xc3':
                raise ValueError('No-op endpoint changed')
        if pe.read(0x4a8550, 5) != bytes.fromhex('33c0c20400'):
            raise ValueError('Callback result changed')
        report['original_entry_anchors'] = {hex(a): pe.read(a, 16).hex()
            for a in [*(a for a, b in RANGES), *NOOPS, 0x4a8550]}
        inventory_name = reg['builds'][0]['inventory']
        inventory = read_json(ROOT/inventory_name, root=ROOT)
        report['inventory_review'] = dict(path=inventory_name,
            sha256=evidence_digest(ROOT/inventory_name, root=ROOT), global_inventory_rewritten=False)
        report['screen_tables'] = ownership_tables(pe, run('table-store-disassembly',
            ['objdump', '-d', '-Mintel', '--wide', image]), inventory)
        for label, flags in [('original', ['-m32', '-O2', '-DMNM_ORIGINAL_REFERENCE']),
                             ('sanitized', ['-m32', '-O1', '-DMNM_ORIGINAL_REFERENCE', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']),
                             ('native64', ['-O1', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]:
            run(label+'-build', ['g++', '-std=c++17', '-mstackrealign', '-Wall', '-Wextra', '-Werror',
                '-fno-pie', '-no-pie', *flags, '-MMD', '-MF', out/(label+'.d'),
                ROOT/'tests/screen-lifetime-reference.cpp', '-o', out/label])
            for token in shlex.split((out/(label+'.d')).read_text().replace('\\\n', ' ').split(':', 1)[1]):
                path = Path(token).resolve()
                if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:
                    raise ValueError('Undeclared compiler source '+token)
            report[label] = json.loads(run(label, [out/label, *([image] if label != 'native64' else [])]))
        if report['original'] != report['sanitized'] or report['original'] != report['native64']:
            raise ValueError('Cross-build results differ')
        report['success'] = True
    except (OSError, ValueError, AssertionError, subprocess.SubprocessError) as exc:
        report['error'] = str(exc)
    finally:
        try:
            run('manifest-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        except (OSError, ValueError, subprocess.SubprocessError) as exc:
            report.update(success=False, manifest_error=str(exc))
        report['sources_stable'] = all(evidence_digest(ROOT/n, root=ROOT) == h for n, h in sources.items())
        report['success'] &= report['sources_stable']
        sha = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'], sha256=sha, error=report.get('error'))), flush=True)
    return int(not report['success'])


if __name__ == '__main__':
    raise SystemExit(main())
