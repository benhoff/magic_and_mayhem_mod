#!/usr/bin/env python3
"""Hash-pinned No-CD save-world serializer evidence; read-only."""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {'world_write': (0x4713a0,0x471af0), 'world_read': (0x471af0,0x4724b3)}
ENTRIES = ['4713a0','471af0','473890','463cb0',
 '4a0ea0','4a0f70','501a60','501e00','4ffc50','4ffcb0','4e0fd0','4e1050',
 '4669c0','466a30','52bcb0','52c160','49d990','49dcf0','544ee0','544fb0',
 '57b670','57b6c0','542cd0','542f60','5346f0','534850','56cbe0','56cd40',
 '464120','4641b0','501940','501970',
 '5334b0','533550','465180','4652a0','49f3f0','49f490','465870','465a50',
 '52c140','524df0','527310','411e50','52c9d0','52c620','52ca70','54de20','54dfc0','49aa90','49b3d0','544020','544350','463a10','463b40']


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
    out = Path(tempfile.mkdtemp(prefix='save-world-support-', dir=parent))
    print(f'save-world evidence: {out}', flush=True)

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
                       '-postScript', 'ExportSaveWorldSupport.java', str(out / 'pseudo'),
                       *(ENTRIES + args.entry), '-max-cpu', '2']
            with (out / 'ghidra.log').open('w') as log:
                subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
            if not all((out / f'pseudo/{entry}.c').exists() for entry in
                       [f'{int(entry, 16):08x}' for entry in ENTRIES + args.entry]) :
                raise ValueError('Ghidra export missing; inspect ghidra.log')
        if args.decompile:
            for entry,end in re.findall(r'entry=([0-9a-f]{8}) end=([0-9a-f]{8})',(out/'ghidra.log').read_text()):
                asm=subprocess.check_output(['objdump','-d','-Mintel',f'--start-address=0x{entry}',f'--stop-address={int(end,16)+1}',str(exe)],text=True)
                (out/f'{entry}.asm').write_text(asm)
        if sha(exe.read_bytes()) != HASH:
            raise ValueError('Executable changed')
        report = dict(executable_sha256=HASH, script_sha256=script_digest,
                      ghidra_script_sha256=sha((REPO / 'tools/ghidra/ExportSaveWorldSupport.java').read_bytes()),
                      scope='Static world reader/writer and nested serializer evidence; no live observation or real-save corpus',
                      input_unchanged=True, ranges=RANGES,
                      summary=dict(functions=len(ENTRIES+args.entry)),
                      artifacts_sha256={p.relative_to(out).as_posix(): sha(p.read_bytes()) for p in out.rglob('*')
                                        if p.is_file() and p.suffix in ('.asm', '.c', '.refs')})
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report['summary']), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
