#!/usr/bin/env python3
"""Execution-input and original-reference research, without acceptance promotion.

Generated reports and retained inputs go directly to SQLite. Build/session output
is disposable RAM staging; the caller must retain referenced unique run data.
"""
import argparse
from collections import Counter
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

from coverage_claims import behavior_contract
from coverage_evidence import Reader, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BEHAVIOR = 'NP.rendering-execution-research'
FOLDERS = ('apps', 'assets', 'audio', 'compat', 'renderer', 'reconstruction', 'runtime', 'protocols', 'tools', 'tests')
EXTENSIONS = {'.py', '.sh', '.c', '.cpp', '.h', '.hpp', '.inc', '.inl', '.S', '.cmake'}
GLOBALS = {'alternate_base': (0x6e1f68, 4), 'alternate_source': (0x6e1f6c, 4),
           'red_mask': (0x6e1f7c, 2), 'green_mask': (0x6e1f7e, 2), 'blue_mask': (0x6e1f80, 2),
           'screen_depth': (0x6f349c, 4), 'screen_stack': (0x6f34a0, 64), 'screen_current': (0x6f34e0, 4)}


def sha(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def sources():
    paths = {p.relative_to(ROOT).as_posix() for folder in FOLDERS for p in (ROOT/folder).rglob('*')
             if p.is_file() and (p.suffix in EXTENSIONS or p.name == 'CMakeLists.txt')}
    paths.update(('run-native.sh', 'run-x86.sh', 'research/original-files.tsv'))
    return {n: sha(ROOT/n) for n in sorted(paths)}


def load(name):
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), ROOT/'tools'/name)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def reference_access(row, address):
    """Syntactic candidate only; no alias, runtime value or reachability claim."""
    operands = [p.strip() for p in row['operand'].lower().split(',')]
    token = r'0x' + format(address, 'x') + r'(?![0-9a-f])'
    matching = [i for i, operand in enumerate(operands) if re.search(token, operand)]
    if not matching:
        return None
    mnemonic = row['mnemonic'].lower()
    if mnemonic == 'lea':
        return 'address_candidate'
    memory = [i for i in matching if '[' in operands[i]]
    if not memory:
        return 'literal_candidate'
    if any(not re.fullmatch(r'0x[0-9a-f]+', re.search(r'\[([^]]+)\]', operands[i])[1].replace(' ', '')) for i in memory):
        return 'indexed_address_candidate'
    if mnemonic in {'cmp', 'test', 'push', 'call', 'jmp'}:
        return 'read_candidate'
    if mnemonic.startswith(('mov', 'stos')) or mnemonic in {'add', 'sub', 'and', 'or', 'xor', 'inc', 'dec', 'xchg', 'xadd', 'cmpxchg', 'not', 'neg'}:
        if 0 in memory or mnemonic in {'xchg', 'xadd'}:
            return 'write_candidate'
        return 'read_candidate'
    return 'access_unclassified'


def retain(report_name, label, path):
    name = str(Path(report_name).with_suffix('')) + '-' + label + '.bin'
    data = Path(path).read_bytes()
    digest = write_report(ROOT, name, data)
    return dict(record=name, sha256=digest, bytes=len(data))


