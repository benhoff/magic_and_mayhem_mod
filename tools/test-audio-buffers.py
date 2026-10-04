#!/usr/bin/env python3
"""Verify native sample upload and duplicate lifetime against installed PCM WAVs."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import wave

REPO = Path(__file__).resolve().parents[1]


def main():
    parent = REPO / 'working/tests/audio-buffers'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Audio evidence: {root}', flush=True)
    subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        spec = importlib.util.spec_from_file_location('audio_export', REPO / 'tools/export-audio-support.py')
        exporter = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(exporter)
        exported = exporter.export(REPO / 'working/game-nocd/Chaos.exe')
        build = REPO / 'working/build/audio'
        subprocess.run(['cmake', '-S', str(REPO / 'audio'), '-B', str(build)], check=True)
        subprocess.run(['cmake', '--build', str(build), '--parallel', '4'], check=True)
        subprocess.run(['ctest', '--test-dir', str(build), '--output-on-failure'], check=True)
        paths = sorted(p for p in (REPO / 'working/game-nocd/Sounds').iterdir() if p.suffix.lower()=='.wav')
        if not paths:
            raise ValueError('No installed WAV assets')
        results = []
        dump = root / 'last-samples.bin'
        installation = REPO / 'working/game-nocd'
        for index, path in enumerate(paths):
            before = hashlib.sha256(path.read_bytes()).hexdigest()
            with wave.open(str(path), 'rb') as wav:
                samples = wav.readframes(wav.getnframes())
                expected = {'channels': wav.getnchannels(), 'rate': wav.getframerate(), 'bits': wav.getsampwidth()*8,
                            'alignment': wav.getnchannels()*wav.getsampwidth(), 'samples': len(samples)}
            relative = path.relative_to(installation).as_posix()
            request = ''.join(c.upper() if i % 2 else c.lower() for i, c in enumerate(relative)).replace('/', '\\')
            if index % 2:
                request = 'c:\\MagicMayhem\\' + request
            completed = subprocess.run([str(build / 'mnm-audio-upload'), '--assets', str(installation),
                '--prefix', 'C:/MagicMayhem', '--path', request, '--dump', str(dump)], check=True, capture_output=True, text=True)
            actual = json.loads(completed.stdout)
            assert all(actual[key]==value for key, value in expected.items()), (path, expected, actual)
            assert actual['bytes_per_second'] == expected['alignment']*expected['rate']
            assert actual['revision'] == 1 and actual['remaining_buffers'] == 0 and not actual['audible_output']
            assert dump.read_bytes() == samples, path
            assert hashlib.sha256(path.read_bytes()).hexdigest() == before, path
            results.append({'asset': path.name, 'request': request, 'file_sha256': before,
                            'pcm_sha256': hashlib.sha256(samples).hexdigest(), **actual})
        report = {'origin': 'offline_native_static_buffer_upload', 'source_sha256': exporter.HASH, 'static_evidence': str(exported),
                  'wav_count': len(results), 'pcm_bytes': sum(r['samples'] for r in results), 'assets': results,
                  'input_backend': 'qt_asset_interface',
                  'upload_binary_sha256': hashlib.sha256((build / 'mnm-audio-upload').read_bytes()).hexdigest(),
                  'live_game_validated': False, 'audible_output_validated': False, 'directsound_intercepted': False,
                  'checks': ['PE32_descriptor_bytes', 'primary_setup_call_order', 'primary_failure_paths', 'original_capability_fallback',
                             'strict_RIFF_validation', 'exact_uploaded_PCM', 'duplicate_shared_storage', 'duplicate_survives_source_release',
                             'wraparound_lock', 'partial_commit', 'invalid_unlock', 'release_while_locked', 'allocation_limits',
                             'asset_interface_WAV_loading', 'mixed_case_windows_paths', 'explicit_drive_aliases',
                             'parsed_PCM_survives_input_close']}
        (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(f'Native audio upload passed for {len(results)} WAVs: {root / "report.json"}')
    finally:
        subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
