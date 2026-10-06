"""Read-only presentation of the reviewed coverage register (stdlib only).

Association counts describe links, never semantic recovery or whole-game progress.
No original executable or installed game artifacts are opened by this module.
"""
from bisect import bisect_right
from collections import Counter
from datetime import datetime, timezone
import hashlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import threading
from urllib.parse import parse_qs, urlsplit

from coverage_audit import audit, STAGES

ROOT = Path(__file__).resolve().parents[1]
REGISTER = 'research/runtime/coverage/register.json'
TRACKS = 'apps/coverage-ui/tracks.json'
ORIGINAL_EVIDENCE = {'isolated_original', 'live_equivalence', 'live_replacement'}


def measure(count, total):
    return {'count': count, 'total': total, 'percent': round(count * 100 / total, 2) if total else None}


def union_size(ranges):
    """Half-open union; overlapping bodies must not inflate byte percentages."""
    size, end = 0, None
    for start, stop in sorted(ranges):
        if stop <= start:
            continue
        size += max(0, stop - max(start, end if end is not None else start))
        end = max(stop, end if end is not None else stop)
    return size


def summarize(behaviors):
    baseline = [b for b in behaviors if b['kind'] == 'recovered']
    return {
        'total': len(behaviors), 'recovered_total': len(baseline),
        'statuses': {stage: dict(Counter(b['status'][stage] for b in behaviors)) for stage in STAGES},
        'scoped': measure(sum(b['status']['implementation'] == 'scoped' for b in behaviors), len(behaviors)),
        'partial': measure(sum(b['status']['implementation'] == 'partial' for b in behaviors), len(behaviors)),
        'missing': measure(sum(b['status']['implementation'] == 'none' for b in behaviors), len(behaviors)),
        'comparison': measure(sum(b['status']['comparison'] == 'recorded' for b in baseline), len(baseline)),
        'current_comparison': measure(sum(b['comparison_freshness'] == 'current' for b in baseline), len(baseline)),
        'replacement': measure(sum(b['status']['replacement'] == 'scoped_live' for b in baseline), len(baseline)),
    }


def track_view(definition, behaviors, evidence):
    """Curated capability goals; all requirements must pass, never partial credit.

    Recorded achievements survive source staleness, shown independently from
    current execution fingerprints. Missing IDs are visible accounting gaps.
    """
    by_id = {b['id']: b for b in behaviors}
    by_evidence = {e['id']: e for e in evidence}
    tracks = []
    for track in definition['tracks']:
        milestones = []
        track_ids = {b['id'] for b in behaviors if b['subsystem'] in track['subsystems']}
        for milestone in track['milestones']:
            checks, missing = [], []
            for requirement in milestone['requirements']:
                ids = requirement.get('behaviors', [])
                if 'subsystem' in requirement:
                    ids = [b['id'] for b in behaviors if b['subsystem'] == requirement['subsystem']]
                    if not ids: missing.append('subsystem:' + requirement['subsystem'])
                for bid in ids:
                    track_ids.add(bid)
                    b = by_id.get(bid)
                    if b is None:
                        missing.append(bid)
                        continue
                    expected = requirement['status']
                    matched = all(b['status'].get(stage) in (target if isinstance(target, list) else [target])
                                  for stage, target in expected.items())
                    if b['kind'] == 'native_policy' and any(s in expected for s in ('comparison', 'replacement')):
                        matched = False
                    kinds = []
                    if 'comparison' in expected: kinds.append(ORIGINAL_EVIDENCE)
                    if 'replacement' in expected: kinds.append({'live_replacement'})
                    if 'integration' in expected:
                        kinds.append({'live_observation', 'live_equivalence', 'live_replacement'}
                                     if expected['integration'] == ['live_observation', 'scoped_live'] else
                                     {'native_integration', 'live_observation', 'live_equivalence', 'live_replacement'})
                    if not kinds: kinds.append({'native_integration', *ORIGINAL_EVIDENCE, 'live_observation'})
                    proofs = [[by_evidence[eid] for eid in b['evidence']
                               if eid in by_evidence and by_evidence[eid]['kind'] in bucket] for bucket in kinds]
                    checks.append({'behavior': bid, 'expected': expected,
                                   'met': matched and all(proofs),
                                   'fresh': matched and all(any(e['freshness'] == 'current' for e in proof) for proof in proofs)})
            ids = sorted({c['behavior'] for c in checks})
            ready = bool(checks) and not missing and all(c['met'] for c in checks)
            started = any(by_id[bid]['status']['implementation'] != 'none' for bid in ids)
            status = 'demonstrated' if ready else 'unknown' if missing else 'in_progress' if started else 'not_started'
            milestones.append({**milestone, 'behaviors': ids, 'checks': checks, 'missing': missing,
                               'state': status, 'current': ready and all(c['fresh'] for c in checks)})
        counts = Counter(m['state'] for m in milestones)
        tracks.append({**track, 'milestones': milestones, 'behaviors': sorted(track_ids & by_id.keys()),
                       'progress': measure(counts['demonstrated'], len(milestones)),
                       'counts': dict(counts), 'current': sum(m['current'] for m in milestones),
                       'next': next((m['id'] for m in milestones if m['state'] != 'demonstrated'), None)})
    return tracks


