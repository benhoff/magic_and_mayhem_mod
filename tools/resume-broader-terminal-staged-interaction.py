#!/usr/bin/env python3
"""Admit and retain the same successfully reanalysed interaction capture once."""
import argparse
import hashlib
import json
from pathlib import Path
import sqlite3
import subprocess
import time
from coverage_evidence import Reader, write_report
from broader_player_derived_matrix import specs, P
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]
CASE = 'bp48-world-interaction-default800-01'


def rows():
    with sqlite3.connect('file:' + str(ROOT / 'working/broader-player/requests.sqlite3') + '?mode=ro', uri=True) as db:
        db.row_factory = sqlite3.Row
        return [dict(v) for v in db.execute('select *from requests order by started_ns')]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--controls', required=True)
    args = parser.parse_args()
    name = P + CASE + '-coordinator-03.json'
    execution_name = P + CASE + '-retained-current-analysis-execution-02.json'
    require(not (ROOT / name).exists() and not (ROOT / execution_name).exists(), 'Fresh retained successor required')
    before = rows()
    row = next(v for v in before if v['case_id'] == CASE)
    require(row['state'] == 'acquired' and not row['validation'], 'Never repeat a retained admission')
    validation_name = P + CASE + '-validation-01.json'
    with Reader(ROOT) as reader:
        ref = lambda n: dict(record=n, sha256=reader.digest(n))
        spec = next(v for v in specs(reader) if v['case']['id'] == CASE)
        phase = ref(spec['plan'])
        controls = reader.json(args.controls)
        require(controls['success'] and controls['tests'] >= 8
                and all(reader.digest(n) == h for n, h in controls['sources'].items()), 'Current successor controls required')
        acquired = ref(spec['case']['report'])
        require(row['acquisition_sha256'] == acquired['sha256'] and json.loads(row['plan']) == phase,
                'Different acquired reservation')
        validation = reader.json(validation_name)
        require(validation['success'] and validation['sources_stable'] and validation['current_case_proved']
                and validation['acquisition'] == acquired and validation['family'] == 'interaction'
                and all(reader.digest(n) == h for n, h in validation['sources'].items()), 'Current complete validation required')
        compared = reader.json(validation['comparison']['record'])
        require(reader.digest(validation['comparison']['record']) == validation['comparison']['sha256']
                and compared['success'] and compared['comparison']['mismatching_pixels'] == 0
                and compared['current_semantic_analysis']['new_pixel_comparisons'] == 0
                and compared['current_semantic_analysis']['source_chunks_credited_as_Worlds'] == 0
                and compared['current_semantic_analysis']['source_chunks_credited_as_action_witnesses'] == 0,
                'Different retained semantic/pixel result')
        old = ref(P + CASE + '-coordinator-01.json')
        require(reader.json(old['record'])['success'] is False, 'Historical failed coordinator changed')
        prior_admission=ref(P+CASE+'-coordinator-02.json');require(reader.json(prior_admission['record'])['success']is False,'Prior admission refusal changed')
        source_names = ['tools/resume-broader-terminal-staged-interaction.py', 'tools/broader_player_terminal_matrix.py', 'tools/broader_terminal_pause.py', 'tools/broader_pause_observation.py', 'tools/broader_terminal_ui_identity.py','tools/broader_retained_staged_manifest.py', 'tools/run-broader-terminal-staged-source-case.py',
                        'tools/broader_interaction_analysis_binding.py', 'tools/compare-broader-current-interaction.py',
                        'tools/broader_interaction_source_chunks.py', 'tools/analyze-broader-observed-pause-interaction.py']
        sources = {n: reader.digest(n) for n in source_names}
        validation_ref = ref(validation_name)
        controls_ref = ref(args.controls)
    out = ROOT / 'working/broader-player/retained-terminal-staged-interaction' / CASE
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').write_text('Retained interaction admission and all success/failure logs. No new game.\n')
    steps, error = [], None

    def run(stage, command):
        start = time.time_ns()
        path = out / (stage + '.log')
        with path.open('xb') as stream:
            result = subprocess.run(command, cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT)
        steps.append(dict(stage=stage, command=command, returncode=result.returncode,
                          start_ns=start, finish_ns=time.time_ns(), log_path=str(path)))
        print(json.dumps(dict(case=CASE, stage=stage, returncode=result.returncode)), flush=True)
        require(result.returncode == 0, 'Retained admission/custody failed: ' + stage)

    try:
        run('admit', ['python3', 'tools/run-broader-terminal-staged-source-case.py', 'admit', '--plan', spec['plan'],
                      '--case', CASE, '--validation', validation_name])
        run('retain-execution', ['python3', 'tools/retire-broader-domain-case.py', '--validation', validation_name,
                                 '--report', P + CASE + '-handoff-01.json'])
        run('retain-RAM-capture', ['python3', 'tools/retire-broader-capture-copy.py', '--validation', validation_name,
                                   '--report', P + CASE + '-RAM-handoff-01.json'])
        after = rows()
        require(len(after) == len(before), 'Retained analysis added a reservation')
        expected = [dict(v, state='admitted', validation=validation_name) if v['case_id'] == CASE else v for v in before]
        require(after == expected, 'Non-admission request fields changed')
        with Reader(ROOT) as reader:
            require(reader.digest(acquired['record']) == acquired['sha256'], 'Original acquisition changed')
            require(all(reader.digest(n) == h for n, h in sources.items()), 'Retained coordinator sources changed')
    except Exception as exc:
        error = repr(exc)
        after = rows()
    finally:
        for step in steps:
            path = P + CASE + '-retained-staged-current-' + step['stage'] + '.bin'
            body = Path(step['log_path']).read_bytes()
            step['log'] = dict(record=path, sha256=write_report(ROOT, path, body))
        execution = dict(schema=1, kind='retained_current_interaction_execution', success=error is None, error=error,
            case=CASE, original_game_runs=0, new_public_player_executions=0, new_request_reservations=0,
            request_rows_unchanged=len(after) == len(before), original_acquisition_preserved=True,
            only_admission_fields_changed=error is None, acquisition=acquired, validation=validation_ref,
            before_request=row, after_request=next(v for v in after if v['case_id'] == CASE),
            request_rows=len(after), conservative_charges=sum(v['reserved_games'] for v in after),
            sources=sources, controls=controls_ref, previous_failed_coordinator=old, steps=steps)
        eh = write_report(ROOT, execution_name, (json.dumps(execution, indent=2) + '\n').encode())
        co = dict(schema=1, kind='broader_domain_case_coordinator', success=error is None, error=error,
            case=CASE, family='interaction', group='world', plan=phase, acquisition=acquired,
            validation=validation_ref, previous_failed_coordinator=old,
            previous_failed_admission_coordinator=prior_admission,retained_analysis_execution=dict(record=execution_name, sha256=eh), original_game_runs_added=0,
            new_acquisition_performed=False, automatic_retries=False, sources=sources, steps=steps,
            source=dict(path='tools/resume-broader-terminal-staged-interaction.py', sha256=sources['tools/resume-broader-terminal-staged-interaction.py']))
        h = write_report(ROOT, name, (json.dumps(co, indent=2) + '\n').encode())
        print(json.dumps(dict(success=co['success'], error=error, sha256=h, new_original_games=0)), flush=True)
    return int(error is not None)


if __name__ == '__main__': raise SystemExit(main())
