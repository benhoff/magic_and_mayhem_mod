#!/usr/bin/env python3
"""Apply the private output boundary after staging and before any Wine bootstrap."""
import fcntl
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from native_output_owner import command, load_receipt

ROOT = Path(__file__).resolve().parents[1]


def main():
    root = Path(os.environ['MNM_MENU_EXPERIMENT']).resolve()
    prefix = Path(os.environ['WINEPREFIX']).resolve()
    if not root.is_relative_to(ROOT/'working') or not prefix.is_relative_to(ROOT/'working'):
        raise ValueError('Owned output requires disposable working paths')
    metadata = json.loads((root/'manifest.json').read_text())
    receipt = load_receipt(os.environ['MNM_NATIVE_OUTPUT_OWNER'])
    build = json.loads((root/'native-draw-build.json').read_text())
    sources = build['sources']
    for name in ('runtime/native-draw/ownership_admission.h', 'runtime/native-draw/factory.c',
                 'runtime/native-draw/cpu_guard.h', 'runtime/native-draw/surfaces.h',
                 'runtime/native-draw/cpu_copy_boundary.h'):
        if sources.get(name) != hashlib.sha256((ROOT/name).read_bytes()).hexdigest():
            raise ValueError('Native backing ownership capability is missing or stale: '+name)
    if build.get('directdraw_imports') is not False or build.get('sha256') != metadata.get('native_draw_sha256'):
        raise ValueError('Native backing ownership binary differs from staging')
    prefix.mkdir(parents=True, exist_ok=True)
    with (prefix/'.native-output-owner.lock').open('a') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        subprocess.run(['wineserver', '-w'], env=dict(os.environ), check=True, timeout=5)
        args = command(root, prefix, metadata, os.environ, receipt)
        admission = dict(schema=1, session=str(root), prefix=str(prefix), private_display=receipt['display'],
            native_build_sha256=hashlib.sha256((root/'native-draw-build.json').read_bytes()).hexdigest(),
            namespace_network=True, namespace_pid=True, namespace_ipc=True,
            host_graphics_devices=False, legacy_embedding=False, required_backing_mode=3,
            complete_replacement=False)
        from native_only_contract import selected, admit, process_identity
        admission['runner_identity']=process_identity()
        if selected(os.environ):
            admission['native_only']=admit(root,ROOT,metadata,os.environ)
        with (root/'native-output-admission.json').open('x') as out:
            json.dump(admission,out,indent=2);out.write('\n')
        journal=Path(os.environ['MNM_NATIVE_OUTPUT_OWNER']).resolve().parent/'sessions'
        journal.mkdir(exist_ok=True)
        with (journal/(hashlib.sha256(str(root).encode()).hexdigest()+'.json')).open('x') as out:
            json.dump(admission,out,indent=2);out.write('\n')
        return subprocess.run(args+[str(ROOT/'tools/menu-game-runner.py'), *sys.argv[1:]],
                              close_fds=True, stdin=subprocess.DEVNULL).returncode


if __name__ == '__main__':
    raise SystemExit(main())
