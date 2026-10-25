"""Strict comparator-only public SHM upload samples; never native inputs."""
from hashlib import sha256
from pathlib import Path
import struct

import numpy as np


def admit_namespace(root, record):
    """Require the recorded task-owned mount AND network namespace, not host X11."""
    from coverage_evidence import evidence_digest, read_json
    root = Path(root).resolve()
    value = read_json(record, root=root)
    if value.get('schema') != 1 or value.get('root') != str(root):
        raise ValueError('Execution namespace owner/root')
    owner = Path('/proc')/str(value['owner_pid'])
    start = (owner/'stat').read_text().rpartition(')')[2].split()[19]
    actual = {k:str((Path('/proc/self/ns')/k).readlink()) for k in ('mnt', 'net')}
    owned = {k:str((owner/'ns'/k).readlink()) for k in ('mnt', 'net')}
    if (start != value['owner_start'] or actual != owned or actual != value['owned'] or
            any(actual[k] == value['host'][k] for k in actual)):
        raise ValueError('Execution must enter both task mount and network namespaces')
    point = str(root/'working/tmp')
    if not any(line.split()[4] == point and line.split(' - ',1)[1].startswith('tmpfs ')
               for line in Path('/proc/self/mountinfo').read_text().splitlines()):
        raise ValueError('Task temporary execution storage is not the admitted tmpfs')
    return dict(record=Path(record).resolve().relative_to(root).as_posix(),
                sha256=evidence_digest(record, root=root), owner_pid=value['owner_pid'],
                namespaces=actual, private_mount_and_network=True)


def decode(raw):
    if len(raw) != 192 or raw[:8] != b'MNMESH01':
        raise ValueError('SHM source metadata extent/magic')
    w = struct.unpack('<46I', raw[8:])
    begin, end = w[20] | w[21] << 32, w[22] | w[23] << 32
    if (w[:2] != (1, 192) or not all(w[2:5]) or not 1 <= w[4] <= 16 or
            w[5] not in (1, 2, 3, 4, 5, 6) or not w[10] or not w[11] or
            w[18] not in (1, 2, 4) or w[19] or w[45] != 2 or not begin or end < begin or
            w[17] not in (0, 1) or w[42] not in (0, 1)):
        raise ValueError('SHM source identity/state')
    args = list(w[24:40])
    if w[8] != args[14] or (w[5] == 1 and (
            not all(w[6:9]) or w[14] != args[1] * args[2] * 4 or not 0 < w[14] <= 8*1024*1024 or
            w[15] < args[15] + w[14] or w[17] != 1 or w[42] != 1 or args[9:14] != [24, 2, 0, 0, 0] or
            args[7:9] != [0, 0] or args[3] + w[10] > args[1] or args[4] + w[11] > args[2] or
            args[5] < w[10] or args[6] < w[11])):
        raise ValueError('SHM source layout/bounds/stability')
    return dict(pid=w[2], thread=w[3], sequence=w[4], status=w[5], connection=w[6],
                generation=w[7], segment=w[8], sysv=w[9], width=w[10], height=w[11],
                previous_width=w[12], previous_height=w[13], bytes=w[14], segment_bytes=w[15],
                cookie=w[16], stable=bool(w[17]), reason=w[18], begin_ns=begin, end_ns=end,
                args=args, previous_image_width=w[40], previous_image_height=w[41],
                duplicate_equal=bool(w[42]), errno_before=w[43], errno_after=w[44],
                extent_source='public_xcb_geometry')


def read_point(path):
    """One complete file pair. This grants no process/cohort completion credit."""
    path = Path(path)
    if path.is_symlink():
        raise ValueError('SHM metadata symlink')
    raw = path.read_bytes()
    row = decode(raw)
    stem = f"shm-source-{row['pid']}-{row['sequence']:06d}"
    if path.name != stem+'.bin':
        raise ValueError('SHM metadata name/identity')
    if row['status'] != 1:
        raise ValueError('SHM source refused/unstable: '+path.name+' status '+str(row['status']))
    pixels = path.parent/(stem+'.bgra')
    if pixels.is_symlink():
        raise ValueError('SHM source pixels symlink')
    data = pixels.read_bytes()
    if len(data) != row['bytes']:
        raise ValueError('SHM source pixel extent')
    return dict(row, path=path.name, sha256=sha256(raw).hexdigest(),
                pixels_path=pixels.name, pixels_sha256=sha256(data).hexdigest(), data=data)


