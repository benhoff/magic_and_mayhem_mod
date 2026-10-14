#!/usr/bin/env python3
"""Source-bound installed-data native encounter, rollback and process resume."""
import argparse, hashlib, json, os, re, shlex, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dependencies(build,run,tag,extra=()):
    found={'tools/test-installed-encounter.py'}
    allowed={'native-encounter-test.cpp','native-command-session-test.cpp','native-command-behavior-session-test.cpp','native-command-action-session-test.cpp','native-command-action-bodies-session-test.cpp','native-creature-providers-test.cpp'}
    allowed.update(extra)
    for i,entry in enumerate(json.loads((build/'compile_commands.json').read_text())):
        source=Path(entry['file']).resolve()
        if source.is_relative_to(ROOT/'tests') and source.name not in allowed:continue
        command=shlex.split(entry['command']);clean=[];skip=False
        for token in command:
            if skip:skip=False;continue
            if token=='-o':skip=True;continue
            if token=='-c':continue
            clean.append(token)
        output=run(tag+'-dependencies-'+str(i),clean+['-MM'])
        for token in shlex.split(output.replace('\\\n',' ')):
            p=Path(token)
            if not p.is_absolute():p=Path(entry['directory'])/p
            if p.is_file() and p.resolve().is_relative_to(ROOT):found.add(p.resolve().relative_to(ROOT).as_posix())
    for makefile in build.rglob('CMakeFiles/Makefile.cmake'):
        for token in re.findall(r'"([^"]+)"',makefile.read_text()):
            p=Path(token)
            if p.is_file() and p.resolve().is_relative_to(ROOT) and not p.resolve().is_relative_to(ROOT/'working'):found.add(p.resolve().relative_to(ROOT).as_posix())
    return found

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--install',type=Path,required=True);p.add_argument('--claims',type=Path);a=p.parse_args();out=a.output.resolve()
    if out.exists():p.error('Refusing existing evidence output')
    claims=json.loads(a.claims.read_text()) if a.claims else {'sources':{},'claims':[]}
    if any(sha(ROOT/k)!=v for k,v in claims['sources'].items()):raise ValueError('Declared source changed')
    out.mkdir(parents=True);r={'success':False,'claims':claims['claims'],'source_sha256':claims['sources'],'scope':'Installed ordinary type17 two-actor native encounter on a sealed12x12 Celtic Plains MAP crop, actual encrypted CFG/HTH and ANI/TTD inputs, recovered movement/AI/action/melee/health/stun/death projections with explicit empty World/owner/region/inventory/effect boundaries. Fixed retained identities, headless transactional and portable continuation integration; composed whole original World/campaign/live equivalence excluded.','wine_required':False,'live_replacement':False,'commands':[]}
    def run(name,command,env=None,timeout=600):
        command=[str(x) for x in command];r['commands'].append(command);x=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,env=env,timeout=timeout);(out/(name+'.stdout')).write_text(x.stdout);(out/(name+'.stderr')).write_text(x.stderr)
        if x.returncode or 'ERROR: AddressSanitizer' in x.stderr or 'runtime error:' in x.stderr:raise RuntimeError(name+' failed: '+x.stdout[-2000:]+x.stderr[-3000:])
        return x.stdout
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
        paths=['CFG/Encrypted/creature.cfg','CFG/Encrypted/HTH.cfg','Creatures/crocodile.ani','Realms/Celtic/Plains/CPsec01.map','Realms/Celtic/Plains/Terrain.ttd'];r['installed_inputs']={k:sha(a.install/k) for k in paths}
        results=[];initial=None
        for tag,options in [('native',[]),('sanitized',['-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=out/tag;configure=['cmake','-S',ROOT/'apps/native-encounter','-B',build,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DMNM_NATIVE_PROVIDER_ANI='+str(a.install/'Creatures/crocodile.ani')]
            run(tag+'-configure',configure+options);deps=dependencies(build,run,tag)
            if initial is None:
                initial={k:sha(ROOT/k) for k in sorted(deps)};r['build_inputs_sha256']=initial
                if a.claims and deps-claims['sources'].keys():raise ValueError('Unbound compiler/configure inputs: '+str(sorted(deps-claims['sources'].keys())))
            elif deps-initial.keys():raise ValueError('Additional sanitizer dependencies: '+str(sorted(deps-initial.keys())))
            targets=['native-encounter','native-encounter-test','native-command-session-test','native-command-behavior-session-test','native-command-action-session-test','native-command-action-bodies-session-test','native-creature-providers-test']
            run(tag+'-build',['cmake','--build',build,'--target',*targets,'-j2'])
            if tag=='native':run('core-ctest',['ctest','--test-dir',build/'command-session','-R','^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$','--output-on-failure'])
            args=[build/'native-encounter-test',a.install]+([out/'trace.bin'] if tag=='native' else [])
            results.append(json.loads(run(tag+'-encounter',args,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
            if any(sha(ROOT/k)!=v for k,v in initial.items()):raise ValueError('Build source changed')
        if results[0]!=results[1]:raise ValueError('Native/sanitized encounter differs')
        exe=out/'native/native-encounter';linkage=run('runtime-linkage',['ldd',exe]);r['runtime_linkage']=linkage
        if any(x in linkage.lower() for x in ['wine','libqt']):raise ValueError('Unexpected runtime dependency')
        whole=out/'whole.snapshot';run('whole',[exe,'run',a.install,0,0,12,12,17,1024,whole])
        prior=out/'resume-0.snapshot';run('initial',[exe,'run',a.install,0,0,12,12,17,0,prior]);last=0;count=0
        for end in [1,8,24,64,128,256,512,600,768,1024]:
            path=out/('resume-'+str(end)+'.snapshot');run('resume-'+str(end),[exe,'resume',prior,end-last,path],env={**os.environ,'PATH':'/nonexistent'});prior=path;last=end;count+=1
        if prior.read_bytes()!=whole.read_bytes():raise ValueError('Fresh process resume diverged')
        if any(sha(a.install/k)!=v for k,v in r['installed_inputs'].items()):raise ValueError('Installed inputs changed')
        r.update(results[0]);r.update(native_passed=True,sanitizer_passed=True,core_ctest_passed=True,process_resumes=count,process_resume_identical=True,installed_inputs_unchanged=True,trace_sha256=sha(out/'trace.bin'),trace_bytes=(out/'trace.bin').stat().st_size,snapshot_sha256=sha(whole),compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0])
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);r['original_manifest_verified_before_after']=True
        r['sources_unchanged']=all(sha(ROOT/k)==v for k,v in claims['sources'].items());r['build_inputs_unchanged']=all(sha(ROOT/k)==v for k,v in r.get('build_inputs_sha256',{}).items());(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
    if not r['sources_unchanged']:raise ValueError('Declared source changed during execution')
    print(json.dumps({k:r[k] for k in ['success','restored_ticks','hits','stuns','knockbacks','deaths','process_resumes','sanitizer_passed']}))
if __name__=='__main__':main()
