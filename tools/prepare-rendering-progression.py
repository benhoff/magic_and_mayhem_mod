#!/usr/bin/env python3
"""Hash finite Celtic progression inputs and retain unchanged save candidates."""
import argparse
import json
from pathlib import Path
import subprocess
from coverage_evidence import Reader, write_report
from rendering_progression import PREFIX, CASES, BUILD, recipes, require, sha, prepare_launch_manifest

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--game', type=Path, required=True)
    p.add_argument('--save', type=Path, action='append', default=[])
    p.add_argument('--restore-pristine-executable', action='store_true',
                   help='Restore the hash-pinned No-CD executable in this worktree\'s private working/game-nocd only')
    p.add_argument('--expected-executable-sha256', help='Required exact preimage when restoring the private executable')
    p.add_argument('--report', required=True)
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(), 'New owned report required')
    report = dict(schema=1, kind='rendering_progression_preparation', success=False,
                  original_game_executed=False, player_accepted=False, status_promotions=[])
    verified = False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        verified = True
        game = a.game.resolve(strict=True)
        if a.restore_pristine_executable:
            target = game/'Chaos.exe'
            require(game == ROOT/'working/game-nocd' and not a.game.is_symlink() and not target.is_symlink() and target.stat().st_nlink == 1,
                    'Executable restoration requires a physical private installation, with no shared hardlink')
            require(a.expected_executable_sha256 and sha(target) == a.expected_executable_sha256,
                    'Private executable preimage mismatch')
            source = ROOT/'original/Arcane_Nocd/Chaos.exe'
            require(sha(source) == BUILD, 'Pristine executable hash mismatch')
            before = str(Path(a.report).with_suffix(''))+'-before-executable.bin'
            write_report(ROOT, before, target.read_bytes())
            report['executable_restoration'] = dict(before_sha256=a.expected_executable_sha256,
                before_record=before, after_sha256=BUILD, source='original/Arcane_Nocd/Chaos.exe',
                scope='Private installed input restoration only; original artifacts remain unchanged')
            target.write_bytes(source.read_bytes())
            require(sha(target) == BUILD, 'Private executable restoration failed')
        report.update(source_game=str(game), maps=recipes(game), saves=[])
        manifest=ROOT/'working/manifests/game-nocd.tsv'
        raw=prepare_launch_manifest(game,manifest)
        record=str(Path(a.report).with_suffix(''))+'-launch-manifest.bin'
        report['launch_manifest']=dict(path='working/manifests/game-nocd.tsv',
            sha256=write_report(ROOT,record,raw),bytes=len(raw),record=record,
            consumer='tools/run-game.sh verify_game',source_game=str(game))
        inputs = ['Chaos.exe', 'Realms/Celtic/Celtic.cfg', 'Realms/Celtic/RealmView.cfg',
                  'Realms/Celtic/CelticScript1.CFG', 'Realms/Celtic/CelticScript2.CFG', 'CFG/prefs.cfg']
        report['configuration_inputs'] = []
        for name in inputs:
            raw = (game/name).read_bytes()
            record = str(Path(a.report).with_suffix(''))+'-input-'+name.replace('/', '-')+'.bin'
            report['configuration_inputs'].append(dict(path=name, sha256=write_report(ROOT, record, raw), bytes=len(raw), record=record))
        require(len(a.save) <= 3, 'At most three unchanged save candidates')
        for i, path in enumerate(a.save):
            path = path.resolve(strict=True)
            require(path.suffix.lower() == '.sav' and 0 < path.stat().st_size <= 16*1024**2, 'Invalid bounded save candidate')
            record = str(Path(a.report).with_suffix(''))+f'-save-{i}.bin'
            raw = path.read_bytes()
            report['saves'].append(dict(path=str(path), name=path.name, bytes=len(raw),
                sha256=write_report(ROOT, record, raw), record=record,
                role='unchanged reload/control candidate; no near-victory or progressed-state claim', used_by_primary_cases=False))
        report['cases'] = [dict(id=case, entry='fresh campaign', difficulty=1,
            expected_outcome=0 if case == 'defeat' else 1,
            expected_regions=[1] if case == 'defeat' else [1, 2],
            deadline_seconds=900, world_driver_seconds=540,
            configurations=['player-default', 'player-software'],
            frame_modes=[[640,480], [800,600]],
            command=['python3', 'tools/run-rendering-progression.py', '--case', case, '--preparation', a.report,
                     '--report', PREFIX+case+'-RUN.json']) for case in CASES]
        report['assertions'] = ['ordinary input only; original simulation and scripts unchanged',
            'natural report discriminator, all 21 texts and original OK acknowledgement',
            'no Quit before the primary report; cleanup Quit is separately labelled',
            'fresh Realm admission and Celtic region 2 World; reload is never progression',
            'all initial and later World output/history and populated handoffs must be independently checked',
            'zero surviving native/original owners after normal Main Quit']
        names = ['tools/rendering_progression.py', 'tools/prepare-rendering-progression.py', 'tools/rendering_build_binding.py',
                 'tools/run-rendering-progression.py', 'tools/check-rendering-progression.py', 'tools/campaign-outcome-input.py',
                 'tools/campaign_route.py','tools/campaign_dialogue.py','tools/campaign_return.py','tools/campaign_follow.py','tools/campaign_realm_exit.py','tools/campaign-realm-exit-input.py','tools/campaign_input_observation.py','tools/campaign_report.py','tools/rendering_capture.py', 'tools/rendering_pilot.py', 'tools/campaign_gameplay.py', 'tools/campaign_spell.py', 'tools/campaign_input_admission.py',
                 'apps/qt-shell/profile_capture_codec.hpp','apps/qt-shell/resident_profile_capture.cpp','apps/qt-shell/resident_profile_capture.hpp','tools/resident_capture.py','tests/acceptance-world-history.cpp','apps/qt-shell/acceptance_capture.hpp','apps/qt-shell/host_timing_trace.hpp','apps/qt-shell/campaign_progression_test.cpp', 'apps/qt-shell/campaign_smoke_test.cpp',
                 'apps/qt-shell/campaign_smoke_test.hpp', 'apps/qt-shell/CMakeLists.txt']
        report['sources'] = {n: sha(ROOT/n) for n in names}
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
    finally:
        if verified:
            try:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
                report['original_manifest_verified_before_after'] = True
            except Exception as error:
                report.update(success=False, original_verification_error=str(error))
        digest = write_report(ROOT, a.report, (json.dumps(report, indent=2)+'\n').encode())
        with Reader(ROOT) as reader:
            require(reader.digest(a.report) == digest and reader.json(a.report) == report, 'SQLite report readback differs')
        print(json.dumps(dict(success=report['success'], record=a.report, sha256=digest, error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
