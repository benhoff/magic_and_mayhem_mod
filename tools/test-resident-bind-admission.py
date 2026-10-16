#!/usr/bin/env python3
"""Actual PE32 resident bind against a briefly held copy/tracker admission domain."""
import argparse
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'protocols/python'))
from mnm_protocols import frame_v1, render_commands_v2 as ring


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--expect-refusal', action='store_true', help='Retain the pre-fix negative control')
    parser.add_argument('--publication-probe', action='store_true', help='Simulate100ms delay only if hooked Unlock pumps on its calling thread')
    parser.add_argument('--expect-inline-publication', action='store_true', help='Retain the pre-isolation publication negative control')
    args = parser.parse_args()
    if args.expect_inline_publication and not args.publication_probe:
        parser.error('--expect-inline-publication requires --publication-probe')
    declaration = json.loads(args.claims.read_text())
    sources = dict(declaration['sources'])
    for name, expected in sources.items():
        if Path(name).is_absolute() or not (ROOT/name).resolve().is_relative_to(ROOT) or sha(ROOT/name) != expected:
            raise ValueError('Invalid or changed declared source: '+name)
    parent = ROOT/'working/tests/resident-bind-admission'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    negative_control = args.expect_refusal or args.expect_inline_publication
    report = dict(success=False, sources=sources, claims=declaration['claims'], expected_refusal=args.expect_refusal,
                  publication_probe=args.publication_probe, expected_inline_publication=args.expect_inline_publication,
                  scope='Synthetic actual PE32 exported resident binding, delayed copy/tracker ownership, original Lock/Unlock results and LastError, poisoned borrowed pixels and ordered owned V5 bytes. No original game, GPU execution, public cadence or pixel equivalence.')
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(out/'wineprefix'), WINEDEBUG='-all')
    child = None
    try:
        spec = importlib.util.spec_from_file_location('bind_build', ROOT/'tools/build-render-bridge.py')
        builder = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(builder)
        dll = builder.build(True)
        shutil.copyfile(dll, out/dll.name)
        flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
                 '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror']
        subprocess.run([*flags, '-c', str(ROOT/'tests/resident-bind-admission.c'), '-o', str(out/'probe.obj')], check=True)
        subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                        '/safeseh:no', '/timestamp:0', '/out:'+str(out/'probe.exe'), str(out/'probe.obj'),
                        str(dll.with_suffix('.lib')), str(dll.parent/'kernel32.lib')], check=True)
        channel, frame, capture = out/'commands.bin', out/'frame.bin', out/'capture'
        capture.mkdir()
        header = bytearray(ring.initial_header())
        struct.pack_into('<I', header, 16, 123)
        for path, initial, size in [(channel, header, ring.SIZE), (frame, frame_v1.initial_header(), frame_v1.SIZE)]:
            with path.open('xb') as file:
                file.write(initial)
                file.truncate(size)
        def wine_path(path):
            return 'Z:'+str(path).replace('/', '\\')
        env.update(MNM_RENDER_COMMAND_CHANNEL=wine_path(channel), MNM_RENDER_STREAM=wine_path(frame),
                   MNM_RENDER_LOCK_CAPTURE_DIR=wine_path(capture), MNM_RENDER_CONTINUOUS='1',
                   MNM_RENDER_ORDERED_COPIES='1', MNM_PUBLIC_RESIDENT_WORLD='1',
                   MNM_RENDER_SESSION_ARCHIVE='1', MNM_RENDER_AUTO_SHUTDOWN='0')
        if args.publication_probe:
            env['MNM_BIND_PUBLICATION_PROBE']='1'
        collected = bytearray()
        consumed = 0
        with channel.open('r+b') as file, mmap.mmap(file.fileno(), 0) as mapping, (out/'wine.log').open('x') as log:
            child = subprocess.Popen(['wine', str(out/'probe.exe')], cwd=out, env=env, stdout=log, stderr=log)
            deadline = time.monotonic()+45
            while child.poll() is None:
                if time.monotonic() >= deadline:
                    raise RuntimeError('Resident admission fixture deadline expired')
                published = struct.unpack_from('<I', mapping, 20)[0]
                if published < consumed or published-consumed > ring.CAPACITY:
                    raise RuntimeError('Invalid ring publication')
                count = published-consumed
                offset = consumed % ring.CAPACITY
                first = min(count, ring.CAPACITY-offset)
                collected.extend(mapping[64+offset:64+offset+first])
                collected.extend(mapping[64:64+count-first])
                consumed = published
                struct.pack_into('<I', mapping, 36, consumed)
                time.sleep(.002)
            if child.returncode:
                raise RuntimeError('PE32 fixture failed: '+str(child.returncode))
            state, reason = struct.unpack_from('<II', mapping, 24)
            assert state == 2 and reason == 0 and struct.unpack_from('<I', mapping, 20)[0] == consumed
        result = struct.unpack('<13I', (out/'bind-result.bin').read_bytes())
        assert result[:6] == (int(not args.expect_refusal), 0x77, 1, 1, 1, 1), result
        assert (result[10] >= 1 if args.expect_inline_publication else result[10] == 0), result
        archive = (capture/'session-00000001.bin').read_bytes()
        assert archive == collected and archive[:16] == b'MNMCMD05'+struct.pack('<II', 5, 16)
        at, records = 16, []
        while at < len(archive):
            op, sequence, size = struct.unpack_from('<III', archive, at)
            assert sequence == len(records)+1 and size <= len(archive)-at-12
            fields = archive[at+12:at+12+size]
            records.append((op, fields))
            at += 12+size
        assert at == len(archive)
        world = [fields for op,fields in records if op == 19]
        assert len(world) == int(not args.expect_refusal)
        if world:
            assert struct.unpack('<5I', world[0]) == (1, 1, 0, 17, 34)
        assert [op for op,_ in records] == ([1, 6, 7, 8] if args.expect_refusal else [1, 19, 6, 7, 8])
        assert len(records[0][1]) == 28+800*600*2 and not any(records[0][1][28:])
        elapsed = result[6] | result[7] << 32
        frequency = result[8] | result[9] << 32
        unlock_elapsed = result[11] | result[12] << 32
        unlock_ms=unlock_elapsed*1000/frequency
        if args.expect_inline_publication:
            assert unlock_ms >= 90, unlock_ms
        report.update(success=not negative_control, negative_control_verified=negative_control,
                      original_locks=result[2], original_unlocks=result[3],
                      last_error=result[1], bind_succeeded=bool(result[0]), complete_shutdown=True,
                      bind_ms=elapsed*1000/frequency, opcodes=[op for op,_ in records],
                      callback_pump_calls=result[10], unlock_ms=unlock_ms,
                      fifo_archive_equal=True, source_pixels_owned_before_poison=True,
                      source_stable=all(sha(ROOT/name)==expected for name,expected in sources.items()),
                      executable_sha256=sha(out/'probe.exe'), dll_sha256=sha(out/dll.name),
                      compiler_manifest=json.loads((dll.parent/'manifest.json').read_text()))
        assert report['source_stable']
    except (OSError, ValueError, AssertionError, RuntimeError, subprocess.SubprocessError) as error:
        report.update(success=False, error=str(error))
    finally:
        if child is not None and child.poll() is None:
            child.terminate()
            child.wait(timeout=5)
        subprocess.run(['wineserver', '-k'], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=15, check=False)
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] or report.get('negative_control_verified') and report.get('source_stable') else 1


if __name__ == '__main__':
    raise SystemExit(main())
