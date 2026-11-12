#!/usr/bin/env python3
"""Compare opt-in packed colour World input with unchanged original547d80."""
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
from native_render_driver import require_nvidia

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def declaration():
    spec = importlib.util.spec_from_file_location('claims555', ROOT/'tools/draft-coverage-claims.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    value = module.declaration(ROOT, register, ['NR.world-rgb555-colour-input'],
                               ['world-rgb555-colour-input-20261026'])
    for folder in ('renderer', 'assets', 'compat/legacy', 'protocols/include', 'reconstruction/rendering'):
        for path in (ROOT/folder).rglob('*'):
            if path.is_file() and (path.suffix in ('.c', '.h', '.cpp', '.hpp', '.S', '.cmake') or path.name == 'CMakeLists.txt'):
                value['sources'][path.relative_to(ROOT).as_posix()] = sha(path)
    for name in ('tools/test-world-colour-555.py', 'tests/world-colour-555-test.cpp',
                 'tests/world-colour-555-reference.cpp', 'tests/sprite-binary-reference.cpp',
                 'tests/resource-fixtures.hpp', 'tools/native-render-profile/CMakeLists.txt',
                 'tools/native-render-profile/egl_platform.cpp', 'tools/native-render-profile/headless.json',
                 'tools/native_render_driver.py', 'tools/original-manifest.sh'):
        value['sources'][name] = sha(ROOT/name)
    return value


