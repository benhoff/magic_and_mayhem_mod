#!/usr/bin/env python3
"""Hash-pinned No-CD legacy ANI/SPR evidence and installed AI input inventory; read-only."""
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
RANGES = {'ani_loader': (0x4644d0,0x464ab4), 'ani_version_dispatch': (0x464ab4,0x464ac8),
          'spr_v4_loader': (0x57d310,0x57d55c)}
ENTRIES = ['4644d0','57d310']


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--decompile', action='store_true')
    parser.add_argument('--entry', action='append', default=[])
    parser.add_argument('--project', type=Path, default=REPO / 'working/decompiled/nocd-96ofmeq_/project')
    args = parser.parse_args()
    script_digest = sha(Path(__file__).read_bytes())
    parent = REPO / 'working/decompiled'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='legacy-assets-support-', dir=parent))
    print(f'Legacy asset evidence: {out}', flush=True)

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
            if not path.is_file() or path.suffix.lower() not in ('.ani','.spr'):
                continue
            data = path.read_bytes()
            version = struct.unpack_from("<I",data,12 if path.suffix.lower()==".ani" else 8)[0]
            if version not in (2,3,4) or (path.suffix.lower()==".spr" and version==4):
                continue
            files.append(dict(version=version,path=path.relative_to(root).as_posix(), source_bytes=len(data),
                              source_sha256=sha(data), first_words=list(struct.unpack_from('<8I', data))))
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
                       '-postScript', 'ExportLegacyAssetSupport.java', str(out / 'pseudo'),
                       *(ENTRIES + args.entry), '-max-cpu', '2']
            with (out / 'ghidra.log').open('w') as log:
                subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
            if not all((out / f'pseudo/{entry}.c').exists() for entry in
                       [f'{int(entry, 16):08x}' for entry in ENTRIES + args.entry]) :
                raise ValueError('Ghidra export missing; inspect ghidra.log')
        if sha(exe.read_bytes()) != HASH:
            raise ValueError('Executable changed')
        for item in files:
            if sha((root / item['path']).read_bytes()) != item['source_sha256']:
                raise ValueError('Installed input changed')
        report = dict(executable_sha256=HASH, script_sha256=script_digest,
                      ghidra_script_sha256=sha((REPO / 'tools/ghidra/ExportLegacyAssetSupport.java').read_bytes()),
                      scope='Static selected reader/caller evidence and installed byte inventory; no live observation',
                      input_unchanged=True, ranges=RANGES,
                      summary=dict(files=len(files), source_bytes=sum(f['source_bytes'] for f in files),
                                   first_words={f['path']: f['first_words'] for f in files}), files=files,
                      artifacts_sha256={p.relative_to(out).as_posix(): sha(p.read_bytes()) for p in out.rglob('*')
                                        if p.is_file() and p.suffix in ('.asm', '.c', '.refs')})
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report['summary']), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
