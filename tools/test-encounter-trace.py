#!/usr/bin/env python3
"""Compare bounded selected original bodies on captured native encounter states."""
import argparse, hashlib, importlib.util, json, os, struct, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES={'combat':[(0x50c0d0,0x50c92f),(0x50e7a0,0x50eb24),(0x54e170,0x54e256),(0x464cb0,0x464e20),(0x464ec0,0x46508c),(0x5075f0,0x5076a9),(0x5076f0,0x507c20),(0x514860,0x514982),(0x514990,0x514c56),(0x51f5e0,0x51fa53)],'control':[(0x50bd60,0x50c0c3),(0x50c930,0x50ca27),(0x5189e0,0x518c49),(0x518c50,0x518f77)],'motion':[(0x5104b0,0x510e3a)]}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--trace',type=Path,required=True);p.add_argument('--producer',type=Path);p.add_argument('--claims',type=Path);a=p.parse_args();out=a.output.resolve()
    if out.exists():p.error('Refusing existing evidence output')
    claims=json.loads(a.claims.read_text()) if a.claims else {'sources':{},'claims':[]}
    if any(sha(ROOT/k)!=v for k,v in claims['sources'].items()):raise ValueError('Declared source changed')
    if a.claims and not a.producer:p.error('Bound trace proof requires its successful source-bound producer')
    out.mkdir(parents=True);r={'success':False,'claims':claims['claims'],'source_sha256':claims['sources'],'source_executable_sha256':HASH,'scope':'Selected unchanged original bodies replay actual captured native encounter states: ordinary main melee/ANI/RNG/health with declared stun-admission/health/World endpoints, target/home selectors with supplied canonical admission, footprint reach/direction with canonical line and supplied endpoint height, and forward within-segment fine motion with declared real ANI event stream and empty lower audio. Native published health/recovery timing and nonterminal fine position projections checked. Whole original frame, segment completion, complete decisions/stun/death/World/campaign/live equivalence excluded.','wine_required':False,'live_replacement':False,'commands':[]}
    def run(name,command,env=None,timeout=600):
        command=[str(x) for x in command];r['commands'].append(command);x=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,env=env,timeout=timeout);(out/(name+'.stdout')).write_text(x.stdout);(out/(name+'.stderr')).write_text(x.stderr)
        if x.returncode or 'ERROR: AddressSanitizer' in x.stderr or 'runtime error:' in x.stderr:raise RuntimeError(name+' failed: '+x.stdout[-2000:]+x.stderr[-3000:])
        return x.stdout
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);exe=ROOT/'original/Arcane_Nocd/Chaos.exe';raw=exe.read_bytes()
        if sha(exe)!=HASH:raise ValueError('Unsupported immutable original')
        r['trace_sha256']=sha(a.trace);r['trace_bytes']=a.trace.stat().st_size
        if a.producer:
            producer=json.loads(a.producer.read_text());assert producer['success'] and producer['sources_unchanged'] and producer['build_inputs_unchanged'] and producer['trace_sha256']==r['trace_sha256']
            if a.claims and (not producer['claims'] or any(sha(ROOT/k)!=v for k,v in producer['source_sha256'].items())):raise ValueError('Trace producer is unbound or stale')
            r['producer_record']={'path':str(a.producer.resolve()),'sha256':sha(a.producer),'source_bound':bool(producer['claims'])}
        pe=struct.unpack_from('<I',raw,60)[0];sec=pe+24+struct.unpack_from('<H',raw,pe+20)[0]
        def body(va,n):
            for i in range(struct.unpack_from('<H',raw,pe+6)[0]):
                rva,size,offset=struct.unpack_from('<3I',raw,sec+40*i+12)
                if 0x400000+rva<=va and va+n<=0x400000+rva+size:return raw[offset+va-0x400000-rva:offset+va-0x400000-rva+n]
            raise ValueError('Unbacked original body')
        r['original_target_sha256']={suite:{hex(x):hashlib.sha256(body(x,y-x)).hexdigest() for x,y in ranges} for suite,ranges in RANGES.items()}
        spec=importlib.util.spec_from_file_location('encounter_native_runner',ROOT/'tools/test-installed-encounter.py');helper=importlib.util.module_from_spec(spec);spec.loader.exec_module(helper)
        suites=['combat','control','motion'];targets=['encounter-'+s+'-trace-reference' for s in suites];extra=[t+'.cpp' for t in targets];results=[];initial=None
        for tag,options in [('native',[]),('original',['-DCMAKE_CXX_FLAGS=-m32 -O1 -g','-DCMAKE_EXE_LINKER_FLAGS=-m32 -no-pie','-DMNM_ENCOUNTER_ORIGINAL_REFERENCE=ON']),('sanitized',['-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=out/tag;run(tag+'-configure',['cmake','-S',ROOT/'apps/native-encounter','-B',build,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',*options]);deps=helper.dependencies(build,run,tag,extra);deps.update({'tools/test-encounter-trace.py','tools/test-installed-encounter.py'})
            if initial is None:initial={k:sha(ROOT/k) for k in sorted(deps)};r['build_inputs_sha256']=dict(initial)
            else:
                for k in sorted(deps-initial.keys()):initial[k]=sha(ROOT/k);r['build_inputs_sha256'][k]=initial[k]
            if a.claims and deps-claims['sources'].keys():raise ValueError('Unbound compiler/configure inputs: '+str(sorted(deps-claims['sources'].keys())))
            run(tag+'-build',['cmake','--build',build,'--target',*targets,'-j2']);row={}
            for suite,target in zip(suites,targets):
                args=[build/target]+([exe] if tag=='original' else [])+[a.trace];row[suite]=json.loads(run(tag+'-'+suite,args,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'}))
            if any(sha(ROOT/k)!=v for k,v in initial.items()):raise ValueError('Build source changed')
            results.append(row)
        if not results[0]==results[1]==results[2]:raise ValueError('Native/original/sanitized trace differs')
        if sha(a.trace)!=r['trace_sha256'] or sha(exe)!=HASH:raise ValueError('Trace/original input changed')
        r.update(success=True,results=results[0],native_passed=True,original_passed=True,sanitizer_passed=True,original_target_text_unchanged=True,input_unchanged=True)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);r['original_manifest_verified_before_after']=True;r['sources_unchanged']=all(sha(ROOT/k)==v for k,v in claims['sources'].items());r['build_inputs_unchanged']=all(sha(ROOT/k)==v for k,v in r.get('build_inputs_sha256',{}).items());(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
    if not r['sources_unchanged']:raise ValueError('Declared source changed')
    print(json.dumps({k:r[k] for k in ['success','results','sanitizer_passed']}))
if __name__=='__main__':main()
