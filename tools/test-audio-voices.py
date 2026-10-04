#!/usr/bin/env python3
"""Export pinned voice-control evidence and run offline reconstruction contracts."""
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]


def main():
    parent = REPO / 'working/tests/audio-voices'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        spec = importlib.util.spec_from_file_location('audio_export', REPO / 'tools/export-audio-support.py')
        exporter = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(exporter)
        evidence = exporter.export(REPO / 'working/game-nocd/Chaos.exe')
        audit = json.loads((evidence / 'voice-call-audit.json').read_text())
        if audit['set_frequency_slot_candidates']:
            raise ValueError('Frequency slot candidates require manual reconstruction review')
        # The only slot +0x10 call is device.GetCaps, not buffer.GetCurrentPosition.
        if audit['get_position_slot_candidates'] != ['0x0056fefb']:
            raise ValueError('New cursor slot candidates require manual reconstruction review')
        build = REPO / 'working/build/audio-voices'
        subprocess.run(['cmake', '-S', str(REPO / 'audio'), '-B', str(build)], check=True)
        subprocess.run(['cmake', '--build', str(build), '--parallel', '4'], check=True)
        result = subprocess.run(['ctest', '--test-dir', str(build), '--output-on-failure'], text=True, capture_output=True)
        (root / 'ctest.log').write_text(result.stdout+result.stderr)
        print(result.stdout, end='')
        result.check_returncode()
        report = {'source_sha256': exporter.HASH, 'static_evidence': str(evidence),
                  'indirect_call_count': len(audit['indirect_calls']), 'tests_passed': True,
                  'set_frequency_observed_in_selected_slots': False,
                  'get_current_position_observed_in_selected_slots': False,
                  'whole_program_absence_proven': False, 'live_game_validated': False,
                  'audible_output': False, 'game_hooks_installed': False,
                  'checks': ['status_failure_is_busy', 'stop_then_zero_reset', 'nonzero_hresult_exits',
                             'cache_before_volume_call', 'equal_cache_skips_duplicates',
                             'duplicate_volume_notifications', 'pan_propagation', 'start_call_order',
                             'loop_flag', 'deadline_boundaries_and_wrap', 'schedule_clear', 'integer_pan_mapping']}
        (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(f'Voice evidence: {root / "report.json"}')
    finally:
        subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
