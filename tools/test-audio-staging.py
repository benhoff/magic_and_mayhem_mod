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
    import argparse
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-game',type=Path,default=REPO/'working/game-nocd')
    args=parser.parse_args();source=args.source_game.resolve()
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
            out.write(b'MNMAUD01'+struct.pack('<II',3,128+16*1024*1024));out.truncate(128+16*1024*1024)
        result=subprocess.run(['python3',str(REPO/'tools/run-opengl-game.py'),'--stream',str(stream),'--voice-channel',str(channel),'--source-game',str(source),'--stage-only'],text=True,capture_output=True,timeout=60)
        (root/'stage.log').write_text(result.stdout+result.stderr);result.check_returncode()
        line=next(line for line in result.stdout.splitlines() if line.startswith('Render experiment: '))
        experiment=Path(line.split(': ',1)[1]);metadata=json.loads((experiment/'manifest.json').read_text())
        assert metadata['voice_channel']==str(channel) and metadata['audio_dll_sha256']
        for name,key in [('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256'),('MnmAudio.dll','audio_dll_sha256')]:
            assert hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()==metadata[key]
        binary=(experiment/'game/Chaos.exe').read_bytes()
        assert b'MnmAudio.dll\0' in binary and b'AudioAnchor\0' in binary
        assert hashlib.sha256((source/'Chaos.exe').read_bytes()).hexdigest()==metadata['source_sha256']
        import importlib.util
        spec=importlib.util.spec_from_file_location('audio_staging',REPO/'tools/prepare-native-audio.py')
        audio=importlib.util.module_from_spec(spec);spec.loader.exec_module(audio)
        layout=audio.module('audio_test_layout','tools/prepare-native-draw.py').layout
        _,offset,descriptors=layout(binary)
        names=[]
        for at in descriptors:
            name=struct.unpack_from('<I',binary,at+12)[0];start=offset(name,1)
            names.append(binary[start:binary.index(0,start)].decode().lower())
        assert 'dsound.dll' not in names and names.count('mnmaudio.dll')==2
        # Exercise later-stage drift guards against the actual pinned input.
        original=(source/'Chaos.exe').read_bytes()
        _,old_offset,old_descriptors=layout(original)
        target=next(at for at in old_descriptors if original[at:at+20]==audio.DESCRIPTOR)
        for at in [target,old_offset(0x197566,6),old_offset(struct.unpack('<5I',audio.DESCRIPTOR)[0],8)]:
            changed=bytearray(original);changed[at]^=1
            try:audio.redirect(original,bytes(changed))
            except ValueError:pass
            else:raise AssertionError('Audio byte drift accepted')
        (root/'report.json').write_text(json.dumps({'scope':'staging only, no game launch','experiment':str(experiment),'source_sha256':metadata['source_sha256'],'staged_sha256':metadata['staged_sha256'],'audio_dll_sha256':metadata['audio_dll_sha256'],'checks':['guarded_source','isolated_import','audio_channel_validation','staged_hashes','unchanged_source','original_manifest_before_after']},indent=2)+'\n')
    finally:verify('original-after')
    print(f'Audio staging evidence: {root/"report.json"}')

if __name__=='__main__':main()
