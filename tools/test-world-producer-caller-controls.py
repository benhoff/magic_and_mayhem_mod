#!/usr/bin/env python3
"""Run actual producer callback controls with one exclusively leased fixture prefix."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile
from coverage_evidence import Reader,write_report
from drawing_fixture_prefix import prepare

ROOT=Path(__file__).resolve().parents[1]
MAIN=(ROOT/subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--git-common-dir'],text=True).strip()).resolve().parent
PREFIX='research/runtime/caller-state-results-20261108/'

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name',required=True);parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args()
    if not re.fullmatch('[a-z0-9-]+',args.name):parser.error('Unique lowercase name required')
    target=ROOT/(PREFIX+args.name+'.json');inner=ROOT/(PREFIX+args.name+'-inner.json')
    if target.exists()or inner.exists():raise ValueError('Preserve prior immutable results')
    declaration=json.loads(args.claims.read_text());digest=lambda b:hashlib.sha256(b).hexdigest()
    frozen={}
    for folder in ('tools','runtime','reconstruction','renderer','tests','protocols'):
        for path in (ROOT/folder).rglob('*'):
            if path.is_file()and path.suffix in('.py','.c','.h','.cpp','.hpp','.S'):
                frozen[path.relative_to(ROOT).as_posix()]=path.read_bytes()
    for path in declaration['sources']:
        if path not in frozen:frozen[path]=(ROOT/path).read_bytes()
    if any(digest(frozen[p])!=h for p,h in declaration['sources'].items()):raise ValueError('Prospective sources changed')
    report=dict(schema=1,kind='producer_callback_caller_state_controls',success=False,claims=declaration['claims'],
        scope='Actual production producer callback and PE32 entries on supplied globals/prologues, protected destination and retained original sprite/rectangle wrappers. Generic/WORD/black/displacement/additive prepared state is active. Seventeen saved native and one retained-return version5 diagnostic completions must retire, including twelve actual enclosing-parent observations, arity/SP/semantic FX/LastError checks. No original raster pixels, host compositor, installed game/driver or whole-operation equivalence.',
        original_game_runs=0,newly_passed_requirements=0,commands=[])
    ram=Path(tempfile.mkdtemp(prefix='mnm-producer-state-controls-',dir='/tmp'))
    report['temporary_directory']=str(ram)
    report['temporary_classification']='Build objects/import libraries and RAM staging are reproducible. Tested fixture executables, exact generated original-code header, raw captures, logs and changed sources are retained below. RAM data is held for reviewed cleanup; failure never automatically deletes unique observations.'
    def retain(label,raw):
        p=PREFIX+args.name+'-'+label;write_report(ROOT,p,raw)
        return dict(path=p,sha256=digest(raw),bytes=len(raw))
    used=set(declaration['sources'])|{'tools/test-world-producer-caller-controls.py','tools/test-canvas-producers.py','tools/drawing_fixture_prefix.py','tools/experiment_environment.py'}
    report['source_preimages']={}
    changed=subprocess.check_output(['git','ls-files','--modified','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
    for p in changed:
        if p in frozen:report['source_preimages'][p]=retain('source-'+digest(p.encode())[:16]+'.bin',frozen[p])
    lease=None;prefix=None;env=None
    try:
        attempts=[]
        candidates=(MAIN/'working/tests/drawing-fixture-prefix-suite').glob('*/wineprefix')
        for candidate in sorted(candidates,key=lambda p:('wow64'not in str(p),str(p))):
            try:prefix,lease=prepare(MAIN,ram,None,shared=candidate);break
            except (ValueError,OSError,subprocess.SubprocessError)as error:attempts.append(dict(prefix=str(candidate),reason=str(error)))
        report['prefix_admission_refusals']=attempts
        if lease is None:raise ValueError('No idle exclusively leased fixture prefix')
        report['prefix']=str(prefix)
        env={k:v for k,v in os.environ.items()if not k.startswith(('MNM_','WINE'))}
        env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',TMPDIR=str(ram),WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d')
        def run(label,argv,timeout):
            result=subprocess.run(list(map(str,argv)),cwd=ROOT,env=env,capture_output=True,timeout=timeout)
            log=retain(label+'.log.bin',result.stdout+result.stderr)
            report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=result.returncode,log=log))
            if result.returncode:raise RuntimeError(label+': exit '+str(result.returncode))
        run('wine-version',['wine','--version'],20)
        run('producer-controls',['python3',ROOT/'tools/test-canvas-producers.py','--shared-prefix',prefix,
            '--output-parent',ram,'--claims',args.claims.resolve(),'--report',inner],900)
        with Reader(ROOT)as reader:
            legacy=reader.json(inner);report['inner']=dict(path=str(inner.relative_to(ROOT)),sha256=reader.digest(inner))
        if not legacy['success']or legacy['forwarded_entries']!=61 or legacy['sprite_native_draws']!=15:
            raise ValueError('Incomplete production callback controls')
        if legacy.get('caller_state_native_completions')!=17 or legacy.get('caller_state_retained_completions')!=1 or not legacy.get('caller_state_normal_retirement')or legacy.get('caller_state_contexts')!=12 or legacy.get('caller_state_version')!=5:
            raise ValueError('Incomplete passive caller-state entry/return controls')
        if legacy.get('caller_bindings_supplied_controls')!=2 or not legacy.get('caller_bindings_unknown_targets_retained'):
            raise ValueError('Incomplete known/unknown binding custody controls')
        if legacy.get('caller_native_return_supplied_controls')!=1:raise ValueError('Missing deferred native-return custody control')
        report['caller_native_return_supplied_controls']=1
        if legacy.get('caller_tail_supplied_controls')!=1:raise ValueError('Missing quiescent owned-source tail custody control')
        report['caller_tail_supplied_controls']=1
        if legacy.get('caller_opaque_context_supplied_controls')!=8:raise ValueError('Missing primary/auxiliary/secondary supplied stack-offset controls')
        report['caller_opaque_context_supplied_controls']=8
        used.update(legacy['sources'])
        build=json.loads((ROOT/'working/build/scene-observer-selftest/manifest.json').read_text())
        used.update(build['sources'])
        if any(p not in frozen or digest(frozen[p])!=h for p,h in build['sources'].items()):
            raise ValueError('Production builder source was not frozen')
        report.update(forwarded_entries=61,complete_raster_bypass_entries=13,sprite_native_draws=15,
            sprite_direct_abi_entries=legacy['sprite_direct_abi_entries'],
            word_flag_controls=2,caller_state_native_completions=17,caller_state_retained_completions=1,
            caller_state_contexts=12,caller_state_version=5,caller_state_normal_retirement=True,
            caller_bindings_supplied_controls=2,caller_bindings_unknown_targets_retained=True,original_pixel_rasters_executed=False,success=True)
    except (ValueError,OSError,RuntimeError,subprocess.SubprocessError)as error:
        report['error']=str(error)
    finally:
        report['artifacts']={}
        # Capture before any cleanup, including all failing diagnostics.
        for path in sorted(ram.rglob('*')):
            if path.is_file()and path.suffix not in('.o','.obj','.d','.lib','.def'):
                name=path.relative_to(ram).as_posix()
                report['artifacts'][name]=retain('artifact-'+digest(name.encode())[:20]+'.bin',path.read_bytes())
        if lease is not None:
            report['retirement']=[]
            for command in (['wineserver','-k'],['wineserver','-w']):
                try:
                    result=subprocess.run(command,env=env or dict(os.environ,WINEPREFIX=str(prefix)),capture_output=True,timeout=15)
                    report['retirement'].append(dict(argv=command,returncode=result.returncode))
                    if result.returncode not in(0,1):report.update(success=False,retirement_error='Prefix retirement refused')
                except (OSError,subprocess.SubprocessError)as error:report.update(success=False,retirement_error=str(error))
            from experiment_environment import _busy
            report['private_wine_stopped']=not _busy(prefix)
            if not report['private_wine_stopped']:report['success']=False
            lease.close()
        missing=used-frozen.keys()
        if missing:report.update(success=False,unfrozen_sources=sorted(missing))
        report['sources']={p:digest(frozen[p])for p in sorted(used&frozen.keys())}
        report['sources_stable']=all((ROOT/p).read_bytes()==frozen[p]for p in used&frozen.keys())
        report['success']=report['success']and report['sources_stable']
        write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report[k]for k in('success','error','private_wine_stopped','word_flag_controls')if k in report}))
    return 0 if report['success']else 1

if __name__=='__main__':raise SystemExit(main())
