#!/usr/bin/env python3
"""Attribute a capped retained COPY suffix and check bounded offline GL replay."""
import argparse
from collections import OrderedDict
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import numpy as np
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from copy_tail_proof import (MAGIC, attribute, check_replay, command_history, digest,
                             plane_bytes, summary, write_case)
from native_copy_routes import decode, validate
from rendering_pilot import History, inflate, require

ROOT = Path(__file__).resolve().parents[1]
ACQUIRED = 'research/runtime/rendering-normal-results-20261031/storage-worker-software-low09.json'
COMPARED = 'research/runtime/rendering-normal-results-20261031/storage-worker-software-low09-comparison.json'
BEHAVIOR = 'TOOL.copy-tail-offline-proof'
SCENARIO = 'SC.copy-tail-offline'


class Worlds:
    """Two verified oracle planes cached at once; never native player pixels."""
    def __init__(self, reader, compared):
        self.reader, self.cache, self.consumed = reader, OrderedDict(), {}
        self.artifacts = {a['path']: a for a in compared['artifacts']}
        self.rows = None
        index = self.artifacts['reference/worlds.json']
        require(reader.digest(index['record']) == index['sha256'], 'Changed World index')
        data = json.loads(reader.read_bytes(index['record']))
        require(data['success'] and data['native_player_pixels_read'] is False
                and data['original_destination_capture_read'] is False, 'Unowned oracle inputs')
        self.rows = {int(x['generation']): x for x in data['worlds']}
        require(len(self.rows) == len(data['worlds']), 'Duplicate original World generation')

    def __contains__(self, generation):return generation in self.rows

    def __getitem__(self, generation):
        if generation in self.cache:
            self.cache.move_to_end(generation); return self.cache[generation]
        row = self.rows[generation]; ref = self.artifacts['reference/'+row['path']]
        raw = self.reader.read_bytes(ref['record'])
        require(digest(raw) == ref['sha256'], 'Changed original World plane')
        body = inflate(raw)
        require(len(body) == row['width']*row['height']*2, 'Original World plane extent')
        plane = np.frombuffer(body, dtype='<u2').astype('uint32').reshape(row['height'], row['width'])
        self.consumed[generation] = dict(record=ref['record'], sha256=ref['sha256'])
        self.cache[generation] = plane
        while len(self.cache) > 2:self.cache.popitem(last=False)
        return plane


