#!/usr/bin/env python3
"""Compare colour boundaries and ordered native World fills with pinned originals."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import resource
import struct
import subprocess
import tempfile

from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def declaration(world_only=False):
    spec = importlib.util.spec_from_file_location('colour_claims', ROOT/'tools/draft-coverage-claims.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    value = module.declaration(ROOT, json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),
                               ['NR.world-constant-fill'] if world_only else
                               ['RS.colour-rectangle-pixels', 'NP.colour-rectangle-admission', 'NR.world-constant-fill'],
                               ['NR.world-constant-fill.original-order'])
    for folder in ('renderer', 'assets', 'compat/legacy', 'protocols/include', 'reconstruction/rendering'):
        for path in (ROOT/folder).rglob('*'):
            if path.is_file() and (path.suffix in ('.c', '.h', '.cpp', '.hpp', '.S', '.cmake') or path.name == 'CMakeLists.txt'):
                value['sources'][path.relative_to(ROOT).as_posix()] = digest(path)
    for name in ('tools/test-colour-coverage.py', 'apps/drawing-session/CMakeLists.txt',
                 'tools/native-render-profile/CMakeLists.txt', 'tools/native-render-profile/egl_platform.cpp',
                 'tools/native-render-profile/headless.json', 'tools/native_render_driver.py'):
        value['sources'][name] = digest(ROOT/name)
    return value


def fixture(width, height, mode, clipped, uniform):
    canvas_w, canvas_h = min(width+4, 2048), min(height+4, 2048)
    if clipped:
        canvas_w, canvas_h = min(canvas_w, 127), min(canvas_h, 95)
    records = []

    def command(op, *, area=None, colour=0, x=0, y=0, w=0, h=0, payload=b''):
        r = [0]*24
        r[0:4] = [96+len(payload), len(records)+1, op, 1]
        r[5:10] = [canvas_w, canvas_h, canvas_w, x & 0xffffffff, y & 0xffffffff]
        r[10:14] = [v & 0xffffffff for v in (area or (0, 0, canvas_w, canvas_h))]
        r[14] = 1 if op in (11, 12) else colour
        if op == 26:
            r[15], r[19], r[20] = mode, w, h
        r[18] = len(payload)
        records.append(struct.pack('<24I', *r)+payload)

    command(1)
    command(5, colour=0x1234)
    command(11)
    w, h = min(64, width), min(64, height)
    words = [0xd534 if uniform else (i*7919+0x1234) & 65535 for i in range(w*h)]
    payload = struct.pack('<'+'H'*len(words), *words)
    command(26, x=max(0, canvas_w-32), y=max(0, canvas_h-32), w=w, h=h, payload=payload)
    left, top = (-5, -3) if clipped else (0, 0)
    command(5, area=(left, top, left+width, top+height), colour=0xabcd)
    command(26, x=min(63, canvas_w-1), y=min(31, canvas_h-1), w=w, h=h, payload=payload)
    command(12)
    header = bytearray(64)
    header[:8] = b'MNMPRO03'
    struct.pack_into('<5I', header, 8, 3, 64, 0x40209ca7, 131072, 32)
    return bytes(header)+b''.join(records), canvas_w, canvas_h


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--backend', choices=('software', 'nvidia-egl'), default='software',
                        help='Explicit headless GPU backend; NVIDIA identity is required without fallback')
    parser.add_argument('--world-only', action='store_true', help='Compare World fixtures without repeating the independent CPU splice suite')
    args = parser.parse_args()
    args.report = args.report.absolute()
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    parent = ROOT/'working/tests/colour-coverage'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    prospective = declaration(args.world_only)
    claims = out/'claims.json'
    claims.write_text(json.dumps(prospective)+'\n')  # Physical input to the existing comparison producer.
    selectors = ('DISPLAY', 'WAYLAND_DISPLAY', 'LIBGL_ALWAYS_SOFTWARE', 'GALLIUM_DRIVER',
                 'MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                 '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_QPA_PLATFORM_PLUGIN_PATH',
                 'QT_XCB_GL_INTEGRATION', 'MNM_PROFILE_EGL_VENDOR')
    env = {k:v for k,v in os.environ.items() if k not in selectors}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe',
               LP_NUM_THREADS='4', QT_FORCE_STDERR_LOGGING='1')
    report = dict(schema=1, success=False, sources=prospective['sources'], claims=prospective['claims'],
                  original_sha256=BUILD, experiment=str(out.relative_to(ROOT)), commands=[], cases=[],
                  original_pixels_used_as_native_inputs=False, actual_game_journey=False,
                  hardware_presentation=False, live_replacement=False, full_renderer_accepted=False)
    report['backend'] = args.backend
    report['cpu_splice_executed'] = not args.world_only
    report['execution_environment'] = dict(uid=os.getuid(), mount_namespace=os.readlink('/proc/self/ns/mnt'),
                                           network_namespace=os.readlink('/proc/self/ns/net'), temporary_directory=tempfile.gettempdir())

    def run(label, argv, timeout=900, allowed=(0,)):
        p = subprocess.run(list(map(str, argv)), cwd=ROOT, env=env, capture_output=True, text=True, timeout=timeout)
        log = out/(label+'.log')
        log.write_text(p.stdout+p.stderr)
        report['commands'].append(dict(label=label, argv=list(map(str, argv)), returncode=p.returncode,
                                       log=str(log.relative_to(ROOT)), sha256=digest(log)))
        print(label, p.returncode, flush=True)
        if p.returncode not in allowed:
            raise ValueError(label+': '+(p.stdout+p.stderr)[-3000:])
        return p.stdout

    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        original = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if digest(original) != BUILD:
            raise ValueError('Unsupported original binary')
        if not args.world_only:
            stdout = run('cpu-splice', ['python3', ROOT/'tools/test-colour-rect-splice.py', '--claims', claims], allowed=(0, 1))
            cpu_path = Path(stdout.strip().splitlines()[-1])
            cpu = json.loads(cpu_path.read_text())
            cpu_record = args.report.with_name(args.report.stem+'-cpu.json')
            write_report(ROOT, cpu_record, cpu_path.read_bytes())
            if not cpu['success'] or not cpu['comparison']['boundaries']['caller_state_equal']:
                raise ValueError('Original CPU splice comparison refused')
            report['cpu'] = dict(record=str(cpu_record.relative_to(ROOT)), comparison=cpu['comparison'])
        build = ROOT/'working/build/colour-coverage'
        run('configure', ['cmake', '-S', ROOT/'apps/drawing-session', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--target', 'colour-world-coverage', 'drawing-scene-test', 'drawing-session-test', '-j2'])
        run('original-reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                                        '-fno-pie', '-no-pie', ROOT/'tests/colour-world-reference.cpp', '-o', out/'reference'])
        for name in ('drawing-scene-test', 'drawing-session-test'):
            run(name, ['xvfb-run', '-a', build/'composition'/name])
        if args.backend == 'nvidia-egl':
            plugin = ROOT/'working/build/colour-coverage-egl'
            run('egl-configure', ['cmake', '-S', ROOT/'tools/native-render-profile', '-B', plugin,
                                  '-DCMAKE_BUILD_TYPE=Debug'])
            run('egl-build', ['cmake', '--build', plugin, '--target', 'mnm_headless_egl', '-j2'])
            env.pop('LIBGL_ALWAYS_SOFTWARE', None)
            env.pop('GALLIUM_DRIVER', None)
            env.update(QT_QPA_PLATFORM='mnm-headless-egl', MNM_PROFILE_EGL_VENDOR='NVIDIA',
                       __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json',
                       QT_QPA_PLATFORM_PLUGIN_PATH=str(plugin/'platforms'))
            report['egl_plugin_sha256'] = digest(plugin/'platforms/libmnm_headless_egl.so')
        inputs, expected, native = out/'owned-inputs', out/'original-planes', out/'native-planes'
        inputs.mkdir()
        expected.mkdir()
        dimensions = [(1, 1), (2, 2), (63, 64), (64, 64), (65, 1), (1, 65), (65, 65), (127, 73), (800, 600), (2048, 1), (1, 2048), (2048, 2048)]
        for width, height in dimensions:
            for mode in range(4):
                for clipped in (False, True):
                    for uniform in (False, True):
                        raw, cw, ch = fixture(width, height, mode, clipped, uniform)
                        name = f'case-{len(report["cases"]):03d}.bin'
                        path = inputs/name
                        path.write_bytes(raw)
                        run(name+'-original', [out/'reference', original, path, expected/name])
                        report['cases'].append(dict(name=name, shape_width=width, shape_height=height, mode=mode,
                                                    clipped=clipped, uniform=uniform, width=cw, height=ch,
                                                    input_sha256=digest(path), original_sha256=digest(expected/name)))
        launch = ['xvfb-run', '-a'] if args.backend == 'software' else []
        stdout = run('native-world', [*launch, build/'composition/colour-world-coverage', inputs, native])
        report['native_controls'] = json.loads(stdout.strip().splitlines()[-1])
        if args.backend == 'nvidia-egl':
            from native_render_driver import require_nvidia
            driver = report['native_controls']['driver']
            if 'NVIDIA' not in driver['vendor']:
                raise ValueError('Native World context vendor is not NVIDIA')
            report['hardware_context'] = require_nvidia([driver['renderer']])
        for case in report['cases']:
            case['native_sha256'] = digest(native/case['name'])
            case['equal'] = (expected/case['name']).read_bytes() == (native/case['name']).read_bytes()
            if not case['equal']:
                raise ValueError('Original/native World mismatch: '+case['name'])
        report.update(success=True, independent_original_pixels_match=True, native_integration=True,
                      pixels_compared=sum(c['width']*c['height'] for c in report['cases']),
                      native_binary_sha256=digest(build/'composition/colour-world-coverage'),
                      original_reference_sha256=digest(out/'reference'),
                      scope='Independent original547d80 comparison with complete native GPU planes in authored ordered RGB565 World fill/blend fixtures. Optional CPU splice suite is reported separately when executed. The original game/World traversal is not run; natural live mode reachability, RGB555 World, physical presentation, performance and whole-renderer acceptance remain pending.')
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = declaration(args.world_only) == prospective
        report['success'] = report['success'] and report['sources_stable']
        write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(args.report, flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