def enrich_behaviors(register, report):
    stale = {e['id']: e['changed_sources'] for e in report['findings']['stale_evidence']}
    evidence = {e['id']: e for e in register['evidence']}
    rows = []
    for behavior in register['behaviors']:
        row = {**behavior}
        row['scenarios'] = [s['id'] for s in register['scenarios'] if row['id'] in s['behaviors']]
        comparisons = [eid for eid in row['evidence'] if eid in evidence and evidence[eid]['kind'] in ORIGINAL_EVIDENCE]
        current = [eid for eid in comparisons if eid not in stale]
        row['comparison_freshness'] = ('not_applicable' if row['kind'] == 'native_policy' else
                                       'none' if row['status']['comparison'] != 'recorded' or not comparisons else
                                       'current' if len(current) == len(comparisons) else 'mixed' if current else 'stale')
        row['stale_evidence'] = {eid: stale[eid] for eid in row['evidence'] if eid in stale}
        row['gaps'] = []
        status = row['status']
        if status['implementation'] == 'none': row['gaps'].append('unimplemented')
        if status['implementation'] == 'partial': row['gaps'].append('partial')
        if status['understanding'] != 'scoped': row['gaps'].append('understanding')
        if row['kind'] == 'recovered' and status['comparison'] == 'none': row['gaps'].append('comparison')
        if row['stale_evidence']: row['gaps'].append('stale')
        if status['implementation'] != 'none' and not row['tests']: row['gaps'].append('tests')
        if status['integration'] == 'none': row['gaps'].append('integration')
        if row['kind'] == 'recovered' and status['replacement'] == 'none': row['gaps'].append('replacement')
        rows.append(row)
    return rows, stale


def binary_view(register, inventories, behaviors):
    functions, builds = [], []
    for build, inventory in inventories.items():
        records = {f['entry']: {**f, 'build': build, 'behaviors': [], 'callers': [], 'callees': []}
                   for f in inventory['functions']}
        spans = sorted((start, end, f['entry']) for f in inventory['functions'] for start, end in f['ranges'])
        starts = [r[0] for r in spans]

        def owner(address):
            # Check all candidate overlapping spans; register/audit mappings are scoped.
            i = bisect_right(starts, address) - 1
            return next((entry for start, end, entry in reversed(spans[:i + 1]) if start <= address < end), None)

        for behavior in behaviors:
            for ref in behavior['original']:
                if ref['build'] == build and ref.get('relation', 'code') == 'code':
                    entry = owner(ref['address'])
                    if entry is not None and behavior['id'] not in records[entry]['behaviors']:
                        records[entry]['behaviors'].append(behavior['id'])
        classifications = {c['entry']: c for c in register['classifications'] if c['build'] == build}
        # Flow owners are Ghidra function entries; targets may be interior addresses.
        for flow in inventory['flows']:
            if flow['kind'] != 'call' or flow.get('computed', False) or flow['owner'] not in records:
                continue
            for address in flow['targets']:
                target = records.get(address)
                if target is None:
                    continue  # Non-entry flows remain explicit in audit findings.
                records[flow['owner']]['callees'].append(target['entry'])
                target['callers'].append(flow['owner'])
        for entry, record in records.items():
            record['classification'] = classifications.get(entry)
            record['callers'] = sorted(set(record['callers']))
            record['callees'] = sorted(set(record['callees']))
            record['bytes'] = union_size(record['ranges'])
            functions.append(record)
        mapped = [r for r in records.values() if r['behaviors']]
        executable = [(s['start'], s['end']) for s in inventory['sections'] if s['executable']]
        linked_ranges = [span for f in mapped for span in f['ranges']]
        builds.append({'id': build, 'source_sha256': inventory['source_sha256'],
                       'functions': measure(len(mapped), len(records)),
                       'bytes': measure(union_size(linked_ranges), union_size(executable)),
                       'classified': len(classifications), 'sections': inventory['sections'],
                       'unassigned_bytes': union_size(inventory['unassigned_executable_ranges']),
                       'unassigned_ranges': len(inventory['unassigned_executable_ranges']),
                       'recovered_ranges': [r for r in register['recovered_ranges'] if r['build'] == build]})
    return {'builds': builds, 'functions': functions}


def safe_file(root, path, allowed):
    relative = Path(path)
    if path not in allowed or relative.is_absolute() or '..' in relative.parts:
        raise ValueError('File is outside the registered read-only links')
    resolved = (root / relative).resolve()
    if not resolved.is_relative_to(root.resolve()) or resolved.relative_to(root.resolve()).parts[0] in {'original', 'working', '.git'}:
        raise ValueError('File is outside the registered read-only links')
    if not resolved.is_file():
        raise ValueError('Registered file is missing')
    return resolved


