#!/usr/bin/env python3
"""Observe actual upstream COPY callers on one bounded diagnostic Quick journey."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

from coverage_evidence import Reader, write_report
from coverage_claims import required_sources, behavior_contract, scenario_contract
from copy_caller_provenance import analyze, decode
from native_copy_routes import decode as routes_decode
from copy_caller_output import analyze as analyze_output

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/copy-caller-results-20261102/'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def image_offset(raw, rva):
    pe = struct.unpack_from('<I', raw, 60)[0]
    count = struct.unpack_from('<H', raw, pe+6)[0]
    optional = struct.unpack_from('<H', raw, pe+20)[0]
    for i in range(count):
        at = pe+24+optional+40*i
        virtual, start, size, file = struct.unpack_from('<IIII', raw, at+8)
        if start <= rva < start+min(virtual, size):
            return file+rva-start
    raise ValueError('Unmapped original code RVA')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tag', required=True)
    p.add_argument('--skip-intros', action='store_true')
    p.add_argument('--cursor-handoff', action='store_true')
    args = p.parse_args()
    if not args.tag.isidentifier():
        p.error('Simple new result tag required')
    prefix = 'research/runtime/cursor-caller-results-20261102/' if args.cursor_handoff else PREFIX
    name = prefix+args.tag+'.json'
    if (ROOT/name).exists():
        p.error('New immutable result required')
    out = ROOT/'working/copy-callers'/args.tag
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').write_text('Active caller evidence and negative attempts; preserve inputs.\n')
    reg = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behavior = next(b for b in reg['behaviors'] if b['id'] == ('NP.cursor-copy-caller-provenance' if args.cursor_handoff else 'NP.copy-caller-provenance'))
    scenario = next(s for s in reg['scenarios'] if s['id'] == ('SC.cursor-copy-caller-provenance' if args.cursor_handoff else 'SC.copy-caller-provenance'))
    sources = {n: sha((ROOT/n).read_bytes()) for n in required_sources(behavior)}
    report = dict(schema=1, kind='live_copy_caller_observation', success=False, sources=sources,
                  player_accepted=False, whole_operations_accepted=0, original_driver_equivalence=False,
                  claims=[dict(behavior=behavior['id'],
                               contract_sha256=behavior_contract(behavior, {v['id']: v for v in reg['builds']}),
                               scenarios={scenario['id']: scenario_contract(scenario)})])
    stage = 'controls'
    try:
        env = dict(os.environ, TMPDIR=os.environ.get('TMPDIR','/tmp'))
        command = ['clang', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
                   str(ROOT/'tests/native-copy-callers-test.c'), '-o', str(out/'caller-controls')]
        subprocess.run(command, check=True, env=env, capture_output=True)
        control = subprocess.run([out/'caller-controls'], check=True, capture_output=True, text=True, env=env)
        report['native_controls'] = json.loads(control.stdout)
        unit = subprocess.run([sys.executable, str(ROOT/'tests/test-copy-caller-provenance.py')],
                              capture_output=True, text=True, env=env)
        report['reader_controls'] = dict(success=unit.returncode == 0, output=unit.stdout+unit.stderr)
        if unit.returncode:
            raise ValueError('Caller reader refusal controls failed')
        if args.cursor_handoff:
            command = ['clang','-std=c11','-O2','-Wall','-Wextra','-Werror','-pedantic',str(ROOT/'tests/native-cursor-copy-callers-test.c'),'-o',str(out/'cursor-controls')]
            subprocess.run(command,check=True,env=env,capture_output=True)
            done = subprocess.run([out/'cursor-controls'],check=True,capture_output=True,text=True,env=env)
            report['cursor_controls'] = json.loads(done.stdout)
            with Reader(ROOT) as reader:
                thunk_name = 'research/runtime/cursor-caller-results-20261102/thunk01.json'
                thunk = reader.json(thunk_name)
                if not thunk['success'] or thunk['checks']!=9 or any(sha((ROOT/n).read_bytes())!=h for n,h in thunk['sources'].items()):
                    raise ValueError('Current actual PE32 thunk controls required')
                report['cursor_pe32_thunks'] = dict(record=thunk_name,sha256=reader.digest(thunk_name),checks=thunk['checks'])
        stage = 'live acquisition'
        acquired_name = 'research/runtime/rendering-normal-results-20261031/copy-callers-'+args.tag+'.json'
        command = [sys.executable, str(ROOT/'tools/run-rendering-acceptance-pilot.py'),
                   '--scenario', 'normal-quick', '--copy-callers', '--report', acquired_name]
        if args.cursor_handoff:
            command.append('--cursor-copy-callers')
        if args.skip_intros:
            command.append('--skip-intros')
        report['command'] = command
        with (out/'acquisition.log').open('xb') as log:
            run = subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=1000)
        report['exit_code'] = run.returncode
        if not (ROOT/acquired_name).exists():
            raise ValueError('Acquisition refused before report creation; inspect retained acquisition log')
        stage = 'caller/output join'
        with Reader(ROOT) as reader:
            live = reader.json(acquired_name)
            report['acquisition'] = dict(record=acquired_name, sha256=reader.digest(acquired_name))
            report['acquisition_success'] = live['success']
            if run.returncode or not live['success']:
                raise ValueError('Actual diagnostic journey failed; retain its immutable acquisition')
            report['sources'].update(live['sources'])
            report['sources'].update(live['native_admission']['native_only']['sources'])
            layouts_name = PREFIX+'layouts01.json'
            layouts = reader.json(layouts_name)['wrappers']
            for v,length,pop in zip(layouts,[5,5,6,6,7,8,8],[12,12,16,16,16,16,12]):v.update(hook_length=length,ret_pop=pop)
            report['layouts'] = dict(record=layouts_name, sha256=reader.digest(layouts_name))
            session = Path(live['session'])
            report['callers'] = analyze(session, layouts)
            rh, routes = routes_decode(session/'lock-capture/copy-routes-v2.bin')
            _, rows = decode(next((session/'lock-capture').glob('copy-callers-v?.bin')).read_bytes(), rh, routes, layouts)
            static_name = 'research/runtime/native-copy-route-static-20261017.json'
            prior = reader.json(static_name)
            report['static_callers'] = dict(record=static_name, sha256=reader.digest(static_name))
            known = {(v['entry']-0x400000, int(c['call_va'], 16)+5-0x400000): c['discovered_owners']
                     for v in prior['selected_wrappers'].values() for c in v['callers']}
            executable = (session/'game/Chaos.exe').read_bytes()
            exe_record = prefix+args.tag+'-staged-executable.bin'
            write_report(ROOT, exe_record, executable)
            report['staged_executable'] = dict(record=exe_record, sha256=sha(executable), bytes=len(executable))
            for layout in layouts:
                at = image_offset(executable, layout['entry_rva'])
                if sha(executable[at:at+layout['end_rva']-layout['entry_rva']]) != layout['body_sha256']:
                    raise ValueError('Staged original wrapper differs from reviewed complete code bytes')
            report['upstream_calls'] = []
            for row in report['callers']['callers']:
                wrapper, returned = int(row['wrapper_entry_rva'], 16), int(row['caller_return_rva'], 16)
                at = image_offset(executable, returned-5)
                call = executable[at:at+5]
                if len(call) != 5 or call[0] != 0xe8 or (returned+struct.unpack_from('<i', call, 1)[0]) != wrapper:
                    raise ValueError('Captured upstream call differs from staged original E8 target')
                report['upstream_calls'].append(dict(row, static_candidate_known=(wrapper, returned) in known,
                                                     discovered_owners=known.get((wrapper, returned), [])))
            report['copy_output'] = analyze_output(session, Path(live['temporary_root'])/'copy-diagnostics', rows)
            report['retirement'] = live['retirement']
            report['original_manifest_verified_before_after'] = live['original_manifest_verified_before_after']
            unsupported = report['callers']['states'].get('unsupported_api', 0)
            report['all_selected_frames_resolved'] = report['callers']['resolved_calls']+unsupported == len(rows)
            report['sources_stable'] = all(sha((ROOT/n).read_bytes()) == h for n, h in report['sources'].items())
            report['success'] = (report['sources_stable']
                                 and report['callers']['resolved_calls'] > 0 and report['copy_output']['verified'])
            report['natural_branch_status'] = dict(self_copy='pending', keyed_gpu_source='pending')
            report['scope'] = scenario['scope']
    except Exception as e:
        report.update(success=False, error=str(e), failed_stage=stage)
    artifacts = []
    for path in out.iterdir():
        if path.name == '.retain' or not path.is_file():
            continue
        data = path.read_bytes()
        record = prefix+args.tag+'-'+path.name+'.bin'
        write_report(ROOT, record, data)
        artifacts.append(dict(record=record, sha256=sha(data), bytes=len(data)))
    report['artifacts'] = artifacts
    write_report(ROOT, name, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k: report[k] for k in ('success', 'error', 'failed_stage', 'callers') if k in report}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
