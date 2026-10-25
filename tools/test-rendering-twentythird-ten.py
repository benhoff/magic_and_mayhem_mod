#!/usr/bin/env python3
"""Ten bounded native rendering services, with prospective SQLite execution proof."""
import argparse
import hashlib
import importlib.util
import json
import os
import re
from pathlib import Path
import shlex
import struct
import subprocess
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import read_bytes, write_report
ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = dict(traversal='RE.terrain_traversal', constraints='RE.terrain_constraints',
           solver='RE.terrain_solver', specific='RE.terrain_specific',
           generation='RE.terrain_generation', generated='RE.terrain_generated',
           lighting='RE.terrain_lighting', creature='RE.terrain_creature_lighting',
           facing='AN.world-facing', initializer='RS.alternate-canvas-initializer')


def digest(path):
    path = Path(path).resolve()
    payload = read_bytes(path, root=ROOT) if path.is_relative_to(ROOT) else path.read_bytes()
    return hashlib.sha256(payload).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--main-root', type=Path, required=True)
    parser.add_argument('--asset-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--reuse-builds', type=Path, help='Reuse only this batch owned idle CMake build trees; sources are reconfigured/rebuilt and fixtures execute again')
    args = parser.parse_args()
    out, report_path = args.output.resolve(), args.report.absolute()
    if out.exists() or report_path.exists() or not out.is_relative_to(ROOT/'working/tests') or not report_path.is_relative_to(ROOT/'research/runtime'):
        parser.error('Require new checkout-local execution and immutable report paths')
    reuse = args.reuse_builds.resolve() if args.reuse_builds else out
    if args.reuse_builds and (not reuse.is_relative_to(ROOT/'working/tests') or not (reuse/'.retain').is_file()):
        parser.error('Reuse requires a retained build root owned by this checkout')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Ten-task rendering execution: preserve exact inputs, logs, binaries and negative evidence.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1', QT_QPA_PLATFORM='offscreen')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    terrain_ids = [bid for key, bid in IDS.items() if key != 'facing']
    report = dict(schema=1, success=False, commands=[], inputs={}, bindings={}, native={}, original={},
                  original_sha256=BUILD, live_replacement=False, original_game_executed=False,
                  scope='Ten bounded native ownership/consumption services. Original references compare safe selected terrain helpers and successful graphics initializer requests. Facing and atomic batch/replay/MAP composition are separately identified native policies; no original whole-scene/targeting/allocator/COM-failure/live/FPS acceptance.')
    for group, bids in [('terrain', terrain_ids), ('facing', [IDS['facing']])]:
        paths = set().union(*(required_sources(behaviors[bid]) | set(behaviors[bid]['documents']) for bid in bids))
        # Conservative declared compiler-input supersets; actual .d inputs are
        # checked separately. These hashes grant no coverage of unrelated code.
        folders = ['reconstruction/rendering', 'reconstruction/animation', 'assets',
                   'renderer', 'apps/terrain-preview', 'apps/scene-session']
        if group == 'facing':
            folders += ['apps/command-session', 'game', 'reconstruction']
        for folder in folders:
            for p in (ROOT/folder).rglob('*'):
                if p.is_file() and (p.name == 'CMakeLists.txt' or p.suffix in {'.cpp', '.hpp', '.c', '.h', '.S', '.inc', '.cmake'}):
                    paths.add(p.relative_to(ROOT).as_posix())
        paths.update(['tests/rendering-owned-boundaries-test.cpp', 'tests/sprite-binary-reference.cpp',
                      'tests/alternate-canvas-initializer.cpp', 'tests/native-world-facing-test.cpp',
                      'apps/command-session/native_facing.hpp', 'tools/test-rendering-twentythird-ten.py',
                      'tools/test-terrain-region.py', 'tools/decode-cfg.py'])
        paths.update('tests/terrain-'+name+'-reference.cpp' for name in ['traversal', 'constraints', 'solver', 'specific', 'generation', 'generated', 'lighting', 'creature-lighting'])
        candidates = {}
        for name in subprocess.check_output(['rg', '--files'], cwd=ROOT, text=True).splitlines():
            path = ROOT/name
            if path.suffix in {'.hpp', '.h', '.inc', '.cpp', '.c', '.S'}:
                candidates.setdefault(path.name, []).append(path)
        pending, visited = list(paths), set()
        while pending:
            name = pending.pop()
            if name in visited:
                continue
            visited.add(name)
            source = ROOT/name
            if source.suffix not in {'.hpp', '.h', '.inc', '.cpp', '.c', '.S'}:
                continue
            for include in re.findall(r'^\s*#\s*include\s+"([^"]+)"', source.read_text(), re.M):
                direct = (source.parent/include).resolve()
                found = [direct] if direct.is_file() else candidates.get(Path(include).name, [])
                for target in found:
                    if target.is_relative_to(ROOT) and not target.is_relative_to(ROOT/'working'):
                        path = target.relative_to(ROOT).as_posix()
                        if path not in paths:
                            paths.add(path)
                            pending.append(path)
        report['bindings'][group] = dict(sources={p: digest(ROOT/p) for p in sorted(paths)},
            claims=[dict(behavior=bid, contract_sha256=behavior_contract(behaviors[bid], builds)) for bid in bids])
    report['bindings']['original'] = dict(sources=dict(report['bindings']['terrain']['sources']),
                                        claims=list(report['bindings']['terrain']['claims']))

    def run(label, argv, structured=False, timeout=900):
        command = [str(v) for v in argv]
        print(label, flush=True)
        p = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, timeout=timeout)
        log = out/(label+'.log')
        log.write_bytes(p.stdout+b'\n'+p.stderr)
        report['commands'].append(dict(label=label, argv=command, exit_code=p.returncode,
            log_sha256=digest(log), output=(p.stdout+b'\n'+p.stderr).decode()))
        if p.returncode:
            raise RuntimeError(label+': '+report['commands'][-1]['output'][-4000:])
        return json.loads(p.stdout) if structured else p.stdout.decode()

    def track(path):
        path = Path(path).resolve()
        report['inputs'].setdefault(str(path), digest(path))

    def dependencies(build, group):
        actual = set()
        for p in build.rglob('*.o.d'):
            for token in shlex.split(p.read_text().replace('\\\n', ' ').split(':', 1)[1]):
                source = Path(token).resolve()
                if source.is_file() and source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):
                    actual.add(source.relative_to(ROOT).as_posix())
        missing = actual-report['bindings'][group]['sources'].keys()
        if missing:
            raise ValueError('Undeclared compiler input: '+repr(sorted(missing)))
        return sorted(actual)

    verified_before = False
    main_root = args.main_root.resolve()
    try:
        run('manifest-before', [main_root/'tools/original-manifest.sh', 'verify'])
        verified_before = True
        exe = main_root/'original/Arcane_Nocd/Chaos.exe'
        if digest(exe) != BUILD:
            raise ValueError('Unsupported original executable')
        track(exe)
        codec = module('n23_codec', ROOT/'tools/decode-cfg.py')
        recipe_parser = module('n23_recipes', ROOT/'tools/test-terrain-region.py')
        assets = args.asset_root.resolve()
        headers = []
        cache = {}

        def decode(path):
            if path not in cache:
                track(path)
                cache[path] = codec.decode(path.read_bytes())[1]
            return cache[path]

        for path in sorted(p for p in assets.rglob('*') if p.suffix.lower() == '.map'):
            raw = decode(path)
            headers.append(struct.pack('<I', len(headers)%100)+raw[:76])
        header_file = out/'headers.bin'
        header_file.write_bytes(struct.pack('<I', len(headers))+b''.join(headers))
        rows, cfgs, missing = [], [], []
        for realm in ['Celtic', 'Greek', 'Medieval']:
            config = assets/f'Realms/{realm}/{realm}.cfg'
            track(config)
            raw = config.read_bytes()
            raw = raw if raw.startswith(b';') else codec.decode(raw)[1]
            cfg = out/(realm+'.cfg')
            cfg.write_bytes(raw)
            cfgs.append(cfg)
            for recipe in recipe_parser.recipe_rows(raw):
                entries = [list(q)+[1] for q in recipe['specific']]+[[q[0], -1, -1, -1, q[1]] for q in recipe['random']]
                row = struct.pack('<5I', recipe['id'], recipe['columns'], recipe['rows'], len(recipe['specific']), len(recipe['random']))
                for entry in entries:
                    path = assets/recipe['path'].replace('\\', '/')/(recipe['prefix']+f'{entry[0]:02}.map')
                    if path.exists():
                        raw_header = decode(path)[:76]
                    else:
                        missing.append(dict(realm=realm, region=recipe['id'], section=entry[0], path=str(path.relative_to(assets))))
                        raw_header = bytes(76)
                    row += struct.pack('<5i', *entry)+raw_header
                rows.append(row)
        catalogs = out/'catalogs.bin'
        catalogs.write_bytes(struct.pack('<I', len(rows))+b''.join(rows))
        report['installed_inputs'] = dict(map_headers=len(headers), recipes=len(rows), missing_headers=missing)
        for path in [header_file, catalogs, *cfgs]:
            track(path)
        targets = ['rendering-owned-boundaries-test', 'terrain-traversal-test', 'terrain-constraints-test',
                   'terrain-solver-test', 'terrain-specific-test', 'terrain-generation-test',
                   'terrain-generated-test', 'terrain-creature-lighting-test']
        regex = '^(rendering-owned-|terrain-traversal-camera$|terrain-region-(constraints|solver|specific|generation)$|terrain-generated-plans$|terrain-creature-lighting$)'
        for mode, options in [('release', ['-DCMAKE_BUILD_TYPE=Release']),
                              ('sanitized', ['-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = ROOT/'working/tests/development-release' if mode == 'release' else reuse/mode
            run(mode+'-configure', ['cmake', '-S', ROOT/'reconstruction/rendering', '-B', build, *options])
            run(mode+'-build', ['cmake', '--build', build, '--target', *targets, '-j2'])
            run(mode+'-ctest', ['ctest', '--test-dir', build, '--output-on-failure', '-R', regex])
            results = {name: run(mode+'-'+name, [build/'rendering-owned-boundaries-test', name], True) for name in IDS}
            report['native'][mode] = dict(boundaries=results, compiled_sources=dependencies(build, 'terrain'), all_passed=all(v['success'] for v in results.values()))
        model = lambda name: ROOT/'reconstruction/rendering'/(name+'.cpp')
        region = [model(n) for n in ['terrain_selection', 'terrain_constraints', 'terrain_solver', 'terrain_specific', 'terrain_generation']]
        references = dict(constraints=region[:2], solver=region[:3], specific=region[:4], generation=region,
            generated=region+[model(n) for n in ['terrain_catalog', 'terrain_generated', 'terrain_region', 'terrain_sections']]+[ROOT/'assets/region_recipe.cpp'],
            traversal=[model('terrain_traversal'), model('terrain_submission'), model('sprite_queue')],
            initializer=[model('graphics_initializer')])
        flags = ['g++', '-std=c++17', '-I'+str(ROOT/'reconstruction/rendering'), '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections']
        qt = shlex.split(run('qt-core-flags', ['pkg-config', '--cflags', '--libs', 'Qt6Core']))
        for name, models in references.items():
            src = ROOT/('tests/alternate-canvas-initializer.cpp' if name == 'initializer' else 'tests/terrain-'+name+'-reference.cpp')
            binary = out/(name+'-original')
            run(name+'-original-build', flags+['-m32', '-O2', '-fno-pie', '-no-pie', src, *models, '-o', binary])
            arguments = [exe] if name in {'traversal', 'initializer'} else [exe, catalogs, *cfgs] if name == 'generated' else [exe, header_file]
            value = run(name+'-original', [binary, *arguments], True)
            if name != 'initializer' and not value['all_match']:
                raise ValueError('Original mismatch: '+name)
            report['original'][name] = value
            if name not in {'traversal', 'initializer'}:
                native = out/(name+'-sanitized-corpus')
                extras = [ROOT/'assets'/p for p in ['asset_file.cpp', 'path_resolver.cpp', 'packed_container.cpp']] if name == 'generated' else []
                run(name+'-corpus-build', flags+['-DMNM_NATIVE_ONLY', '-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie', src, *models, *extras, *(qt if extras else []), '-o', native])
                actual = run(name+'-corpus', [native, *arguments], True)
                if actual != value:
                    raise ValueError('Native sanitizer corpus differs: '+name)
                report['native'][name+'-corpus'] = actual
        creature_src = [ROOT/'tests/terrain-creature-lighting-reference.cpp', model('terrain_lighting'), model('terrain_creature_lighting')]
        creature = []
        for mode, options in [('original', ['-m32', '-O2', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE']),
                              ('native', ['-O2']), ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'])]:
            binary = out/('creature-'+mode)
            run('creature-'+mode+'-build', flags+options+creature_src+['-o', binary])
            data = out/('creature-'+mode+'.bin')
            stats = run('creature-'+mode, [binary, *([exe] if mode == 'original' else []), data], True)
            creature.append(dict(stats=stats, payload_sha256=digest(data)))
        if creature[1:] != creature[:1]*2:
            raise ValueError('Creature buffers/state differ')
        report['original']['creature'] = creature
        binary = out/'lighting-original'
        run('lighting-original-build', flags+['-m32', '-O2', '-fno-pie', '-no-pie', ROOT/'tests/terrain-lighting-reference.cpp', model('terrain_lighting'), '-o', binary])
        fixtures = [(32, 32, 5, '50', str(ramp), '0,0,0,17;31,31,4,12') for ramp in [-127, -16, 0, 7, 9, 127]]
        fixtures += [(size, size+1, 3, '50', '7', f'0,0,0,{size};{size-1},{size},2,{size}') for size in range(2, 18)]
        fixtures += [(18, 19, 5, ambient, ramp, '0,0,0,12;17,18,4,17') for ambient, ramp in [('missing', 'missing'), ('2147483647', '-2147483648'), ('-2147483648', '2147483647'), ('0', '0')]]
        lights = []
        for i, fixture in enumerate(fixtures):
            prefix = out/('lighting-'+str(i))
            stats = run('lighting-'+str(i), [binary, exe, *fixture, prefix], True)
            lights.append(dict(fixture=fixture, stats=stats, buffers_sha256=digest(Path(str(prefix)+'.buffers')), kernels_sha256=digest(Path(str(prefix)+'.kernels'))))
        report['original']['lighting'] = lights
        # Production native provider proof uses the actual orient callback.
        project = reuse/'facing-project'
        project.mkdir(exist_ok=True)
        project_text = ('cmake_minimum_required(VERSION 3.16)\nproject(native_facing_controls LANGUAGES CXX)\n'+
            f'add_subdirectory("{ROOT}/apps/command-session" command-session)\n'+
            f'add_executable(native-world-facing "{ROOT}/tests/native-world-facing-test.cpp")\n'+
            'target_link_libraries(native-world-facing PRIVATE mnm-native-command-session)\n'+
            'target_compile_options(native-world-facing PRIVATE -Wall -Wextra -Werror -pedantic)\n')
        if (project/'CMakeLists.txt').exists():
            if (project/'CMakeLists.txt').read_text() != project_text:
                raise ValueError('Reused facing project differs')
        else:
            (project/'CMakeLists.txt').write_text(project_text)
        build = reuse/'facing-build'
        run('facing-configure', ['cmake', '-S', project, '-B', build, '-DCMAKE_BUILD_TYPE=Release'])
        run('facing-build', ['cmake', '--build', build, '--target', 'native-world-facing', '-j2'], timeout=1800)
        report['native']['production-facing'] = run('production-facing', [build/'native-world-facing'], True)
        report['native']['production-facing-compiled'] = dependencies(build, 'facing')
        preview = reuse/'preview-build'
        run('preview-configure', ['cmake', '-S', ROOT/'apps/terrain-preview', '-B', preview, '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF'])
        run('preview-build', ['cmake', '--build', preview, '--target', 'mnm-terrain-preview', '-j2'])
        run('preview-help', [preview/'mnm-terrain-preview', '--help'])
        report['native']['preview-compiled'] = dependencies(preview, 'terrain')
        report['native_all_passed'] = True
        report['original_all_passed'] = True
        report['sources_stable'] = all(digest(ROOT/p) == h for binding in report['bindings'].values() for p, h in binding['sources'].items())
        report['inputs_stable'] = all(digest(Path(p)) == h for p, h in report['inputs'].items())
        if not report['sources_stable'] or not report['inputs_stable']:
            raise ValueError('Execution inputs changed')
        report['success'] = True
    except BaseException as error:
        report['failure'] = dict(type=type(error).__name__, message=str(error))
        raise
    finally:
        try:
            run('manifest-after', [main_root/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = verified_before
        except BaseException:
            report['success'] = False
            raise
        finally:
            write_report(ROOT, report_path, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, report=str(report_path), native_boundaries=10,
                          original_domains=9, installed_headers=len(headers), lighting_fixtures=len(lights))))


if __name__ == '__main__':
    main()
