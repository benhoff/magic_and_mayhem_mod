#!/usr/bin/env python3
"""Trace original COPY operands and inspect two retained original API journals."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import struct
import subprocess

from copy_alias_callers import inspect
from coverage_evidence import Reader, write_report
from drawing_api_session import BUILD, decode
from drawing_call_intervals import decode_intervals
from drawing_census import direct_call, instructions

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-alias-reachability-results-20261108/'
REFERENCES = ('research/runtime/draw02-integrated-original120-02.json',
              'research/runtime/draw02-integrated-original32-03.json')
SOURCES = ('tools/inspect-copy-alias-callers.py', 'tools/copy_alias_callers.py',
           'tools/drawing_census.py', 'tools/drawing_api_session.py',
           'tools/drawing_call_intervals.py', 'tools/coverage_evidence.py',
           'tests/test-copy-alias-callers.py', 'tests/test-drawing-surface-lifetimes.py',
           'tests/test-drawing-call-intervals.py', 'tools/drawing_surface_lifetimes.py',
           'tools/drawing_lease_intervals.py')


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def original_aliases(journal, intervals):
    decoded = decode(journal)
    ordering = decode_intervals(intervals, journal)
    calls = {r[29]: r for r in struct.iter_unpack('<32I', journal[64:-128])}
    parents = {}

    def owner(identity):
        parents.setdefault(identity, identity)
        while parents[identity] != identity:
            identity = parents[identity]
        return identity

    copies = []
    queries = 0
    for event in struct.iter_unpack('<16I', intervals[64:-64]):
        _, phase, entry = event[:3]
        row = calls[entry]
        if phase == 1 and row[1] == 1 and row[2] in (5, 7) and row[4]:
            copies.append(dict(entry=entry, caller=row[5], method=row[2],
                               source=row[4], destination=row[3], hresult=row[8],
                               same_generation=row[3] == row[4],
                               prior_query_alias=owner(row[3]) == owner(row[4])))
        if phase == 2 and row[1] in (1, 5) and row[2] == 0 and not row[8] & 0x80000000 and row[3] and row[4]:
            parents[owner(row[4])] = owner(row[3])
            queries += 1
    return dict(copies=len(copies), same_generation=sum(v['same_generation'] for v in copies),
                prior_query_alias=sum(v['prior_query_alias'] for v in copies),
                successful_queries=queries, alias_candidates=[v for v in copies if v['prior_query_alias']],
                caller_counts={hex(pc): sum(v['caller'] == pc for v in copies)
                               for pc in sorted({v['caller'] for v in copies})},
                original_records=decoded['records'], interval_events=ordering['events'],
                unknown_backing_relationships=sum(not v['prior_query_alias'] for v in copies),
                global_absence_proved=False, output_compared=False)


def archived_bytes(reader, archive, digest, size):
    if not 0 <= size <= 8 * 1024**2:
        raise ValueError('Journal input exceeds bounded metadata budget')
    try:
        store = reader.resolver.find(digest, size)
    except ValueError:
        store = None
    if store is not None:
        output = io.BytesIO()
        store.copy(digest, size, output)
        raw = output.getvalue()
        source = dict(kind='verified_sqlite_object', sha256=digest, size=size)
    else:
        path = archive/'objects'/digest[:2]/(digest+'.zst')
        # Read only the named small metadata object; never restore an execution tree.
        with subprocess.Popen(['zstd', '-q', '-dc', '--', str(path)], stdout=subprocess.PIPE) as process:
            raw = process.stdout.read(size + 1)
            if len(raw) > size:
                process.kill()
                raise ValueError('Archive exceeded declared metadata extent')
            if process.wait():
                raise ValueError('Archive metadata decompression failed')
        source = dict(kind='legacy_compressed_object', path=str(path),
                      compressed_sha256=sha(path.read_bytes()), sha256=digest, size=size)
    if len(raw) != size or sha(raw) != digest:
        raise ValueError('Retained journal identity differs')
    return raw, source


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--archive-root', type=Path, default=Path('/var/local/magic-and-mayhem-archive'))
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    if not args.report.startswith(PREFIX) or '..' in Path(args.report).parts or (ROOT/args.report).exists():
        parser.error('Fresh owned report path required')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        raw = args.executable.read_bytes()
        if sha(raw) != BUILD:
            raise ValueError('Unsupported executable; refusing fixed addresses')
        with Reader(ROOT) as reader:
            sources = {name: reader.digest(name) for name in SOURCES}
            assembly = subprocess.check_output(['objdump', '-w', '-d', '-Mintel', str(args.executable)], text=True)
            code = instructions(assembly)
            result = inspect(code)
            prior_name = 'research/runtime/native-copy-route-static-20261017.json'
            prior = reader.json(prior_name)
            expected = {(int(c['call_va'], 16), wrapper['entry'])
                        for wrapper in prior['selected_wrappers'].values() for c in wrapper['callers']}
            if {(v['call'], v['wrapper']) for v in result['calls']} != expected or len(expected) != 156:
                raise ValueError('Original complete direct-call candidate inventory changed')
            for row in result['calls']:
                row['instructions'] = [v for pc, v in sorted(code.items()) if row['block_start'] <= pc <= row['call']]
            ordered = list(code.values())
            publications = {}
            for label, target in [('image_destination_setter', 0x572f20), ('bar_draw', 0x4dd9b0),
                                  ('deferred_queue_writer', 0x58d5b0)]:
                publications[label] = [dict(call=row['address'], target=target, instructions=ordered[max(0, i-8):i+1])
                                       for i, row in enumerate(ordered) if direct_call(row) == target]
            setters = publications['image_destination_setter']
            if {v['call'] for v in setters} != {0x4b539f, 0x55176b, 0x57907b} or any(
                    v['instructions'][-2]['bytes'] != '68e0496a00' for v in setters):
                raise ValueError('Generic image destination publication differs')
            if {v['call'] for v in publications['bar_draw']} != {0x4b506c, 0x4c8351}:
                raise ValueError('Two-piece bar direct-caller leads differ')
            windows = {name: [v for pc, v in sorted(code.items()) if start <= pc < end]
                       for name, (start, end) in {
                           'deferred_queue_writer': (0x58d5b0, 0x58d63a),
                           'deferred_queue_drain': (0x58d660, 0x58d743),
                           'generic_image_draw_and_destination_setter': (0x572e90, 0x572f2a),
                           'two_piece_bar_copy': (0x4dd9b0, 0x4ddaca),
                           'bar_default_fields': (0x4dd970, 0x4dd992),
                       }.items()}
            journals = []
            for name in REFERENCES:
                ref = reader.json(name)
                if not ref['success'] or ref['source_sha256'] != BUILD or not ref['original_drawing_active']:
                    raise ValueError('Complete original-active reference required')
                js = ref['observation']['journal_sha256']
                journal, jsource = archived_bytes(reader, args.archive_root, js, 64 + ref['observation']['records'] * 128)
                ins = ref['call_intervals']['interval_sha256']
                intervals, isource = archived_bytes(reader, args.archive_root, ins, 64 + ref['call_intervals']['events'] * 64)
                tag = 'original120' if 'original120' in name else 'original32'
                for label, body, source in [('journal', journal, jsource), ('intervals', intervals, isource)]:
                    record = args.report.removesuffix('.json') + '-' + tag + '-' + label + '.bin'
                    source['record'] = record
                    write_report(ROOT, record, body)
                journals.append(dict(reference=dict(record=name, sha256=reader.digest(name)),
                                     journal=jsource, intervals=isource, **original_aliases(journal, intervals)))
            if sources != {name: reader.digest(name) for name in SOURCES} or sha(args.executable.read_bytes()) != BUILD:
                raise ValueError('Sources changed during analysis')
            result.update(schema=1, kind='original_copy_alias_reachability_leads', success=True,
                          original_sha256=BUILD, sources=sources, sources_stable=True,
                          static_analysis_only=True, original_game_runs=0, new_acceptance_rows=0,
                          original_driver_journals=journals, selected_original_windows=windows,
                          publication_leads=publications,
                          static_inventory=dict(record=prior_name, sha256=reader.digest(prior_name)),
                          objdump_version=subprocess.check_output(['objdump', '--version'], text=True).splitlines()[0],
                          remaining=['Straight-line expressions do not resolve predecessor/indirect flow, object allocation or backing aliases.',
                                     'Distinct wrapper constants and absent prior QueryInterface links never prove disjoint backing.',
                                     'No natural overlapping invocation or independently compared output; both alias rows remain pending.'])
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)
    result['original_manifest_verified_before_after'] = True
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, record=args.report, sha256=digest, direct_calls=result['direct_calls'],
                          static_relationships=result['counts'], original_copies=sum(v['copies'] for v in journals),
                          original_aliases=sum(v['prior_query_alias'] for v in journals), newly_closed_rows=0)))


if __name__ == '__main__':
    main()
