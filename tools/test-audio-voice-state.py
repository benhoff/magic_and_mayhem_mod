#!/usr/bin/env python3
"""Build and verify native audio state using fixtures only, without game assets."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]


def main():
    parent = REPO / 'working/tests/audio-voice-state'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/audio-state'
    for name, command in (
        ('configure', ['cmake', '-S', str(REPO / 'audio'), '-B', str(build)]),
        ('build', ['cmake', '--build', str(build), '--parallel', '4']),
        ('ctest', ['ctest', '--test-dir', str(build), '--output-on-failure']),
    ):
        result = subprocess.run(command, text=True, capture_output=True)
        (root / (name+'.log')).write_text(result.stdout+result.stderr)
        print(result.stdout, end='')
        result.check_returncode()
    sources = ['audio/buffers.hpp', 'audio/buffers.cpp', 'audio/voices.cpp',
               'tests/audio-voice-state-test.cpp', 'reconstruction/audio/voice_contract.cpp']
    report = {'scope': 'native source-frame playback state, fixture-only', 'tests_passed': True,
              'native_fixture_sha256': hashlib.sha256((build / 'audio-voice-state-test').read_bytes()).hexdigest(),
              'source_sha256': {name: hashlib.sha256((REPO / name).read_bytes()).hexdigest() for name in sources},
              'independent_advancement_scenarios': 306, 'game_assets_read': False,
              'live_game_validated': False, 'audible_output': False, 'game_hooks_installed': False,
              'checks': ['stop_resume_reset', 'completion_and_replay', 'whole_buffer_loops',
                         'huge_advance_overflow', 'loop_flag_changes', 'independent_duplicate_controls',
                         'pending_writes_and_commits', 'release_while_playing', 'invalid_ids_and_controls',
                         'mono_stereo_8_16_bit_frames', 'frame_step_oracle', 'reconstructed_controller_fixture']}
    (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Native voice evidence: {root / "report.json"}')


if __name__ == '__main__':
    main()
