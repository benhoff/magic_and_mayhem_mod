#!/usr/bin/env python3
"""Export bounded pinned original copy caller windows without executing them."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
WINDOWS = {
    'shared_menu_tick_context': (0x5595d0, 0x559850),
    'ordinary_frame_copy_context': (0x46b970, 0x46ba24),
    'cursor_background_save_context': (0x4a4bf0, 0x4a4c80),
    'cursor_restore_helpers': (0x4a4370, 0x4a44ae),
    'rectangular_copy_receiver': (0x58c360, 0x58c492),
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--executable', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    helper = ROOT / 'tools/export-menu-support.py'
    spec = importlib.util.spec_from_file_location('copy_pe', helper)
    pe = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(pe)
    sources = {name: sha((ROOT / name).read_bytes()) for name in
               ['tools/inspect-drawing-return-copy.py', 'tools/export-menu-support.py']}
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        raw = pe.pinned_image(a.executable)
        windows = {}
        for name, (start, end) in WINDOWS.items():
            at = pe.image_offset(raw, start, end - start)
            data = raw[at:at + end - start]
            result = subprocess.run(['objdump', '-d', '-Mintel', '--wide',
                                     '--start-address=' + hex(start),
                                     '--stop-address=' + hex(end), str(a.executable)],
                                    check=True, capture_output=True, text=True)
            windows[name] = dict(start=start, end=end, file_offset=at,
                                 bytes_hex=data.hex(), sha256=sha(data),
                                 disassembly=result.stdout)
        calls = []
        for call, target, receiver, destination in [
                (0x5597f8, 0x58bf40, 0x6a49e0, 0x6a4fe0),
                (0x46b9fc, 0x58bf40, 0x6a49e0, 0x6a4fe0),
                (0x4a4c21, 0x58c360, 0x6a4fe0, 0x641fc8)]:
            at = pe.image_offset(raw, call - 10, 15)
            expected = (b'\x68' + destination.to_bytes(4, 'little') + b'\xb9' +
                        receiver.to_bytes(4, 'little') + b'\xe8' +
                        (target - call - 5).to_bytes(4, 'little', signed=True))
            if raw[at:at + 15] != expected:
                raise ValueError('Exact copy receiver/destination/call bytes changed')
            calls.append(dict(call=call, return_address=call + 5, target=target,
                              receiver_source_wrapper=receiver,
                              first_argument_destination_wrapper=destination))
        if sha(a.executable.read_bytes()) != pe.HASH or any(
                sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
            raise ValueError('Pinned executable or inspection source changed')
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    report = dict(schema=1, success=True, sources=sources,
                  source_executable_sha256=pe.HASH, executable=str(a.executable),
                  windows=windows, exact_copy_calls=calls,
                  original_manifest_verified_before_after=True,
                  original_code_executed=False, function_boundaries_recovered=False,
                  branch_execution_proved=False, source_backing_owned=False,
                  scope='Static bounded copy caller windows only. Retain undiscovered function and indirect-flow debt; no original execution, full menu semantics, ordering repair or source ownership.')
    with a.report.open('x') as output:
        json.dump(report, output, indent=2)
        output.write('\n')
    print(json.dumps(dict(success=True, windows=len(windows), original_code_executed=False)))


if __name__ == '__main__':
    main()
