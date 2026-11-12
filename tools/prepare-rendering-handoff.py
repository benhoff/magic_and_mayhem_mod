#!/usr/bin/env python3
"""Freeze a new two-case diagnostic; never extend the exhausted picking plan."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
from coverage_evidence import Reader, write_report
from rendering_handoff import CONTROLS, PREFIX
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    p.add_argument('--from-checkout', type=Path, help='Stopped donor with private game, music and admitted prefix')
    p.add_argument('--preparation-refusal', help='One retained zero-game staging refusal, before either diagnostic game')
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),
            'Fresh owned diagnostic plan required')
    if a.from_checkout:
        donor = a.from_checkout.resolve()
        require(donor != ROOT and (donor/'working/game-nocd').is_dir(), 'Existing separate donor required')
        from experiment_environment import _idle
        _idle(donor/'working/game-nocd', 'game')
        original = ROOT/'original'
        if not original.exists():
            original.symlink_to((donor/'original').resolve(), target_is_directory=True)
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        try:
            game = ROOT/'working/game-nocd'
            require(not game.exists(), 'Fresh private diagnostic game required')
            shutil.copytree(donor/'working/game-nocd', game)
            for relative in ('fixtures/prefix-seed', 'assets/native-cd-audio-20261012'):
                source = (donor/'working'/relative).resolve()
                require(source.is_dir(), 'Missing donor prefix/music: '+relative)
                target = ROOT/'working'/relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.symlink_to(source, target_is_directory=True)
        finally:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
    with Reader(ROOT) as r:
        old_name = 'research/runtime/rendering-picking-resize-results-20261107/input-plan-07.json'
        old = r.json(old_name)
        require(all(r.digest(n) == h for section in ('runtime_sources', 'compiler_sources')
                    for n, h in old[section].items()), 'Changed original/native source phase')
        require(all(r.digest(n) == h for n, h in old['sources'].items()), 'Changed retained controls')
        binary = old['player_binary']
        if a.from_checkout:
            target = ROOT/binary['path']
            require(not target.exists(), 'Fresh retained-player target required')
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(r.read_bytes(binary['record']))
            target.chmod(0o755)
        require(r.digest(binary['record']) == binary['sha256']
                == __import__('hashlib').sha256((ROOT/binary['path']).read_bytes()).hexdigest(),
                'Changed retained player')
        refusals = []
        if a.preparation_refusal:
            failed = r.json(a.preparation_refusal)
            require(failed['success'] is False and failed['original_game_runs'] == 0
                    and failed['kind'] == 'second_spellbox_diagnostic_acquisition', 'Exact zero-game refusal required')
            refusals = [dict(record=a.preparation_refusal, sha256=r.digest(a.preparation_refusal))]
        q = dict(old, kind='second_spellbox_diagnostic_plan',
                 parent=dict(record=old_name, sha256=r.digest(old_name)),
                 sources=old['sources'] | {n: r.digest(n) for n in CONTROLS},
                 preparation_refusals=refusals,
                 bounds=dict(maximum_requests=2+len(refusals), maximum_original_games=2,
                             modes=['same-size', 'low-size'], diagnostic_only=True,
                             maximum_samples=8000, sample_hz=20,
                             unchanged_menu_deadline_seconds=10, unchanged_command_watchdog_seconds=5),
                 scope='Two new causal diagnostics only: immediate High Quick exit -> Main Preferences '
                       'High or Low -> recreated same-map Quick. One request per mode, stop on its first '
                       'failure, no retries or acceptance promotion. Preserve all six exhausted picking '
                       'requests/three original startups. Read-only menu/command/callback/process waits; '
                       'no diagnostic channel writes, deadline changes, original pixel oracle or driver claim.')
    print(write_report(ROOT, a.report, (json.dumps(q, indent=2)+'\n').encode()))


if __name__ == '__main__':
    main()
