"""Caller-bound GPU branch geometry; complete calls and whole sessions are distinct."""
from collections import Counter
import hashlib
import json
from pathlib import Path
from native_copy_routes import decode, validate as validate_routes
from native_gpu_copy_diagnostics import census, check_pixels


def signed(value):
    return value if value < 0x80000000 else value - 0x100000000


def union_area(rectangles):
    """Exact half-open rectangle union, including overlapping clip pieces."""
    if any(not (x0 < x1 and y0 < y1) for x0, y0, x1, y1 in rectangles):
        raise ValueError('Empty or inverted emitted geometry')
    edges = sorted({x for r in rectangles for x in (r[0], r[2])})
    area = 0
    for left, right in zip(edges, edges[1:]):
        spans = sorted((y0, y1) for x0, y0, x1, y1 in rectangles if x0 <= left and right <= x1)
        length, end = 0, None
        for low, high in spans:
            if end is None or low > end:
                length += high-low
                end = high
            elif high > end:
                length += high-end
                end = high
        area += (right-left)*length
    return area


def classify_call(row, gpu, checked, *, module_base, module_extent):
    """Requires strictly decoded/bijectively joined v2 metadata and checked pixels.

    No command count or clipper attachment substitutes for reduced geometry.
    A caller must have every output command, including a self snapshot, checked.
    """
    if len(row) != 44 or len(gpu) != 32 or row[15] != 2 or not row[10] & 4:
        raise ValueError('Actual GPU-source route required')
    if not module_base or not module_extent or not module_base <= row[1] < module_base+module_extent or row[2] != row[1]-module_base:
        raise ValueError('Original-module caller identity required')
    if row[13] & 0x80000000 or gpu[17] & 0x80000000 or gpu[18]:
        raise ValueError('Unsuccessful GPU call cannot prove a branch')
    first, last, source, target = row[40:44]
    if not first or not source or not target or last < first:
        raise ValueError('Emitted caller-bound commands required')
    self_copy = bool(row[10] & 128)
    keyed = bool(row[12] & (1 if row[11] else 0x8000))
    attached = bool(row[10] & 32)
    if attached and row[11]:
        raise ValueError('Attached-clipper GPU output requires Blt')
    if keyed and not row[10] & 64:
        raise ValueError('Known original source key required')
    if bool(gpu[15] & 1) != self_copy or bool(gpu[15] & 2) != attached or bool(gpu[15] & 4) != keyed:
        raise ValueError('GPU branch metadata disagreement')
    if (gpu[7], gpu[8], gpu[4], gpu[5]) != (first, last, source, target):
        raise ValueError('GPU resource/command interval disagreement')
    if last-first+1 != gpu[16]+self_copy or not gpu[16]:
        raise ValueError('GPU piece interval disagreement')
    if any(sequence not in checked for sequence in range(first, last+1)):
        raise ValueError('Missing completed per-call output')
    sw, sh, dw, dh = row[16], row[17], row[19], row[20]
    if any(not 1 <= n <= 2048 for n in (sw, sh, dw, dh)) or (row[18], row[21]) != (16, 16):
        raise ValueError('Unsupported copy surface dimensions/format')
    sr, dr = list(map(signed, row[24:28])), list(map(signed, row[28:32]))
    width, height = dr[2]-dr[0], dr[3]-dr[1]
    if min(width, height) <= 0 or [sr[2]-sr[0], sr[3]-sr[1]] != [width, height]:
        raise ValueError('Original unscaled rectangles required')
    domain = [max(0, dr[0]), max(0, dr[1]), min(dw, dr[2]), min(dh, dr[3])]
    bounded = max(0, domain[2]-domain[0])*max(0, domain[3]-domain[1])
    expected_source = source
    if self_copy:
        snapshot = checked[first]
        scratch = gpu[6]
        if not scratch or scratch in (source, target) or source != target:
            raise ValueError('Distinct frozen self-copy resource required')
        if snapshot['words'] != [source, scratch, 0, 0, sw, sh, 0, 0, 0, 0, 0, 0, 0] or [snapshot[k] for k in ('source_width', 'source_height', 'target_width', 'target_height')] != [sw, sh, sw, sh]:
            raise ValueError('Full opaque self snapshot required')
        expected_source = scratch
    pieces = []
    for sequence in range(first+self_copy, last+1):
        record = checked[sequence]
        w = record['words']
        if len(w) != 13 or w[:2] != [expected_source, target] or w[8:10] != [int(keyed), row[22] if keyed else 0] or any(w[10:]):
            raise ValueError('Copy command resource/key identity')
        if [record[k] for k in ('source_width', 'source_height', 'target_width', 'target_height')] != [sw, sh, dw, dh]:
            raise ValueError('Copy command surface extents')
        if not (0 <= w[2] < w[4] <= sw and 0 <= w[3] < w[5] <= sh and sr[0] <= w[2] and w[4] <= sr[2] and sr[1] <= w[3] and w[5] <= sr[3]):
            raise ValueError('Copy command source geometry')
        rect = [w[6], w[7], w[6]+w[4]-w[2], w[7]+w[5]-w[3]]
        if not (domain[0] <= rect[0] < rect[2] <= domain[2] and domain[1] <= rect[1] < rect[3] <= domain[3]) or w[6]-w[2] != dr[0]-sr[0] or w[7]-w[3] != dr[1]-sr[1]:
            raise ValueError('Copy command original destination/translation')
        pieces.append(rect)
    covered = union_area(pieces)
    if covered > bounded or not attached and covered != bounded:
        raise ValueError('Unexplained reduction outside attached clipper')
    return dict(call=row[0], caller=hex(row[1]), first=first, last=last, self_copy=self_copy,
                keyed_gpu=keyed, attached_clipper=attached, requested_rectangle=dr,
                emitted_rectangles=pieces, requested_area=width*height, bounded_area=bounded,
                emitted_union_area=covered, geometrically_reduced=covered < width*height,
                bounds_reduced=bounded < width*height,
                clipper_reduced_within_bounds=attached and covered < bounded,
                retained_commands=sum('files' in checked[s] for s in range(first, last+1)))


