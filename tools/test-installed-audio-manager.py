#!/usr/bin/env python3
"""Installed catalog -> native manager -> independently checked PCM; no game/device."""
import argparse
import configparser
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import wave
REPO=Path(__file__).resolve().parents[1]

def profile_value(value):
    value=value.strip(' \t\r')
    return value[1:-1] if len(value)>=2 and value[0] in "'\"" and value[-1]==value[0] else value

def source_name(value):
    value=profile_value(value)
    if ';' in value:
        closing=value.rfind("'",0,value.index(';'))
        if closing<0:raise ValueError('Unsupported installed comment name')
        value=value[:closing+1]
    return value

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('sounds',nargs='?',type=Path,default=REPO/'working/game-nocd/Sounds')
    parser.add_argument('--executable',type=Path,help='Use a prebuilt fixture (also supports sanitizer builds)')
    parser.add_argument('--dequote-source-leaf',action='store_true',help='Enable explicit native missing quoted-leaf compatibility')
    args=parser.parse_args();sounds=args.sounds.resolve()
    parent=REPO/'working/tests/installed-audio-manager';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(f'Installed manager evidence: {root}',flush=True)
    def run(name,command):
        result=subprocess.run(command,capture_output=True,text=True,timeout=180)
        (root/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
    try:
        run('manifest-before',[str(REPO/'tools/original-manifest.sh'),'verify'])
        profile=(sounds/'Sounds.ini').read_bytes();ini=configparser.ConfigParser(interpolation=None,allow_no_value=True)
        ini.read_string(profile.decode('ascii'))
        source_ids=sorted(int(key) for key in ini['Sounds']);groups={int(key):[int(re.match(r'\s*([+-]?\d+)',part)[1]) for part in profile_value(value).split(',') if part] for key,value in ini['Randomised'].items()}
        names={int(key):source_name(value) for key,value in ini['Sounds'].items()}
        files={p.name.lower():p for p in sounds.iterdir() if p.is_file()};before={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in files.values() if p.suffix.lower()=='.wav'}
        build=REPO/'working/build/installed-audio-manager'
        if args.executable:binary=args.executable.resolve()
        else:
            run('configure',['cmake','-S',str(REPO/'audio'),'-B',str(build),'-DBUILD_TESTING=OFF'])
            run('build',['cmake','--build',str(build),'--target','mnm-audio-installed-manager','--parallel','4']);binary=build/'mnm-audio-installed-manager'
        run('native',[str(binary),str(sounds),str(root),*(['--dequote-source-leaf'] if args.dequote_source_leaf else [])])
        native=json.loads((root/'native.json').read_text());assert native['sourceIds']==source_ids
        assert {g['id']:g['members'] for g in native['groups']}==groups
        assert native['simultaneousLimit']==int(ini['Optimisation']['MaxSimultaneousSounds'])
        decoded={};checked=0;failures=[];quoted=set();completed=0;counts={}
        def pcm(path):
            if path not in decoded:
                with wave.open(str(path),'rb') as wav:
                    channels,bits,rate=wav.getnchannels(),wav.getsampwidth()*8,wav.getframerate();data=wav.readframes(wav.getnframes())
                assert channels in (1,2) and bits in (8,16)
                samples=list(struct.unpack('<'+'h'*(len(data)//2),data)) if bits==16 else [(b-128)*256 for b in data]
                decoded[path]=(channels,bits,rate,data,samples)
            return decoded[path]
        for row in native['cases']:
            stage=row['stage'];counts[stage]=counts.get(stage,0)+1
            expected=row['requested']
            if stage!='upload' and expected in groups:expected=groups[expected][row.get('rng',0)%len(groups[expected])]
            if expected not in names:expected=410
            name=names[expected];path=files.get((name+'.wav').lower())
            if path is None and args.dequote_source_leaf and len(name)>=2 and name[0]==name[-1]=="'":path=files.get((name[1:-1]+'.wav').lower())
            if path is None:
                assert row['status']==-2147467259, row
                classification='logical_group_without_wave' if expected in groups else 'missing_wave'
                if len(name)>=2 and name[0]==name[-1]=="'" and (name[1:-1]+'.wav').lower() in files:
                    classification='quoted_leaf_gap';quoted.add(expected)
                failures.append({'stage':stage,'requested':row['requested'],'source':expected,'name':name,'classification':classification})
                continue
            assert row['status']==0 and row['source']==expected, row
            channels,bits,rate,data,samples=pcm(path)
            assert row['pcm_bytes']==len(data) and row['pcm_sha256']==hashlib.sha256(data).hexdigest(), row
            assert row['duration']==((len(data)*1000)&0xffffffff)//(rate*channels*(bits//8)), row
            frames=len(samples)//channels;expected_pcm=[]
            looping=stage!='upload';copies=2 if stage in ('overlap','master-gain') else 1
            gain=0.1 if stage=='master-gain' else 1.0
            for i in range(256):
                frame,remainder=divmod(i*rate,48000)
                if frame>=frames and not looping:expected_pcm.extend([0,0]);continue
                frame%=frames;following=(frame+1)%frames if looping else min(frame+1,frames-1)
                for channel in (0,1):
                    index=min(channel,channels-1);a=samples[frame*channels+index];b=samples[following*channels+index]
                    value=(a+(b-a)*(remainder/48000))*copies*gain
                    value=max(-32768,min(32767,value));expected_pcm.append(int(math.copysign(math.floor(abs(value)+0.5),value)))
            actual=list(struct.unpack('<512h',(root/row['mix']).read_bytes()));assert actual==expected_pcm,(row, next((i for i,(a,b) in enumerate(zip(actual,expected_pcm)) if a!=b),None))
            checked+=1
            if stage=='upload':assert row['completed'];completed+=1
        assert native['cleanup'] and native['restart'] and native['retainedBuffers']==1
        assert (sounds/'Sounds.ini').read_bytes()==profile
        assert all(hashlib.sha256((sounds/name).read_bytes()).hexdigest()==digest for name,digest in before.items())
        sources=['tests/audio-installed-manager.cpp','tools/test-installed-audio-manager.py','reconstruction/audio/native_manager_backend.cpp','reconstruction/audio/manager_lifecycle.cpp','reconstruction/audio/manager_configuration.cpp','reconstruction/audio/source_cache.cpp','reconstruction/audio/voice_admission.cpp','audio/mixer.cpp','audio/buffers.cpp','audio/voices.cpp','audio/wave_loader.cpp','assets/profile.cpp','assets/asset_file.cpp','assets/path_resolver.cpp','reconstruction/audio/voice_scheduler.cpp','reconstruction/audio/voice_lifetime.cpp','reconstruction/audio/manager_contract.cpp','reconstruction/audio/dsound_setup.cpp']
        report={'source_path_policy':native['sourcePathPolicy'],'scope':'installed native manager/upload/admission/PCM/lifecycle; offline only, expected path failures retained',
                'game_launched':False,'audio_device_opened':False,'sounds_root':str(sounds),'cases':len(native['cases']),
                'case_counts':counts,'pcm_cases_checked':checked,'one_shots_completed':completed,
                'expected_failures':failures,'quoted_leaf_gap_ids':sorted(quoted),'unresolved_admission_sources':sorted({f['source'] for f in failures if f['stage'] in ('admission','group')}),'cleanup':True,'restart':True,
                'profile_sha256':hashlib.sha256(profile).hexdigest(),'wave_sha256':before,
                'source_sha256':{p:hashlib.sha256((REPO/p).read_bytes()).hexdigest() for p in sources},
                'fixture_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),
                'artifact_sha256':{str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [root/'native.json',*sorted((root/'pcm').glob('*.s16'))]},
                'tests_passed':True,'original_manifest_verified_before_and_after':False}
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(f"Checked {checked} exact PCM cases, {completed} one-shot completions; {len(failures)} expected path failures; quoted path gaps: {sorted(quoted)}",flush=True)
        print(f'Report: {root/"report.json"}',flush=True)
    finally:
        run('manifest-after',[str(REPO/'tools/original-manifest.sh'),'verify'])
        p=root/'report.json'
        if p.exists():
            report=json.loads(p.read_text());report['original_manifest_verified_before_and_after']=True;p.write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
