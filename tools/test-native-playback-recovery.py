#!/usr/bin/env python3
"""Injected device topology around actual Qt finite movie/file-sound decoding."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile
import wave

ROOT=Path(__file__).resolve().parents[1]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/name)!=digest for name,digest in declaration['sources'].items()):
        raise ValueError('Playback recovery source declaration drift')
    parent=ROOT/'working/tests/native-playback-recovery';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(label,command,env=None):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(value) for value in command],cwd=ROOT,env=env,
                           stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
    pcm=b''.join(struct.pack('<hh',(i*37)%8001-4000,(i*61)%4001-2000) for i in range(22050*2))
    sound=out/'sound.wav'
    with wave.open(str(sound),'wb') as wav:
        wav.setnchannels(2);wav.setsampwidth(2);wav.setframerate(22050);wav.writeframes(pcm)
    rgb=b''.join(bytes(((x*7+n*31)&255,(y*11+n*13)&255,(x+y+n*29)&255))
                 for n in range(20) for y in range(32) for x in range(48))
    rgba=b''.join(rgb[i:i+3]+b'\xff' for i in range(0,len(rgb),3))
    (out/'frames.rgb').write_bytes(rgb);movie=out/'movie.avi'
    run('fixture',['ffmpeg','-v','error','-f','rawvideo','-pixel_format','rgb24','-video_size','48x32',
                   '-framerate','10','-i',out/'frames.rgb','-i',sound,'-c:v','ffv1','-pix_fmt','bgr0',
                   '-c:a','pcm_s16le',movie])
    build=out/'build'
    run('configure',['cmake','-S',ROOT/'tests/native-playback-recovery','-B',build,'-DCMAKE_BUILD_TYPE=Release'])
    run('build',['cmake','--build',build,'-j4'])
    dependencies={'tests/native-playback-recovery/CMakeLists.txt','tools/test-native-playback-recovery.py'}
    for path in build.rglob('*.o.d'):
        for name in shlex.split(path.read_text().replace('\\\n',' ')):
            if not Path(name).is_absolute():continue
            dependency=Path(name).resolve()
            if dependency.is_relative_to(ROOT) and not dependency.is_relative_to(ROOT/'working'):
                dependencies.add(str(dependency.relative_to(ROOT)))
    if dependencies-set(declaration['sources']):
        raise ValueError('Undeclared compiler sources: '+repr(sorted(dependencies-set(declaration['sources']))))
    env={name:value for name,value in os.environ.items() if not name.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='offscreen',QT_MEDIA_BACKEND='ffmpeg')
    cases=[]
    for mode,kind in [('recover','movie'),('recover','sound'),('cancel','sound'),('explicit','sound')]:
        label=mode+'-'+kind;path=out/(label+'.json')
        run(label,[build/'native-playback-recovery-test',mode,kind,movie if kind=='movie' else sound,path],env)
        record=json.loads(path.read_text());assert record['success'],record
        if mode!='cancel':
            assert record['audio_bytes']==len(pcm),record
            assert record['audio_s16le_stereo_22050_sha256']==hashlib.sha256(pcm).hexdigest(),record
            if kind=='movie':
                assert record['video_frames']==20 and record['video_rgba_sha256']==hashlib.sha256(rgba).hexdigest(),record
        cases.append(dict(mode=mode,kind=kind,report=str(path),sha256=sha(path),checks=record))
    report=dict(schema=1,success=True,scope='Injected topology with actual Qt decoder/output at muted gain; exact full PCM and all movie RGBA, initial absence/timer retry/loss/caller pause/explicit ownership/cancel. Physical unplug, audible waveform/tail and original equivalence excluded.',
                claims=declaration['claims'],sources=declaration['sources'],cases=cases,
                sources_stable=all(sha(ROOT/name)==digest for name,digest in declaration['sources'].items()),
                compiled_sources={name:sha(ROOT/name) for name in sorted(dependencies)},
                inputs={name:sha(out/name) for name in ['sound.wav','movie.avi','frames.rgb']})
    assert report['sources_stable']
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Playback recovery passed',out/'report.json')

if __name__=='__main__':main()