def fixture(width, height, mode, clipped, uniform, rgb555):
    records = []
    mask = 0x7fff if rgb555 else 0xffff

    def colour(x, y, w, h, selected_mode, value=None, clip=None):
        words = [((value if value is not None else 0x1357+i*7919) & mask) for i in range(w*h)]
        header = [0]*16
        header[0:8] = [76+w*h*2, 7, x & 0xffffffff, y & 0xffffffff,
                       *(clip or (0, 0, width, height))]
        header[10], header[13] = 12+w*h*2, 12
        records.append(struct.pack('<16I3I', *header, w, h, selected_mode)+struct.pack('<'+'H'*len(words), *words))

    # A declared all-one owned background leaves an untouched high-bit border.
    # Native receives no original destination image. Fill tiles own their words.
    for y in range(1, height-1, 64):
        for x in range(1, width-1, 64):
            colour(x, y, min(64, width-1-x), min(64, height-1-y), 0, 0x5235)
    w, h = min(width, 64), min(height, 64)
    x, y = (-3, -2) if clipped else (width//3, height//3)
    clip = (1, 1, width-1, height-1) if clipped else (0, 0, width, height)
    for offset in (0, 1, 2):
        colour(x+offset, y+offset, w, h, mode, 0x6d35 if uniform else None, clip)
    header = bytearray(80)
    header[:8] = b'MNMWRLD1'
    struct.pack_into('<10I', header, 8, 1, 80, 80+sum(map(len, records)), 1,
                     width, height, width, len(records), 0, 0x40209ca7)
    struct.pack_into('<I', header, 72, int(rgb555))
    return bytes(header)+b''.join(records)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', type=Path, required=True)
    p.add_argument('--backend', choices=('software', 'nvidia-egl'), default='software')
    args = p.parse_args()
    args.report = args.report.absolute()
    if args.report.exists():
        p.error('Refusing evidence overwrite')
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    parent = ROOT/'working/tests/world-colour-555'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    prospective = declaration()
    selectors = ('DISPLAY', 'WAYLAND_DISPLAY', 'LIBGL_ALWAYS_SOFTWARE', 'GALLIUM_DRIVER',
                 'MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                 '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_QPA_PLATFORM_PLUGIN_PATH',
                 'QT_XCB_GL_INTEGRATION', 'MNM_PROFILE_EGL_VENDOR')
    env = {k:v for k,v in os.environ.items() if k not in selectors and not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4')
    report = dict(schema=1, success=False, sources=prospective['sources'], claims=prospective['claims'],
                  backend=args.backend, original_sha256=BUILD, experiment=str(out.relative_to(ROOT)),
                  commands=[], cases=[], original_pixels_used_as_native_inputs=False,
                  actual_game_journey=False, hardware_presentation=False, live_replacement=False)

    def run(label, command, timeout=900):
        result = subprocess.run(list(map(str, command)), cwd=ROOT, env=env, capture_output=True, text=True, timeout=timeout)
        log = out/(label+'.log')
        log.write_text(result.stdout+result.stderr)
        report['commands'].append(dict(label=label, argv=list(map(str, command)), returncode=result.returncode,
                                       log=str(log.relative_to(ROOT)), sha256=sha(log)))
        print(label, result.returncode, flush=True)
        if result.returncode:
            raise ValueError(label+': '+(result.stdout+result.stderr)[-2500:])
        return result.stdout

    before = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        before = True
        original = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(original) != BUILD:
            raise ValueError('Unsupported original executable')
        build = ROOT/'working/build/world-colour-555'
        run('configure', ['cmake', '-S', ROOT/'compat/legacy', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--target', 'world-colour-555-test',
                      'world-frame-test', 'mnm-world-frame-preview', '-j2'])
        executable = next(build.rglob('world-colour-555-test'))
        run('default-world-controls', ['xvfb-run', '-a', next(build.rglob('world-frame-test'))])
        run('reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                                '-fno-pie', '-no-pie', ROOT/'tests/world-colour-555-reference.cpp', '-o', out/'reference'])
        inputs, expected, native = out/'owned-inputs', out/'original-planes', out/'native-planes'
        inputs.mkdir()
        expected.mkdir()
        for rgb555 in (False, True):
            for width, height in ((9, 7), (65, 65), (127, 73), (800, 600)):
                for mode in range(4):
                    for clipped in (False, True):
                        for uniform in (False, True):
                            name = f'case-{len(report["cases"]):03d}.world'
                            path = inputs/name
                            path.write_bytes(fixture(width, height, mode, clipped, uniform, rgb555))
                            run(name+'-original', [out/'reference', original, path, expected/name])
                            report['cases'].append(dict(name=name, width=width, height=height, mode=mode,
                                clipped=clipped, uniform=uniform, format='RGB555' if rgb555 else 'RGB565',
                                input_sha256=sha(path), original_sha256=sha(expected/name)))
        if args.backend == 'nvidia-egl':
            plugin = ROOT/'working/build/colour-coverage-egl'
            run('egl-configure', ['cmake', '-S', ROOT/'tools/native-render-profile', '-B', plugin, '-DCMAKE_BUILD_TYPE=Debug'])
            run('egl-build', ['cmake', '--build', plugin, '--target', 'mnm_headless_egl', '-j2'])
            env.pop('LIBGL_ALWAYS_SOFTWARE', None)
            env.pop('GALLIUM_DRIVER', None)
            env.update(QT_QPA_PLATFORM='mnm-headless-egl', MNM_PROFILE_EGL_VENDOR='NVIDIA',
                       __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json',
                       QT_QPA_PLATFORM_PLUGIN_PATH=str(plugin/'platforms'))
            report['egl_plugin_sha256'] = sha(plugin/'platforms/libmnm_headless_egl.so')
        launch = ['xvfb-run', '-a'] if args.backend == 'software' else []
        stdout = run('native-world', [*launch, executable, inputs, native])
        report['native_controls'] = json.loads(stdout.strip().splitlines()[-1])
        if args.backend == 'nvidia-egl':
            driver = report['native_controls']['driver']
            if 'NVIDIA' not in driver['vendor']:
                raise ValueError('Unexpected hardware vendor')
            report['hardware_context'] = require_nvidia([driver['renderer']])
        for case in report['cases']:
            for suffix in ('.plain', '.cached'):
                output = native/(case['name']+suffix)
                case[suffix[1:]+'_sha256'] = sha(output)
                if output.read_bytes() != (expected/case['name']).read_bytes():
                    raise ValueError('Complete packed words differ: '+case['name']+suffix)
            case['equal'] = True
        # Exercise the public offline preview with explicit opt-in and export tag.
        asset_root = out/'empty-assets'
        asset_root.mkdir()
        bindings = out/'bindings.json'
        bindings.write_text('[]\n')  # Bounded authored input consumed by the public preview.
        first555 = inputs/next(c['name'] for c in report['cases'] if c['format']=='RGB555')
        preview = next(build.rglob('mnm-world-frame-preview'))
        run('offline-preview', [*launch, preview, '--root', asset_root, '--bindings', bindings,
            '--snapshot', first555, '--background', '65535', '--rgb555-colour', '--output', out/'preview'])
        if (out/'preview.555').read_bytes() != (expected/first555.name).read_bytes():
            raise ValueError('Public RGB555 preview output differs')
        report.update(success=True, independent_original_pixels_match=True, native_integration=True,
                      pixels_compared=sum(c['width']*c['height']*2 for c in report['cases']),
                      native_binary_sha256=sha(executable), reference_binary_sha256=sha(out/'reference'),
                      scope='Explicit offline colour-only RGB555 and RGB565 World requests, exact original547d80 output words, cached/uncached ordered modes0..3, clipped draws, owned fills and untouched high-bit borders. Default/live consumers remain RGB565-only; sprites/effects, natural reachability, physical presentation and replacement remain unaccepted.')
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = before
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = declaration() == prospective
        report['success'] = report['success'] and report['sources_stable']
        write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(args.report, flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
