"""Verified content-addressed cold storage for completed working-run payloads."""
from contextlib import contextmanager
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tempfile
import threading
import time

try:
    import zstandard as ZSTD
except ImportError:
    ZSTD = None  # The installed zstd CLI remains a supported fallback.

GIB = 1024 ** 3
CHUNK = 4 * 1024 * 1024
# Reports, configuration and source remain readable in the original directories.
KEEP = {'.json', '.jsonl', '.md', '.txt', '.log', '.tsv', '.csv', '.cfg', '.ini',
        '.py', '.sh', '.c', '.cpp', '.h', '.hpp', '.inc', '.java', '.asm', '.s',
        '.cmake', '.mk', '.pro', '.pri', '.html', '.js', '.css', '.yml', '.yaml'}


def digest(stream):
    h = hashlib.sha256()
    size = 0
    while data := stream.read(CHUNK):
        h.update(data)
        size += len(data)
    return h.hexdigest(), size


def stamp(s):
    return s.st_dev, s.st_ino, s.st_size, s.st_mtime_ns, s.st_ctime_ns


def sync_directory(path):
    fd = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


def safe_source(root, name):
    p = Path(name)
    if p.is_absolute() or not p.parts or p.parts[0] != 'working' or any(
            part in ('.', '..', 'original', '.git') for part in p.parts):
        raise ValueError('Unsafe archive source: ' + name)
    target = root / p
    for parent in (target, *target.parents):
        if parent == root:
            break
        if parent.is_symlink():
            raise ValueError('Archive source traverses a symlink: ' + name)
    return target


def write_record(path, record):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('a') as out:
        out.write(json.dumps(record, sort_keys=True) + '\n')
        out.flush()
        os.fsync(out.fileno())
    sync_directory(path.parent)


def verify_object(blob, expected, size):
    code = 0
    if ZSTD is not None:
        with blob.open('rb') as source, ZSTD.ZstdDecompressor().stream_reader(source) as decoded:
            result = digest(decoded)
    else:
        with subprocess.Popen(['zstd', '-q', '-dc', '--', str(blob)], stdout=subprocess.PIPE) as process:
            result = digest(process.stdout)
            process.stdout.close()
            code = process.wait()
    if code or result != (expected, size):
        raise ValueError('Archive verification failed: ' + str(blob))
    with blob.open('rb') as source:
        compressed_hash, _ = digest(source)
    return compressed_hash, stamp(blob.stat())


def active_paths(root):
    """Best-effort open-file/cwd/executable check; never inspect command arguments."""
    prefix = str(root / 'working') + '/'
    result = set()
    own = os.getpid()
    for proc in Path('/proc').iterdir():
        if not proc.name.isdigit() or int(proc.name) == own:
            continue
        try:
            paths = [proc / 'cwd', proc / 'exe', *(proc / 'fd').iterdir()]
        except OSError:
            continue
        for path in paths:
            try:
                target = os.readlink(path).removesuffix(' (deleted)')
            except OSError:
                continue
            if target.startswith(prefix):
                result.add(Path(target))
    return result


def candidate_runs(root, rules, include_worktrees=False):
    roots = [root]
    if include_worktrees:
        output = subprocess.check_output(['git', '-C', str(root), 'worktree', 'list', '--porcelain', '-z'])
        for row in output.split(b'\0'):
            if row.startswith(b'worktree '):
                p = Path(os.fsdecode(row[9:]))
                if p != root and p.is_relative_to(root / 'working'):
                    roots.append(p)
    cutoff = time.time() - rules.get('archive_quiet_minutes', 60) * 60
    active = active_paths(root)
    result = []
    for checkout in roots:
        for category in ('experiments', 'tests'):
            parent = checkout / 'working' / category
            if parent.is_symlink() or not parent.is_dir():
                continue
            for group in sorted(parent.iterdir()):
                if group.is_symlink() or not group.is_dir() or (group / '.retain').exists():
                    continue
                runs = [p for p in group.iterdir() if p.name.startswith('run-') and p.is_dir() and not p.is_symlink()]
                runs.sort(key=lambda p: (p.stat().st_mtime_ns, str(p)), reverse=True)
                for run in runs[rules['keep_latest_runs']:]:
                    if (run / '.retain').exists() or any(p == run or run in p.parents for p in active):
                        continue
                    payloads, latest, pinned = [], run.stat().st_mtime, False
                    for directory, dirs, files in os.walk(run, followlinks=False):
                        here = Path(directory)
                        if '.retain' in files:
                            pinned = True
                            break
                        # Preserve nested source checkouts and unique source snapshots whole.
                        if '.git' in dirs or '.git' in files or 'AGENTS.md' in files:
                            dirs[:] = []
                            continue
                        dirs[:] = [n for n in dirs if n not in ('original', '.git') and not (here / n).is_symlink()]
                        latest = max(latest, here.stat().st_mtime)
                        for name in files:
                            path = here / name
                            s = path.lstat()
                            latest = max(latest, s.st_mtime)
                            if stat.S_ISREG(s.st_mode) and s.st_size >= rules.get('archive_minimum_file_mib', 1) * 1024**2 and path.suffix.lower() not in KEEP and not name.startswith('.'):
                                payloads.append((s.st_blocks * 512, path))
                    if not pinned and latest <= cutoff and payloads:
                        result.append((sum(n for n, _ in payloads), run, payloads))
    return sorted(result, key=lambda x: (-x[0], str(x[1])))