def build_snapshot(root=ROOT):
    root = Path(root).resolve()
    source = (root / REGISTER).read_bytes()
    register = json.loads(source)
    inventories = {b['id']: json.loads(safe_file(root, b['inventory'], {b['inventory']}).read_text())
                   for b in register['builds']}
    report = audit(root, register)
    behaviors, stale = enrich_behaviors(register, report)
    snapshot = {
        'schema': 1, 'generated_at': datetime.now(timezone.utc).isoformat(),
        'register_sha256': hashlib.sha256(source).hexdigest(),
        'audit': {k: report.get(k, {}) for k in ('errors', 'warnings', 'counts', 'findings')},
        'behaviors': behaviors, 'binary': binary_view(register, inventories, behaviors),
        'evidence': [{**e, 'freshness': 'stale' if e['id'] in stale else 'current',
                      'changed_sources': stale.get(e['id'], [])} for e in register['evidence']],
        'scenarios': register['scenarios'], 'dispatch_tables': register['dispatch_tables'],
        'summary': {kind: summarize([b for b in behaviors if kind == 'all' or b['kind'] == kind])
                    for kind in ('all', 'recovered', 'native_policy')},
    }
    snapshot['tracks'] = track_view(json.loads((root / TRACKS).read_text()), behaviors, snapshot['evidence'])
    allowed = {REGISTER, TRACKS}
    for b in behaviors:
        for role in ('documents', 'implementation', 'tests'): allowed.update(b[role])
    allowed.update(e['record'] for e in register['evidence'])
    allowed.update(t['document'] for t in register['dispatch_tables'])
    allowed.update(r['document'] for r in register['recovered_ranges'])
    return snapshot, allowed


class CoverageServer(ThreadingHTTPServer):
    daemon_threads = True

    def __init__(self, address, root=ROOT):
        self.root = Path(root).resolve()
        self.refresh_lock = threading.Lock()
        self.snapshot, self.allowed_files = build_snapshot(self.root)
        super().__init__(address, CoverageHandler)


class CoverageHandler(BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def respond(self, content, content_type, status=200):
        self.send_response(status)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(content)))
        self.send_header('Cache-Control', 'no-store')
        self.send_header('X-Content-Type-Options', 'nosniff')
        self.send_header('Content-Security-Policy', "default-src 'self'; style-src 'self' 'unsafe-inline'; script-src 'self'; frame-ancestors 'none'; object-src 'none'")
        self.end_headers()
        self.wfile.write(content)

    def json_response(self, value, status=200):
        self.respond(json.dumps(value).encode(), 'application/json; charset=utf-8', status)

    def local_request(self):
        host = self.headers.get('Host', '')
        accepted = {f'127.0.0.1:{self.server.server_port}', f'localhost:{self.server.server_port}'}
        origin = self.headers.get('Origin')
        return host in accepted and (not origin or origin in {f'http://{h}' for h in accepted})

    def do_GET(self):
        if not self.local_request():
            return self.json_response({'error': 'Use the local dashboard URL'}, 403)
        request = urlsplit(self.path)
        if request.path == '/api/coverage':
            with self.server.refresh_lock:
                return self.json_response(self.server.snapshot)
        if request.path == '/files':
            try:
                path = parse_qs(request.query).get('path', [''])[0]
                with self.server.refresh_lock:
                    target = safe_file(self.server.root, path, self.server.allowed_files)
                return self.respond(target.read_bytes(), 'text/plain; charset=utf-8')
            except (ValueError, OSError) as exc:
                return self.json_response({'error': str(exc)}, 404)
        assets = {'/': ('index.html', 'text/html'), '/index.html': ('index.html', 'text/html'),
                  '/app.js': ('app.js', 'text/javascript'), '/style.css': ('style.css', 'text/css')}
        if request.path not in assets:
            return self.json_response({'error': 'Unknown route'}, 404)
        name, mime = assets[request.path]
        return self.respond((self.server.root / 'apps/coverage-ui' / name).read_bytes(), mime + '; charset=utf-8')

    def do_POST(self):
        if not self.local_request():
            return self.json_response({'error': 'Use the local dashboard URL'}, 403)
        if self.path != '/api/refresh':
            return self.json_response({'error': 'Unknown route'}, 404)
        try:
            with self.server.refresh_lock:
                snapshot, allowed = build_snapshot(self.server.root)
                self.server.snapshot, self.server.allowed_files = snapshot, allowed
            self.json_response(snapshot)
        except (ValueError, KeyError, OSError, TypeError) as exc:
            self.json_response({'error': f'Refresh failed; previous snapshot retained: {exc}'}, 500)
