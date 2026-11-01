#!/usr/bin/env python3
"""Retain bounded execution research and join its exact as-built identities.

This collector cannot admit player acceptance or manufacture presented history.
It archives closed-run observations, failures and actual binaries in SQLite.
"""
import argparse
from collections import Counter
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import re
import subprocess

from coverage_evidence import Reader, write_report

ROOT = Path(__file__).resolve().parents[1]
MAX_FILE = 256 * 1024 * 1024
MAX_SPARSE_CHANNEL = 2 * 1024 * 1024 * 1024


def sha(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def selection_identity(flow):
    row, value = flow.get('selected_map_row'), flow.get('selected_map_id', '')
    if value and (type(row) is not int or value != str(row+1)):
        raise ValueError('Selection acknowledgement is not the reviewed one-based row')
    return dict(row=row, selection_id=value, name=flow.get('selected_map_name', ''),
                id_kind='one_based_ui_selection', filename_identity_established=False,
                generated_world_identity_established=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--record', required=True)
    parser.add_argument('--report', required=True)
    parser.add_argument('--relocated-from', type=Path,
                        help='Explicit old research root; require identical mapped directory inode')
    args = parser.parse_args()
    if (ROOT/args.report).exists() or not args.report.startswith('research/runtime/'):
        parser.error('Require a new research/runtime report')
    with Reader(ROOT) as reader:
        observation = reader.json(args.record)
        sources = {n:reader.digest(n) for n in ['tools/collect-rendering-execution-research.py',
            'tools/test-native-owned-campaign.py', 'tools/native_only_contract.py', 'tools/native_output_owner.py',
            'apps/qt-shell/live_battle_menu_controller.cpp', 'apps/qt-shell/live_quick_battle_app_test.cpp',
            'runtime/menu/observer.c', 'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/gl_viewport.cpp']}
        record_sha = reader.digest(args.record)
    report = dict(schema=1, kind='rendering_execution_observation_retention', success=False, sources=sources,
        observation=dict(record=args.record, sha256=record_sha, success=observation['success'],
                         original_game_executed=observation['original_game_executed']),
        player_acceptance_assessed=False, independent_presented_history=False,
        original_pixels_used_as_native_inputs=False, status_promotions=[],
        scope='Closed-run source/binary/input/observed-state identities and lossless raw retention only. '
              'A successful run or retention pass is not complete presented-frame acceptance, original router ownership, '
              'global caller absence or arbitrary map/save support.')
    retained = []
    try:
        case = (ROOT/observation['artifacts']).resolve()
        if not case.is_relative_to(ROOT/'working/tests'):
            raise ValueError('Foreign research case')
        if observation.get('prefix_template'):
            prefix = Path(observation['prefix_template']['target'])
            subprocess.run(['wineserver', '-w'], env=dict(os.environ, WINEPREFIX=str(prefix)),
                           check=True, timeout=10)
        session_name = observation.get('session')
        # Early failures may precede the owning runner journal. Retain their actual staging tree too.
        if not session_name and (case/'shell.log').exists():
            found = re.findall(r'Evidence directory: (.+)', (case/'shell.log').read_text(errors='replace'))
            if found:
                candidate = Path(found[-1])
                if args.relocated_from and candidate.is_relative_to(args.relocated_from):
                    mapped = ROOT/candidate.relative_to(args.relocated_from)
                    if mapped.stat().st_dev != candidate.stat().st_dev or mapped.stat().st_ino != candidate.stat().st_ino:
                        raise ValueError('Relocated observation is not the same retained directory')
                    report['relocation'] = dict(previous=str(candidate), current=str(mapped), same_inode=True)
                    candidate = mapped
                if candidate.is_relative_to(ROOT/'working/experiments/menu-observer'):
                    session_name = candidate.relative_to(ROOT).as_posix()
        selected = []
        for path in sorted(case.rglob('*')):
            if not path.is_file() or path.is_symlink() or 'wineprefix' in path.relative_to(case).parts:
                continue
            if path.name == 'Xauthority' or path.suffix not in {'.json', '.bin', '.log', '.png', '.tsv'}:
                continue
            selected.append(('case/'+path.relative_to(case).as_posix(), path))
        if session_name:
            session = (ROOT/session_name).resolve()
            if not session.is_relative_to(ROOT/'working/experiments/menu-observer'):
                raise ValueError('Foreign staging tree')
            report['session'] = str(session)
            for path in sorted(session.rglob('*')):
                if not path.is_file() or path.is_symlink():
                    continue
                relative = path.relative_to(session)
                if relative.parts[0] == 'game':
                    include = (len(relative.parts) == 2 and path.suffix.lower() in {'.bin', '.dll', '.exe', '.log'}) or (
                        len(relative.parts) > 2 and relative.parts[1].lower() in {'save', 'cfg'})
                else:
                    include = path.suffix in {'.json', '.bin', '.log', '.png'}
                if include:
                    selected.append(('session/'+relative.as_posix(), path))
            admission = session/'native-output-admission.json'
            if admission.exists():
                value = json.loads(admission.read_text())
                report['actual_admission'] = value
                report['native_binary_hashes'] = value.get('native_only', {}).get('binaries', {})
                report['native_compiler_source_closure'] = value.get('native_only', {}).get('sources', {})
            report['staged_inputs'] = {str(p.relative_to(session)):dict(bytes=p.stat().st_size, sha256=sha(p))
                for p in sorted((session/'game').rglob('*')) if p.is_file() and not p.is_symlink()}
        for label, path in selected:
            size = path.stat().st_size
            if size > MAX_SPARSE_CHANNEL:
                raise ValueError('Unbounded selected raw input: '+label)
            before = sha(path)
            name = str(Path(args.report).with_suffix(''))+'-raw-'+hashlib.sha256(label.encode()).hexdigest()[:20]+'.bin'
            codec = 'identity'
            if size > MAX_FILE:
                # JPEG's bounded sparse shared channel includes unused capacity.
                # Preserve every byte, streaming its zero tail without a GiB copy.
                buffer = io.BytesIO()
                with path.open('rb') as stream, gzip.GzipFile(fileobj=buffer, mode='wb', mtime=0, compresslevel=1) as compressed:
                    for data in iter(lambda:stream.read(1024*1024), b''):
                        compressed.write(data)
                data = buffer.getvalue()
                codec = 'gzip'
                name = name.removesuffix('.bin')+'.gz'
            else:
                data = path.read_bytes()
            digest = write_report(ROOT, name, data)
            if (codec == 'identity' and digest != before) or sha(path) != before:
                raise ValueError('Run artifact changed during retention: '+label)
            retained.append(dict(path=label, record=name, sha256=digest, bytes=size,
                                 raw_sha256=before, codec=codec, payload_bytes=len(data)))
        binary = ROOT/'working/build/qt-shell/mnm-qt-shell'
        if binary.exists():
            name = str(Path(args.report).with_suffix(''))+'-shell.bin'
            report['actual_shell'] = dict(record=name, sha256=write_report(ROOT, name, binary.read_bytes()), bytes=binary.stat().st_size)
            result = subprocess.run(['ldd', str(binary)], capture_output=True, text=True, check=True)
            libraries = sorted(set(re.findall(r'(?:=> )?(/[^\s]+) \(', result.stdout)))
            report['host_linked_libraries'] = {n:sha(n) for n in libraries if Path(n).is_file()}
        report['selection'] = selection_identity(observation.get('flow', {}))
        report['live_summary'] = {n:observation.get(n) for n in ['resident', 'native_only_outcomes', 'graphics_retirement',
            'copy_census_verified', 'copy_census_error', 'natural_gpu_callers', 'gpu_copy', 'copy_routes', 'original_process',
            'native_compiler_inputs_checked', 'storage', 'storage_flow'] if n in observation}
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
    report['retained_artifacts'] = retained
    report['retained_bytes'] = sum(r['bytes'] for r in retained)
    with Reader(ROOT) as reader:
        report['sources_stable'] = all(reader.digest(n) == h for n,h in sources.items())
    report['success'] &= report['sources_stable']
    digest = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], sha256=digest, files=len(retained),
                         retained_bytes=report['retained_bytes'], error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
