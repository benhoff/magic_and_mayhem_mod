#!/usr/bin/env python3
"""Execute unchanged-scope continuous, public-World and native-only controls."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
from coverage_evidence import evidence_digest, read_json, write_report
from drawing_fixture_prefix import prepare
from rendering_seventeenth_controls import BEHAVIORS, SCENARIO, STAGES, PYTHON_TESTS, source_closure, require_continuous, require_public_implementation, public_portrait_pixels

ROOT = Path(__file__).resolve().parents[1]
TARGETS = ['render-resident-world-test', 'world-deferred-test', 'world-source-regions-test',
           'world-raster-batch-test', 'minimap-producer-test', 'minimap-candidates-test',
           'menu-resident-quit-test', 'menu-campaign-preferences-test', 'scene-renderer-test',
           'scene-upload-test', 'native-only-session-test', 'game-hard-restart-test', 'native-direct-store-test', 'pixel-store-backend-test', 'native-sampling-test', 'native-lease-row-copy-test', 'native-dirty-byte-test', 'native-frame-recipe-test', 'world-gpu-resident-test', 'live-render-channel-test']


def module(name, relative):
    spec = importlib.util.spec_from_file_location(name, ROOT/relative)
    value = importlib.util.module_from_spec(spec); spec.loader.exec_module(value)
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stage', choices=STAGES, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--prepared', type=Path, help='Exact continuous build with passing guards and unchanged compiler closure')
    parser.add_argument('--prefix-template', type=Path, required=True)
    parser.add_argument('--source-game', type=Path, required=True)
    args = parser.parse_args()
    destination = args.report if args.report.is_absolute() else ROOT/args.report
    if destination.exists(): parser.error('Require new immutable evidence output')
    parent = ROOT/'working/tests/rendering-seventeenth-five'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix=args.stage+'-', dir=parent))
    (out/'.retain').write_text('Seventeenth rendering execution: retain inputs, outputs, failure evidence and binary provenance.\n')
    print(out, flush=True)
    claims_tool = module('seventeenth_claims', 'tools/draft-coverage-claims.py')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration = source_closure(ROOT, claims_tool.declaration(ROOT, register, BEHAVIORS, [SCENARIO]))
    auxiliary = None
    if args.stage == 'controls':
        auxiliary = claims_tool.declaration(ROOT, register, ['NP.protected-native-store-service'], ['NP.protected-native-store-service.controls-20261023'])
        if any(declaration['sources'].get(p)!=h for p,h in auxiliary['sources'].items()):raise ValueError('Undeclared direct-store control input')
    claims = out/'claims.json'; claims.write_text(json.dumps(declaration, indent=2)+'\n')
    report = dict(declaration, schema=1, success=False, stage=args.stage, commands=[], components={},
                  reviewed_head=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  artifacts=out.relative_to(ROOT).as_posix(), original_game_executed=False,
                  original_comparison_promoted=False, live_replacement=False, complete_directdraw_retirement=False,
                  scope='Five existing bounded implementation contracts with exact sources. Continuous verification/performance retain their complete unchanged gates. Public implementation credit is separate from the unmodified ordinary19FPS campaign harness, whose actual pass/failure remains explicit; no wider acceptance, original comparison or replacement credit follows.')
    if auxiliary is not None:
        report['auxiliary_claims']=auxiliary['claims']
        report['optimization_claims']=claims_tool.declaration(ROOT, register, ['NP.native-copy-transforms','NR.owned-session-unlock-packed','NR.continuous-update-partition','NP.protected-cpu-lease-storage','NR.masked-update-scratch-reuse','HOST.native-command-responsiveness'], [])['claims']
    env = {k:v for k,v in os.environ.items() if not k.startswith(('MNM_', 'WINE')) and k not in {
        'DISPLAY', 'WAYLAND_DISPLAY', 'LIBGL_ALWAYS_SOFTWARE', '__EGL_VENDOR_LIBRARY_FILENAMES',
        '__GLX_VENDOR_LIBRARY_NAME', 'MESA_LOADER_DRIVER_OVERRIDE', 'GALLIUM_DRIVER', 'QT_XCB_GL_INTEGRATION',
        'LP_NUM_THREADS', 'XDG_CACHE_HOME', 'MESA_SHADER_CACHE_DIR', '__GL_SHADER_DISK_CACHE_PATH'}}
    env.update(PYTHONDONTWRITEBYTECODE='1', GIT_OPTIONAL_LOCKS='0', TMPDIR=str(ROOT/'working/tmp'),
               MM_EVIDENCE_DB=str(ROOT/'working/coverage/evidence-pending.sqlite3'), QT_QPA_PLATFORM='xcb')
    lock = None
    verified = False

    def stable():
        return all(evidence_digest(ROOT/name, root=ROOT) == h for name,h in declaration['sources'].items())

    def contracts():
        current = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        return (claims_tool.declaration(ROOT, current, BEHAVIORS, [SCENARIO])['claims'] == declaration['claims'] and
                (auxiliary is None or claims_tool.declaration(ROOT, current, ['NP.protected-native-store-service'], ['NP.protected-native-store-service.controls-20261023'])['claims']==auxiliary['claims'] and claims_tool.declaration(ROOT, current, ['NP.native-copy-transforms','NR.owned-session-unlock-packed','NR.continuous-update-partition','NP.protected-cpu-lease-storage','NR.masked-update-scratch-reuse','HOST.native-command-responsiveness'], [])['claims']==report['optimization_claims']))

    def run(label, command, *, expected=(0,), timeout=900, graphics=False, isolated=True):
        if not stable() or not contracts(): raise ValueError('Prospective inputs changed before '+label)
        if shutil.disk_usage(ROOT).free < 40*2**30: raise ValueError('Third-drive execution reservation refused')
        if shutil.disk_usage('/var/local/magic-and-mayhem-archive').free < 100*2**30: raise ValueError('Archive reserve refused')
        argv = list(map(str, command))
        if graphics: argv = ['xvfb-run', '-a', '-s', '-screen 0 1280x1024x24', *argv]
        if isolated:
            private = Path(tempfile.mkdtemp(prefix='private-tmp-', dir=out))
            (private/'.X11-unix').mkdir(mode=0o1777); (private/'.X11-unix').chmod(0o1777)
            argv = ['bwrap', '--die-with-parent', '--new-session', '--unshare-user', '--unshare-pid',
                    '--unshare-net', '--unshare-ipc', '--unshare-uts', '--ro-bind', '/', '/',
                    '--bind', str(ROOT), str(ROOT), '--ro-bind', str((ROOT/'original').resolve()),
                    str((ROOT/'original').resolve()), '--bind', str(private), '/tmp',
                    '--proc', '/proc', '--dev-bind', '/dev', '/dev', '--',
                    sys.executable, '-B', '-c', 'import subprocess,sys; raise SystemExit(subprocess.call(sys.argv[1:]))',
                    sys.executable, '-B', '-c', 'import subprocess,sys; raise SystemExit(subprocess.call(sys.argv[1:]))', *argv]
        print(label, flush=True)
        path = out/(label+'.log')
        failure = None
        with path.open('x') as stream:
            try:
                result = subprocess.run(argv, cwd=ROOT, env=env, stdout=stream, stderr=subprocess.STDOUT, timeout=timeout)
            except subprocess.TimeoutExpired as error:
                failure = error
                result = subprocess.CompletedProcess(argv, 'timeout')
        row = dict(label=label, argv=argv, exit_code=result.returncode, log_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                   log=path.read_text(errors='replace'))
        report['commands'].append(row)
        if failure is not None: raise failure
        if result.returncode not in expected: raise ValueError(label+' failed: '+row['log'][-5000:])
        return row

    def child_path(label):
        return destination.with_name(destination.stem+'-'+label+'.json')

    try:
        _, lock = prepare(ROOT, out, args.prefix_template, args.prefix_template)
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'], isolated=False)
        verified = True
        if args.stage == 'controls':
            tests = 0
            for relative in PYTHON_TESTS:
                row = run(Path(relative).stem, [sys.executable, '-B', ROOT/relative])
                count = re.search(r'Ran (\d+) tests?', row['log'])
                if count is None: raise ValueError('Python suite did not report its actual count: '+relative)
                tests += int(count[1])
            report['python_tests_passed'] = tests
            build = ROOT/'working/build/seventeenth-controls'
            run('native-configure', ['cmake', '-S', ROOT/'tests/rendering-seventeenth-five', '-B', build,
                                    '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DBUILD_TESTING=ON'])
            run('native-build', ['cmake', '--build', build, '--target', *TARGETS, '-j4'], timeout=1800)
            report['native_controls'] = []
            for name in TARGETS:
                hits = [p for p in build.rglob(name) if p.is_file() and os.access(p, os.X_OK)]
                if len(hits) != 1: raise ValueError('Missing/ambiguous asserted native control: '+name)
                flags = hits[0].parent/'CMakeFiles'/(name+'.dir')/'flags.make'
                if '-UNDEBUG' not in flags.read_text(): raise ValueError('Control assertions disabled: '+name)
                row = run(name, [hits[0]], graphics=True)
                report['native_controls'].append(dict(name=name, binary_sha256=hashlib.sha256(hits[0].read_bytes()).hexdigest(), log_sha256=row['log_sha256']))
            facade = module('seventeenth_facade', 'tools/build-native-draw.py').build(out/'facade', declaration['sources'])
            for label, tool, extra in [
                ('output-owner', 'test-native-output-owner.py', []),
                ('backing-owner', 'test-native-backing-owner.py', ['--library', facade, '--prefix', out/'backing-prefix']),
                ('native-only', 'test-native-only.py', ['--library', facade, '--output', out/'strict-controls'])]:
                record = child_path(label)
                run(label, [sys.executable, '-B', ROOT/'tools'/tool, '--claims', claims, '--report', record, *extra])
                value = read_json(record, root=ROOT)
                if value.get('success') is not True or value.get('sources_stable') is not True: raise ValueError('Owned control refused: '+label)
                report['components'][label] = dict(path=str(record.relative_to(ROOT)), sha256=evidence_digest(record, root=ROOT), result=value)
            for label, directory, tool, extra in [
                ('publication-queue', 'render-command-queue', 'test-render-command-queue.py', []),
                ('dirty-regions', 'render-dirty-region', 'test-render-dirty-region.py', []),
                ('publication-idle', 'render-command-idle', 'test-render-command-idle.py', []),
                ('publication-backpressure', 'render-backpressure', 'test-render-backpressure.py', ['--claims', claims, '--admission']),
                ('publication-backpressure-general', 'render-backpressure', 'test-render-backpressure.py', ['--claims', claims]),
                ('publication-isolation', 'resident-bind-admission', 'test-resident-bind-admission.py', ['--claims', claims, '--wine-prefix', args.prefix_template, '--publication-probe'])]:
                before = set((ROOT/'working/tests'/directory).glob('run-*'))
                run(label, [sys.executable, '-B', ROOT/'tools'/tool, *extra])
                created = set((ROOT/'working/tests'/directory).glob('run-*')) - before
                if len(created) != 1: raise ValueError('Ambiguous publication control: '+label)
                physical = created.pop()/'report.json'; value = read_json(physical, root=ROOT)
                if value.get('success') is not True: raise ValueError('Publication control refused: '+label)
                record = child_path(label); write_report(ROOT, record, physical.read_bytes())
                report['components'][label] = dict(path=str(record.relative_to(ROOT)), sha256=evidence_digest(record, root=ROOT), result=value)
            functional = out/'functional-controls'
            run('functional-copy', [sys.executable, '-B', ROOT/'tools/test-native-functional-closure.py', '--claims', claims, '--output', functional])
            value = read_json(functional/'report.json', root=ROOT)
            if not value.get('success') or value.get('transform_cases')!=150 or value.get('transform_pixels')!=17550:raise ValueError('Actual PE32 copy-transform oracle refused')
            record = child_path('functional-copy');write_report(ROOT, record, (functional/'report.json').read_bytes())
            report['components']['functional-copy']=dict(path=str(record.relative_to(ROOT)),sha256=evidence_digest(record,root=ROOT),result=value)
            before=set((ROOT/'working/tests/render-lock-blits').glob('run-*'))
            run('locked-blits',[sys.executable,'-B',ROOT/'tools/test-render-lock-blits.py'])
            created=set((ROOT/'working/tests/render-lock-blits').glob('run-*'))-before
            if len(created)!=1:raise ValueError('Ambiguous actual PE32 locked-blit control')
            physical=created.pop()/'report.json';value=read_json(physical,root=ROOT)
            if value.get('origin')!='synthetic_game_owned_blits' or len(value.get('fixtures',[]))!=24:raise ValueError('Incomplete actual PE32 locked-blit cases')
            wrapped=dict(schema=1,success=True,sources=declaration['sources'],claims=declaration['claims'],sources_stable=stable(),original_game_executed=False,result=value,raw_report_sha256=evidence_digest(physical,root=ROOT))
            record=child_path('locked-blits');write_report(ROOT,record,(json.dumps(wrapped,indent=2)+'\n').encode())
            report['components']['locked-blits']=dict(path=str(record.relative_to(ROOT)),sha256=evidence_digest(record,root=ROOT),result=wrapped)
        elif args.stage in ('verification', 'normal', 'impossible', 'consumer-death', 'producer-death', 'recovery'):
            old = set((ROOT/'working/tests/native-world-continuous').glob('run-*'))
            queue_count = 4096 if args.stage in ('verification', 'normal') else 128
            command = [sys.executable, '-B', ROOT/'tools/test-native-world-continuous.py', '--build-current',
                       '--hardware', '--hardware-driver', 'zink', '--queues', str(queue_count),
                       '--claims', claims, '--source-game', args.source_game]
            if args.stage != 'verification': command.append('--normal')
            if args.stage == 'impossible': command.append('--impossible-budget')
            if args.stage in ('consumer-death', 'producer-death'): command += ['--mode', args.stage]
            if args.prepared: command += ['--prepared', args.prepared]
            run('continuous', command, expected=(1,) if args.stage == 'impossible' else (0,), timeout=1800)
            new = set((ROOT/'working/tests/native-world-continuous').glob('run-*')) - old
            if len(new) != 1: raise ValueError('Ambiguous continuous execution')
            directory = new.pop(); value = read_json(directory/'report.json', root=ROOT)
            record = child_path('continuous')
            write_report(ROOT, record, (json.dumps(value, indent=2)+'\n').encode())
            report['components']['continuous'] = dict(path=str(record.relative_to(ROOT)), sha256=evidence_digest(record, root=ROOT), result=value)
            require_continuous(value, args.stage)
            report['continuous_directory'] = str(directory)
            report['original_game_executed'] = True
        elif args.stage in ('owned-campaign', 'strict-campaign', 'strict-restart'):
            record = child_path('journey')
            command = [sys.executable, '-B', ROOT/'tools/test-native-owned-campaign.py', '--claims', claims,
                       '--prefix-template', args.prefix_template, '--source-game', args.source_game, '--report', record]
            if args.stage != 'owned-campaign': command.append('--native-only')
            if args.stage == 'strict-restart': command.append('--restart')
            # These policies explicitly exclude ordinary campaign acceptance.
            # The separate public-campaign stage retains its mandatory stress.
            if args.stage != 'strict-restart': command += ['--stress-seconds', '0']
            run('owned-journey', command, timeout=1200)
            value = read_json(record, root=ROOT)
            if value.get('success') is not True or not value.get('original_game_executed') or not value.get('sources_stable'):
                raise ValueError('Owned actual journey refused')
            report['components']['journey'] = dict(path=str(record.relative_to(ROOT)), sha256=evidence_digest(record, root=ROOT), result=value)
            report['original_game_executed'] = True
        else:
            before = set((ROOT/'working/tests/native-campaign').glob('run-*'))
            command = [sys.executable, '-B', ROOT/'tools/test-native-campaign.py', '--resident-world',
                       '--other-pixel-producers', '--no-pixel-forwarding', '--normal-quit', '--hardware',
                       '--require-casting-combat', '--spell-cases', '--portrait-stress-seconds', '10',
                       '--stress-seconds', '10', '--min-fps', '19', '--difficulty', '1', '--menu-cycles', '1',
                       '--skip-movies', '--claims', claims, '--prefix-template', args.prefix_template,
                       '--source-game', args.source_game, '--timeout', '600']
            run('ordinary-public-campaign', command, expected=(0,1), timeout=1800)
            after = set((ROOT/'working/tests/native-campaign').glob('run-*')) - before
            if len(after) != 1: raise ValueError('Ambiguous public campaign')
            physical = after.pop()/'report.json'; value = read_json(physical, root=ROOT); record = child_path('public')
            write_report(ROOT, record, (json.dumps(value, indent=2)+'\n').encode())
            flow=read_json(physical.parent/'flow.json',root=ROOT)
            flow_record=child_path('flow');write_report(ROOT,flow_record,(json.dumps(flow,indent=2)+'\n').encode())
            report['components']['flow']=dict(path=str(flow_record.relative_to(ROOT)),sha256=evidence_digest(flow_record,root=ROOT),result=flow)
            pixels=public_portrait_pixels(read_json(physical.parent/'input-0.json',root=ROOT),flow,physical.parent)
            report['portrait_implementation']=pixels
            report['implementation_admission']=require_public_implementation(value,flow=flow,portrait_pixels=pixels)
            report['components']['public'] = dict(path=str(record.relative_to(ROOT)), sha256=evidence_digest(record, root=ROOT), result=value)
            report['original_game_executed'] = True
        report['success'] = True
    except (AssertionError, OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        report['error'] = repr(error)
    finally:
        if verified:
            try:
                run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'], isolated=False)
                report['original_manifest_verified_before_after'] = True
            except (OSError, ValueError, subprocess.SubprocessError) as error:
                report.update(success=False, cleanup_error=repr(error))
        if lock is not None: lock.close()
        report['sources_stable'] = stable(); report['contracts_stable'] = contracts()
        report['success'] = report['success'] and report['sources_stable'] and report['contracts_stable']
        write_report(ROOT, destination, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('stage', 'success', 'sources_stable', 'error', 'continuous_directory')}), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__': raise SystemExit(main())