def inputs(report, args):
    source = args.source_game.resolve()
    target = ROOT/'working/game-nocd'
    if target.exists() or source == target or sha(source/'Chaos.exe') != BUILD:
        raise ValueError('Require new private input copy and pinned source executable')
    before = {p.relative_to(source).as_posix(): dict(sha256=sha(p), bytes=p.stat().st_size)
              for p in sorted(source.rglob('*')) if p.is_file()}
    shutil.copytree(source, target)
    copied = {p.relative_to(target).as_posix(): dict(sha256=sha(p), bytes=p.stat().st_size)
              for p in sorted(target.rglob('*')) if p.is_file()}
    if before != copied:
        raise ValueError('Private input clone differs')
    report.update(source_installation=str(source), private_installation=str(target),
                  installation_inputs=copied, file_count=len(copied), bytes=sum(v['bytes'] for v in copied.values()),
                  installation_clone_matches=True, executable_sha256=BUILD,
                  maps=[n for n in copied if n.lower().endswith('.map')],
                  saves=[n for n in copied if n.lower().startswith('save/')],
                  input_selection=dict(pilot='public Quick Battle list row 0; obtain actual map identity from its acknowledgement',
                      second_map='public list row 2; selection is not map identity',
                      campaign='fresh Apprentice (difficulty 1), followed by exact saved-file identity',
                      results='both actual victory and defeat required; Escape/abandon is not victory',
                      duration='finite bounded research runs; no arbitrary duration support promise'),
                  retained_inputs=[])
    decoder = load('decode-cfg.py')
    report['map_metadata'] = []
    for name in report['maps']:
        outer, payload = decoder.decode((target/name).read_bytes())
        if len(payload) < 76:
            raise ValueError('Truncated selected installation map: '+name)
        version, width, height, layers, plane, total = struct.unpack_from('<6I', payload)
        if version != 6 or plane != width*height or total != plane*layers or len(payload) != 76+12*total:
            raise ValueError('Unexpected map structure: '+name)
        report['map_metadata'].append(dict(path=name, raw_sha256=copied[name]['sha256'],
            decoded_sha256=hashlib.sha256(payload).hexdigest(), version=version, width=width,
            height=height, layers=layers, cells=total, container_mode=outer['mode']))
    for name in copied:
        if name.lower().startswith(('cfg/', 'save/')) and copied[name]['bytes'] < 2*1024*1024:
            report['retained_inputs'].append(dict(path=name, **retain(args.report, 'input-'+hashlib.sha256(name.encode()).hexdigest()[:16], target/name)))


def original(report, args):
    scanner = load('inspect-five-pixel-callers.py')
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        inventory_name = register['builds'][0]['inventory']
        inventory = reader.json(inventory_name)
        report['sources'][inventory_name] = reader.digest(inventory_name)
    image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
    if image.sha256 != BUILD or inventory['source_sha256'] != BUILD:
        raise ValueError('Unsupported executable/inventory')
    roots = {image.entry, *(f['entry'] for f in inventory['functions']),
             *(f['site'] for f in inventory['flows']),
             *(t for f in inventory['flows'] for t in f['targets'] if isinstance(t, int)),
             *(start for start, end in scanner.WINDOWS.values())}
    decoder = Decoder()
    try:
        rows, graph = scanner.decode_graph(image, decoder, roots)
        references = {}
        for label, (base, size) in GLOBALS.items():
            candidates = []
            for row in rows:
                literals = {int(x, 16) for x in re.findall(r'0x([0-9a-f]+)', row['operand'])}
                for address in sorted(a for a in literals if base <= a < base+size):
                    candidates.append(dict(row, referenced_address=address, access=reference_access(row, address)))
            references[label] = dict(base=base, size=size, candidates=candidates,
                                     access_counts=dict(Counter(r['access'] for r in candidates)))
        # Candidate vtables: pointer data plus executable targets, never a type or lifetime inference.
        tables = []
        for section in image.sections:
            if section['executable']:
                continue
            for address in range(section['address'], section['address']+section['stored']-56+1, 4):
                if image.word(address+0x10) != 0x5595d0:
                    continue
                slots = [image.word(address+i*4) for i in range(14)]
                writers = [r for r in rows if re.search(r'0x'+format(address, 'x')+r'(?![0-9a-f])', r['operand'])]
                tables.append(dict(address=address, slots=slots, executable_slots=sum(image.text(v) for v in slots),
                                   references=writers, type_recovered=False, actual_lifetime_recovered=False))
        def owners(row):
            row['discovered_function_owners'] = [f['entry'] for f in inventory['functions']
                if any(start <= row['address'] < end for start, end in f['ranges'])]
        for ref in references.values():
            for row in ref['candidates']:
                owners(row)
        for table in tables:
            for row in table['references']:
                owners(row)
        report.update(original_sha256=BUILD, decoder_version=decoder.version, graph=graph,
                      global_references=references, shared_tick_vtable_candidates=tables,
                      dispatch_witnesses=scanner.dispatches(image, decoder),
                      global_alias_closure=False, natural_reachability_proved=False,
                      scope='Fresh original-byte absolute/literal reference census for alternate pointers, RGB masks and screen stack; candidate vtable publications. Explicit references are not all aliases, executed routing or lifetime recovery.')
    finally:
        decoder.close()


