"""Validate retained camera packets while keeping source chunks outside World credit."""
import hashlib
import importlib.util
from pathlib import Path
from resident_capture import records


def require(value, message):
    if not value:
        raise ValueError(message)


def contexts_from_records(entries, decode, context):
    contexts, chunks, diagnostics = {}, 0, []
    for generation, chunk, stream, request in entries:
        require(len(contexts) + chunks < 8192 and generation == len(contexts) + 1,
                'Source generation missing/reordered')
        _, rows = decode(stream, complete=False)
        if chunk:
            require(chunk == chunks + 1 and not request, 'Source chunk identity/request differs')
            require(not any(fields[2] in (11, 12, 13, 27) for fields, body in rows),
                    'World/failure operation in source chunk')
        camera = context(rows)  # Existing complete multi-Cell camera/terrain/creature/raster guards.
        if chunk:
            chunks += 1
            diagnostics.append(dict(chunk=chunk, next_World_generation=generation,
                source_sha256=hashlib.sha256(stream).hexdigest(), complete_camera_packet=camera is not None,
                credited_as_World=False, credited_as_action_witness=False))
            continue
        require(64 <= len(request) <= 197184, 'World request extent differs')
        contexts[generation] = dict(generation=generation,
            source_sha256=hashlib.sha256(stream).hexdigest(), camera=camera)
    require(contexts and chunks > 0, 'Missing owned source prefix/history')
    return contexts, chunks, diagnostics


class RetainedCameraHistory:
    def __init__(self, root):
        self.root = Path(root)
        self.calls = 0
        self.diagnostics = None
        self.source = None

    def module(self, name):
        path = self.root / 'tools' / (name + '.py')
        spec = importlib.util.spec_from_file_location('retained_camera_' + name.replace('-', '_'), path)
        result = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(result)
        return result

    def __call__(self, reader, acquired):
        require(self.calls == 0, 'Camera history consumed more than once')
        self.calls += 1
        path = Path(acquired['temporary_root']) / 'worlds.bin'
        require(path.resolve().is_relative_to((self.root / 'working').resolve()), 'Foreign source materialization')
        relative = path.relative_to(self.root).as_posix()
        matches = [v for v in acquired['artifacts'] if v['path'] == relative]
        require(len(matches) == 1, 'Missing unique retained source')
        artifact = matches[0]
        def digest():
            with path.open('rb') as stream:
                return hashlib.file_digest(stream, 'sha256').hexdigest()
        require(digest() == artifact['sha256'] == reader.digest(artifact['record']), 'Source materialization changed')
        decoder = self.module('inspect-canvas-producers')
        camera = self.module('close-rendering-camera-matrix')
        contexts, chunks, diagnostics = contexts_from_records(records(path), decoder.decode, camera.context_many_cells)
        require(digest() == artifact['sha256'], 'Source changed during camera analysis')
        self.diagnostics = diagnostics
        self.source = dict(record=artifact['record'], sha256=artifact['sha256'])
        return contexts, chunks


def bind_assessor(function, source_contexts):
    """Keep the exact semantic function code and replace only its history reader."""
    from types import FunctionType
    require(isinstance(function, FunctionType) and callable(source_contexts), 'Invalid semantic binding')
    namespace = dict(function.__globals__, source_contexts=source_contexts)
    result = FunctionType(function.__code__, namespace, function.__name__, function.__defaults__, function.__closure__)
    result.__kwdefaults__ = function.__kwdefaults__
    require(result.__code__ is function.__code__, 'Semantic function changed')
    return result
