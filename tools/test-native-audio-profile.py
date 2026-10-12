#!/usr/bin/env python3
"""Installed native audio profile transport vs the original Wine reader; no game."""
import argparse,hashlib,importlib.util,json,os,shlex,struct,subprocess,tempfile,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--profile',type=Path,default=ROOT/'working/experiments/native-audio-input/run-h4r5z9xx/game/Sounds/Sounds.ini');args=parser.parse_args()
    declared=json.loads(args.claims.read_text());assert all(sha(ROOT/p)==h for p,h in declared['sources'].items())
    parent=ROOT/'working/tests/native-audio-profile';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,sources=declared['sources'],claims=declared['claims']);host=None;env=None
    def run(label,argv,**kw):
        with (out/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in argv],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180,**kw)
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before']=True;data=args.profile.read_bytes();assert len(data)<=1024*1024
        sounds=out/'sounds';sounds.mkdir();(sounds/'Sounds.ini').write_bytes(data);(out/'fixture.ini').write_bytes(data)
        sections={};current=None
        for line in data.decode('ascii').splitlines():
            line=line.strip(' \t\r')
            if not line or line.startswith(';'):continue
            if line.startswith('['):current=line[1:line.index(']')].strip();assert current not in sections;sections[current]=[]
            elif '=' in line:sections[current].append(line.split('=',1)[0].strip())
        queries=[]
        for section,keys in sections.items():
            queries.append((2,16384,0,section,''))
            queries.extend((1,256 if section=='Randomised' else 260,0,section,key) for key in keys)
        queries.extend([(3,16384,16,'Optimisation','MaxSimultaneousSounds'),(1,260,0,'Sounds','missing'),(2,16384,0,'MissingSection','')]);assert len(queries)<4096
        with (out/'queries.bin').open('xb') as f:
            f.write(struct.pack('<III',0x504e4d4d,1,len(queries)))
            for q in queries:f.write(struct.pack('<III128s128s',q[0],q[1],q[2],q[3].encode('ascii'),q[4].encode('ascii')))
        build=out/'build';run('configure',['cmake','-S',ROOT/'tests/native-audio-profile','-B',build,'-DCMAKE_BUILD_TYPE=Release'],cwd=ROOT)
        run('build',['cmake','--build',build,'--target','native-audio-profile-host','-j4'],cwd=ROOT)
        deps={'tests/native-audio-profile/CMakeLists.txt','tools/test-native-audio-profile.py','tests/native-audio-profile/producer.c','tests/profile-api/installed_reference.c','tests/profile-api/installed_wire.h','audio/CMakeLists.txt','assets/CMakeLists.txt','assets/resources/CMakeLists.txt'}
        for path in build.rglob('*.o.d'):
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                if Path(name).is_absolute():
                    source=Path(name).resolve()
                    if source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):deps.add(str(source.relative_to(ROOT)))
        if deps-set(declared['sources']):raise ValueError('Undeclared profile compiler closure: '+repr(sorted(deps-set(declared['sources']))))
        run('adapter',['python3',ROOT/'tools/build-audio-bridge.py','--selftest'],cwd=ROOT)
        dll=ROOT/'working/build/audio-bridge-selftest/MnmAudio.dll';(out/'MnmAudio.dll').write_bytes(dll.read_bytes());report['adapter_sha256']=sha(dll)
        spec=importlib.util.spec_from_file_location('builder',ROOT/'tools/build-shadow-bridge.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        imports=dict(builder.IMPORTS,GetPrivateProfileStringA=24,GetPrivateProfileSectionA=16,GetPrivateProfileIntA=16,GetFullPathNameA=16,ReadFile=20,WriteFile=20,LoadLibraryA=4,GetProcAddress=8)
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
        for name,source in [('reference','tests/profile-api/installed_reference.c'),('producer','tests/native-audio-profile/producer.c')]:
            run(name+'-compile',flags+['-c',ROOT/source,'-o',out/(name+'.obj')]);run(name+'-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/(name+'.exe')),out/(name+'.obj'),out/'kernel32.lib'])
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree=;mshtml=',QT_QPA_PLATFORM='offscreen')
        run('reference',['wine',out/'reference.exe'],cwd=out,env=env)
        channel=out/'voice.bin';stop=out/'stop';final=out/'host.json'
        with (out/'host.log').open('x') as log:
            host=subprocess.Popen([str(build/'native-audio-profile-host'),str(channel),str(sounds),str(stop),str(final)],env=env,stdout=log,stderr=subprocess.STDOUT)
            end=time.monotonic()+10
            while not channel.exists() or channel.stat().st_size<128:
                if host.poll() is not None or time.monotonic()>end:raise RuntimeError('Native profile host not ready')
                time.sleep(.02)
            end=time.monotonic()+10
            def ready():
                with channel.open('rb') as f:return struct.unpack_from('<I',f.read(128),80)[0]==1
            while not ready():
                if host.poll() is not None or time.monotonic()>end:raise RuntimeError('Profile readiness not published')
                time.sleep(.02)
            env.update(MNM_AUDIO_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_PROFILE_DLL='Z:'+str(out/'MnmAudio.dll').replace('/','\\'))
            run('producer',['wine',out/'producer.exe'],cwd=out,env=env)
            native=(out/'transport-results.bin').read_bytes();reference=(out/'wine-results.bin').read_bytes();assert len(native)==len(queries)*16388 and native==reference,'Native profile transport differs from Wine'
            assert (out/'producer-success.bin').read_bytes()==struct.pack('<I',1)
            stop.write_text('stop\n');host.wait(timeout=10);assert host.returncode==0
            report['native']=json.loads(final.read_text());assert report['native']['failures']==0 and report['native']['ready_after_stop'] is False
        assert args.profile.read_bytes()==data
        report.update(success=True,queries=len(queries),matched_bytes=len(native),profile_sha256=hashlib.sha256(data).hexdigest(),input_stable=True,original_manifest_verified_before_and_after=False,seven_prewrite_guard_refusals=True,reinstall_refused=True,compiler_dependencies=sorted(deps),results_sha256=hashlib.sha256(native).hexdigest())
    except Exception as error:report['error']=repr(error);raise
    finally:
        if host is not None and host.poll() is None:host.terminate();host.wait(timeout=5)
        if env is not None:
            stopped=subprocess.run(['wineserver','-k'],env=env,timeout=10);assert stopped.returncode in (0,1)
            subprocess.run(['wineserver','-w'],env=env,timeout=10,check=True);report['private_wine_stopped']=True
        if report.get('original_manifest_verified_before'):
            run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_and_after']=True
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in declared['sources'].items());(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_stable'];print('Native audio profile transport passed',out/'report.json')
if __name__=='__main__':main()
