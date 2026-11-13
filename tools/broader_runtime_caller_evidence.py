"""Bind verified evidence payload bytes for a legacy literal-source snapshot loop."""
import hashlib
from coverage_evidence import identity
from rendering_pilot import require


def payload_preimages(existing, sources, physical, payload, retain):
    result = {}
    references = {}
    for name, expected in sorted(sources.items()):
        if name in existing:
            require(existing[name]['sha256'] == expected, 'Native preimage source digest changed')
            continue
        raw = physical(name)
        reference = identity(raw)
        if reference is None:
            require(hashlib.sha256(raw).hexdigest() == expected,
                    'Ordinary analysis source changed before snapshot')
            continue
        sha, size = reference
        require(sha == expected, 'Evidence source used a pointer-file hash instead of payload identity')
        body = payload(name)
        require(len(body) == size and hashlib.sha256(body).hexdigest() == expected,
                'Evidence analysis source payload changed')
        snapshot = retain(dict(sha256=sha, size=size), body)
        require(snapshot['sha256'] == expected and snapshot['record'].startswith('research/runtime/'),
                'Evidence source snapshot identity changed')
        result[name] = snapshot
        references[name] = dict(sha256=sha, size=size,
                               physical_reference_sha256=hashlib.sha256(raw).hexdigest())
    return result, references
