#!/usr/bin/env python3
"""Build and verify native stereo mixing using fixtures only, without game assets."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]


def main():
    parent = REPO / 'working/tests/audio-mixer'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/audio-mixer'
    for name, command in (
        ('configure', ['cmake', '-S', str(REPO / 'audio'), '-B', str(build)]),
        ('build', ['cmake', '--build', str(build), '--parallel', '4']),
        ('ctest', ['ctest', '--test-dir', str(build), '--output-on-failure']),
    ):
        result = subprocess.run(command, text=True, capture_output=True)
        (root / (name+'.log')).write_text(result.stdout+result.stderr)
        print(result.stdout, end='')
        result.check_returncode()
    sources = ['audio/buffers.hpp', 'audio/buffers.cpp', 'audio/voices.cpp', 'audio/mixer.cpp',
               'tests/audio-mixer-test.cpp', 'reconstruction/audio/dsound_setup.cpp']
    report = {'scope': 'native stereo PCM mixing, fixture-only', 'tests_passed': True,
              'native_fixture_sha256': hashlib.sha256((build / 'audio-mixer-test').read_bytes()).hexdigest(),
              'source_sha256': {name: hashlib.sha256((REPO / name).read_bytes()).hexdigest() for name in sources},
              'resampling_scenarios': 120, 'reference_tolerance_lsb': 1, 'game_assets_read': False,
              'live_game_validated': False, 'audible_output': False, 'game_hooks_installed': False,
              'checks': ['split_block_exact_continuity', 'linear_interpolation', 'final_sum_clipping', 'stop_resume_reset', 'completion_and_replay', 'whole_buffer_loops',
                         'huge_advance_overflow', 'loop_flag_changes', 'independent_duplicate_controls',
                         'pending_writes_and_commits', 'release_while_playing', 'invalid_ids_and_controls',
                         'mono_stereo_8_16_bit_frames', 'absolute_time_pcm_oracle', 'volume_and_pan_gains']}
    (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Native mixer evidence: {root / "report.json"}')


if __name__ == '__main__':
    main()