def build(report, args):
    directory = ROOT/'working/build/qt-shell'
    commands = [['cmake', '-S', str(ROOT/'apps/qt-shell'), '-B', str(directory),
                 '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF'],
                ['cmake', '--build', str(directory), '--target', 'mnm-qt-shell', '-j4']]
    report['commands'] = []
    for i, command in enumerate(commands):
        print('build', i, flush=True)
        result = subprocess.run(command, capture_output=True, cwd=ROOT, timeout=900)
        name = str(Path(args.report).with_suffix(''))+'-command-'+str(i)+'.log'
        digest = write_report(ROOT, name, result.stdout+b'\n'+result.stderr)
        report['commands'].append(dict(command=command, returncode=result.returncode, log=name, sha256=digest))
        if result.returncode:
            raise ValueError('Build failed: '+result.stderr.decode(errors='replace')[-1500:])
    campaign = load('test-native-campaign.py')
    compiled = campaign.linked_compiler_sources(directory, ROOT)
    if compiled-set(report['sources']):
        raise ValueError('Undeclared actual compiler input')
    binary = directory/'mnm-qt-shell'
    report.update(binary=retain(args.report, 'mnm-qt-shell', binary),
                  compiled_sources={n:report['sources'][n] for n in sorted(compiled)},
                  build_type='Release', player_executed=False)


def declaration(report, args):
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
    behavior = next(b for b in register['behaviors'] if b['id'] == BEHAVIOR)
    claims = [dict(behavior=BEHAVIOR, contract_sha256=behavior_contract(behavior, {b['id']: b for b in register['builds']}))]
    data = (json.dumps(dict(sources=report['sources'], claims=claims), indent=2)+'\n').encode()
    target = ROOT/'working/session-declaration.json'
    if target.exists():
        raise ValueError('Refusing declaration overwrite')
    target.write_bytes(data)  # Required bounded interchange input to existing campaign producer.
    report.update(claims=claims, declaration=retain(args.report, 'declaration', target),
                  consumer='tools/test-native-owned-campaign.py --claims working/session-declaration.json')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('phase', choices=('inputs', 'original', 'build', 'declaration'))
    parser.add_argument('--source-game', type=Path)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    if (ROOT/args.report).exists() or not args.report.startswith('research/runtime/'):
        parser.error('Require a new research/runtime report')
    if args.phase == 'inputs' and args.source_game is None:
        parser.error('Input inventory requires --source-game')
    report = dict(schema=1, kind='rendering_execution_research', phase=args.phase, success=False,
                  sources=sources(), engine_acceptance_assessed=False, complete_replacement=False,
                  original_game_executed=False, status_promotions=[])
    consuming = args.phase in {'inputs', 'original'}
    verified = False
    try:
        if consuming:
            subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)
            verified = True
        globals()[args.phase](report, args)
        report['success'] = True
    except Exception as error:
        report.update(success=False, error=str(error))
    finally:
        if verified:
            result = subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'])
            report['original_manifest_verified_before_after'] = result.returncode == 0
            report['success'] &= result.returncode == 0
        with Reader(ROOT) as reader:
            report['sources_stable'] = all(reader.digest(n) == h for n, h in report['sources'].items())
        report['success'] &= report['sources_stable']
        digest = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'], phase=args.phase, sha256=digest, error=report.get('error'))), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
