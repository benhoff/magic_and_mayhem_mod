#!/usr/bin/env python3
"""Build a separate diagnostic observer with public read-only SHM samples."""
import hashlib
import json
from pathlib import Path
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def build(root=ROOT):
    root = Path(root).resolve()
    out = root/'working/build/storage-upload-observer'
    out.mkdir(parents=True, exist_ok=True)
    helper = out/'libmnm-shm-source.so'
    library = out/'libmnm-storage-upload.so'
    flags = ['gcc', '-std=c11', '-O2', '-fPIC', '-shared', '-Wall', '-Wextra',
             '-Werror', '-pthread']
    commands = [
        [*flags, '-MMD', '-MF', out/'shm.d', root/'runtime/display/egl_shm_source.c',
         '-Wl,-soname,libmnm-shm-source.so', '-ldl', '-lxcb', '-lxcb-shm', '-o', helper],
        [*flags, '-MMD', '-MF', out/'observer.d', '-c', root/'runtime/display/egl_delivery.c', '-o', out/'observer.o'],
        [*flags, '-MMD', '-MF', out/'symbol.d', '-c', root/'runtime/display/glx_symbol.S', '-o', out/'symbol.o'],
        [*flags, out/'observer.o', out/'symbol.o', '-Wl,--no-as-needed', helper,
         '-Wl,-rpath,'+str(out), '-ldl', '-lX11', '-lxcb', '-lxcb-shm', '-o', library],
    ]
    for command in commands:
        subprocess.run(list(map(str, command)), cwd=root, check=True)
    names = ['tools/build-storage-upload-observer.py']
    names += [p.relative_to(root).as_posix() for p in (root/'runtime/display').iterdir()
              if p.suffix in ('.h', '.c', '.S')]
    names += [p.relative_to(root).as_posix() for p in (root/'protocols/include').rglob('*')
              if p.is_file() and p.suffix in ('.h', '.hpp')]
    sources = {name: hashlib.sha256((root/name).read_bytes()).hexdigest() for name in sorted(names)}
    compiled = set()
    for path in out.glob('*.d'):
        for token in shlex.split(path.read_text().replace('\\\n', ' ').split(':', 1)[1]):
            p = Path(token).resolve()
            if p.is_file() and p.is_relative_to(root) and not p.is_relative_to(root/'working'):
                compiled.add(p.relative_to(root).as_posix())
    if not compiled or compiled-sources.keys():
        raise ValueError('Unbound observer compiler inputs: '+repr(sorted(compiled-sources.keys())))
    # Required physical interchange: capture-drawing-interactive checks the manifest
    # adjacent to its selected library. Durable results import this exact payload.
    manifest = dict(schema=1, sources=sources,
                    binary_sha256=hashlib.sha256(library.read_bytes()).hexdigest(),
                    helper_sha256=hashlib.sha256(helper.read_bytes()).hexdigest(),
                    helper=str(helper), compiled_sources=sorted(compiled),
                    commands=[list(map(str, c)) for c in commands],
                    scope='Explicit diagnostic dependency; existing observer source is unchanged. '
                          'Additional geometry queries/read-only SysV attachments perturb timing. '
                          'Sample pixels are comparator oracles and never native rendering inputs.')
    (out/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
    return library, helper, manifest


if __name__ == '__main__':
    print(build()[0])
