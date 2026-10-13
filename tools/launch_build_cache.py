"""Optional source/toolchain-bound cache for public launcher adapter builds."""
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOTS = ('runtime', 'reconstruction/rendering', 'renderer', 'protocols')
SUFFIXES = {'.c', '.h', '.cpp', '.hpp', '.S', '.inc', '.py', '.json'}


def sha(path):
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def fingerprint(root, builder, options):
    # Conservatively include shared headers, assembly, generators and builders.
    paths = {p for folder in SOURCE_ROOTS for p in (root/folder).rglob('*')
             if p.is_file() and p.suffix in SUFFIXES and '__pycache__' not in p.parts}
    paths.update((root/'tools').glob('build-*.py'))
    helper = root/'tools/launch_build_cache.py'
    if helper.is_file():
        paths.add(helper)
    sources = {p.relative_to(root).as_posix(): sha(p) for p in sorted(paths)}
    tools = {}
    for name in ('clang', 'lld-link', 'llvm-dlltool', 'llvm-readobj'):
        located = shutil.which(name)
        if located is None:
            raise FileNotFoundError(f'Adapter build tool is unavailable: {name}')
        path = Path(located).resolve()
        stat = path.stat()
        tools[name] = [str(path), stat.st_size, stat.st_mtime_ns, stat.st_ctime_ns]
    value = {'schema': 1, 'builder': builder, 'options': options,
             'sources': sources, 'tools': tools}
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def build_adapter(builder, build, *, options=None, root=ROOT):
    """Diagnostics build directly; public launches reuse verified DLL/manifest pairs."""
    if os.environ.get('MNM_CACHE_ADAPTERS', '0') != '1':
        return build()
    root = Path(root)
    parent = root/'working/build/launch-adapters'/Path(builder).stem
    parent.mkdir(parents=True, exist_ok=True)
    # Existing builders share output directories, so serialize cache miss builds.
    with (parent/'build.lock').open('a') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        key = fingerprint(root, builder, options or {})
        cached = parent/key
        receipt = cached/'cache.json'
        try:
            record = json.loads(receipt.read_text())
            name = record['dll']
            if Path(name).name != name or not name.endswith('.dll'):
                raise ValueError('Invalid cached adapter name')
            dll = cached/name
            manifest = cached/'manifest.json'
            if (record['key'] == key and not dll.is_symlink() and not manifest.is_symlink()
                    and sha(dll) == record['dll_sha256']
                    and sha(manifest) == record['manifest_sha256']):
                return dll
        except (OSError, ValueError, KeyError, TypeError):
            pass
        dll = Path(build())
        manifest = dll.parent/'manifest.json'
        if json.loads(manifest.read_text()).get('sha256') != sha(dll):
            raise ValueError('Adapter DLL and build manifest disagree')
        if fingerprint(root, builder, options or {}) != key:
            raise ValueError('Adapter compiler inputs changed during build; retry launch')
        cached.mkdir(parents=True, exist_ok=True)
        # Publish the receipt last; interrupted writes cannot become cache hits.
        for source in (dll, manifest):
            temporary = cached/(source.name+'.tmp')
            shutil.copy2(source, temporary)
            temporary.replace(cached/source.name)
        record = {'key': key, 'dll': dll.name, 'dll_sha256': sha(cached/dll.name),
                  'manifest_sha256': sha(cached/'manifest.json')}
        temporary = cached/'cache.json.tmp'
        temporary.write_text(json.dumps(record, indent=2)+'\n')
        temporary.replace(receipt)
        return cached/dll.name
