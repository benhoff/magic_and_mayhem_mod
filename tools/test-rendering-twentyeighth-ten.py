#!/usr/bin/env python3
"""Prospective SQLite evidence for ten native sprite transformations."""
import argparse,hashlib,json,os,re,subprocess,sys
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import read_bytes,write_report
ROOT=Path(__file__).resolve().parents[1]
IDS=['NP.sprite-transform.crop', 'NP.sprite-transform.trim', 'NP.sprite-transform.mirror-x', 'NP.sprite-transform.mirror-y', 'NP.sprite-transform.rotate-cw', 'NP.sprite-transform.rotate-half', 'NP.sprite-transform.rotate-ccw', 'NP.sprite-transform.nearest-resize', 'NP.sprite-transform.silhouette', 'NP.sprite-transform.outer-outline']

def digest(p):return hashlib.sha256(read_bytes(p,root=ROOT)).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path);parser.add_argument('--report',type=Path);parser.add_argument('--reuse-builds',type=Path);parser.add_argument('--build-root',type=Path);args=parser.parse_args()
    if not args.output or not args.report:parser.error('Require output and report')
    out=args.output.resolve();destination=args.report.absolute()
    if out.exists() or destination.exists() or not out.is_relative_to(ROOT/'working/tests') or not destination.is_relative_to(ROOT/'research/runtime'):parser.error('Require new owned immutable paths')
    buildroot=(args.reuse_builds or args.build_root or out).resolve()
    if args.build_root:
        if buildroot.exists() or not buildroot.is_relative_to(Path('/tmp')):parser.error('Require new task-owned /tmp build root')
        if subprocess.check_output(['findmnt','-n','-o','FSTYPE','-T',str(buildroot.parent)],text=True).strip()!='tmpfs':parser.error('Build root must use tmpfs')
        buildroot.mkdir();(buildroot/'.retain').write_text('Temporary N27 build output; raw logs and test binaries are retained durably by the runner.\n')
    if args.reuse_builds and (not buildroot.is_relative_to(ROOT/'working/tests') or not (buildroot/'.retain').is_file()):parser.error('Require retained owned build root')
    out.mkdir(parents=True);(out/'.retain').write_text('Retain N28 native transform binaries, raw fixture logs and execution records.\n');(buildroot/'tmp').mkdir(exist_ok=True)
    env=dict(os.environ,TMPDIR=str(buildroot/'tmp'),ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2')
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes());bs={b['id']:b for b in r['behaviors']};builds={b['id']:b for b in r['builds']}
    paths=set().union(*(required_sources(bs[i])|set(bs[i]['documents']) for i in IDS));candidates={}
    # Pin the native asset/renderer build input domain before execution. Compiler
    # dependency files below must be a subset; this includes linked utility code.
    for base in ['assets','renderer']:
        paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/base).rglob('*') if p.is_file() and (p.suffix in {'.hpp','.h','.inc','.cpp','.c','.S'} or p.name=='CMakeLists.txt'))
    for name in subprocess.check_output(['rg','--files'],cwd=ROOT,text=True).splitlines():
        p=ROOT/name
        if p.suffix in {'.hpp','.h','.inc','.cpp','.c','.S'}:candidates.setdefault(p.name,[]).append(p)
    pending=list(paths);visited=set()
    while pending:
        name=pending.pop()
        if name in visited:continue
        visited.add(name);p=ROOT/name
        if p.suffix not in {'.hpp','.h','.inc','.cpp','.c','.S'}:continue
        for include in re.findall(r'^\s*#\s*include\s+"([^"]+)"',p.read_text(),re.M):
            direct=(p.parent/include).resolve()
            for t in [direct] if direct.is_file() else candidates.get(Path(include).name,[]):
                if t.is_relative_to(ROOT) and not t.is_relative_to(ROOT/'working'):
                    rel=t.relative_to(ROOT).as_posix()
                    if rel not in paths:paths.add(rel);pending.append(rel)
    sources={p:digest(ROOT/p) for p in sorted(paths)}
    report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],builds)) for i in IDS],commands=[],native={},inputs={},original_game_executed=False,original_artifacts_consumed=False,live_replacement=False,
        source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix(),scope='Ten owned native crop/trim/mirror/rotation/resize/silhouette/outline policies. Forward-mapped independent geometry and CPU/GPU delivery checks; no original pivot/filter/encoded-outline, widget/world, hardware/performance or live claim.')
    def run(label,argv,timeout=1800):
        argv=[str(x) for x in argv];res=subprocess.run(argv,cwd=ROOT,env=env,capture_output=True,timeout=timeout);data=(res.stdout+b'\n'+res.stderr).decode(errors='replace');(out/(label+'.log')).write_text(data)
        report['commands'].append(dict(label=label,argv=argv,exit_code=res.returncode,output_sha256=hashlib.sha256(data.encode()).hexdigest(),output_tail=data[-4000:]));res.check_returncode();return res.stdout.decode()
    try:
        report['sanitizer_scope']='ASan and UBSan. External Qt/driver leak scanning disabled; decoded/GPU retirement balances checked explicitly.'
        for mode,flags in [('release',['-DCMAKE_BUILD_TYPE=Release']),('sanitized',['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=buildroot/(mode+'-build');print('Building '+mode,flush=True)
            run(mode+'-configure',['cmake','-S',ROOT/'renderer/sprite-transforms','-B',build,*flags]);run(mode+'-build',['cmake','--build',build,'--target','sprite-transform-test','sprite-session-test','-j2'])
            binary=build/'sprite-transform-test'
            report['native'][mode]=json.loads(run(mode+'-native',[sys.executable,ROOT/'tools/run-surface-wrapper-control.py','--log-root',out/(mode+'-display'),'--',binary]))
            assert report['native'][mode]['success']
            run(mode+'-session',[sys.executable,ROOT/'tools/run-surface-wrapper-control.py','--log-root',out/(mode+'-session-display'),'--',build/'session/sprite-session-test'])
            report['native'][mode]['session_regression_pass']=True
            run(mode+'-ctest',['ctest','--test-dir',build,'--output-on-failure','-R','^native-(sprite-transforms|sprite-session)$'])
            report['native'][mode]['ctest_wiring_pass']=True
            binary_data=binary.read_bytes();(out/(mode+'-sprite-transform-test')).write_bytes(binary_data)
            report['native'][mode]['binary_sha256']=hashlib.sha256(binary_data).hexdigest()
            deps=set()
            for p in build.rglob('*.o.d'):
                for token in p.read_text().replace('\\\n',' ').split()[1:]:
                    t=Path(token).resolve()
                    if t.is_file() and t.is_relative_to(ROOT) and not t.is_relative_to(ROOT/'working'):deps.add(t.relative_to(ROOT).as_posix())
            if deps-set(sources):raise ValueError('Undeclared compiler inputs: '+str(sorted(deps-set(sources))))
            report['native'][mode]['declared_compiler_inputs']=sorted(deps)
        report['success']=True
    except Exception as error:
        report['error']=repr(error)
        report['owned_source_variants']={p:(ROOT/p).read_text() for p in paths if p in {'renderer/sprite-transforms/sprite_transforms.cpp','renderer/sprite-transforms/sprite_transforms.hpp','renderer/sprite-transforms/CMakeLists.txt','tests/sprite-transform-test.cpp','tools/test-rendering-twentyeighth-ten.py'}}
    finally:
        report['sources_stable']=all(digest(ROOT/p)==h for p,h in sources.items());report['inputs_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inputs'].items());report['success']=bool(report['success'] and report['sources_stable'] and report['inputs_stable'])
        write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','sources_stable','inputs_stable']}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
