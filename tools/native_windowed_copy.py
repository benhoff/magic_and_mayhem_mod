"""Explicit original windowed launch and bounded natural copy/reader proof.

No original dispatch, clip rectangles, pixel inputs or fallback policy is edited.
"""
from collections import Counter


def screen_spec(windowed):
    # Original windowed initialization consumes the desktop's 16-bit mode.
    return '1280x1024x16' if windowed else '1280x1024x24'


def launch_arguments(environment):
    value = environment.get('MNM_ORIGINAL_WINDOWED_DIAGNOSTIC', '0')
    if value not in ('0', '1'):
        raise ValueError('Invalid original windowed diagnostic selection')
    return ['--', '-win'] if value == '1' else []


def validate(rows, output, *, module_base, module_extent):
    """Require actual original-module Blt callers with clipped GPU command output.

    The caller supplies already verified v2 census rows and independent all-COPY
    output closure. Merely attaching a clipper or counting a call grants no proof.
    """
    if not output.get('verified') or output.get('application_calls') != len(rows):
        raise ValueError('Independent complete caller/output closure required')
    if not module_base or not module_extent:
        raise ValueError('Observed original module identity required')
    selected = []
    for row in rows:
        if row[15] != 2 or not row[10] & 32:
            continue
        if row[11] or not row[10] & 4 or not row[23]:
            raise ValueError('Clipped GPU call lacks Blt/stale-source/clipper identity')
        if not module_base <= row[1] < module_base + module_extent or row[2] != row[1] - module_base:
            raise ValueError('Clipped GPU call lacks original module caller binding')
        first, last, source, target = row[40:44]
        if not first or last < first or not source or not target:
            raise ValueError('Clipped GPU call lacks checked native COPY output')
        if row[13] & 0x80000000:
            raise ValueError('Natural clipped GPU call failed')
        selected.append(row)
    if not selected:
        raise ValueError('No natural original clipped GPU output calls observed')
    return dict(verified=True, calls=len(selected),
                command_count=sum(r[41] - r[40] + 1 for r in selected),
                callers={hex(pc): n for pc, n in sorted(Counter(r[1] for r in selected).items())},
                scope='Natural original-module Blt with attached destination clipper and independently checked emitted COPY output. Actual geometric clipping, arbitrary raw-reader coherence, self/keyed-GPU reachability and original-driver equivalence remain separate.')
