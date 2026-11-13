"""Resolve a verified native-build object root for the legacy caller analyser."""
import hashlib
import json
from pathlib import PurePosixPath
from rendering_pilot import require


def resolve(reference, expected_reference, body, expected_sources, read, retain):
    require(isinstance(reference, dict) and isinstance(expected_reference, dict)
            and isinstance(body, bytes) and reference == expected_reference and set(reference) == {'sha256', 'size'}
            and type(reference['size']) is int and reference['size'] == len(body)
            and hashlib.sha256(body).hexdigest() == reference['sha256'],
            'Caller source root identity differs from its tested build')
    manifest = json.loads(body)
    require(isinstance(manifest, dict) and isinstance(expected_sources, dict)
            and bool(expected_sources) and set(manifest) == {'schema', 'sources'} and manifest['schema'] == 1
            and isinstance(manifest['sources'], dict)
            and set(manifest['sources']) == set(expected_sources),
            'Caller source root omits or adds a tested native source')
    result = {}
    for name, value in sorted(manifest['sources'].items()):
        path = PurePosixPath(name)
        require(name and '\0' not in name and not path.is_absolute()
                and bool(path.parts) and path.as_posix() == name and '..' not in path.parts
                and path.parts[0] not in {'original', '.git', 'working'}
                and isinstance(value, dict) and set(value) == {'sha256', 'size'}
                and value['sha256'] == expected_sources[name]
                and type(value['size']) is int and value['size'] >= 0,
                'Unsafe or mismatched caller source-root member')
        raw = read(value)
        require(len(raw) == value['size']
                and hashlib.sha256(raw).hexdigest() == value['sha256'],
                'Caller source preimage bytes differ')
        record = retain(value, raw)
        require(isinstance(record, dict) and set(record) == {'record', 'sha256'}
                and isinstance(record['record'], str)
                and record['record'].startswith('research/runtime/')
                and '..' not in PurePosixPath(record['record']).parts
                and record['sha256'] == value['sha256'],
                'Caller source preimage reference differs')
        result[name] = record
    return result
