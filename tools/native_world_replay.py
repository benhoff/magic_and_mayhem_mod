"""Immutable SQLite cases and bounded interchange for offline World replay."""
import hashlib
from contextlib import contextmanager, closing
import ctypes as c
import json
from pathlib import Path
import sqlite3
import struct

from resident_capture import records, PrefixDecoder

MAX_EVENTS = 8192
MAX_RAW = 16 * 1024**3
MAX_STORED = 512 * 1024**2
MAX_SOURCE = 16 * 1024**2
MAX_REQUEST = 197184
MAGIC = b'MNMREP02'


def sha_file(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def asset_manifest(directory):
    root = Path(directory).resolve(strict=True)
    if not root.is_dir():
        raise ValueError('Assets must be an existing directory')
    result = {}
    for path in sorted(root.rglob('*')):
        if path.is_symlink():
            raise ValueError('Asset manifest refuses nested symlinks: ' + str(path))
        if path.is_file():
            name = path.relative_to(root).as_posix()
            result[name] = [path.stat().st_size, sha_file(path)]
    return result


def check_assets(directory, manifest):
    # Extra unrelated files are harmless; every pinned input must still match.
    root = Path(directory).resolve(strict=True)
    for name, (size, digest) in manifest.items():
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts or not name:
            raise ValueError('Unsafe asset path')
        path = root / relative
        if not path.resolve(strict=True).is_relative_to(root):
            raise ValueError('Asset escapes its root')
        if path.stat().st_size != size or sha_file(path) != digest:
            raise ValueError('Asset changed: ' + name)


class PrefixEncoder:
    """Same bounded zstd prefix contract as the native capture codec."""
    def __init__(self):
        self.api = c.CDLL('libzstd.so.1')
        declarations = {
            'ZSTD_createCCtx': ([], c.c_void_p), 'ZSTD_freeCCtx': ([c.c_void_p], c.c_size_t),
            'ZSTD_CCtx_setParameter': ([c.c_void_p, c.c_int, c.c_int], c.c_size_t),
            'ZSTD_CCtx_refPrefix': ([c.c_void_p, c.c_void_p, c.c_size_t], c.c_size_t),
            'ZSTD_compressBound': ([c.c_size_t], c.c_size_t),
            'ZSTD_compress2': ([c.c_void_p, c.c_void_p, c.c_size_t, c.c_void_p, c.c_size_t], c.c_size_t),
            'ZSTD_isError': ([c.c_size_t], c.c_uint)}
        for name, (args, result) in declarations.items():
            fn = getattr(self.api, name); fn.argtypes = args; fn.restype = result
        self.context = self.api.ZSTD_createCCtx()
        if not self.context:
            raise ValueError('Replay compressor allocation')
        self.previous = b''
        self.count = 0

    def valid(self, result):
        if self.api.ZSTD_isError(result):
            raise ValueError('Replay compressor failure')
        return result

    def encode(self, body):
        key = self.count % 128 == 0
        for parameter, value in [(100, 3), (101, 26), (201, 1)]:
            self.valid(self.api.ZSTD_CCtx_setParameter(self.context, parameter, value))
        previous = c.c_char_p(self.previous) if not key else None
        self.valid(self.api.ZSTD_CCtx_refPrefix(self.context, previous, 0 if key else len(self.previous)))
        output = c.create_string_buffer(self.api.ZSTD_compressBound(len(body)))
        size = self.valid(self.api.ZSTD_compress2(self.context, output, len(output), c.c_char_p(body), len(body)))
        self.previous = body
        self.count += 1
        return output.raw[:size]

    def close(self):
        if self.context:
            self.api.ZSTD_freeCCtx(self.context)
            self.context = None


def event_header(ordinal, generation, chunk, source, request, digest, packed):
    key = (ordinal - 1) % 128 == 0
    return (struct.pack('<QQIIIIQQ', generation, chunk, source, request, len(packed),
                        int(not key), ordinal, 0 if key else ordinal - 1)
            + bytes.fromhex(digest))


def create_case(destination, events, assets, *, label='', provenance=None, prefix=None):
    """Events are (generation, chunk, source, request, kind).

    Kind0 is source history,1 an unreturned captured World plus request,2 an
    already closed owned queue. Kind2 is also used by authored native controls;
    it does not assert original-engine provenance.
    """
    if prefix is not None and (type(prefix) is not int or not 1 <= prefix <= MAX_EVENTS):
        raise ValueError('Replay prefix outside1..8192')
    destination = Path(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb'):
        pass
    db = sqlite3.connect(destination)
    encoder = None
    try:
        db.executescript('''
            CREATE TABLE metadata(key TEXT PRIMARY KEY,value TEXT NOT NULL);
            CREATE TABLE events(ordinal INTEGER PRIMARY KEY,generation INTEGER NOT NULL,
              chunk INTEGER NOT NULL,kind INTEGER NOT NULL,source_bytes INTEGER NOT NULL,
              request_bytes INTEGER NOT NULL,sha256 TEXT NOT NULL,packed BLOB NOT NULL);
        ''')
        encoder = PrefixEncoder()
        worlds = chunks = count = selected = raw_size = stored = 0
        for generation, chunk, source, request, kind in events:
            count += 1
            if (count > MAX_EVENTS or type(generation) is not int or generation != worlds + 1
                    or not 1 <= generation <= MAX_EVENTS or type(chunk) is not int
                    or not 16 <= len(source) <= MAX_SOURCE):
                raise ValueError('Replay source order/extent')
            if kind == 0:
                if chunk != chunks + 1 or request:
                    raise ValueError('Replay source-history order')
                chunks += 1
            elif kind in (1, 2):
                if chunk or (not 64 <= len(request) <= MAX_REQUEST if kind == 1 else bool(request)):
                    raise ValueError('Replay World request extent')
                worlds += 1
            else:
                raise ValueError('Unknown replay event kind')
            body = source + request
            raw_size += len(body)
            if raw_size > MAX_RAW:
                raise ValueError('Replay raw-byte limit')
            # Parse and validate the excluded tail too; never silently repair it.
            if prefix is not None and generation > prefix:
                continue
            packed = encoder.encode(body)
            stored += len(packed)
            if stored > MAX_STORED:
                raise ValueError('Replay case exceeds512MiB compressed budget')
            selected += 1
            db.execute('INSERT INTO events VALUES(?,?,?,?,?,?,?,?)',
                       (selected, generation, chunk, kind, len(source), len(request),
                        hashlib.sha256(body).hexdigest(), packed))
        selected_worlds = min(worlds, prefix) if prefix is not None else worlds
        last = db.execute('SELECT kind FROM events ORDER BY ordinal DESC LIMIT 1').fetchone()
        if not selected_worlds or (prefix is not None and worlds < prefix) or not last or last[0] == 0:
            raise ValueError('Replay needs a complete World prefix from generation1')
        meta = dict(schema=1, codec='zstd-prefix-sha256-v1', label=label, sealed=True, worlds=selected_worlds, events=selected,
                    captured_worlds=worlds, captured_source_chunks=chunks, source_raw_bytes=raw_size,
                    compressed_bytes=stored, assets=assets, provenance=provenance or {},
                    timing='Fixed replay rate; capture ordinals do not preserve original wall-clock intervals',
                    scope='World renderer only. No original simulation, AI, input, HUD/audio or original pixel-equivalence claim.')
        db.executemany('INSERT INTO metadata VALUES(?,?)',
                       [(key, json.dumps(value)) for key, value in meta.items()])
        db.commit()
        return meta
    except BaseException as error:
        db.rollback()
        db.execute('INSERT OR REPLACE INTO metadata VALUES(?,?)', ('failure', json.dumps(str(error))))
        db.commit()
        raise
    finally:
        if encoder:
            encoder.close()
        db.close()


def import_capture(source, destination, assets, *, label='', prefix=None):
    source = Path(source)
    if source.stat().st_size > MAX_RAW:
        raise ValueError('Capture file budget')
    initial = sha_file(source)
    def events():
        for g, c, s, q in records(source):
            yield g, c, s, q, 0 if c else 1
        if sha_file(source) != initial:
            raise ValueError('Capture changed during import')
    with source.open('rb') as stream:
        version = stream.read(8).decode('ascii')
    return create_case(destination, events(), assets, label=label, prefix=prefix,
                       provenance=dict(capture_sha256=initial, format=version))


@contextmanager
def open_case(path):
    path = Path(path).resolve(strict=True)
    if path.stat().st_size > MAX_STORED + 32 * 1024**2:
        raise ValueError('Replay database bound')
    db = sqlite3.connect(path.as_uri() + '?mode=ro', uri=True)
    db.execute('PRAGMA query_only=ON')
    db.execute('PRAGMA trusted_schema=OFF')
    db.setlimit(sqlite3.SQLITE_LIMIT_LENGTH, MAX_SOURCE + MAX_REQUEST + 32768)
    try:
        yield db
    finally:
        db.close()


def validate_case(path):
    with open_case(path) as db, closing(PrefixDecoder()) as decoder:
        meta = {key: json.loads(value) for key, value in db.execute('SELECT key,value FROM metadata')}
        if (meta.get('schema') != 1 or meta.get('sealed') is not True or 'failure' in meta
                or meta.get('codec') != 'zstd-prefix-sha256-v1'):
            raise ValueError('Incomplete or unsupported replay case')
        worlds = chunks = count = raw_size = stored = 0
        last_kind = None
        for ordinal, generation, chunk, kind, source, request, digest, packed in db.execute('SELECT * FROM events ORDER BY ordinal'):
            count += 1
            if (ordinal != count or count > MAX_EVENTS or generation != worlds + 1
                    or not 16 <= source <= MAX_SOURCE):
                raise ValueError('Replay database event order/extent')
            if kind == 0:
                if chunk != chunks + 1 or request:
                    raise ValueError('Replay database source order')
                chunks += 1
            elif kind in (1, 2):
                if chunk or (not 64 <= request <= MAX_REQUEST if kind == 1 else bool(request)):
                    raise ValueError('Replay database request extent')
                worlds += 1
            else:
                raise ValueError('Replay database event kind')
            raw_size += source + request
            stored += len(packed)
            if raw_size > MAX_RAW or stored > MAX_STORED:
                raise ValueError('Replay database byte budget')
            header = event_header(ordinal, generation, chunk, source, request, digest, packed)
            body = decoder.decode(header, packed, source + request)
            if hashlib.sha256(body).hexdigest() != digest:
                raise ValueError('Replay event hash mismatch')
            last_kind = kind
        if (count != meta.get('events') or worlds != meta.get('worlds')
                or not worlds or last_kind == 0):
            raise ValueError('Replay database incomplete prefix')
        return meta


def export_tape(case, destination):
    meta = validate_case(case)
    with open_case(case) as db, Path(destination).open('xb') as stream:
        stream.write(MAGIC + struct.pack('<II', meta['events'], meta['worlds']))
        for ordinal, generation, chunk, kind, source, request, digest, packed in db.execute('SELECT * FROM events ORDER BY ordinal'):
            stream.write(event_header(ordinal, generation, chunk, source, request, digest, packed))
            stream.write(struct.pack('<I', kind))
            stream.write(packed)
    return sha_file(destination)