def native(binary, fixture, extra=()):
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4')
    run = subprocess.run(['xvfb-run', '-a', str(binary), str(fixture), *extra],
                         capture_output=True, text=True, env=env, timeout=240)
    require(run.returncode == 0, 'Offline native replay failed: '+run.stderr[-2000:])
    rows = [json.loads(line) for line in run.stdout.splitlines()]
    result = json.loads(run.stderr.splitlines()[-1])
    require(result['success'] and result['remaining_surfaces'] == 0, 'Offline GL retirement failed')
    return rows, result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith('research/runtime/copy-tail-results-20261102/')
            and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'New owned immutable report required')
    binary = args.binary.resolve(); require(binary.is_file(), 'Missing offline GL executable')
    reg = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behavior = next(b for b in reg['behaviors'] if b['id'] == BEHAVIOR)
    scenario = next(s for s in reg['scenarios'] if s['id'] == SCENARIO)
    sources = required_sources(behavior)
    sources.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'renderer').glob('*.cpp'))
    sources.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'renderer').glob('*.hpp'))
    report = dict(schema=1, kind='retained_copy_tail_offline_proof', success=False,
                  original_game_executed=False, new_game_acquisitions=0,
                  original_live_outputs_recovered=0, whole_operations_accepted=0,
                  player_accepted=False, original_recovery_accepted=False,
                  claims=[dict(behavior=BEHAVIOR,
                      contract_sha256=behavior_contract(behavior, {b['id']:b for b in reg['builds']}),
                      scenarios={SCENARIO:scenario_contract(scenario)})])
    out = ROOT/'working/copy-tail-proof'/Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False); (out/'.retain').write_text('Bounded offline COPY-tail analysis; preserve exact inputs/failures.\n')
    try:
        with Reader(ROOT) as reader:
            report['sources'] = {n:reader.digest(n) for n in sorted(sources)}
            report['binary'] = dict(sha256=digest(binary.read_bytes()), path=str(binary))
            acquired, compared = reader.json(ACQUIRED), reader.json(COMPARED)
            require(acquired['success'] and compared['success'] and acquired['sources_stable']
                    and acquired['original_manifest_verified_before_after'], 'Failed retained execution')
            require(compared['acquisition'] == dict(record=ACQUIRED, sha256=reader.digest(ACQUIRED)), 'Unbound comparison')
            require(all(reader.digest(n) == h for n,h in acquired['sources'].items()), 'Retained acquisition sources changed')
            report['inputs'] = [dict(record=n, sha256=reader.digest(n)) for n in (ACQUIRED, COMPARED)]
            selected = {}
            def artifact(suffix):
                matches = [x for x in acquired['artifacts'] if x['path'].endswith(suffix)]
                require(len(matches) == 1, 'Ambiguous retained input: '+suffix)
                ref = matches[0]; body = reader.read_bytes(ref['record'])
                require(digest(body) == ref['sha256'] and len(body) == ref['bytes'], 'Changed retained input')
                selected[suffix] = dict(record=ref['record'], sha256=ref['sha256'])
                return body
            sensor = out/'lock-capture'; sensor.mkdir()
            for name in ('copy-routes-v2.bin', 'gpu-copy-census-v1.bin'):
                (sensor/name).write_bytes(artifact('/'+name))
            census = validate(out); header, routes = decode(sensor/'copy-routes-v2.bin')
            require((header[15], header[16]) == (0x400000, 3207168), 'Unknown original build/module range')
            live = [json.loads(line) for line in artifact('/copies.jsonl').splitlines()]
            diagnostic = json.loads(artifact('/copy-diagnostics/report.json'))
            flow = json.loads(artifact('/flow.json'))
            stream, decoded = command_history(artifact('/capture/commands-2.bin'), acquired['session_identity'])
            tail = attribute(decoded, routes, live, diagnostic, flow['checkpoints'])
            require(len(tail) == 613, 'Selected retained gap changed')
            report.update(summary(tail)); report['commands'] = tail
            report['original_census'] = census
            report['command_stream_sha256'] = digest(stream)
            report['live_diagnostic_preserved'] = dict(success=diagnostic['success'], truncated=diagnostic['truncated'],
                copies_seen=diagnostic['copies_seen'], copies_checked=diagnostic['copies_checked'])
            print(json.dumps(dict(attribution=summary(tail))), flush=True)
            worlds = Worlds(reader, compared); history = History(worlds)
            targets = {x['sequence']: x for x in tail}; expected = []; offsets = []
            fixture = Path(tempfile.mkdtemp(prefix='mnm-copy-tail-', dir=os.environ.get('TMPDIR')))/'cases.bin'
            report['fixture_path'] = str(fixture)
            with fixture.open('xb') as sink:
                sink.write(MAGIC)
                for op, seq, payload in decoded:
                    item = targets.get(seq)
                    if item:
                        w = item['words']; src, dst = history.handles[w[0]], history.handles[w[1]]
                        require([src['pixels'].shape[1],src['pixels'].shape[0]] == item['source_extent']
                                and [dst['pixels'].shape[1],dst['pixels'].shape[0]] == item['target_extent'], 'Caller/history extents differ')
                        before, source = plane_bytes(dst), plane_bytes(src)
                        offsets.append(sink.tell())
                        write_case(sink, item, source, before)
                    history.apply(op, seq, payload)
                    if op == 21:history.reads.pop(seq)
                    if item:
                        expected.append(dict(sequence=seq, source_sha256=digest(source), before_sha256=digest(before),
                                             expected_sha256=digest(plane_bytes(history.handles[item['words'][1]]))))
                sink.write(struct.pack('<I', 0))
            require(not history.handles and not history.palette and len(expected) == 613,
                    'Independent history did not close')
            with fixture.open('rb') as f:
                import hashlib
                fixture_hash = hashlib.file_digest(f, 'sha256').hexdigest()
            report['fixture'] = dict(bytes=fixture.stat().st_size, sha256=fixture_hash,
                policy='Independent retained history supplies source/before fixture inputs only; no new live output observation.')
            print(json.dumps(dict(fixture_bytes=fixture.stat().st_size, oracle_worlds=len(worlds.consumed))), flush=True)
            actual, gl = native(binary, fixture)
            report['replay'] = dict(cases=check_replay(tail, expected, actual), driver=gl,
                pixels_compared=sum(x['target_extent'][0]*x['target_extent'][1] for x in tail),
                live_diagnostics_recovered=False, scope='Fresh offline copies from independently reconstructed input states; original-run omitted outputs remain unavailable.')
            report['bound_artifacts'] = selected
            report['oracle_worlds_consumed'] = list(worlds.consumed.values())
            controls = subprocess.run([sys.executable, '-B', str(ROOT/'tests/test-copy-tail-proof.py')],
                                      cwd=ROOT, capture_output=True, text=True, timeout=30)
            report['join_controls'] = dict(returncode=controls.returncode, output=controls.stdout+controls.stderr)
            require(controls.returncode == 0, 'Tail join controls failed')
            # Pick an actual changing COPY, so an omitted draw must be observable.
            index = next(i for i,x in enumerate(expected) if x['before_sha256'] != x['expected_sha256'])
            with fixture.open('rb') as f:
                f.seek(offsets[index]); header = f.read(4); size = struct.unpack('<I', header)[0]; case = header+f.read(size)
            control = fixture.parent/'omitted-draw.bin'; control.write_bytes(MAGIC+case+b'\0'*4)
            bad, _ = native(binary, control, ('--skip-copy',))
            refused = False
            try:check_replay(tail[index:index+1], expected[index:index+1], bad)
            except ValueError:refused = True
            require(refused, 'Omitted native draw was not detected')
            report['omitted_draw_rejected'] = True
            report['omitted_draw_control_sequence'] = tail[index]['sequence']
            require(digest(binary.read_bytes()) == report['binary']['sha256'], 'Replay executable changed')
            report['sources_stable'] = all(reader.digest(n) == h for n,h in report['sources'].items())
            report['success'] = report['sources_stable']
    except Exception as error:
        report['error'] = str(error)
    sha = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('success','error','unchecked_commands','sources_stable','omitted_draw_rejected')}|dict(record=args.report,sha256=sha)), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':raise SystemExit(main())
