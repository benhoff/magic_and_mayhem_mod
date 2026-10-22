#!/usr/bin/env python3
"""Review current renderer limits and retained evidence; never launch the game."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCES = [
    'tools/review-native-rendering-gaps.py', 'runtime/render/command_queue.h',
    'runtime/render/command_channel.h', 'runtime/render/command_pressure.h',
    'runtime/render/session_emission.h',
    'protocols/include/mnm/render_command_ring.h',
    'protocols/include/mnm/render_commands_v1.h', 'protocols/include/mnm/render_commands_v2.h',
    'protocols/include/mnm/render_commands_v3.h', 'renderer/commands.cpp',
    'tests/render-command-queue-test.c', 'tests/render-command-writer-test.c',
    'tools/borrowed_pixel_backend_launch.py', 'tools/native_pixel_counters.py',
    'tools/pixel_route_requirements.py', 'tools/test-native-campaign.py',
    'runtime/native-draw/history.h', 'runtime/native-draw/surfaces.h',
    'runtime/render/bridge.c', 'runtime/render/owned_session.h',
    'renderer/command_consumer.cpp', 'apps/qt-shell/live_command_session.cpp',
    'apps/qt-shell/main.cpp', 'apps/qt-shell/game_restart_action.hpp',
    'apps/qt-shell/resident_world_session.cpp', 'tools/drawing_journeys.py',
    'tools/check-drawing-journeys.py', 'tests/fixtures/drawing-journeys/suite.json',
    'tests/test-drawing-journeys.py', 'tests/test-drawing-session.py',
    'tests/test-pixel-route-requirements.py', 'tests/test-native-render-storage.py',
]
RECORDS = [
    'research/runtime/native-pixel-camera-passed-20261015.json',
    'research/runtime/native-pixel-spells-passed-20261015.json',
    'research/runtime/native-storage-save-load-passed-20261015.json',
    'research/runtime/resident-pressure-final-initiate-live-20261016.json',
    'research/runtime/native-render-completion-controls-20261015.json',
    'research/runtime/native-render-master-integration-20261014.md',
]


def sha(path):
    return hashlib.sha256(evidence_bytes(path)).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError('Refusing to overwrite a historical review')
    parent = ROOT/'working/tests/native-render-gap-review'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    sources = {p: sha(ROOT/p) for p in SOURCES}
    report = dict(schema=1, review_only=True, success=False,
                  scope='Selected current-code inspection, native C queue/ring guards with Win32 fakes, '
                        'portable acceptance guards and retained-result inspection. No original media, '
                        'game/driver execution, GL benchmark, new pixel comparison or replacement proof.',
                  reviewed_head=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  sources=sources, retained_records={p: sha(ROOT/p) for p in RECORDS},
                  witnesses=[], executions=[], findings=[])

    def witness(path, token):
        lines = evidence_text(ROOT/path).splitlines()
        matches = [i for i, line in enumerate(lines) if token in line]
        if len(matches) != 1:
            raise ValueError(f'Review witness changed: {path}: {token!r}')
        i = matches[0]
        report['witnesses'].append(dict(path=path, line=i+1, token=token,
                                        excerpt='\n'.join(lines[max(0, i-1):i+3])))

    for path, token in [
        ('runtime/render/command_queue.h', 'if(an>remaining || bn>remaining-an || cn>remaining-an-bn)'),
        ('renderer/commands.cpp', 'p.total+=bytes.size();'),
        ('runtime/render/session_emission.h', 'if(!game_session_continuous)session_bytes+=bytes;'),
        ('protocols/include/mnm/render_command_ring.h', 'length>UINT32_MAX-w->published'),
        ('runtime/render/command_pressure.h', '#define COMMAND_PRESSURE_HIGH'),
        ('runtime/native-draw/history.h', 'if(!s->history_stale)return;'),
        ('runtime/native-draw/history.h', 'NativeDrawCpuTransferV1('),
        ('runtime/native-draw/surfaces.h', 'if(!s || s==d || !s->history_stale || clips'),
        ('runtime/render/bridge.c', 'RenderPrepareStorageV1(u32 action)'),
        ('runtime/render/owned_session.h', 'Explicit CPU reader only.'),
        ('renderer/command_consumer.cpp', 'Explicit CPU reader/session unavailable'),
        ('apps/qt-shell/resident_world_session.cpp', 'Async World completion/readback contract'),
        ('tools/test-native-campaign.py', "parser.add_argument('--other-pixel-producers'"),
        ('tools/test-native-campaign.py', "env['MNM_OTHER_PIXEL_PRODUCERS']='1'"),
        ('tools/test-native-campaign.py', 'validate_routes(report[\'pixel_producers\'],args.require_pixel_route)'),
        ('tools/test-native-campaign.py', "'complete_drawing_replacement': False"),
        ('tools/native_pixel_counters.py', 'history_refused=refusal.exists(), complete_replacement=False'),
        ('tools/pixel_route_requirements.py', "if counters['history_refused'] or forwarded or not native:"),
        ('apps/qt-shell/live_command_session.cpp', 'if(residentWorld){const auto cause='),
        ('apps/qt-shell/main.cpp', 'void residentFailed(const QString& message)'),
        ('apps/qt-shell/main.cpp', 'void hardRestart()'),
        ('apps/qt-shell/main.cpp', 'if(liveMenus_&&liveMenus_->running())liveMenus_->forceStop(stopped);'),
    ]:
        witness(path, token)

    def execute(label, command):
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=90)
        log = run/(label+'.log');log.write_text(result.stdout+result.stderr)
        report['executions'].append(dict(command=command, exit_code=result.returncode,
                                         log=str(log.relative_to(ROOT)), log_sha256=sha(log),
                                         output=result.stdout+result.stderr))
        result.check_returncode()
        return result.stdout

    # Exercise the actual guards at an empty queue/ring close to UINT32_MAX;
    # seeding private test state avoids producing gigabytes or weakening guards.
    probe = run/'lifetime.c'
    probe.write_text('#define QUEUE_TEST_MAIN existing_queue_controls\n#include "'+
                     str(ROOT/'tests/render-command-queue-test.c')+'"\n'+r'''
int main(void){
    existing_queue_controls();u8 bytes[8]={0};
    ring_reset_version(3);command_queue_written=command_queue_read=0xfffffff8u;
    check(command_queue_written-command_queue_read==0);
    check(command_channel_append(bytes,8,0,0,0,0));
    check(!command_queue_failure && command_queue_peak==8 && !command_queue_written);
    command_channel_pump();check(!command_queue_read && storage[5]==8);
    command_channel_close();
    ring_reset();command_ring.published=command_ring.acknowledged=0xfffffff8u;
    storage[5]=storage[9]=0xfffffff8u;
    check(mnm_ring_write(&command_ring,bytes,8)==MNM_RING_REJECTED);
    check(storage[6]==3 && storage[7]==MNM_RENDER_COMMANDS_V2_REASON_OVERFLOW);
    check(storage[5]==storage[9]);command_channel_close();
    ring_reset_version(3);command_ring.published=command_ring.acknowledged=0xfffffff8u;
    storage[5]=storage[9]=0xfffffff8u;
    check(mnm_ring_write(&command_ring,bytes,8)==MNM_RING_WRITTEN && !storage[5]);command_channel_close();
    puts("{\"success\":true,\"v3_private_queue_wraps\":1,\"v3_ring_wraps\":1,\"legacy_v2_lifetime_refusals\":1}");return 0;
}
''')
    execute('compile-lifetime', ['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', str(probe), '-o', str(run/'lifetime')])
    report['lifetime_controls'] = [json.loads(line) for line in execute('lifetime', [str(run/'lifetime')]).splitlines()]
    for name in ['drawing-journeys', 'drawing-session', 'pixel-route-requirements', 'native-render-storage']:
        execute(name, ['python3', '-B', 'tests/test-'+name+'.py'])

    def load(name, path):
        spec = importlib.util.spec_from_file_location(name, ROOT/path)
        module = importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        return module

    counters = load('gap_review_counters', 'tools/native_pixel_counters.py')
    launcher = load('gap_review_launcher', 'tools/borrowed_pixel_backend_launch.py')
    selected = launcher.selection({'MNM_OTHER_PIXEL_PRODUCERS': '1'})
    flags = sorted(k for k, v in selected.items() if k.endswith('_TAKEOVER') and v == '1')
    assert len(flags) == 6 and 'MNM_DC_COPY_TAKEOVER' not in selected
    suite = json.loads(evidence_text(ROOT/'tests/fixtures/drawing-journeys/suite.json'))
    all_routes = {family+'/'+route for family, routes in counters.ROUTES.items() for route in routes}
    assert len(all_routes) == 43
    route_evidence = [];exercised = set()
    for path in RECORDS[:4]:
        record = json.loads(evidence_text(ROOT/path));p = record['pixel_producers']
        assert record['success'] and p['forwarded'] == 0 and not p['history_refused']
        active = {family+'/'+row['route'] for family, rows in p['families'].items() for row in rows if row['native']}
        exercised |= active
        route_evidence.append(dict(record=path, exercised=len(active), unexercised=len(all_routes-active)))
    report['policy_checks'] = dict(combined_flags=flags, valid_routes=len(all_routes),
                                   retained_route_evidence=route_evidence,
                                   retained_exercised_union=sorted(exercised),
                                   retained_unexercised=sorted(all_routes-exercised),
                                   suite_adapters=[c['adapter'] for c in suite['cases']],
                                   full_session_cases=sum(c['adapter']=='session' for c in suite['cases']),
                                   recorded_session_cases=sum(c['adapter']=='recorded_session' for c in suite['cases']),
                                   journey_requirements=len(suite['requirements']),
                                   operation_requirements=len(suite['operation_requirements']))
    latest = json.loads(evidence_text(ROOT/RECORDS[3]))
    world = latest['resident_world']
    assert world['completed'] == world['imports'] == world['verified_returns']
    assert latest['normal_quit_verified'] and latest['world_canvas_transition_verified']
    report['retained_latest_journey'] = dict(record=RECORDS[3], balanced_worlds=world['completed'],
        normal_quit=True, changed_canvas=True, renderer=world['renderer'],
        engine_fps=world['engine_fps'], minimum_fps=latest['minimum_fps'],
        pressure=latest['command_pressure'],
        scope='Inspection of the retained executed result, not new live execution or renewed source freshness')
    report['findings'] = [
        dict(id='lifetime', classification='removed_byte_limit_with_legacy_compatibility', confidence='high_within_scope',
             finding='Private queue and V3 mapped cursors wrap with bounded occupancy, and continuous emission/streaming decoding no longer cap total bytes. Explicit V2 retains its legacy UINT32_MAX refusal. Finite sequence/identity, capacity and no-progress guards remain; separate actual >4GiB execution evidence validates the changed path.'),
        dict(id='history', classification='supported_subset', confidence='high_within_scope',
             finding='Current selected CPU reads have generation/nonce-bound materialization and guarded GPU-copy services. Clipped/self GPU source copies refuse; arbitrary raw CPU reads are outside interception. Async World source completion enforces zero readbacks, distinct from explicit command CPU_READ.'),
        dict(id='complete-mode', classification='missing_release_contract', confidence='high_within_scope',
             finding='Combined mode checks all counters, zero forwarding/history refusal and requested route execution. Reports still explicitly deny complete replacement; no complete capability/session release path is established by these flags.'),
        dict(id='routes', classification='live_coverage_gap', confidence='high_within_scope',
             finding=f'{len(exercised)}of{len(all_routes)} optional routes execute across the four retained reports. Others are unexercised in those runs; World batching bypasses some CPU interiors. This is not proof of missing implementation or global unreachability.'),
        dict(id='sessions', classification='acceptance_gap', confidence='high_within_scope',
             finding='Save/Load, changed canvas after display rebuild and normal Quit have retained successful native journeys. Shipped suite has a dedicated recorded-EGL adapter but no generic full-presented-frame session adapters; later-map progression, enabled-movie/session composition and full action-bound independent presented-frame parity remain incomplete.'),
        dict(id='restart', classification='implemented_policy_with_live_validation_gap', confidence='high_within_scope',
             finding='Resident failure clears presentation/suspends input and offers force-stop/relaunch; in-place recovery explicitly refuses. Existing synthetic fixtures model worker failures/new channels and shortcut dispatch, not an actual game fault/restart or physical GL context-loss experiment.'),
        dict(id='performance', classification='acceptance_gap', confidence='scoped_evidence_review',
             finding='Retained TITAN results distinguish engine cadence from about59.4 visible native World FPS. Latest delayed Initiate proof is software llvmpipe with a5FPS functional floor. Neither is sustained complete-session60FPS/hardware proof; hardware availability was not probed by this review.'),
    ]
    report['sources_stable'] = all(sha(ROOT/p) == digest for p, digest in sources.items())
    report['retained_records_stable'] = all(sha(ROOT/p) == digest for p, digest in report['retained_records'].items())
    assert report['sources_stable'] and report['retained_records_stable']
    report['success'] = True
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as out:
        json.dump(report, out, indent=2);out.write('\n')
    (run/'.retain').write_text('Renderer gap review and native fake-API boundary controls; '+str(args.output)+'\n')
    print(args.output, flush=True)


if __name__ == '__main__':
    main()
