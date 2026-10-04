#!/usr/bin/env python3
"""Compare native profile policy to real PE32 Wine APIs using generated input only."""
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile
REPO = Path(__file__).resolve().parents[1]
FIXTURE = '''; generated; no installed game input
[Sounds]
Tone = alpha
Single = 'tone'
Double = " other "
Inline = empty ; literal inline
Empty =
[Map]
10
20
[Numbers]
Decimal=42
Zero=0
Quoted="42"
Hex=0x10
Negative=-1
Suffix=3junk
Empty=
Overflow=4294967296
'''
POLICY = {'section_truncated', 'section_exact', 'integer_hex', 'integer_negative',
          'integer_suffix', 'integer_empty', 'integer_overflow', 'integer_negative'}

def main():
    parent = REPO/'working/tests/profile-api'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Profile API evidence: {root}', flush=True)
    def run(name, command, **kwargs):
        result = subprocess.run(command, capture_output=True, text=True, timeout=180, **kwargs)
        (root/(name+'.log')).write_text(result.stdout+result.stderr)
        result.check_returncode()
        return result.stdout.strip()
    build = REPO/'working/build/profile-api'
    run('configure', ['cmake','-S',str(REPO/'assets'),'-B',str(build)])
    run('native-build', ['cmake','--build',str(build),'--target','mnm-profile-compare','--parallel','4'])
    imports = {'GetPrivateProfileStringA':24,'GetPrivateProfileSectionA':16,
               'GetPrivateProfileIntA':16,'GetFullPathNameA':16,'CreateFileA':28,
               'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
    definition = root/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in imports.items()))
    run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'kernel32.lib')])
    run('reference-build',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding',
                          '-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx',
                          '-Wall','-Wextra','-Werror','-c',str(REPO/'tests/profile-api/reference.c'),'-o',str(root/'reference.obj')])
    run('reference-link',['lld-link','/machine:x86','/entry:start','/subsystem:console',
                         '/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{root/"reference.exe"}',
                         str(root/'reference.obj'),str(root/'kernel32.lib')])
    (root/'fixture.ini').write_bytes(FIXTURE.replace('\n','\r\n').encode('ascii'))
    run('native',[str(build/'mnm-profile-compare')],cwd=root)
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'), WINEDEBUG='-all')
    version = run('wine-version',['wine','--version'],env=env)
    run('wine',['wine',str(root/'reference.exe')],cwd=root,env=env)
    names = re.findall(r'^Q\((\w+),', (REPO/'tests/profile-api/queries.inc').read_text(),re.M)
    def records(path):
        data=path.read_bytes()
        if len(data)!=len(names)*132: raise RuntimeError(f'Invalid result size: {path}')
        return list(struct.iter_unpack('<I128s',data))
    native=records(root/'native-results.bin');wine=records(root/'wine-results.bin')
    rejections={int(i) for i in (root/'native-rejections.txt').read_text().splitlines()}
    cases=[]
    for index,(name,n,w) in enumerate(zip(names,native,wine)):
        matched=n==w and index not in rejections
        status='matched' if matched else 'native_policy_difference' if name in POLICY else 'unexpected_mismatch'
        cases.append({'name':name,'status':status,'native_rejected':index in rejections,'native_return':n[0],'wine_return':w[0],
                      'native_bytes_hex':n[1].hex(),'wine_bytes_hex':w[1].hex()})
    files=['assets/profile.cpp','assets/profile.hpp','tests/profile-api/queries.inc',
           'tests/profile-api/native.cpp','tests/profile-api/reference.c','tools/test-profile-api.py']
    report={'scope':'generated ASCII profile calls, PE32 Wine vs native; not Windows 98 or installed game validation',
            'game_launched':False,'game_assets_read':False,'wine_version':version,
            'source_sha256':{p:hashlib.sha256((REPO/p).read_bytes()).hexdigest() for p in files},
            'artifact_sha256':{str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest()
                               for p in [root/'reference.exe',root/'fixture.ini',root/'native-results.bin',root/'wine-results.bin',root/'native-rejections.txt']},
            'native_fixture_sha256':hashlib.sha256((build/'mnm-profile-compare').read_bytes()).hexdigest(),
            'cases':cases,'unexpected_mismatches':sum(c['status']=='unexpected_mismatch' for c in cases)}
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    for c in cases:
        print(c['name'],c['status'],c['native_return'],c['wine_return'])
    print(f'Report: {root/"report.json"}')
    return bool(report['unexpected_mismatches'])
if __name__=='__main__':raise SystemExit(main())
