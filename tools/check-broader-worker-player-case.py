#!/usr/bin/env python3
"""Admit a current failed/fresh public-player pair with exact native provenance."""
import argparse
import ast
import hashlib
import json
import math
import re
from pathlib import Path

from broader_player_acceptance import PREFIX, checked, native_phase
from coverage_evidence import Reader, write_report
from fault_lifetime_gate import Join
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def python_sources():
    pending = [Path(__file__), ROOT/'tools/broader_player_acceptance.py', ROOT/'tools/fault_lifetime_gate.py']
    seen = set()
    while pending:
        path = pending.pop().resolve()
        name = path.relative_to(ROOT).as_posix()
        if name in seen:
            continue
        seen.add(name)
        for node in ast.walk(ast.parse(path.read_bytes())):
            imports = ([v.name for v in node.names] if isinstance(node, ast.Import) else
                       [node.module] if isinstance(node, ast.ImportFrom) and node.module else [])
            for value in imports:
                for base in (ROOT/'tools', ROOT/'protocols/python'):
                    target = base/Path(*value.split('.')).with_suffix('.py')
                    if target.is_file():
                        pending.append(target)
            if isinstance(node, ast.Call) and node.args and isinstance(node.args[0], ast.Constant):
                fn = node.func
                if ((isinstance(fn, ast.Name) and fn.id == 'module') or
                    (isinstance(fn, ast.Attribute) and fn.attr == 'module')) and isinstance(node.args[0].value, str):
                    target = ROOT/'tools'/(node.args[0].value+'.py')
                    if len(node.args) > 1 and isinstance(node.args[1], ast.Constant) and isinstance(node.args[1].value, str):
                        target = ROOT/node.args[1].value
                    if target.is_file():
                        pending.append(target)
    return seen


def epoch_progress(epoch, comparison):
    resident = epoch['resident']
    profile = resident['stage_profile']
    samples = profile['samples']
    require(profile['samples_dropped'] == 0 and len(samples) == int(resident['completed'])
            and [int(v['generation']) for v in samples] == list(range(1, len(samples)+1)),
            'Incomplete epoch timing/source history')
    fields = ('producer_wait_ack_ms', 'native_consume_ms')
    require(all(k in row and math.isfinite(float(row[k])) and float(row[k]) >= 0
                for row in samples for k in fields), 'Missing or invalid epoch deadline measurement')
    maximum = {k: max(float(row[k]) for row in samples) for k in fields}
    require(all(v < 120000 for v in maximum.values()), 'Existing source acknowledgement deadline exceeded')
    capture = profile['capture']
    require(capture['peak_queue_bytes'] <= capture['queue_limit_bytes'] == 256*1024**2
            and capture['queue_bytes'] == 0 and capture['file_bytes'] <= 2*1024**3,
            'Epoch capture resource bound exceeded')
    worlds = comparison['world_domain']['worlds']
    require(worlds and len(worlds) <= len(samples) and comparison['comparison']['world_presentations'] > 0,
            'Missing independently presented epoch progress')
    return dict(verified=True, source_generations=len(samples), compared_generations=len(worlds),
                maximum_observed_ms=maximum, source_ack_deadline_ms=120000,
                capture_queue_limit_bytes=256*1024**2, source_file_limit_bytes=2*1024**3,
                fps_diagnostic=resident.get('engine_fps'), fixed_fps_threshold=None,
                scope='Finite expected-failure prefix or fresh positive epoch; global sustained-World proof remains a separate normal-journey obligation.')



