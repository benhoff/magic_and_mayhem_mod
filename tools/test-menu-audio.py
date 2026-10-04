#!/usr/bin/env python3
"""Validate native menu audio routing with synthetic assets and a fake sink."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
BUILD = REPO / 'working/build/qt-shell'


def main():
    parent = REPO / 'working/tests/menu-audio'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Menu audio evidence: {evidence}', flush=True)
    widgets = ['main-menu', 'quick-battle-menu', 'mini-menu', 'battle-result', 'quick-battle-result',
               'map-selection', 'load-game', 'save-game', 'preferences', 'multiplayer-setup',
               'multiplayer-game-selection', 'single-player-battle', 'multiplayer-lobby',
               'region-entry', 'character-screen', 'grimoire', 'spellbox']
    targets = ['mnm-qt-shell', 'menu-audio-test', 'menu-music-test', 'menu-music-recovery-test', 'audio-recovery-test', 'audio-session-test', 'audio-output-test',
               'audio-native-manager-test', *[w + '-test' for w in widgets]]
    pattern = '^(qt-menu-audio|qt-menu-music|qt-menu-music-recovery|qt-audio-output-recovery|qt-audio-manager-session|audio-qt-output|audio-native-manager|qt-shell-help|qt-shell-startup|' + \
              '|'.join('qt-' + w for w in widgets if w not in ['battle-result', 'quick-battle-result']) + \
              '|qt-battle-results|qt-quick-battle-results)$'
    commands = [('configure', ['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(BUILD)]),
                ('build', ['cmake', '--build', str(BUILD), '--target', *targets, '--parallel', '4']),
                ('ctest', ['ctest', '--test-dir', str(BUILD), '--output-on-failure', '--timeout', '15', '-R', pattern])]
    for name, command in commands:
        result = subprocess.run(command, cwd=REPO, text=True, capture_output=True, timeout=300)
        (evidence / f'{name}.log').write_text(result.stdout + result.stderr)
        if result.returncode:
            raise RuntimeError(f'{name} failed; see {evidence / (name + ".log")}')
    # The shell must reject audio-only options without a menu preview, even in a
    # headless invocation. Invalid options must never reach device/game startup.
    import os
    environment = dict(os.environ, QT_QPA_PLATFORM='offscreen')
    for number, args in enumerate([['--menu-audio'], ['--menu-audio-policy', 'literal'], ['--menu-click-sound', '822'], ['--menu-music', 'missing.wav']]):
        result = subprocess.run([str(BUILD / 'mnm-qt-shell'), *args], cwd=REPO, env=environment,
                                capture_output=True, text=True, timeout=10)
        (evidence / f'arguments-{number}.log').write_text(result.stdout + result.stderr)
        assert result.returncode == 2
    sources = ['apps/qt-shell/menu_audio_controller.hpp', 'apps/qt-shell/menu_audio_controller.cpp',
               'apps/qt-shell/audio_session.hpp', 'apps/qt-shell/audio_session.cpp', 'apps/qt-shell/main.cpp',
               'apps/qt-shell/menu_preview.hpp', 'apps/qt-shell/menu_preview.cpp', 'apps/qt-shell/CMakeLists.txt',
               'apps/qt-shell/menu_audio_preferences.hpp', 'apps/qt-shell/menu_music_controller.hpp',
               'apps/qt-shell/menu_music_controller.cpp', 'apps/qt-shell/menu_music_output.cpp', 'apps/qt-shell/menu_music_devices.hpp', 'apps/qt-shell/native_playback.hpp', 'apps/qt-shell/native_playback.cpp',
               'tests/menu-audio-fixture.hpp', 'tests/menu-music-test.cpp', 'tests/menu-music-recovery-test.cpp', 'tests/menu-audio-test.cpp', 'tests/audio-recovery-test.cpp', 'tests/audio-catalog-fixture.hpp', 'tests/grimoire-fixtures.hpp',
               'tools/test-menu-audio.py', 'audio/qt_output.cpp']
    sources += [str(p.relative_to(REPO)) for p in sorted((REPO / 'apps/qt-shell').glob('*widget.*pp'))]
    digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    report = {'scope': 'native menu semantic cues, independent persisted music/effects gains, explicit looping track lifecycle and output recovery, shared manager and queue transitions',
              'gameLaunched': False, 'installedAssetsRead': False, 'physicalAudioDeviceOpened': False,
              'testsPassed': True, 'sourceSha256': {p: digest(REPO / p) for p in sources},
              'binarySha256': {p: digest(BUILD / p) for p in ['menu-audio-test', 'menu-music-test', 'menu-music-recovery-test', 'audio-recovery-test', 'mnm-qt-shell', 'audio-session-test']}}
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'Offline menu audio checks passed: {evidence / "report.json"}', flush=True)


if __name__ == '__main__':
    main()
