#!/usr/bin/env python3
"""Validate optional voice-import staging on a disposable game, without launching it."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol


def main():
    parent=REPO/'working/tests/audio-staging';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    def verify(label):
        result=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],text=True,capture_output=True)
        (root/(label+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
    verify('original-before')
    try:
        stream=root/'frame.bin'
        with stream.open('wb') as out:
            out.write(frame_protocol.initial_header());out.truncate(frame_protocol.SIZE)
        channel=root/'voices.bin'
        with channel.open('wb') as out:
            out.write(b'MNMAUD01'+struct.pack('<II',2,128+16*1024*1024));out.truncate(128+16*1024*1024)
        result=subprocess.run(['python3',str(REPO/'tools/run-opengl-game.py'),'--stream',str(stream),'--voice-channel',str(channel),'--stage-only'],text=True,capture_output=True,timeout=60)
        (root/'stage.log').write_text(result.stdout+result.stderr);result.check_returncode()
        line=next(line for line in result.stdout.splitlines() if line.startswith('Render experiment: '))
        experiment=Path(line.split(': ',1)[1]);metadata=json.loads((experiment/'manifest.json').read_text())
        assert metadata['voice_channel']==str(channel) and metadata['audio_dll_sha256']
        for name,key in [('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256'),('MnmAudio.dll','audio_dll_sha256')]:
            assert hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()==metadata[key]
        binary=(experiment/'game/Chaos.exe').read_bytes()
        assert b'MnmAudio.dll\0' in binary and b'AudioAnchor\0' in binary
        assert hashlib.sha256((REPO/'working/game-nocd/Chaos.exe').read_bytes()).hexdigest()==metadata['source_sha256']
        (root/'report.json').write_text(json.dumps({'scope':'staging only, no game launch','experiment':str(experiment),'source_sha256':metadata['source_sha256'],'staged_sha256':metadata['staged_sha256'],'audio_dll_sha256':metadata['audio_dll_sha256'],'checks':['guarded_source','isolated_import','audio_channel_validation','staged_hashes','unchanged_source','original_manifest_before_after']},indent=2)+'\n')
    finally:verify('original-after')
    print(f'Audio staging evidence: {root/"report.json"}')

if __name__=='__main__':main()
