#!/usr/bin/env python3
"""Analyze retained second-Spellbox diagnostics without running a game or oracle."""
import argparse
from contextlib import closing
import hashlib
import importlib.util
import json
from pathlib import Path
import sqlite3
import struct
import tempfile
import subprocess
import sys
from coverage_evidence import Reader, write_report
from rendering_handoff import CONTROLS, PREFIX, menu_header, observed
from rendering_menu_acceptance import artifact
from rendering_pilot import commands, require
from rendering_capture import capture
from collections import deque
ROOT = Path(__file__).resolve().parents[1]


def analyze(reader, name):
    a = reader.json(name)
    require(a['kind'] in ('public_native_picking_resize_acquisition', 'second_spellbox_diagnostic_acquisition'),
            'Unknown handoff acquisition')
    require(a['sources_stable'] and a['original_manifest_verified_before_after']
            and a['original_game_runs'] == 1, 'Unstable or missing original execution')
    session = Path(a['session']).name
    wire = artifact(reader, a, session+'/channel.bin')
    terminal = menu_header(wire[:160])
    require(terminal is not None, 'Torn terminal menu lane')
    raw_events = artifact(reader, a, session+'/events.bin')
    require(raw_events[:16] == b'MNMMENU1'+struct.pack('<II', 1, 64)
            and (len(raw_events)-16) % 64 == 0, 'Incomplete callback journal')
    events = [struct.unpack_from('<16I', raw_events, at) for at in range(16, len(raw_events), 64)]
    require([e[0] for e in events] == list(range(1, len(events)+1)) and len(events) <= 256,
            'Callback sequence/cap differs')
    finishes = [dict(ordinal=e[0], event=e[1], thread=e[2], owner=e[4], argument=e[9], result=e[10])
                for e in events if e[3] == 7 and e[9] == 12 and e[1] in (2, 3)]
    shell = json.loads(artifact(reader, a, session+'/native-only-session.json'))
    ring = struct.unpack('<16I', artifact(reader, a, session+'/render-commands.bin')[:64])
    lifecycle = artifact(reader, a, 'lock-capture/lifecycle.log').decode('ascii')
    rejected = [line.split() for line in lifecycle.splitlines() if line.startswith('blit_reject_source_rect ')]
    result = dict(acquisition=dict(record=name, sha256=reader.digest(name)),
                  mode=a.get('mode', 'historical-picking'), terminal_menu=terminal,
                  callback_rows=len(events), callback_log_cap_reached=len(events) == 256,
                  spell_finish_receipts=finishes,
                  last_callback=dict(ordinal=events[-1][0], event=events[-1][1], screen=events[-1][3],
                                     thread=events[-1][2], owner=events[-1][4]),
                  shell_state=shell, command_ring=dict(state=ring[6], reason=ring[7], cancelled=ring[8],
                       published=ring[5], acknowledged=ring[9], session=ring[4]),
                  source_rect_refusals=rejected,
                  normal_acceptance=False, original_game_runs=0,
                  source_phase=a['sources'], current_native_execution=False,
                  diagnostic_cause_proved=False,
                  limitations=['Callback logging can drop contended records; missing rows alone do not prove no callback.',
                               'Terminal command cancellation follows failure/cleanup; it does not timestamp the initiating cause.',
                               'An out-of-bounds source rectangle is a retained error, not causal proof of the engine freeze.',
                               'Diagnostic sessions and process waits cannot close picking/resize or independent pixel rows.'])
    streams = [v for v in a['artifacts'] if '/capture/commands-' in v['path']]
    if streams:
        require(len(streams) == 1, 'Ambiguous diagnostic command capture')
        captured = list(capture(artifact(reader, a, streams[0]['path'])))
        fragments = [(h, b) for h, b in captured if h['kind'] == 'fragment']
        require(fragments and captured[-1][0]['kind'] == 'close', 'Missing command capture prefix/close')
        tail, count, ended = deque(maxlen=16), 0, True
        try:
            for op, seq, body in commands(b''.join(b for h, b in fragments)):
                count += 1
                tail.append(dict(operation=op, sequence=seq, bytes=len(body)))
        except ValueError as error:
            require(str(error) == 'Missing command END' and a['success'] is False,
                    'Malformed command prefix cannot supply diagnostic chronology: '+str(error))
            ended = False
        result['command_prefix'] = dict(reference=streams[0], records=count, ended=ended,
              last_fragment_host_us=int(fragments[-1][0]['host_us']),
              capture_close_host_us=int(captured[-1][0]['host_us']),
              capture_complete=captured[-1][0].get('complete'), tail=list(tail))
        clicks = [v for v in a['actions'] if v.get('object') == 'spellboxOK']
        ready = [v for v in a['states'] if v['screen'] == 7]
        if len(clicks) == len(ready) == 2:
            result['second_finish_chronology'] = dict(ready_host_ns=ready[-1]['host_ns'],
                   click_host_ns=clicks[-1]['host_ns'],
                   last_fragment_before_ready=int(fragments[-1][0]['host_us'])*1000 < ready[-1]['host_ns'],
                   capture_closed_after_click=int(captured[-1][0]['host_us'])*1000 > clicks[-1]['host_ns'],
                   close_after_click_seconds=(int(captured[-1][0]['host_us'])*1000-clicks[-1]['host_ns'])/1e9)
    traces = [v for v in a['artifacts'] if Path(v['path']).name == 'handoff.sqlite3']
    if traces:
        require(len(traces) == 1, 'Ambiguous diagnostic timeline')
        trace = traces[0]
        raw = artifact(reader, a, trace['path'])
        # Invocation-scoped physical interchange for SQLite's file reader.
        with tempfile.TemporaryDirectory(prefix='mnm-handoff-analysis-') as tmp:
            path = Path(tmp)/'observations.sqlite3'
            path.write_bytes(raw)
            result['timeline'] = observed(path)
            with closing(sqlite3.connect(path.as_uri()+'?mode=ro', uri=True)) as db:
                result['process_waits'] = [dict(kind=k, field=f, value=bytes(b).decode(errors='replace'), samples=n)
                    for k, f, b, n in db.execute("SELECT kind,field,body,COUNT(*) FROM processes WHERE field='wchan' GROUP BY kind,field,body")]
        result['timeline_reference'] = trace
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition', action='append', required=True)
    p.add_argument('--report', required=True)
    p.add_argument('--run-guards', action='store_true', help='Bind fresh retained-data/parser controls; no game execution')
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts
            and not (ROOT/a.report).exists(), 'Fresh owned analysis required')
    with Reader(ROOT) as r:
        from coverage_claims import required_sources, behavior_contract, scenario_contract
        register = r.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id'] == 'NP.second-spellbox-diagnostic')
        sources = {n: r.digest(n) for n in set(CONTROLS) | required_sources(behavior)}
        q = dict(schema=1, kind='second_spellbox_retained_analysis', success=False,
                 original_game_runs=0, nominal_acceptance=False, sources=sources)
        try:
            q['cases'] = [analyze(r, n) for n in a.acquisition]
            if a.run_guards:
                command = [sys.executable, '-B', 'tests/test-rendering-handoff.py']
                guards = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=120)
                q['guards'] = dict(command=command, exit_code=guards.returncode,
                                   stdout=guards.stdout, stderr=guards.stderr)
                require(guards.returncode == 0, 'Handoff retained-data guards failed')
                scenario = next(s for s in register['scenarios'] if s['id'] == 'native.second-spellbox-diagnostic-controls')
                q['claims'] = [dict(behavior=behavior['id'], contract_sha256=behavior_contract(
                    behavior, {v['id']: v for v in register['builds']}),
                    scenarios={scenario['id']: scenario_contract(scenario)})]
            q['success'] = all(r.digest(n) == h for n, h in sources.items())
        except Exception as error:
            q['error'] = str(error)
    print(write_report(ROOT, a.report, (json.dumps(q, indent=2)+'\n').encode()))
    return int(not q['success'])


if __name__ == '__main__':
    raise SystemExit(main())
