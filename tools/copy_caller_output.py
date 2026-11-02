"""Positive GPU output links; unresolved callers and missing intervals stay gaps."""
import json
from pathlib import Path

from natural_gpu_copy_branches import analyze as analyze_gpu, classify_call
from native_gpu_copy_diagnostics import census
from native_copy_routes import decode


def join_resolved_gpu(callers, routes, proven):
    """Only credit an original caller after its entire GPU interval was proved."""
    if len(callers) != len(routes):
        raise ValueError('Caller/output row bijection')
    eligible = set()
    for ordinal, (caller, route) in enumerate(zip(callers, routes), 1):
        if caller[0] != ordinal or route[0] != ordinal or caller[10] != route[15]:
            raise ValueError('Caller/output ordinal or route binding')
        if route[15] == 2:
            eligible.add(ordinal)
    if not set(proven) <= eligible:
        raise ValueError('Output proof has no GPU application call')
    return [dict(proven[c[0]], upstream_return_rva=hex(c[6]), wrapper_entry_rva=hex(c[3]))
            for c, r in zip(callers, routes) if c[9] == 1 and r[15] == 2 and c[0] in proven]


def analyze(experiment, directory, callers):
    experiment, directory = Path(experiment), Path(directory)
    # Existing bounded proof checks every diagnostic record/counter, separately
    # replays retained pixels, and preserves strict whole-output refusal.
    gpu = analyze_gpu(experiment, directory)
    header, routes = decode(experiment/'lock-capture/copy-routes-v2.bin')
    _, gpu_rows = census(experiment/'lock-capture/gpu-copy-census-v1.bin')
    records = [json.loads(line) for line in (directory/'copies.jsonl').read_text().splitlines()]
    checked = {record['sequence']: record for record in records}
    proven = {}
    for route in routes:
        if route[15] != 2 or route[2] == 0xffffffff or route[13] & 0x80000000:
            continue
        row = gpu_rows[route[33]-1]
        if row[18] or not route[40] or any(seq not in checked for seq in range(route[40], route[41]+1)):
            continue
        proven[route[0]] = classify_call(route, row, checked, module_base=header[15], module_extent=header[16])
    positive = join_resolved_gpu(callers, routes, proven)
    missing = [dict(call=r[0], first=r[40], last=r[41], missing_sequences=[s for s in range(r[40], r[41]+1) if s not in checked])
               for r in routes if r[15] in (1, 2) and r[40] and any(s not in checked for s in range(r[40], r[41]+1))]
    return dict(verified=bool(positive), upstream_gpu_calls_with_complete_output=len(positive),
                positive_calls=positive, missing_output_intervals=missing,
                cpu_upstream_pixel_links_claimed=False, gpu_output=gpu,
                all_application_output_closed=gpu['all_application_output_closed'],
                scope='Only resolved upstream original callers with complete independently checked GPU command intervals earn output credit. CPU provenance has no new per-caller pixel claim. Missing intervals, unresolved callers and full-output refusal remain explicit.')