@contextmanager
def archive_lock(destination):
    if not destination.is_dir() or destination.is_symlink():
        raise ValueError('Create a real archive directory owned by your account first: ' + str(destination))
    with (destination / '.archive.lock').open('a') as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        yield


class Archive:
    def __init__(self, root, destination, minimum_free_gib=100):
        self.root, self.destination = root, destination
        self.minimum_free = minimum_free_gib * GIB
        self.verified = {}
        self.mutex = threading.Lock()
        self.local = root / 'working/retention/archive-catalog.jsonl'
        self.remote = destination / 'catalog.jsonl'
        self.small_compressor = ZSTD.ZstdCompressor(level=3) if ZSTD else None

    def record(self, row):
        # Both durable receipts precede unlinking. A crash leaves either a copy
        # still present or a verified, catalogued object available to restore.
        write_record(self.remote, row)
        write_record(self.local, row)

    def store(self, path, prune=False):
        relative = path.relative_to(self.root).as_posix()
        path = safe_source(self.root, relative)
        with self.mutex:
            with os.fdopen(os.open(path, os.O_RDONLY | os.O_NOFOLLOW), 'rb') as source:
                before = os.fstat(source.fileno())
                if not stat.S_ISREG(before.st_mode):
                    raise ValueError('Archive input is not a regular file')
                raw_hash, size = digest(source)
                if stamp(os.fstat(source.fileno())) != stamp(before):
                    raise ValueError('Source changed while hashing: ' + relative)
                blob = self.destination / 'objects' / raw_hash[:2] / (raw_hash + '.zst')
                blob.parent.mkdir(parents=True, exist_ok=True)
                for directory in (self.destination, blob.parent.parent, blob.parent):
                    sync_directory(directory)
                if not blob.exists():
                    if shutil.disk_usage(self.destination).free - size < self.minimum_free:
                        raise ValueError('Archive disk free-space reserve would be exceeded')
                    fd, temporary = tempfile.mkstemp(prefix='.partial-', dir=blob.parent)
                    partial = Path(temporary)
                    try:
                        source.seek(0)
                        with os.fdopen(fd, 'wb') as out:
                            if self.small_compressor and size <= 8 * 1024**2:
                                out.write(self.small_compressor.compress(source.read()))
                            elif ZSTD:
                                with ZSTD.ZstdCompressor(level=3, threads=2).stream_writer(out, closefd=False) as encoded:
                                    shutil.copyfileobj(source, encoded, CHUNK)
                            else:
                                subprocess.run(['zstd', '-q', '-T2', '-3', '-c'], stdin=source, stdout=out, check=True)
                            out.flush()
                            os.fsync(out.fileno())
                        checked = verify_object(partial, raw_hash, size)
                        os.link(partial, blob)
                        sync_directory(blob.parent)
                    finally:
                        partial.unlink(missing_ok=True)
                    self.verified[raw_hash] = (checked[0], stamp(blob.stat()))
                checked = self.verified.get(raw_hash)
                if checked is None or checked[1] != stamp(blob.stat()):
                    checked = verify_object(blob, raw_hash, size)
                    self.verified[raw_hash] = checked
                source.seek(0)
                if digest(source) != (raw_hash, size) or stamp(os.fstat(source.fileno())) != stamp(before) or stamp(path.lstat()) != stamp(before):
                    raise ValueError('Source changed before archive receipt: ' + relative)
                row = {'schema': 1, 'state': 'verified', 'created_utc': datetime.now(timezone.utc).isoformat(),
                       'path': relative, 'sha256': raw_hash, 'size': size,
                       'allocated_bytes': before.st_blocks * 512, 'mode': stat.S_IMODE(before.st_mode),
                       'mtime_ns': before.st_mtime_ns, 'blob': blob.relative_to(self.destination).as_posix(),
                       'compressed_sha256': checked[0], 'archive_root': str(self.destination)}
                self.record(row)
                released = 0
                if prune:
                    safe_source(self.root, relative)
                    if stamp(path.lstat()) != stamp(before) or stamp(blob.stat()) != checked[1]:
                        raise ValueError('Source/archive changed before pruning: ' + relative)
                    path.unlink()
                    sync_directory(path.parent)
                    released = before.st_blocks * 512 if before.st_nlink == 1 else 0
                    self.record(dict(row, state='pruned'))
                return released


