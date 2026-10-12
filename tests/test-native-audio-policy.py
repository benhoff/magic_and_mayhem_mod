#!/usr/bin/env python3
"""Exercise public CLI audio selection without launching Wine or reading assets."""
import argparse
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=ROOT/'working/build/qt-shell/mnm-qt-shell')
    args = parser.parse_args()
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env['QT_QPA_PLATFORM'] = 'offscreen'
    cases = [([], 'native'), (['--live-menus', '--native-commands'], 'native'),
             (['--native-voices'], 'native'), (['--legacy-audio'], 'legacy'),
             (['--live-menus', '--native-commands', '--legacy-audio'], 'legacy'),
             (['--frame-readback'], 'legacy'), (['--capture-locks'], 'legacy'),
             (['--frame-readback', '--native-voices'], 'native'),
             (['--live-menus'], 'legacy'), (['--renderer', 'native'], 'legacy')]
    for flags, expected in cases:
        result = subprocess.run([str(args.binary.resolve()), '--repo', str(ROOT), '--smoke-test', *flags],
                                env=env, capture_output=True, text=True, timeout=20)
        assert result.returncode == 0, (flags, result.stdout, result.stderr)
        assert f'Native audio policy: {expected}' in result.stdout, (flags, result.stdout)
    for flags in [['--native-voices', '--legacy-audio'], ['--live-menus', '--native-voices']]:
        result = subprocess.run([str(args.binary.resolve()), '--repo', str(ROOT), '--smoke-test', *flags],
                                env=env, capture_output=True, text=True, timeout=20)
        assert result.returncode == 2, (flags, result.stdout, result.stderr)
    print(f'Public audio selection: {len(cases)} admissions and 2 refusals; no game or audio device')


if __name__ == '__main__':main()
