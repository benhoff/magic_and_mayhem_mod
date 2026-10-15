#!/usr/bin/env python3
"""Compare source-only native JPEG cache painting with installed original JPEG.DLL."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

from jpeg_pixel_cache import stage, decoder
from pixel_backend_launch import load

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path, help='Reuse a stopped owned test prefix; no cold-start claim')
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p, h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/jpeg-pixels'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, original_sha256=HASH, sources=claims['sources'], claims=claims['claims'],
        scope='Installed static JPEG corpus: native source-only RGB cache and production PE32 decoder-call patch versus original JPEG.DLL through unchanged mapped original58d320 wrapper bytes; supplied Lock/Unlock exact counts/receivers/pointers, both RGB565/555 and padded larger/clipped targets, full physical guards and native3008/original0 bypass counters. Caller volatile external-decoder GPR/FX history, actual game/driver integration, malformed/dynamic JPEG and live replacement excluded.',
        experiment=str(out.relative_to(ROOT)), live_replacement=False, original_work_bypassed=False)
    commands = []
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEDEBUG='-all', LIBGL_ALWAYS_SOFTWARE='1', WINEPREFIX=str(out/'wineprefix'))
    if args.wine_prefix:
        prefix = args.wine_prefix.resolve()
        if not prefix.is_relative_to(ROOT/'working/tests') or not prefix.is_dir() or prefix.is_symlink():
            raise ValueError('Reused prefix must be an owned test directory')
        env['WINEPREFIX'] = str(prefix)
        report['reused_wine_prefix'] = str(prefix.relative_to(ROOT))

    def run(label, argv, extra=None, cwd=None):
        with (out/(label+'.log')).open('x') as log:
            r = subprocess.run([str(a) for a in argv], cwd=cwd or ROOT, env={**env, **(extra or {})},
                               stdout=log, stderr=log, timeout=300)
        commands.append(dict(label=label, argv=[str(a) for a in argv], returncode=r.returncode,
                             log=str((out/(label+'.log')).relative_to(ROOT))))
        r.check_returncode()
    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        if sha(ROOT/'original/Arcane_Nocd/Chaos.exe')!=HASH:
            raise ValueError('Original build identity changed')
        game = out/'game'
        game.mkdir()
        source = ROOT/'working/game-nocd'
        for p in source.rglob('*'):
            if p.is_file() and (p.suffix.lower() in ('.jpg', '.jpeg') or p.name.lower()=='jpeg.dll'):
                target = game/p.relative_to(source)
                target.parent.mkdir(parents=True, exist_ok=True)
                if p.is_symlink() or not p.resolve().is_relative_to(source.resolve()):
                    raise ValueError('Installed source escapes game')
                shutil.copyfile(p, target)
        report['original_jpeg_dll_sha256'] = sha(game/'jpeg.dll')
        original = (ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        offset = load('jpeg_wrapper_offsets','tools/export-menu-support.py').image_offset(original,0x58d320,77)
        (game/'jpeg-wrapper.bin').write_bytes(original[offset:offset+77])
        report['cache'] = stage(out, decoder())
        run('native-dll', ['python3', ROOT/'tools/build-jpeg-pixels.py'])
        shutil.copyfile(ROOT/'working/build/jpeg-pixels/MnmJpegPixels.dll', game/'MnmJpegPixels.dll')
        if not args.wine_prefix:
            run('prefix', ['cp', '-a', '--reflink=auto', (ROOT/'working/wineprefix-x86_64').resolve(), out/'wineprefix'])
        imports = {'LoadLibraryA': 4, 'GetProcAddress': 8, 'GetEnvironmentVariableA': 12, 'GetFileSize': 8,
                   'CreateFileA': 28, 'CloseHandle': 4, 'CreateFileMappingA': 24, 'MapViewOfFile': 20,
                   'VirtualAlloc': 16, 'VirtualProtect': 16, 'ReadFile': 20, 'WriteFile': 20, 'ExitProcess': 4}
        definition = out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n, s in imports.items()))
        run('imports', ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', out/'kernel32.lib'])
        run('probe-build', ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
            '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
            '-c', ROOT/'tests/jpeg-pixels-probe.c', '-o', out/'probe.obj'])
        run('probe-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
            '/safeseh:no', '/timestamp:0', '/out:'+str(game/'probe.exe'), out/'probe.obj', out/'kernel32.lib'])
        extra = {'MNM_JPEG_PIXELS': 'Z:'+str(out/'jpeg-pixels-v1.bin').replace('/', '\\'),
                 'MNM_JPEG_ROOT': 'Z:'+str(game).replace('/', '\\'), 'MNM_JPEG_TAKEOVER':'1'}
        run('original-native', ['xvfb-run', '-a', 'wine', game/'probe.exe'], extra, cwd=game)
        raw = (game/'jpeg-results.bin').read_bytes()
        count = report['cache']['decoded_images']
        if len(raw)!=count*4*8*4:
            raise ValueError('Incomplete original/native JPEG output')
        names = list(report['cache']['inputs'])
        mismatch, words = [], 0
        for i in range(count):
            for k in range(4):
                row = struct.unpack_from('<8I', raw, (i*4+k)*32)
                words += row[2]
                if row[0]!=1 or row[1]!=1 or row[3]:
                    mismatch.append(dict(path=names[i], case=k, original_return=row[0], native_return=row[1], words=row[2], mismatches=row[3]))
        report.update(cases=count*4, physical_words=words, mismatches=mismatch, jpeg_results_sha256=sha(game/'jpeg-results.bin'))
        report.update(isolated_hook_bypass=True,native_decoder_calls=count*4,original_decoder_forwards=0,
                      original_wrapper_sha256=sha(game/'jpeg-wrapper.bin'),lock_unlock_cases=count*8)
        report['success'] = not mismatch
    except Exception as error:
        report['error'] = repr(error)
    finally:
        if Path(env['WINEPREFIX']).exists():
            subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False)
            subprocess.run(['wineserver', '-w'], env=env, timeout=10, check=False)
        try:
            run('manifest-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/p)==h for p, h in claims['sources'].items())
        report['inputs_stable'] = sha(ROOT/'original/Arcane_Nocd/Chaos.exe')==HASH and all(
            sha(out/'game'/v['path'])==v['source_sha256'] for v in report.get('cache', {}).get('inputs', {}).values())
        if report.get('original_jpeg_dll_sha256'):
            report['inputs_stable'] = report['inputs_stable'] and sha(out/'game/jpeg.dll')==report['original_jpeg_dll_sha256']
        report['success'] = report['success'] and report['sources_stable'] and report['inputs_stable']
        report['commands'] = commands
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
