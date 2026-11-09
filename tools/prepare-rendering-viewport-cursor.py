#!/usr/bin/env python3
"""Pin exact current inputs and a separate bounded viewport/cursor scenario."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import shlex
import subprocess
from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract, scenario_contract
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
from viewport_cursor import PREFIX, CONTROLS, SIZES, edge_points, analysis_paths

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    p.add_argument('--stage-from', type=Path)
    p.add_argument('--diagnose-rebuild', action='store_true')
    p.add_argument('--maximum-original-games',type=int,choices=(4,5),default=4)
    p.add_argument('--bound-reason')
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(), 'Fresh owned plan required')
    require(args.maximum_original_games==4 or bool(args.bound_reason and args.bound_reason.strip()),
            'Prospective bound adjustment requires a concrete new observation need')
    subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
    try:
        if args.stage_from:
            from experiment_environment import _idle
            source = args.stage_from.resolve()/'working/game-nocd'
            _idle(source, 'game')
            target = ROOT/'working/game-nocd'
            if target.is_symlink():
                require(target.resolve().is_relative_to(ROOT.parents[1]/'check-inputs'),
                        'Only the fresh shared-input alias may be replaced')
                target.unlink()
            require(not target.exists(), 'Existing private installation must be retained')
            shutil.copytree(source, target)
            original = ROOT/'original/Arcane_Nocd/Chaos.exe'
            require(hashlib.sha256(original.read_bytes()).hexdigest() ==
                    '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168',
                    'Pinned original executable differs')
            shutil.copyfile(original, target/'Chaos.exe')
        with Reader(ROOT) as r:
            old_name = 'research/runtime/rendering-copy-natural-results-20261108/input-plan-10.json'
            old = r.json(old_name)
            require(all(r.digest(n) == h for n,h in old['compiler_sources'].items()),
                    'Retained player compiler closure changed')
            player_path = 'working/build/qt-shell/mnm-qt-shell'
            require(hashlib.sha256((ROOT/player_path).read_bytes()).hexdigest() == old['player_binary']['sha256'],
                    'Retained player executable changed')
            old_oracle = r.json('research/runtime/cursor-hotspot-results-20261108/input-plan-06.json')['oracle']
            oracle_sources=set(old_oracle['native_sources'])
            dependencies=[ROOT/'working/original-reference.d',
                          *Path('/tmp/mnm-viewport-cursor-build-20261108').rglob('*.o.d')]
            for dep in dependencies:
                for word in shlex.split(dep.read_text().replace('\\\n',' ')):
                    candidate=Path(word).resolve()
                    if candidate.is_file() and candidate.is_relative_to(ROOT) and not candidate.is_relative_to(ROOT/'working'):
                        oracle_sources.add(candidate.relative_to(ROOT).as_posix())
            sources = {n:r.digest(n) for n in sorted(set(CONTROLS)|analysis_paths(ROOT))}
            source_artifacts=[]
            for name in sources:
                raw=(ROOT/name).read_bytes();record=PREFIX+'source-'+sources[name]+'.bin'
                source_artifacts.append(dict(path=name,record=record,
                    sha256=write_report(ROOT,record,raw)))
            binaries = []
            for label,path in [('original-reference','working/original-reference'),
                               ('original-history','working/build/qt-shell/mnm-acceptance-world-history')]:
                raw = (ROOT/path).read_bytes(); name = args.report.removesuffix('.json')+'-'+label+'.bin'
                binaries.append(dict(path=path, record=name, sha256=write_report(ROOT,name,raw)))
            player = dict(old['player_binary'], path=player_path)
            register=r.json('research/runtime/coverage/register.json')
            behavior=next(v for v in register['behaviors'] if v['id']=='NP.viewport-cursor-continuity')
            scenario=next(v for v in register['scenarios'] if v['id']=='native.viewport-cursor-continuity')
            plan = dict(schema=1, kind='viewport_cursor_prospective_plan', success=True,
                claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(
                    behavior,{v['id']:v for v in register['builds']}),
                    scenarios={scenario['id']:scenario_contract(scenario)})],
                sources=sources, source_artifacts=source_artifacts,
                compiler_sources=old['compiler_sources'], player_binary=player,
                retained_player_parent=dict(record=old_name,sha256=r.digest(old_name)),
                source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                runtime_sources={n:r.digest(n) for n in sorted(native_runtime_source_paths(ROOT)|set(CONTROLS))},
                oracle=dict(native_sources={n:r.digest(n) for n in sorted(oracle_sources)}, binaries=binaries,
                            compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0]),
                bounds=dict(maximum_requests=6, maximum_original_games=args.maximum_original_games, primary_games=2,
                            stop_on_first_failure=True, successful_pilot_comparison_before_expansion=True),
                diagnose_rebuild=args.diagnose_rebuild,prospective_bound_reason=args.bound_reason,
                domains=[dict(software=s,sizes=SIZES,scaling=['sharp','integer']) for s in (False,True)],
                edge_schedule=[dict(size=z,points=edge_points(z)) for z in SIZES[:2]],
                camera_schedule=dict(directions=['up','right','down','left'],hold_seconds=2,
                                     recenter_after_each=True,derive_actual_limits=True),
                rebuild_route='Quick Results -> Main Preferences -> same-map recreated Quick World, one original process',
                excluded=['same-World continuity','whole picking','gameplay changes','original driver parity',
                          'arbitrary maps/providers/durations','final rendering decisions'],
                previous_caps_preserved=True, original_game_runs=0,
                evidence_staging=dict(database='working/coverage/evidence-pending.sqlite3',temporary=True,
                    paired_import_and_read_verification_required=True,
                    wrapper_sha256=hashlib.sha256((ROOT/'working/viewport-cursor/with-staged-evidence.py').read_bytes()).hexdigest()),
                scope=f'User-requested viewport-boundaries and cursor-motion only. Reuse prior evidence as exact historical leads. Independent six-request/{args.maximum_original_games}-start ceiling includes every setup refusal after request admission and every failed game; no prior exhausted budget is reset. Stop on a failure and retain diagnostics before a specific repair. Diagnostic mode may acquire only the minimal High-to-Low caller trace and earns no player-row credit. Full output, fresh original input/source/paint binding and normal resource retirement are mandatory before any row promotion.')
        print(write_report(ROOT,args.report,(json.dumps(plan,indent=2)+'\n').encode()))
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)


if __name__ == '__main__': main()
