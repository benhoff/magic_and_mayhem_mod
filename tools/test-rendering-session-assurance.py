#!/usr/bin/env python3
"""Renew the three complete recorded rendering cohorts without changing oracles."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys

from coverage_evidence import Reader, evidence_digest, read_json, write_report
from rendering_seventeenth_controls import source_closure

ROOT = Path(__file__).resolve().parents[1]
PROFILES = {
    'quick_battle': ('drawing-recorded-session', ['NR.recorded-session-corpus'],
                     ['NR.recorded-session-corpus.headless'], 0, 323, 0),
    'legacy_storage': ('drawing-storage-session', ['NR.storage-session-corpus',
                       'NR.drawing-storage-session-comparison'],
                      ['NR.storage-session-corpus.headless',
                       'NR.drawing-storage-session-comparison.storage'], 1, 238, 252),
    'corrected_storage': ('drawing-storage-corrected-session', ['NR.storage-session-corpus',
                          'NR.drawing-storage-session-comparison'],
                         ['NR.storage-session-corpus.corrected',
                          'NR.drawing-storage-session-comparison.storage'], 0, 222, 0),
}


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--backend', choices=('software', 'hardware'), required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or args.report.exists():
        parser.error('Require new acquisition and SQLite report paths')
    out = args.output.absolute()
    if not out.is_relative_to(ROOT/'working'):
        parser.error('Execution outputs must remain under working/')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain complete recorded cohorts and negative evidence.\n')
    env = dict(os.environ, TMPDIR=str(ROOT/'working/tmp'), PYTHONDONTWRITEBYTECODE='1')
    draft = module('assurance_claims', ROOT/'tools/draft-coverage-claims.py')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    declarations = {}
    for name, (_, behaviors, scenarios, _, _, _) in PROFILES.items():
        d = source_closure(ROOT, draft.declaration(ROOT, register, behaviors, scenarios))
        d['sources']['tools/test-rendering-session-assurance.py'] = evidence_digest(Path(__file__), root=ROOT)
        for directory in ('drawing-recorded-session', 'drawing-storage-session', 'drawing-storage-corrected-session'):
            for path in (ROOT/'tests/fixtures'/directory).iterdir():
                if path.is_file():
                    d['sources'][path.relative_to(ROOT).as_posix()] = evidence_digest(path, root=ROOT)
        declarations[name] = d
        (out/(name+'-claims.json')).write_text(json.dumps(d, indent=2)+'\n')
    sources = {p: h for d in declarations.values() for p, h in d['sources'].items()}
    result = dict(schema=1, success=False, backend=args.backend, sources=sources,
                  profiles={}, commands=[], complete_replacement=False,
                  scope='Full unchanged retained Quick Battle, unmodified storage and independently '
                        'corrected storage cohorts. An expected legacy mismatch is retained separately '
                        'from positive profile acceptance. No new original capture or all-game claim.')
    server = None

    def run(label, command, expected=0, timeout=900):
        process = subprocess.run(list(map(str, command)), cwd=ROOT, env=env,
                                 capture_output=True, timeout=timeout)
        diagnostic = process.stdout+b'\n'+process.stderr
        (out/(label+'.log')).write_bytes(diagnostic)
        result['commands'].append(dict(label=label, argv=list(map(str, command)),
            returncode=process.returncode, expected=expected,
            diagnostic=diagnostic.decode(errors='replace')))
        print(label, process.returncode, flush=True)
        if process.returncode != expected:
            raise ValueError(label+': unexpected exit '+str(process.returncode))

    try:
        build = ROOT/'working/build/rendering-session-assurance'
        run('configure', ['cmake', '-S', ROOT/'apps/drawing-session', '-B', build,
                          '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--target', 'mnm-drawing-session-replay',
                      'mnm-drawing-present', 'drawing-scene-test', 'drawing-session-test', '-j2'])
        run('composition-tests', ['ctest', '--test-dir', build/'composition',
                                  '-R', '^native-drawing-(scene|session|session-runner)$',
                                  '--output-on-failure'])
        if args.backend == 'hardware':
            helper = module('assurance_hardware', ROOT/'tools/test-native-campaign.py')
            hardware = out/'hardware'; hardware.mkdir()
            server = helper.start_native_hardware(hardware, env)
        for name, (directory, _, _, expected, count, mismatches) in PROFILES.items():
            child = args.report.with_name(args.report.stem+'-'+name+'.json')
            command = [sys.executable, '-B', ROOT/'tools/check-recorded-drawing-session.py',
                '--catalog', 'tests/fixtures/'+directory+'/session.json',
                '--native', build/'composition/mnm-drawing-session-replay',
                '--presenter', build/'mnm-drawing-present',
                '--output-parent', out/name, '--claims', out/(name+'-claims.json'),
                '--report', child, '--hardware' if args.backend == 'hardware' else '--headless']
            run(name, command, expected)
            with Reader(ROOT):
                value = read_json(child)
                digest = evidence_digest(child, root=ROOT)
            if (value['frames'] != count or value['mismatches'] != mismatches or
                    value['sources_stable'] is not True or
                    value['original_oracles_read_by_native'] is not False or
                    value['physical_generation_frames'] != count):
                raise ValueError(name+': incomplete or changed reference cohort')
            if name == 'legacy_storage':
                failed = [(r['ordinal'], r['gpu']['mismatching_pixels'],
                           r['physical']['mismatching_pixels']) for r in value['comparisons']
                          if r['gpu']['mismatching_pixels'] or r['physical']['mismatching_pixels']]
                if failed != [(170, 126, 126)]:
                    raise ValueError('Legacy diagnostic changed: '+repr(failed))
            result['profiles'][name] = dict(record=child.relative_to(ROOT).as_posix(),
                sha256=digest, success=value['success'], frames=count,
                mismatches=mismatches, reference_profile=value['reference_profile'])
        result['success'] = True
    except Exception as error:
        result['error'] = repr(error)
    finally:
        if server is not None:
            helper.stop_group(server)
        result['sources_stable'] = all(evidence_digest(ROOT/p, root=ROOT) == h for p, h in sources.items())
        result['success'] &= result['sources_stable']
        result['retained_legacy_negative'] = result['profiles'].get('legacy_storage', {}).get('success') is False
        for path in out.rglob('*.log'):
            if path.name.startswith('private-gpu') or path.name == 'hardware-driver.log':
                result.setdefault('hardware_diagnostics', {})[path.relative_to(out).as_posix()] = path.read_text(errors='replace')
        write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps({k: result.get(k) for k in ('success', 'backend', 'error', 'profiles')}))
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
