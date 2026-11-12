"""Retained COPY branch observations, without turning absence into acceptance."""
from collections import Counter


def signed(word):
    return word if word < 0x80000000 else word - 0x100000000


def classify(row):
    """Geometry of prepared copies only; early refusals can contain defaults."""
    if len(row) != 44:
        raise ValueError('Version2 COPY metadata required')
    if not row[5]:
        return dict(source_copy=False)
    alias = bool(row[10] & 128)
    result = dict(source_copy=True, tracked_alias=alias,
                  geometry_known=row[15] in (1, 2),
                  succeeded=not bool(row[13] & 0x80000000))
    if not result['geometry_known']:
        return result
    source = list(map(signed, row[24:28]))
    target = list(map(signed, row[28:32]))
    sw, sh = source[2]-source[0], source[3]-source[1]
    dw, dh = target[2]-target[0], target[3]-target[1]
    if min(sw, sh, dw, dh) <= 0:
        raise ValueError('Prepared COPY has nonpositive geometry')
    result.update(stretched=(sw, sh) != (dw, dh),
                  overlapping=alias and max(source[0], target[0]) < min(source[2], target[2])
                  and max(source[1], target[1]) < min(source[3], target[3]))
    return result


def summarize(rows):
    counts = Counter()
    examples = {}
    for row in rows:
        value = classify(row)
        counts['calls'] += 1
        if not value['source_copy']:
            counts['source_less_calls'] += 1
            continue
        for key in ('tracked_alias', 'geometry_known', 'stretched', 'overlapping'):
            if value.get(key):
                counts[key] += 1
                if len(examples.setdefault(key, [])) < 3:
                    examples[key].append(list(row))
        if not value['geometry_known']:
            counts['unknown_geometry'] += 1
        if value.get('stretched') and value['succeeded']:
            counts['successful_prepared_stretch'] += 1
        if value.get('overlapping') and value['succeeded']:
            counts['successful_prepared_overlap'] += 1
        if value['tracked_alias'] and not value['succeeded']:
            counts['failed_tracked_alias'] += 1
    names = ('calls', 'source_less_calls', 'tracked_alias', 'geometry_known',
             'stretched', 'overlapping', 'unknown_geometry',
             'successful_prepared_stretch', 'successful_prepared_overlap',
             'failed_tracked_alias')
    return dict(counts={key: counts[key] for key in names}, examples=examples,
                output_verified=False, natural_reachability_proved=False,
                acceptance_ready=False)
