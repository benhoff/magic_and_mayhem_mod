#!/usr/bin/env python3
"""Manifest-guarded installed audio profile/catalog comparison; no game launch."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
CAPACITY=16384

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('profile',nargs='?',type=Path,default=REPO/'working/game-nocd/Sounds/Sounds.ini')
    args=parser.parse_args();source=args.profile.resolve()
    parent=REPO/'working/tests/installed-audio-profile';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(f'Installed profile evidence: {root}',flush=True)
    def run(name,command,**kwargs):
        result=subprocess.run(command,capture_output=True,text=True,timeout=180,**kwargs)
        (root/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode();return result.stdout
    try:
        run('manifest-before',[str(REPO/'tools/original-manifest.sh'),'verify'])
        data=source.read_bytes()
        if len(data)>1024*1024:raise ValueError('Installed profile exceeds native bound')
        text=data.decode('ascii');sections={};current=None
        # Inventory selects queries only. Both tested readers receive the untouched bytes.
        for line in text.splitlines():
            line=line.strip(' \t\r')
            if not line or line.startswith(';'):continue
            if line.startswith('[') and ']' in line:
                current=line[1:line.index(']')].strip()
                if current in sections:raise ValueError('Duplicate section in query inventory')
                sections[current]=[]
            elif current is None:raise ValueError('Entry outside section')
            elif '=' in line:sections[current].append(line.split('=',1)[0].strip())
        if 'Sounds' not in sections or 'Randomised' not in sections:raise ValueError('Missing audio catalog sections')
        queries=[]
        def query(op,section,key='',capacity=CAPACITY,fallback=0):
            if len(section.encode('ascii'))>=128 or len(key.encode('ascii'))>=128:raise ValueError('Query text exceeds wire bound')
            queries.append({'op':op,'section':section,'key':key,'capacity':capacity,'fallback':fallback})
        for section,keys in sections.items():
            query(2,section)
            for key in keys:query(1,section,key,256 if section=='Randomised' else 260)
        query(3,'Optimisation','MaxSimultaneousSounds',fallback=16)
        for section in ['0 Load Permanent','0 Load Temporary','3 Load Permanent','3 Load Temporary']:
            if section not in sections:query(2,section)
        if len(queries)>4096:raise ValueError('Too many fixture queries')
        (root/'fixture.ini').write_bytes(data)
        (root/'queries.json').write_text(json.dumps(queries,indent=2)+'\n')
        with (root/'queries.bin').open('wb') as output:
            output.write(struct.pack('<III',0x504e4d4d,1,len(queries)))
            for q in queries:output.write(struct.pack('<III128s128s',q['op'],q['capacity'],q['fallback'],q['section'].encode('ascii'),q['key'].encode('ascii')))
        build=REPO/'working/build/installed-audio-profile'
        run('configure',['cmake','-S',str(REPO/'audio'),'-B',str(build),'-DBUILD_TESTING=OFF'])
        run('native-build',['cmake','--build',str(build),'--target','mnm-audio-profile-compare','--parallel','4'])
        binary=build/'mnm-audio-profile-compare'
        imports={'GetPrivateProfileStringA':24,'GetPrivateProfileSectionA':16,'GetPrivateProfileIntA':16,
                 'GetFullPathNameA':16,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=root/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'kernel32.lib')])
        run('reference-build',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin',
                               '-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
                               '-c',str(REPO/'tests/profile-api/installed_reference.c'),'-o',str(root/'reference.obj')])
        run('reference-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib',
                              '/safeseh:no','/timestamp:0',f'/out:{root/"reference.exe"}',str(root/'reference.obj'),str(root/'kernel32.lib')])
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'),WINEDEBUG='-all')
        version=run('wine-version',['wine','--version'],env=env).strip()
        run('wine',['wine',str(root/'reference.exe')],cwd=root,env=env)
        run('native',[str(binary)],cwd=root)
        def records(name):
            raw=(root/name).read_bytes()
            if len(raw)!=len(queries)*(CAPACITY+4):raise ValueError('Invalid result size')
            return list(struct.iter_unpack(f'<I{CAPACITY}s',raw))
        native=records('native-results.bin');wine=records('wine-results.bin');cases=[]
        for q,n,w in zip(queries,native,wine):
            item=dict(q,matched=n==w,native_return=n[0],wine_return=w[0])
            if n!=w:item.update(native_hex=n[1].hex(),wine_hex=w[1].hex())
            cases.append(item)
        catalogs={}
        for flavor in ['native','wine']:
            output=run(flavor+'-catalog',[str(binary),flavor+'-results.bin'],cwd=root)
            catalogs[flavor]=json.loads(output);(root/(flavor+'-catalog.json')).write_text(output)
        catalog=catalogs['native'];known=set(catalog['Sounds'])|set(catalog['Randomised'])
        unresolved=sorted({i for group in catalog['groups'] for i in group['members'] if i not in known})
        files=['tests/profile-api/installed_wire.h','tests/profile-api/installed_reference.c',
               'tests/profile-api/installed_native.cpp','tools/test-installed-audio-profile.py',
               'assets/profile.cpp','assets/asset_file.cpp','reconstruction/audio/manager_configuration.cpp',
               'reconstruction/audio/source_cache.cpp','audio/CMakeLists.txt']
        report={'scope':'installed profile through Qt and PE32 Wine; recovered table/group/source-name decoding; no live game',
                'game_launched':False,'installed_profile':str(source),'profile_bytes':len(data),
                'profile_sha256':hashlib.sha256(data).hexdigest(),'wine_version':version,
                'queries':cases,'mismatches':sum(not c['matched'] for c in cases),
                'catalogs_match':catalogs['native']==catalogs['wine'],'source_count':len(catalog['Sounds']),
                'group_count':len(catalog['Randomised']),'unresolved_group_members':unresolved,
                'source_sha256':{p:hashlib.sha256((REPO/p).read_bytes()).hexdigest() for p in files},
                'artifact_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in
                                   [root/'fixture.ini',root/'queries.bin',root/'reference.exe',root/'native-results.bin',root/'wine-results.bin',root/'native-catalog.json',root/'wine-catalog.json']},
                'native_binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest()}
        if source.read_bytes()!=data:raise RuntimeError('Installed profile changed during comparison')
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(f"Compared {len(cases)} calls: {report['mismatches']} mismatches; catalog agreement: {report['catalogs_match']}",flush=True)
        print(f'Report: {root/"report.json"}',flush=True)
        return bool(report['mismatches'] or not report['catalogs_match'])
    finally:
        run('manifest-after',[str(REPO/'tools/original-manifest.sh'),'verify'])
        report_path=root/'report.json'
        if report_path.exists():
            final_report=json.loads(report_path.read_text())
            final_report['original_manifest_verified_before_and_after']=True
            final_report['tests_passed']=not final_report['mismatches'] and final_report['catalogs_match']
            report_path.write_text(json.dumps(final_report,indent=2)+'\n')
if __name__=='__main__':raise SystemExit(main())
