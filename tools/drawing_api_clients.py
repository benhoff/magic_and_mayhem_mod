"""Bind reviewed client layouts to exact CALL routes, never offset-only types."""
import hashlib
import json
from pathlib import Path

from rendering_coverage import evidence_record

# PE32 SDK formal counts include the interface receiver. These constrain the
# reviewed declarations; receiver provenance still needs an exact site review.
PROTOTYPES = {
    ('IDirectDraw', 'SetCooperativeLevel'): (3, 'render_state'),
    ('IUnknown', 'QueryInterface'): (3, 'lifecycle'),
    ('IUnknown', 'Release'): (1, 'lifecycle'),
    ('IDirectDraw2', 'SetDisplayMode'): (6, 'render_state'),
    ('IDirectDraw2', 'RestoreDisplayMode'): (1, 'render_state'),
    ('IDirectDraw2', 'CreateClipper'): (4, 'lifecycle'),
    ('IDirectDraw2', 'CreateSurface'): (4, 'lifecycle'),
    ('IDirectDraw2', 'CreatePalette'): (5, 'lifecycle'),
    ('IDirectDrawClipper', 'GetHWnd'): (2, 'render_state'),
    ('IDirectDrawClipper', 'SetHWnd'): (3, 'render_state'),
    ('IDirectDrawSurface2', 'Blt'): (6, 'composition'),
    ('IDirectDrawSurface2', 'BltFast'): (6, 'composition'),
    ('IDirectDrawSurface2', 'GetSurfaceDesc'): (2, 'surface_access'),
    ('IDirectDrawSurface2', 'Lock'): (5, 'surface_access'),
    ('IDirectDrawSurface2', 'Unlock'): (2, 'surface_access'),
    ('IDirectDrawSurface2', 'GetDC'): (2, 'surface_access'),
    ('IDirectDrawSurface2', 'ReleaseDC'): (2, 'surface_access'),
    ('IDirectDrawSurface2', 'PageLock'): (2, 'surface_access'),
    ('IDirectDrawSurface2', 'PageUnlock'): (2, 'surface_access'),
    ('IDirectDrawSurface2', 'Restore'): (1, 'lifecycle'),
    ('IDirectDrawSurface2', 'SetColorKey'): (3, 'render_state'),
    ('IDirectDrawSurface2', 'SetPalette'): (2, 'render_state'),
    ('IDirectDrawSurface2', 'SetClipper'): (2, 'render_state'),
}


def exact_address(value):
    if type(value) is not int or not 0 <= value <= 0xffffffff:
        raise ValueError('Reviewed client needs an exact PE32 address')
    return value


def reviewed_clients(root, sources, evidence, builds, build, api_sites):
    clients = {}
    for source in sources:
        ev = evidence[source['evidence']]
        if ev['kind'] != 'static':
            raise ValueError('Reviewed API clients require static review evidence')
        record = evidence_record(root, ev, builds)
        if type(record['schema']) is not int or record['schema'] != 1 or record['original_sha256'] != build['source_sha256']:
            raise ValueError('Reviewed API client build/schema mismatch')
        path = record['layouts']
        target = (root / path).resolve()
        if Path(path).is_absolute() or '..' in Path(path).parts or not target.is_relative_to(root.resolve()) or path.split('/')[0] in {'original', 'working', '.git'}:
            raise ValueError('Unsafe reviewed API layout path')
        raw = target.read_bytes()
        digest = hashlib.sha256(raw).hexdigest()
        if record['sources'].get(path) != digest:
            raise ValueError('Reviewed API layout source hash mismatch')
        layouts = json.loads(raw)
        if type(layouts['schema']) is not int or layouts['schema'] != 1 or layouts['original_sha256'] != build['source_sha256']:
            raise ValueError('Reviewed API layout build/schema mismatch')
        rows = layouts['contracts']
        excluded_rows = layouts['excluded_reload_callbacks']
        excluded = {exact_address(site) for site in excluded_rows}
        if len(excluded) != len(excluded_rows) or type(record['excluded_callbacks']) is not int or record['excluded_callbacks'] != len(excluded):
            raise ValueError('Reviewed callback exclusions disagree')
        if type(record['reviewed_layouts']) is not int or record['reviewed_layouts'] != len(rows):
            raise ValueError('Reviewed API layout count mismatch')
        seen = set()
        for row in rows:
            site = exact_address(row['site'])
            if site in seen or site in excluded:
                raise ValueError('Duplicate or excluded reviewed API client')
            seen.add(site)
            if site not in api_sites or row['call_bytes'] != api_sites[site]['bytes'] or api_sites[site]['target_mode']['kind'] != 'register_memory':
                raise ValueError('Reviewed API client differs from exact CALL census')
            prototype = PROTOTYPES.get((row['interface'], row['method']))
            if not prototype or type(row['formal_words']) is not int or row['formal_words'] != prototype[0]:
                raise ValueError('Reviewed API client prototype mismatch')
            if not isinstance(row['receiver_evidence'], str) or not row['receiver_evidence'].strip():
                raise ValueError('Reviewed API client needs receiver provenance')
            contract = {k: row[k] for k in ('interface', 'method', 'formal_words', 'receiver_evidence')}
            contract.update(source_evidence=ev['id'], layout_sha256=digest)
            client = {'site': site, 'contract': contract, 'classification': prototype[1]}
            if site in clients and clients[site] != client:
                raise ValueError('Conflicting reviewed API client sources')
            clients[site] = client
    return clients
