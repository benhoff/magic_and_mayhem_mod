#!/usr/bin/env python3
"""Test Qt media decoding and its PE32 request bridge, without running Chaos.exe."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import time
import wave

REPO = Path(__file__).resolve().parents[1]

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol, media_v1 as media_protocol



def load(name, path):
    spec = importlib.util.spec_from_file_location(name, REPO / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    parent = REPO / 'working/tests/native-media'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Media evidence: {root}', flush=True)
    assets = root / 'assets'
    (assets / 'FMV').mkdir(parents=True)
    (assets / 'Sounds').mkdir()
    pcm = b''.join(struct.pack('<hh', 4000 if (i//50)%2 else -4000, 2000 if (i//75)%2 else -2000) for i in range(22050))
    sound = assets / 'Sounds/Test.wav'
    with wave.open(str(sound), 'wb') as wav:
        wav.setnchannels(2)
        wav.setsampwidth(2)
        wav.setframerate(22050)
        wav.writeframes(pcm)
    rgb = b''.join(bytes(((255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 255))[(y >= 24)*2+(x >= 32)]) for y in range(48) for x in range(64))
    raw = root / 'frames.rgb'
    raw.write_bytes(rgb*10)
    movie = assets / 'FMV/Test.avi'
    subprocess.run(['ffmpeg', '-v', 'error', '-f', 'rawvideo', '-pixel_format', 'rgb24', '-video_size', '64x48', '-framerate', '10', '-i', str(raw), '-i', str(sound), '-c:v', 'ffv1', '-pix_fmt', 'bgr0', '-c:a', 'pcm_s16le', str(movie)], check=True)
    shutil.copy2(movie, assets / 'FMV/Skip.avi')
    build = REPO / 'working/build/qt-shell'
    subprocess.run(['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--parallel', '4'], check=True)
    shell = build / 'mnm-qt-shell'
    env = {key: value for key, value in os.environ.items() if not key.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_MEDIA_BACKEND='ffmpeg')
    decoded = {}
    for kind, path in [('movie', movie), ('sound', sound), ('missing', assets / 'FMV/Missing.avi')]:
        evidence = root / f'{kind}.json'
        with (root / f'{kind}.log').open('w') as log:
            result = subprocess.run(['xvfb-run', '-a', str(shell), '--media', str(path), '--media-test', '--media-report', str(evidence)], env=env, stdout=log, stderr=log, timeout=20)
        data = json.loads(evidence.read_text())
        assert result.returncode == (8 if kind == 'missing' else 0), (kind, result.returncode, data)
        if kind != 'missing':
            assert data['audio_bytes'] == len(pcm), data
            assert data['audio_s16le_stereo_22050_sha256'] == hashlib.sha256(pcm).hexdigest(), data
            assert data['accepted'] and data['result'] == 3 and not data['audible_output_requested'], data
        else:
            assert data['result'] == 5 and not data['accepted'] and data['error'], data
        decoded[kind] = data
    rgba = b''.join(rgb[i:i+3]+b'\xff' for i in range(0, len(rgb), 3))
    assert decoded['movie']['video_frames'] == 10 and decoded['movie']['width'] == 64 and decoded['movie']['height'] == 48, decoded['movie']
    assert decoded['movie']['first_frame_rgba_sha256'] == hashlib.sha256(rgba).hexdigest(), decoded['movie']
    dll = load('media_build', 'tools/build-render-bridge.py').build(True)
    stage = load('media_stage', 'tools/prepare-shadow-experiment.py')
    shutil.copy2(dll, root / dll.name)
    (root / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(), dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
    stream = root / 'frame.bin'
    with stream.open('wb') as file:
        file.write(frame_protocol.initial_header())
        file.truncate(frame_protocol.SIZE)
    channel = root / 'media.bin'
    history = root / 'broker.json'
    with (root / 'broker.log').open('w') as log:
        server = subprocess.Popen(['xvfb-run', '-a', str(shell), '--media-server-test', str(channel), '--assets', str(assets), '--media-count', '8', '--media-report', str(history)], env=env, stdout=log, stderr=log)
        try:
            deadline = time.monotonic()+5
            while not channel.exists() or channel.stat().st_size != media_protocol.SIZE:
                if server.poll() is not None or time.monotonic()>deadline:
                    raise RuntimeError('Media server failed to create its channel')
                time.sleep(0.02)
            env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all', MNM_MEDIA_SELFTEST='1',
                       MNM_RENDER_MEDIA='Z:'+str(channel).replace('/', '\\'), MNM_RENDER_STREAM='Z:'+str(stream).replace('/', '\\'))
            with (root / 'wine.log').open('w') as wine_log:
                subprocess.run(['wine', str(root / 'selftest.exe')], cwd=root, env=env, stdout=wine_log, stderr=wine_log, check=True, timeout=30)
            assert server.wait(timeout=5) == 0
        finally:
            if server.poll() is None:
                server.terminate()
                server.wait(timeout=5)
    records = json.loads(history.read_text())
    assert [(r['operation'], r['status']) for r in records] == [(1, 3), (1, 4), (2, 4), (2, 7), (2, 4), (3, 3), (1, 6), (1, 6)], records
    assert records[0]['first_frame_rgba_sha256'] == hashlib.sha256(rgba).hexdigest(), records[0]
    assert records[0]['audio_s16le_stereo_22050_sha256'] == hashlib.sha256(pcm).hexdigest(), records[0]
    report = {'origin': 'synthetic_qt_media_to_pe32', 'architecture': 'PE32 i386', 'live_game_validated': False,
              'audible_output_validated': False, 'directsound_replaced': False, 'decoded': decoded,
              'checks': ['exact_movie_rgba', 'exact_movie_pcm', 'exact_wav_pcm', 'missing_media', 'movie_complete', 'movie_skip',
                         'async_sound', 'loop_stop', 'nostop_busy', 'path_traversal_rejected', 'unsupported_sound_fallback',
                         'native_to_legacy_sound_replacement', 'stale_host_fallback', 'last_error', 'fastcall_trampoline', 'prologue_guard'],
              'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest()}
    (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Native media checks passed: {root / "report.json"}')


if __name__ == '__main__':
    main()