def analyze(experiment, directory, *, published_only=False):
    """Prove completed individual calls even when a larger journey failed.

    The closed scalar census remains mandatory. Missing outputs stay explicit;
    a failed END/normal Quit never turns into whole-copy/session closure.
    """
    experiment, directory = Path(experiment), Path(directory)
    if published_only:
        from published_copy_routes import validate_published
        header,rows,routes=validate_published(experiment)
    else:
        routes = validate_routes(experiment)
        if routes['census_version'] != 2:
            raise ValueError('Version2 original call bindings required')
        header, rows = decode(experiment/'lock-capture/copy-routes-v2.bin')
    _, calls = census(experiment/'lock-capture/gpu-copy-census-v1.bin')
    diagnostic = json.loads((directory/'report.json').read_text())
    if diagnostic.get('selection') != 'all-copies' or diagnostic.get('original_pixels_used_as_native_inputs') is not False or diagnostic.get('error'):
        raise ValueError('Independent all-copy diagnostic acquisition required')
    records = [json.loads(line) for line in (directory/'copies.jsonl').read_text().splitlines()]
    checked = {}
    for record in records:
        sequence = record['sequence']
        w = record['words']
        if type(sequence) is not int or sequence <= 0 or sequence in checked or len(w) != 13 or any(type(v) is not int or not 0 <= v <= 0xffffffff for v in w) or any(w[10:]) or w[8] > 1:
            raise ValueError('Invalid/duplicate output command identity')
        if record['mismatches'] or record['actual_sha256'] != record['expected_sha256'] or record['pixels'] != record['target_width']*record['target_height']:
            raise ValueError('Independent native destination mismatch')
        for key in ('actual_sha256', 'expected_sha256', 'source_sha256', 'before_sha256'):
            value=record[key]
            if len(value)!=64 or any(c not in '0123456789abcdef' for c in value):raise ValueError('Invalid pixel digest')
        checked[sequence] = record
    if diagnostic['copies_checked'] != len(records) or diagnostic['copies_seen'] < len(records) or diagnostic['diagnostic_native_readbacks'] != len(records)*3 or diagnostic['pixels_compared'] != sum(x['pixels'] for x in records):
        raise ValueError('Incomplete per-command comparison accounting')
    expected_bytes = sum((x['source_width']*x['source_height']+2*x['target_width']*x['target_height'])*4 for x in records)
    if diagnostic['diagnostic_readback_bytes'] != expected_bytes or expected_bytes > 32*1024**3 or len(records) > 8192:
        raise ValueError('Diagnostic budget/byte accounting')
    retained = [x for x in records if 'files' in x]
    if not retained or len(retained) != diagnostic['retained_pixel_cases']:
        raise ValueError('Missing independently replayable witness')
    retained_pixels = sum(check_pixels(directory, x) for x in retained)
    ownership = set()
    for row in rows:
        first, last = row[40:42]
        if not first:continue
        interval = set(range(first, last+1))
        if ownership & interval:raise ValueError('Application COPY intervals overlap')
        ownership.update(interval)
    proven, missing, failed, empty, foreign = [], [], [], [], []
    eligible = [row for row in rows if row[15] == 2]
    for row in eligible:
        gpu = calls[row[33]-1]
        if row[2] == 0xffffffff:foreign.append(row[0]);continue
        if row[13] & 0x80000000 or gpu[18]:failed.append(row[0]);continue
        if not row[40]:empty.append(row[0]);continue
        if any(s not in checked for s in range(row[40], row[41]+1)):missing.append(row[0]);continue
        proven.append(classify_call(row, gpu, checked, module_base=header[15], module_extent=header[16]))
    selected = dict(attached_clipper=lambda x:x['attached_clipper'], geometric_clipping=lambda x:x['geometrically_reduced'],
                    clipper_reduction=lambda x:x['clipper_reduced_within_bounds'], self_copy=lambda x:x['self_copy'], keyed_gpu=lambda x:x['keyed_gpu'])
    branches = {}
    for name, predicate in selected.items():
        matches = [x for x in proven if predicate(x)]
        branches[name] = dict(proven=bool(matches), calls=len(matches), callers=dict(Counter(x['caller'] for x in matches)), retained_commands=sum(x['retained_commands'] for x in matches), examples=matches[:4])
    original = [x for x in eligible if x[2] != 0xffffffff]
    from native_copy_output import validate as whole_output
    closure, closure_error = None, None
    try:closure = whole_output(experiment, directory)
    except ValueError as error:closure_error = str(error)
    return dict(verified=True, application_observation_complete=routes.get('application_observation_complete',True),active_miss_reason_bits=routes.get('active_miss_reason_bits',0),absence_claimed=False,application_calls=len(rows), gpu_calls=len(eligible), checked_original_gpu_calls=len(proven),
                metadata_self_calls=sum(bool(x[10]&128) for x in original), metadata_keyed_gpu_calls=sum(bool(x[12]&(1 if x[11] else 0x8000)) for x in original),
                missing_output_calls=missing, failed_calls=failed, empty_calls=empty, foreign_calls=foreign,
                branches=branches, diagnostic_ended=diagnostic['ended'], diagnostic_success=diagnostic['success'], diagnostic_truncated=diagnostic['truncated'],
                all_application_output_closed=closure is not None, whole_output_closure=closure, whole_output_error=closure_error,
                routes=routes['routes'], retained_python_pixels=retained_pixels,
                records_sha256=hashlib.sha256((directory/'copies.jsonl').read_bytes()).hexdigest(),census_sha256=routes['census_sha256'],
                scope='Closed actual original-module GPU caller metadata joined only to complete independently compared command intervals. Rectangle unions distinguish attachment,bounds clipping and extra clipper reduction. Individual call proof does not close missing outputs,END,normal Quit,owners,arbitrary readers,other contexts or original-driver parity.')
