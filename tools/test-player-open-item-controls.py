#!/usr/bin/env python3
"""Bounded process destruction, current-frame input and refused-copy controls."""
import argparse
import fcntl
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shlex
import shutil
import struct
import subprocess
import sys
import tarfile
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('claims','output','report','prefix'):parser.add_argument('--'+name,type=Path,required=True)
    a=parser.parse_args();out=a.output.resolve();prefix=a.prefix.resolve()
    if out.exists() or a.report.exists() or not out.is_relative_to(ROOT/'working') or not prefix.is_relative_to(ROOT/'working'):
        parser.error('Use fresh owned output/report and an admitted private prefix')
    declaration=json.loads(a.claims.read_text());sources=declaration['sources']
    def stable():return all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
    if not stable():parser.error('Prospective sources changed')
    out.mkdir(parents=True);env=os.environ|dict(TMPDIR=str(out),ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    report=dict(schema=1,success=False,scope=__doc__+'; supplied processes/X11/PE32 facade only, no original game, original-driver or historical-crash attribution.',sources=sources,claims=declaration['claims'],steps=[])
    def run(name,command,*,cwd=ROOT,environment=None,expected=0,timeout=180):
        command=list(map(str,command));p=subprocess.run(command,cwd=cwd,env=env|(environment or {}),capture_output=True,text=True,timeout=timeout)
        report['steps'].append(dict(name=name,command=command,exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
        if expected is not None and p.returncode!=expected:raise ValueError(name+': unexpected exit '+str(p.returncode))
        return p
    try:
        build=ROOT/'working/build/qt-shell'
        run('qt-build',['cmake','--build',build,'--target','mnm-qt-shell','qt-input-test','menu-session-destruction-test','game-hard-restart-test','--parallel','2'])
        run('qt-tests',['ctest','--test-dir',build,'--output-on-failure','-R','^qt-(menu-(staging|launcher)-destruction|game-(process|stopping)-destruction|game-hard-restart|shell-input(-hidpi)?)$'])
        flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True))
        cpp=['tests/menu-session-destruction-test.cpp','apps/qt-shell/live_menu_session.cpp','apps/qt-shell/menu_bridge.cpp','apps/qt-shell/engine_preferences_store.cpp']
        run('sanitizer-build',['c++','-std=c++17','-g','-O1','-fno-omit-frame-pointer','-fsanitize=address,undefined','-fPIC','-Iapps/qt-shell',*cpp,*flags,'-o',out/'sanitized'])
        for mode in ('staging','active','process','stop'):run('sanitizer-'+mode,[out/'sanitized',mode],timeout=15)
        run('deadline-build',['c++','-std=c++17','-shared','-fPIC','tests/qt-exit-during-staging.cpp',*flags,'-ldl','-o',out/'deadline.so'])
        fixture=out/'fixture';(fixture/'tools').mkdir(parents=True)
        preparer=fixture/'tools/prepare-menu-observer.py'
        preparer.write_text('#!/usr/bin/python3\nfrom pathlib import Path\nimport time\nPath(__file__).with_name("started").write_text("active")\ntime.sleep(30)\n');preparer.chmod(0o700)
        shell=build/'mnm-qt-shell';report['shell_sha256']=hashlib.sha256(shell.read_bytes()).hexdigest()
        result=run('full-shell-deadline',['xvfb-run','-a','gdb','--batch','-ex','set pagination off','-ex','set environment LD_PRELOAD '+str(out/'deadline.so'),'-ex','run','-ex','bt','--args',shell,'--repo',fixture,'--live-menus','--native-commands','--resident-world','--legacy-audio','--skip-movies','--no-native-music','--live-menu-test',out/'flow.json'],environment=dict(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2',MNM_LIVE_MENU_TEST_QT_JOURNEY='1',XDG_CONFIG_HOME=str(out/'config')),expected=1,timeout=45)
        # GDB's final bt fails after a clean exit: its own status is not the
        # inferior's status. Require both the fixture and explicit inferior exit.
        if 'exited with code 01' not in result.stdout or 'received signal' in result.stdout or not (fixture/'tools/started').exists():
            raise ValueError('Full shell did not execute the active-staging deadline without a signal')
        run('facade-build',[sys.executable,'tools/build-native-draw.py'])
        run('bridge-build',[sys.executable,'tools/build-render-bridge.py','--selftest'])
        spec=importlib.util.spec_from_file_location('native_draw_builder',ROOT/'tools/build-native-draw.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        run('refusal-build',[*builder.FLAGS,'-MD','-MF',out/'probe.d','-c','tests/native-copy-refusal-probe.c','-o',out/'probe.obj'])
        closure=set()
        for word in (out/'probe.d').read_text().replace('\\\n',' ').split():
            path=Path(word).resolve()
            if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):closure.add(path.relative_to(ROOT).as_posix())
        if not closure<=set(sources):raise ValueError('Undeclared refusal compiler closure: '+repr(sorted(closure-set(sources))))
        definition=out/'load.def';definition.write_text('LIBRARY kernel32.dll\nEXPORTS\nLoadLibraryA@4\n')
        run('load-import',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'load.lib'])
        pe=ROOT/'working/build/render-selftest'
        run('refusal-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',pe/'kernel32.lib',pe/'kernelbase.lib',pe/'gdi32.lib',out/'load.lib'])
        shutil.copyfile(ROOT/'working/build/native-draw/MnmDraw.dll',out/'MnmDraw.dll')
        wine=dict(WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=')
        with prefix.with_name(prefix.name+'.native-player.lock').open('a') as lease:
            fcntl.flock(lease,fcntl.LOCK_EX|fcntl.LOCK_NB)
            try:run('refusal-execute',['wine',out/'probe.exe'],cwd=out,environment=wine,timeout=60)
            finally:
                stopped=run('wine-stop',['wineserver','-k'],environment=wine,expected=None,timeout=10)
                if stopped.returncode!=0 and not (stopped.returncode==1 and not stopped.stdout and not stopped.stderr):
                    raise ValueError('Wine stop failed with diagnostics or an unexpected status')
                report['wine_server_already_absent']=stopped.returncode==1
                run('wine-wait',['wineserver','-w'],environment=wine,timeout=10)
        rows=[list(v) for v in struct.iter_unpack('<44I',(out/'refusals.bin').read_bytes())]
        if len(rows)!=5 or any(r[13]!=0x88760096 or r[14] or r[15] or any(r[40:44]) for r in rows):raise ValueError('Refusal result/output interval mismatch')
        report.update(success=True,refused_calls=rows,all_source_target_words_unchanged=True,zero_native_facade_owners=True)
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        report['sources_stable']=stable();report['success'] &= report['sources_stable']
        raw=io.BytesIO()
        with tarfile.open(fileobj=raw,mode='w:gz',compresslevel=1) as archive:
            archive.add(out,arcname='controls')
            for name in ('tools/test-player-open-item-controls.py','tests/menu-session-destruction-test.cpp','tests/native-copy-refusal-probe.c','tests/qt-exit-during-staging.cpp'):
                archive.add(ROOT/name,arcname='sources/'+name)
        archive=a.report.with_name(a.report.stem+'-raw.tar.gz');write_report(ROOT,archive,raw.getvalue());report['raw_record']=str(archive)
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('success','error','sources_stable','shell_sha256')}),flush=True)
    return int(not report['success'])

if __name__=='__main__':raise SystemExit(main())