def samples(directory):
    directory = Path(directory)
    result = [read_point(p) for p in sorted(directory.glob('shm-source-*-*.bin'))]
    if not result:
        raise ValueError('No public SHM source samples')
    for pid in {r['pid'] for r in result}:
        group = [r for r in result if r['pid'] == pid]
        if sorted(r['sequence'] for r in group) != list(range(1, len(group)+1)):
            raise ValueError('Missing/duplicate SHM source sequence')
        terminal = directory/f'shm-source-{pid}-000000.terminal'
        if terminal.is_symlink():
            raise ValueError('SHM terminal symlink')
        raw = terminal.read_bytes()
        if len(raw) != 24:
            raise ValueError('SHM terminal extent')
        version, owner, count, exhausted, generations, connections = struct.unpack('<6I', raw)
        if (version, owner, count, exhausted) != (1, pid, len(group), 0) or not generations or not connections:
            raise ValueError('SHM terminal count/budget')
    return result


def bind(sample, row):
    if any(sample[k] != row[k] for k in ('pid', 'thread', 'width', 'height')):
        raise ValueError('SHM sample/frame identity')
    matches = []
    for packet in row['xcb_images']:
        args = [packet['drawable'], packet['total_width'], packet['total_height'],
                packet['source_x'], packet['source_y'], packet['source_width'], packet['source_height'],
                packet['destination_x'] & 0xffffffff, packet['destination_y'] & 0xffffffff,
                packet['depth'], packet['format'], packet['send_event'], packet['left_pad'],
                packet['data_bytes'], packet['shm_segment'], packet['shm_offset']]
        if (packet['method'] in (1, 2) and args == sample['args'] and
                sample['cookie'] == packet['cookie_sequence'] and
                packet['begin_ns'] <= sample['begin_ns'] <= sample['end_ns'] <= packet['end_ns']):
            matches.append(packet)
    if len(matches) != 1:
        raise ValueError('SHM sample requires one exact enclosing forwarded upload')
    return matches[0]


