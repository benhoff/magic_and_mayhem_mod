#!/usr/bin/env python3
"""Offline catalog and Qt manager/output session validation; never launches a game."""
import argparse
import configparser
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import wave

REPO = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sounds-root', type=Path, help='Optional installed Sounds directory (manifest guarded)')
    parser.add_argument('--build-dir', type=Path, default=REPO / 'working/build/qt-shell')
    args = parser.parse_args()
    parent = REPO / 'working/tests/audio-session'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = args.build_dir.resolve()
    print(f'Audio session evidence: {evidence}', flush=True)

    def run(name, command, allowed=(0,), env=None, timeout=240):
        result = subprocess.run(command, cwd=REPO, text=True, capture_output=True, env=env, timeout=timeout)
        (evidence / f'{name}.log').write_text(result.stdout + result.stderr)
        if result.returncode not in allowed:
            raise RuntimeError(f'{name} exited {result.returncode}; see {evidence / (name + ".log")}')
        return result.returncode

    run('configure', ['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(build)])
    targets = ['mnm-qt-shell', 'audio-session-test', 'audio-catalog-preflight-test', 'audio-native-manager-test',
               'audio-output-test', 'audio-scheduler-test', 'audio-admission-test', 'audio-manager-lifecycle-test']
    run('build', ['cmake', '--build', str(build), '--target', *targets, '--parallel', '4'])
    run('ctest', ['ctest', '--test-dir', str(build), '--output-on-failure', '-R',
                 '^(qt-audio-manager-session|audio-catalog-preflight|audio-native-manager|audio-qt-output|audio-voice-scheduler|audio-voice-admission|audio-manager-lifecycle|qt-shell-help|qt-shell-startup)$'])
    env = dict(os.environ, QT_QPA_PLATFORM='offscreen')
    fixture = evidence / 'synthetic'
    fixture.mkdir()
    with wave.open(str(fixture / 'Tone.wav'), 'wb') as wav:
        wav.setparams((1, 2, 48000, 0, 'NONE', 'not compressed'))
        wav.writeframes(struct.pack('<h', 4000) * 128)
    (fixture / 'Sounds.ini').write_text("[Sounds]\n10='Tone' ; comment\n20=Stream\n90=Logical\n[Randomised]\n90=10\n[Optimisation]\nMaxSimultaneousSounds=4\n")
    shell = build / 'mnm-qt-shell'

    def check_catalog(root, label, installed=False):
        before = {p.name: digest(p) for p in root.iterdir() if p.is_file()}
        results = {}
        for policy in ('literal', 'dequote-missing-leaf'):
            file = evidence / f'{label}-{policy}.json'
            code = run(f'{label}-{policy}', [str(shell), '--audio-catalog', str(root), '--audio-preflight',
                       '--audio-path-policy', policy, '--audio-report', str(file)], allowed=(0, 3), env=env)
            report = json.loads(file.read_text())
            assert report['catalogValid'] and not report['gameLaunched'] and report['outputRate'] == 0
            assert not report['startupError'] and report['policy'] == policy
            gaps = [row for row in report['sources'] if not row['playable']]
            assert (code == 3) == (bool(gaps) or any(not g['playable'] for g in report['groups']))
            if installed:
                assert len(report['sources']) == 344 and len(report['groups']) == 69
                assert sum(len(g['members']) for g in report['groups']) == 168
                assert report['simultaneousLimit'] == 12 and all(g['playable'] for g in report['groups'])
                assert {g['id'] for g in gaps} == ({812, 813, 1016, 1017, 1018} if policy == 'literal' else {1016})
                assert next(g for g in gaps if g['id'] == 1016)['path'] == 'Stream.wav'
                # Independent Python PCM parsing checks every playable leaf.
                profile = configparser.ConfigParser(interpolation=None, strict=True)
                profile.read(root / 'Sounds.ini')
                names = {p.name.lower(): p for p in root.iterdir() if p.is_file()}
                for source in report['sources']:
                    value = profile['Sounds'][str(source['id'])].strip()
                    if len(value) >= 2 and value[0] == value[-1] and value[0] in "'\"":
                        value = value[1:-1]
                    if ';' in value:
                        value = value[:value.rfind("'", 0, value.index(';')) + 1]
                    leaf = value + '.wav'
                    assert source['path'] == leaf
                    chosen = names.get(leaf.lower())
                    if not chosen and policy == 'dequote-missing-leaf' and leaf.startswith("'") and leaf.endswith("'.wav"):
                        chosen = names.get((leaf[1:-5] + '.wav').lower())
                    assert bool(chosen) == source['playable']
                    if chosen:
                        with wave.open(str(chosen), 'rb') as wav:
                            count, channels, width, rate = wav.getnframes(), wav.getnchannels(), wav.getsampwidth(), wav.getframerate()
                            assert source['pcmBytes'] == count * channels * width
                            assert source['durationMs'] == count * 1000 // rate
            else:
                assert len(report['sources']) == 2 and len(report['groups']) == 1
                assert {g['id'] for g in gaps} == ({10, 20} if policy == 'literal' else {20})
                assert report['groups'][0]['playable'] == (policy != 'literal')
            results[policy] = {'playableSources': len(report['sources']) - len(gaps), 'missingSourceIds': [g['id'] for g in gaps],
                               'playableGroups': sum(g['playable'] for g in report['groups']), 'report': file.name}
        assert before == {p.name: digest(p) for p in root.iterdir() if p.is_file()}
        return {'policies': results, 'inputsUnchanged': True, 'inputSha256': before}

    synthetic = check_catalog(fixture, 'synthetic')
    installed = None
    if args.sounds_root:
        run('manifest-before', ['./tools/original-manifest.sh', 'verify'])
        try:
            installed = check_catalog(args.sounds_root.resolve(), 'installed', True)
        finally:
            run('manifest-after', ['./tools/original-manifest.sh', 'verify'])
    sources = ['reconstruction/audio/catalog_preflight.hpp', 'reconstruction/audio/catalog_preflight.cpp',
               'apps/qt-shell/audio_session.hpp', 'apps/qt-shell/audio_session.cpp', 'apps/qt-shell/audio_cli.hpp',
               'apps/qt-shell/audio_cli.cpp', 'tests/audio-catalog-fixture.hpp', 'tests/audio-catalog-preflight-test.cpp',
               'tests/audio-session-test.cpp', 'tools/test-audio-session.py', 'audio/qt_output.cpp',
               'reconstruction/audio/native_manager_backend.cpp', 'apps/qt-shell/main.cpp',
               'apps/qt-shell/CMakeLists.txt', 'audio/CMakeLists.txt']
    report = {'gameLaunched': False, 'physicalAudioDeviceOpened': False, 'testsPassed': True,
              'synthetic': synthetic, 'installed': installed,
              'sourceSha256': {p: digest(REPO / p) for p in sources},
              'binarySha256': {p: digest(build / p) for p in ['mnm-qt-shell', 'audio-session-test', 'audio/audio-catalog-preflight-test']}}
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'Offline audio session checks passed: {evidence / "report.json"}', flush=True)


if __name__ == '__main__':
    main()
