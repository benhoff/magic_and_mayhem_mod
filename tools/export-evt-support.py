#!/usr/bin/env python3
"""Hash-pinned No-CD EVT evidence and installed placement inventory; read-only."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {'reset': (0x49ee50, 0x49ee74), 'reader': (0x49ee80, 0x49f016),
          'append': (0x49f020, 0x49f0f9), 'writer': (0x49f100, 0x49f24b),
          'section_load_call': (0x4ee02a, 0x4ee0c4),
          'area_transform': (0x4ef2c0, 0x4ef5ce)}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--decompile', action='store_true')
    parser.add_argument('--project', type=Path, default=REPO / 'working/decompiled/nocd-96ofmeq_/project')
    args = parser.parse_args()
    script_digest = sha(Path(__file__).read_bytes())
    parent = REPO / 'working/decompiled'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='evt-support-', dir=parent))
    print(f'EVT evidence: {out}', flush=True)

    def verify(phase):
        run = subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True, timeout=120)
        (out / f'original-{phase}.log').write_text(run.stdout + run.stderr)
        print(run.stdout, end='', flush=True)
        run.check_returncode()

    verify('before')
    try:
        exe = REPO / 'working/game-nocd/Chaos.exe'
        binary = exe.read_bytes()
        if sha(binary) != HASH:
            raise ValueError('Unsupported executable hash')
        for name, (start, end) in RANGES.items():
            asm = subprocess.check_output(['objdump', '-d', '-Mintel', f'--start-address={start}', f'--stop-address={end}', str(exe)], text=True)
            (out / f'{name}.asm').write_text(asm)
        files = []
        root = REPO / 'working/game-clean'
        for path in sorted(root.rglob('*')):
            if not path.is_file() or path.suffix.lower() != '.evt':
                continue
            data = path.read_bytes()
            magic, size_word, version, count = struct.unpack_from('<4I', data)
            assert magic == 0x00545645 and version == 1
            assert len(data) == 16 + 72 * count
            assert size_word == 16 + 16 * count
            records = list(struct.iter_unpack('<6i48s', data[16:]))
            files.append(dict(path=path.relative_to(root).as_posix(), source_bytes=len(data),
                              source_sha256=sha(data), header_size_word=size_word, version=version,
                              record_count=count, reversed_endpoints=sum(any(r[i] > r[i+3] for i in range(3)) for r in records),
                              nonzero_name_tail=sum(b'\0' in r[6] and any(r[6].split(b'\0', 1)[1]) for r in records)))
        if args.decompile:
            candidates = sorted((REPO / 'working/toolchain').glob('ghidra*/support/analyzeHeadless'))
            if not candidates:
                raise ValueError('No workspace Ghidra launcher')
            env = os.environ.copy()
            env['XDG_CONFIG_HOME'] = str(out / 'config')
            env['XDG_CACHE_HOME'] = str(out / 'cache')
            env['GHIDRA_HEADLESS_JAVA_OPTIONS'] = f'-Dapplication.cachedir={out / "cache"} -Dapplication.tempdir={out / "tmp"}'
            command = [str(candidates[0]), str(args.project), 'MagicMayhem', '-process', 'Chaos.exe',
                       '-readOnly', '-noanalysis', '-scriptPath', str(REPO / 'tools/ghidra'),
                       '-postScript', 'ExportEvtSupport.java', str(out / 'pseudo'),
                       '49ee80', '49f020', '49f100', '4ee0ba', '4ef2c0', '-max-cpu', '2']
            with (out / 'ghidra.log').open('w') as log:
                subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
            if not all((out / f'pseudo/{entry}.c').exists() for entry in
                       ('0049ee80', '0049f020', '0049f100', '004eda90', '004ef2c0')):
                raise ValueError('Ghidra reader export missing; inspect ghidra.log')
        if sha(exe.read_bytes()) != HASH:
            raise ValueError('Executable changed')
        for item in files:
            if sha((root / item['path']).read_bytes()) != item['source_sha256']:
                raise ValueError('Installed input changed')
        report = dict(executable_sha256=HASH, script_sha256=script_digest,
                      scope='Static selected reader/caller evidence and installed byte inventory; no live observation',
                      input_unchanged=True, ranges=RANGES,
                      summary=dict(files=len(files), source_bytes=sum(f['source_bytes'] for f in files),
                                   records=sum(f['record_count'] for f in files)), files=files,
                      artifacts_sha256={p.relative_to(out).as_posix(): sha(p.read_bytes()) for p in out.rglob('*')
                                        if p.is_file() and p.suffix in ('.asm', '.c', '.refs')})
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report['summary']), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