def restore(root, destination, name):
    target = safe_source(root, name)
    row = None
    with (destination / 'catalog.jsonl').open() as catalog:
        for line in catalog:
            item = json.loads(line)
            if item['path'] == name and item['state'] in ('verified', 'pruned'):
                row = item
    if row is None:
        raise ValueError('Path not present in archive catalog: ' + name)
    expected_blob = 'objects/' + row['sha256'][:2] + '/' + row['sha256'] + '.zst'
    if row['blob'] != expected_blob or len(row['sha256']) != 64 or any(c not in '0123456789abcdef' for c in row['sha256']):
        raise ValueError('Unsafe archive catalog blob')
    blob = destination / expected_blob
    if target.exists():
        with target.open('rb') as source:
            if digest(source) != (row['sha256'], row['size']):
                raise ValueError('Refusing to overwrite existing changed file')
        return
    with blob.open('rb') as source:
        if digest(source)[0] != row['compressed_sha256']:
            raise ValueError('Compressed archive hash mismatch')
    target.parent.mkdir(parents=True, exist_ok=True)
    safe_source(root, name)
    fd, temporary = tempfile.mkstemp(prefix='.restore-', dir=target.parent)
    partial = Path(temporary)
    try:
        with os.fdopen(fd, 'wb') as out:
            if ZSTD:
                with blob.open('rb') as source, ZSTD.ZstdDecompressor().stream_reader(source) as decoded:
                    shutil.copyfileobj(decoded, out, CHUNK)
            else:
                subprocess.run(['zstd', '-q', '-dc', '--', str(blob)], stdout=out, check=True)
            out.flush()
            os.fsync(out.fileno())
        with partial.open('rb') as source:
            if digest(source) != (row['sha256'], row['size']):
                raise ValueError('Restored bytes do not match source hash')
        partial.chmod(row['mode'])
        os.utime(partial, ns=(row['mtime_ns'], row['mtime_ns']))
        os.link(partial, target)
    finally:
        partial.unlink(missing_ok=True)


def maintain(root, rules, prune=False, include_worktrees=False, max_files=None):
    destination = Path(rules['archive_root'])
    maximum = rules.get('max_working_gib')
    target = maximum * GIB if maximum is not None else None

    def enough_space(released):
        within_size = target is None or total - released <= target
        return within_size and shutil.disk_usage(root / 'working').free >= rules['minimum_free_gib'] * GIB
    print(json.dumps({'event': 'archive_measure_and_scan', 'prune': prune}), flush=True)
    total = int(subprocess.check_output(['du', '-x', '-s', '-B1', '--', str(root / 'working')]).split()[0])
    count, released, failures = 0, 0, []
    with archive_lock(destination):
        archive = Archive(root, destination, rules.get('minimum_archive_free_gib', 100))
        runs = candidate_runs(root, rules, include_worktrees)
        print(json.dumps({'event': 'archive_start', 'working_gib': total / GIB, 'eligible_runs': len(runs), 'prune': prune}), flush=True)
        for _, run, payloads in runs:
            if prune and enough_space(released):
                break
            if any(p == run or run in p.parents for p in active_paths(root)):
                continue
            print(json.dumps({'event': 'archive_run', 'path': run.relative_to(root).as_posix(), 'payloads': len(payloads)}), flush=True)
            for _, path in sorted(payloads, key=lambda x: (-x[0], str(x[1]))):
                if max_files is not None and count >= max_files:
                    break
                if prune and enough_space(released):
                    break
                try:
                    released += archive.store(path, prune)
                    count += 1
                    if count % 64 == 0:
                        print(json.dumps({'event': 'archive_progress', 'files_verified': count,
                                          'released_gib': round(released / GIB, 3),
                                          'last_path': path.relative_to(root).as_posix()}), flush=True)
                except (ValueError, OSError, subprocess.CalledProcessError) as exc:
                    failures.append({'path': path.relative_to(root).as_posix(), 'reason': str(exc)})
                    print(json.dumps({'event': 'archive_refusal', **failures[-1]}), flush=True)
                    break  # Stop this run on change/error; preserve its remaining files.
            if max_files is not None and count >= max_files:
                break
        result = {'schema': 1, 'created_utc': datetime.now(timezone.utc).isoformat(),
                  'archive_root': str(destination), 'files_verified': count, 'released_bytes': released,
                  'prune': prune, 'include_worktrees': include_worktrees, 'failures': failures,
                  'estimated_working_bytes': total - released, 'target_bytes': target,
                  'minimum_free_bytes': rules['minimum_free_gib'] * GIB,
                  'scope': 'Verified storage copies and restoration identities only; no engine validation claim.'}
        name = 'archive-result-' + datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ') + '.json'
        for directory in (root / 'working/retention', destination / 'reports'):
            directory.mkdir(parents=True, exist_ok=True)
            with (directory / name).open('x') as out:
                json.dump(result, out, indent=2)
                out.write('\n')
        print(json.dumps({'event': 'archive_complete', **result}), flush=True)
        return result