def input_binding(reader, acquired):
    phase = checked(reader, acquired['prospective_plan'])
    global_plan = checked(reader, phase['broader_plan'])
    fixture = checked(reader, global_plan['owned_input'])
    initial = fixture['input_fingerprints']
    actual = acquired['installation_inputs']
    allowed = set(fixture['permitted_mutations'])
    require(actual.keys() == initial.keys() and all(actual[n] == v for n, v in initial.items()
            if n not in allowed), 'Undeclared game input change')
    wanted = b'TRUE' if acquired['configuration']['actual_size'] == [800, 600] else b'FALSE'
    join = Join(reader)
    configs = []
    for epoch in acquired['proof']['epochs']:
        path = join.at(acquired, epoch['root'], 'game/CFG/prefs.cfg')
        raw = join.raw(acquired, path)
        require(re.findall(rb'(?mi)^IsHighRes\s*=\s*(TRUE|FALSE)', raw) == [wanted],
                'Staged original preference differs from the actual World domain')
        configs.append(dict(path=path, sha256=hashlib.sha256(raw).hexdigest()))
    return dict(verified=True, fixture=global_plan['owned_input'],
                exact_input_count=len(actual), permitted_source_mutations=sorted(allowed),
                actual_staged_preferences=configs)


def assess(reader, acquisition_name, comparison_name):
    ref = lambda n: dict(record=n, sha256=reader.digest(n))
    pair = dict(acquisition=ref(acquisition_name), comparison=ref(comparison_name))
    a, c = checked(reader, pair['acquisition']), checked(reader, pair['comparison'])
    require(c['acquisition'] == pair['acquisition'] and c['sources_stable'] is True
            and c['comparison_processes_stopped'] is True, 'Incomplete independent comparison ownership')
    built = reader.json('research/runtime/broader-player-acceptance-results-20261110/build-current-04.json')
    phase = native_phase(ROOT, reader, a, built['player']['sha256'])
    require(phase['current_native_source_candidate'] is True, 'Historical or incomplete native execution phase')
    require(a['installation_inputs']['Chaos.exe']['sha256'] ==
            '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168', 'Wrong original executable')
    profile = 'player-software' if a['configuration']['software_flag'] else 'player-default'
    leaf = ('operation.surface-loss-restore' if a['kind'] == 'public_native_surface_loss_acquisition'
            else 'operation.transition-recovery')
    inputs = input_binding(reader, a)
    edge_policy = None
    if 'native_edge_policy' in c:
        from broader_policy_acceptance import assess as assess_edge_policy
        edge_policy = assess_edge_policy(reader, c)
    lifetime = Join(reader).case(pair, profile, leaf)
    progress = [epoch_progress(e, p) for e, p in zip(a['proof']['epochs'], c['epochs'])]
    require(len(progress) == 2 and a['original_game_runs'] == 2, 'Missing failed/fresh epoch')
    sources = dict(a['sources'])
    for n, h in c['sources'].items():
        require(n not in sources or sources[n] == h, 'Mixed acquisition/oracle source phases')
        sources[n] = h
    sources.update({n: reader.digest(n) for n in sorted(python_sources())})
    if edge_policy is not None:
        sources.update(edge_policy['oracle_sources'])
    require(all(reader.digest(n) == h for n, h in sources.items()), 'Current case analysis source drift')
    return dict(acquisition=pair['acquisition'], comparison=pair['comparison'], sources=sources,
                native_phase=phase, input_binding=inputs, lifetime=lifetime, epoch_progress=progress,
                profile=profile, size=a['configuration']['actual_size'],
                declared_fault=a['configuration']['declared_fault'],
                independent_pixels=c['comparison'], native_edge_policy=edge_policy, original_game_runs=2,
                existing_case_deadline_ms=600000, expected_failed_epoch_normal_credit=False,
                physical_driver_reset_proved=False, current_whole_player_proved=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('acquisition', 'comparison', 'report'):
        parser.add_argument('--'+name, required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh immutable validation required')
    result = dict(schema=1, kind='broader_player_current_fault_case_validation', success=False,
                  current_whole_player_proved=False, supported_player_accepted=False,
                  original_recovery_complete=False)
    with Reader(ROOT, memoize=True) as reader:
        try:
            result.update(assess(reader, args.acquisition, args.comparison))
            result.update(success=True, sources_stable=True)
        except (ValueError, KeyError, OSError, TypeError) as error:
            result['error'] = str(error)
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], sha256=sha, error=result.get('error'))), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