def rgb565(words):
    words = np.asarray(words, dtype=np.uint16)
    return np.stack([(words >> 11) * 255 // 31, ((words >> 5) & 63) * 255 // 63,
                     (words & 31) * 255 // 31], axis=2).astype(np.int16)


def image(sample):
    a = sample['args']
    if sample['status'] != 1 or not sample['stable'] or not sample['duplicate_equal']:
        raise ValueError('Refused or unstable SHM image')
    if len(sample['data']) != sample['bytes'] or sha256(sample['data']).hexdigest() != sample['pixels_sha256']:
        raise ValueError('SHM immutable pixel sample changed')
    words = np.frombuffer(sample['data'], dtype='<u4').reshape(a[2], a[1])
    return np.stack([(words >> 16) & 255, (words >> 8) & 255, words & 255], axis=2).astype(np.int16)


def difference(a, b):
    if a.shape != b.shape:
        raise ValueError('Full-plane comparison extent')
    return int(np.count_nonzero(np.max(abs(a.astype(np.int16)-b.astype(np.int16)), axis=2) > 1))


def compare(sample, source_words, destination_words, physical):
    """Explain one actual upload, independently of a proposed crop policy."""
    width, height = sample['width'], sample['height']
    a = sample['args']
    if source_words.shape != (height, width) or destination_words.shape != source_words.shape or physical.shape != (height, width, 3):
        raise ValueError('Causal witness full-plane extent')
    plane = image(sample)
    submitted = plane[a[4]:a[4]+height, a[3]:a[3]+width]
    source, destination = rgb565(source_words), rgb565(destination_words)
    result = dict(submitted_physical_mismatches=difference(submitted, physical),
                  submitted_destination_mismatches=difference(submitted, destination),
                  source_destination_mismatching_words=int(np.count_nonzero(source_words != destination_words)),
                  destination_physical_mismatches=difference(destination, physical),
                  channel_tolerance=1, alpha_excluded=True,
                  original_pixels_used_as_native_inputs=False)
    # This is a diagnostic comparison, never a rewritten upload or accepted oracle.
    delta = sample['previous_height']-height
    if sample['previous_width'] >= width and delta > 0 and delta+height <= a[2]:
        candidate = plane[delta:delta+height, :width]
        result.update(logical_height_delta=delta,
                      geometry_delta_destination_mismatches=difference(candidate, destination))
    result['after_nominal_destination'] = (
        result['source_destination_mismatching_words'] == 0 and
        result['destination_physical_mismatches'] > 0 and
        result['submitted_physical_mismatches'] == 0)
    return result


def analyze_original(original, directory, require_samples=True, point_only=False):
    """Bind actual submitted bytes to independent original display witnesses."""
    from drawing_egl_destination import plane as destination_plane
    from drawing_egl_gpu_reference import compare as validate_source
    rows = original['egl_swap_completion_observation']['frames']
    physical_rows = original['egl_physical_observation']['frames']
    if not rows or len(rows) != len(physical_rows):
        raise ValueError('Incomplete original source/destination/physical cohort')
    directory = Path(directory)
    planes, checked = {}, []
    for row, physical in zip(rows, physical_rows, strict=True):
        if (row['ordinal'] != physical['ordinal'] or row['call'] != physical['entry'] or
                any(row[k] != physical[k] for k in ('pid', 'thread', 'width', 'height'))):
            raise ValueError('Original causal frame identity')
        w, h = row['width'], row['height']
        name = row['gpu_source']['path']
        if Path(name).name != name or (directory/name).is_symlink():
            raise ValueError('Unsafe original source witness')
        source = (directory/name).read_bytes()
        # Validate the independent source projection and exact original hash only;
        # these oracle bytes are never passed to a native rendering process.
        validate_source(original, directory, row['ordinal'], source, w, h)
        source = np.frombuffer(source, dtype='<u2').reshape(h, w)
        destination = destination_plane(directory, row)
        name = physical['path']
        if Path(name).name != name or (directory/name).is_symlink():
            raise ValueError('Unsafe original physical witness')
        rgba = (directory/name).read_bytes()
        if len(rgba) != w*h*4 or sha256(rgba).hexdigest() != physical['sha256']:
            raise ValueError('Original physical witness changed')
        visible = np.frombuffer(rgba, dtype=np.uint8).reshape(h, w, 4)[:,:,:3]
        checked.append(dict(ordinal=row['ordinal'],
            source_destination_mismatching_words=int(np.count_nonzero(source != destination)),
            destination_physical_mismatches=difference(rgb565(destination), visible)))
        planes[row['ordinal']] = source, destination, visible
    bound, unbound = [], []
    selected = ([read_point(p) for p in sorted(directory.glob('shm-source-*-*.bin'))]
                if point_only else samples(directory)) if require_samples else []
    for sample in selected:
        matches = []
        for row in rows:
            try:
                packet = bind(sample, row)
            except ValueError:
                continue
            matches.append((row, packet))
        if not matches:
            unbound.append({k:v for k,v in sample.items() if k != 'data'})
            continue
        if len(matches) != 1:
            raise ValueError('Ambiguous original upload/source binding')
        row, packet = matches[0]
        metrics = compare(sample, *planes[row['ordinal']])
        bound.append(dict(ordinal=row['ordinal'], call=row['call'], packet=packet['packet'],
            sample={k:v for k,v in sample.items() if k != 'data'}, **metrics))
    shrinks = [row['ordinal'] for row in rows if any(
        p.get('previous_width', 0) >= row['width'] and p.get('previous_height', 0) > row['height']
        for p in row['xcb_images'])]
    if not shrinks or require_samples and any(sum(b['ordinal'] == n for b in bound) != 1 for n in shrinks):
        raise ValueError('Missing exact source sample for an observed logical shrink')
    if any(r['source_destination_mismatching_words'] for r in checked):
        raise ValueError('Disagreement precedes the nominal destination')
    explained = {r['ordinal'] for r in bound if r['after_nominal_destination']}
    failures = {r['ordinal'] for r in checked if r['destination_physical_mismatches']}
    if require_samples and failures-explained:
        raise ValueError('A physical mismatch lacks an exact submitted-source explanation')
    return dict(frames=len(rows), comparisons=checked, upload_samples=bound,
                submitted_source_captured=require_samples,
                complete_process_sample_cohort_accepted=require_samples and not point_only,
                sample_admission='individually_complete_points' if point_only else 'closed_process_cohort' if require_samples else 'none',
                unbound_diagnostic_samples=unbound, shrink_ordinals=shrinks,
                physical_mismatch_ordinals=sorted(failures), explained_ordinals=sorted(explained),
                every_physical_difference_explained=bool(failures) and failures == explained,
                all_source_destination_words_match=True,
                channel_tolerance=1, alpha_excluded=True,
                original_pixels_used_as_native_inputs=False)
