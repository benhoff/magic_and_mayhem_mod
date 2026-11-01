#!/usr/bin/env python3
"""Recheck retained COPY refusals, clipped output and late destination history."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract, scenario_contract, required_sources
from native_copy_routes import validate as validate_routes, decode
from native_cursor_journal import decode as decode_cursor
from natural_gpu_copy_branches import analyze
from rendering_operations_history import (selected_members, pinned_member, refused_copies,
                                         plane_differences, locate_wave, sha)

ROOT = Path(__file__).resolve().parents[1]
RESULTS = 'research/runtime/rendering-operations-history-results-20261101'
SOURCES = ['tools/review-rendering-operations-history.py', 'tools/rendering_operations_history.py',
           'tools/native_copy_routes.py', 'tools/native_copy_output.py',
           'tools/native_gpu_copy_diagnostics.py', 'tools/natural_gpu_copy_branches.py',
           'tools/published_copy_routes.py', 'tools/native_cursor_journal.py',
           'tests/test-rendering-operations-history.py', 'tools/coverage_claims.py', 'runtime/render/lock_surfaces.h',
           'compat/legacy/world_frame.cpp', 'renderer/blit.cpp']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    path = args.report if args.report.is_absolute() else ROOT/args.report
    if path.exists() or not path.resolve().is_relative_to(ROOT/'research/runtime'):
        parser.error('Require a new durable research report path')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='TOOL.rendering-operations-history-review')
    scenario=next(s for s in register['scenarios'] if s['id']=='SC.tooling.rendering-operations-history')
    if not required_sources(behavior)<=set(SOURCES):
        parser.error('Declared review source closure is incomplete')
    claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{}),
                 scenarios={scenario['id']:scenario_contract(scenario)})]
    report = dict(schema=1, success=False, claims=claims, inputs={}, sources={p:sha((ROOT/p).read_bytes()) for p in SOURCES},
                  fresh_live_execution=False, fresh_native_renderer_execution=False,
                  original_pixels_used_as_native_inputs=False, complete_replacement=False,
                  scope='Fresh offline reader/control execution over immutable historical acquisitions. No new game, driver or native raster execution; no operation/configuration acceptance promotion.')
    try:
        control = subprocess.run([sys.executable, '-B', str(ROOT/'tests/test-rendering-operations-history.py')],
                                 cwd=ROOT, capture_output=True, text=True, timeout=60)
        report['controls'] = dict(success=control.returncode == 0, returncode=control.returncode,
                                  output=(control.stdout+control.stderr)[-16000:])
        if control.returncode:
            raise ValueError('Residual review controls failed')
        with Reader(ROOT) as reader, tempfile.TemporaryDirectory(prefix='mnm-operations-history-') as temp:
            temp = Path(temp)
            def read(name, binary=False):
                report['inputs'][name] = reader.digest(name)
                return reader.read_bytes(name) if binary else reader.json(name)
            retention = read('research/runtime/player-open-items-merge-results-20261027/storage01-retention.json')
            raw = read(retention['archive'], True)
            if sha(raw) != retention['archive_sha256']:
                raise ValueError('Cursor archive pin differs')
            wanted = {}
            for tree in retention['trees'].values():
                for name, row in tree['files'].items():
                    if name in ('lock-capture/copy-routes-v2.bin', 'lock-capture/gpu-copy-census-v1.bin', 'native-cursor-lifetime.bin'):
                        if name in wanted or row['archive'] != retention['archive']:
                            raise ValueError('Mixed cursor capture')
                        wanted[name] = row
            if len(wanted) != 3:
                raise ValueError('Missing cursor capture members')
            by_hash = {v['sha256']: k for k,v in wanted.items()}
            files = selected_members(raw, by_hash.get)
            cursor_root = temp/'cursor'
            for name, row in wanted.items():
                data = pinned_member(files[name], row)
                target = cursor_root/name;target.parent.mkdir(parents=True, exist_ok=True);target.write_bytes(data)
            routes = validate_routes(cursor_root)
            _, rows = decode(cursor_root/'lock-capture/copy-routes-v2.bin')
            refusals = refused_copies(rows)
            report['cursor'] = dict(routes=routes['routes'], application_calls=len(rows),
                                    failed_calls=routes['failed_calls'], refusals=refusals,
                                    journal=decode_cursor(files['native-cursor-lifetime.bin']),
                                    invalid_rectangle_refusals_established=bool(refusals) and all(r['invalid_fast_rectangle_established'] for r in refusals),
                                    original_driver_return_equivalence=False,
                                    caller_coordinate_provenance_complete=False)
            historical = read('research/runtime/cpu-copy-results-20261026/replay02.json')
            live = read('research/runtime/cpu-copy-results-20261026/quick02.json')
            raw_name = live['raw_record']
            # Older reports may retain an absolute producer checkout path.
            raw_name = 'research/runtime/'+raw_name.split('/research/runtime/')[-1] if '/research/runtime/' in raw_name else raw_name
            raw = read(raw_name, True)
            if sha(raw) != historical['input_archive']['sha256']:
                raise ValueError('CPU-copy archive pin differs')
            def copy_selection(name):
                return name if name.endswith(('/lock-capture/copy-routes-v2.bin','/lock-capture/gpu-copy-census-v1.bin')) or '/copy-diagnostics/' in name else None
            files = selected_members(raw, copy_selection)
            copy_root = temp/'copy'
            for name, data in files.items():
                target=copy_root/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
            censuses=list(copy_root.rglob('copy-routes-v2.bin'))
            diagnostics=[p for p in copy_root.rglob('report.json') if p.parent.name=='copy-diagnostics']
            if len(censuses)!=1 or len(diagnostics)!=1:
                raise ValueError('Require one complete unmixed COPY acquisition')
            report['cpu_copy']=analyze(censuses[0].parent.parent, diagnostics[0].parent)
            report['cpu_copy']['historical_analysis_equal'] = report['cpu_copy'] == historical['analysis']
            report['cpu_copy_lifecycle'] = {k:live.get(k) for k in ('original_game_executed','lifecycle_passed','retirement_passed','sources_stable')}
            natural = read('research/runtime/rendering-remaining-results-20261026/natural-late-comparison-02.json')
            catalog = read('research/runtime/rendering-remaining-results-20261026/raw-catalog-01.json')
            raw = read('research/runtime/rendering-remaining-results-20261026/raw-execution-01.tar.gz', True)
            if sha(raw) != catalog['archive_sha256']:
                raise ValueError('History archive pin differs')
            selected = {}
            for frame in natural['frames']:
                seq = frame['sequence']
                for suffix in ('before.565','after.565','bin'):
                    name=f'working/experiments/scene-observer/run-kinxgh80/capture/{seq}.{suffix}'
                    selected[name] = natural['inputs'][name]
                selected[f'working/tests/natural-late-comparison-02/{seq}-original-zero.565'] = frame['original_zero_sha256']
            files=selected_members(raw, lambda name:name if name in selected else None)
            if set(files)!=set(selected) or any(sha(files[n])!=h for n,h in selected.items()):
                raise ValueError('History selected member pin differs')
            history=[]
            for frame in natural['frames']:
                seq=frame['sequence'];prefix=f'working/experiments/scene-observer/run-kinxgh80/capture/{seq}'
                before,after=files[prefix+'.before.565'],files[prefix+'.after.565']
                zero=files[f'working/tests/natural-late-comparison-02/{seq}-original-zero.565']
                if sha(zero)!=frame['native_sha256']:
                    raise ValueError('Retained native/zero identity differs')
                differences=plane_differences(zero,after,frame['width'],frame['height'])
                history.append(dict(sequence=seq, differences=differences,
                                    native_zero_identity_reverified=True,
                                    retained_native_sha256=frame['native_sha256'],
                                    before_at_target=struct.unpack_from('<H',before,(433*frame['width']+548)*2)[0]))
            first=natural['frames'][0];prefix=f"working/experiments/scene-observer/run-kinxgh80/capture/{first['sequence']}"
            dependency=locate_wave(files[prefix+'.bin'],1261,548,433)
            sx,sy=dependency['source']
            dependency['frame_entry_source_value']=struct.unpack_from('<H',files[prefix+'.before.565'],(sy*dependency['stride']+sx)*2)[0]
            report['history']=dict(frames=history, selected_dependency=dependency,
                                   all_eight_reproduce_one_pixel=len(history)==8 and all(f['differences']==dict(mismatches=1,samples=[dict(x=548,y=433,left=0,right=6338)]) for f in history),
                                   prior_writer_provenance_complete=False,
                                   fresh_raster_execution=False,
                                   scope='Full retained zero/live planes rechecked. Zero-plane bytes match the historical native output hash. Selected dependency locator identifies a required source coordinate, but does not replay the prefix, prove sprite coverage or identify its prior writer.')
            # Compare historical source inventories without refreshing any old hash.
            cache={}
            applicability={}
            for label, previous in [('cpu_copy',historical),('late_history',natural)]:
                changed=[];missing=[]
                for name, expected in previous['sources'].items():
                    if name not in cache:
                        p=ROOT/name;cache[name]=sha(p.read_bytes()) if p.is_file() else None
                    if cache[name] is None:missing.append(name)
                    elif cache[name]!=expected:changed.append(name)
                applicability[label]=dict(declared_files=len(previous['sources']),changed=changed,missing=missing,
                                           all_declared_sources_current=not changed and not missing,
                                           historical_hashes_preserved=True,
                                           fresh_reader_does_not_validate_changed_engine=True)
            report['source_applicability']=applicability
        report['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in report['sources'].items())
        report['success']=(report['sources_stable'] and report['cursor']['invalid_rectangle_refusals_established']
                           and report['cpu_copy']['all_application_output_closed']
                           and report['cpu_copy']['historical_analysis_equal'] and report['history']['all_eight_reproduce_one_pixel'])
    except Exception as error:
        report['error']=str(error)
    write_report(ROOT,path,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('success','error','sources_stable')}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
