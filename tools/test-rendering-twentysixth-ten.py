#!/usr/bin/env python3
"""Prospective SQLite evidence for ten owned native text services."""
import argparse,hashlib,json,os,re,subprocess,sys
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import read_bytes,write_report
ROOT=Path(__file__).resolve().parents[1]
IDS=['NP.text.'+n for n in ['font-resource','byte-advance','byte-placement','string-placement','rectangle-layout','height-layout','measurement','cpu-pixels','gpu-pixels','gpu-cache']]

def digest(p):return hashlib.sha256(read_bytes(p,root=ROOT)).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path);parser.add_argument('--report',type=Path);parser.add_argument('--reuse-builds',type=Path);args=parser.parse_args()
    if not args.output or not args.report:parser.error('Require output and report')
    out=args.output.resolve();destination=args.report.absolute()
    if out.exists() or destination.exists() or not out.is_relative_to(ROOT/'working/tests') or not destination.is_relative_to(ROOT/'research/runtime'):parser.error('Require new owned immutable paths')
    buildroot=args.reuse_builds.resolve() if args.reuse_builds else out
    if args.reuse_builds and (not buildroot.is_relative_to(ROOT/'working/tests') or not (buildroot/'.retain').is_file()):parser.error('Require retained owned build root')
    out.mkdir(parents=True);(out/'.retain').write_text('Retain N26 native text binaries, raw fixture logs and execution records.\n');(out/'tmp').mkdir()
    env=dict(os.environ,TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2')
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
        source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix(),scope='Ten owned native text resource/application/raster/cache components under partial actor/SFT parents. Native policy only: existing byte/layout suppliers, independent fixture pixels and software OpenGL delivery. No new original, tinted coverage, RGB555, installed/hardware/performance/full-scene/live claim.')
    def run(label,argv,timeout=1800):
        argv=[str(x) for x in argv];res=subprocess.run(argv,cwd=ROOT,env=env,capture_output=True,timeout=timeout);data=(res.stdout+b'\n'+res.stderr).decode(errors='replace');(out/(label+'.log')).write_text(data)
        report['commands'].append(dict(label=label,argv=argv,exit_code=res.returncode,output_sha256=hashlib.sha256(data.encode()).hexdigest(),output_tail=data[-4000:]));res.check_returncode();return res.stdout.decode()
    try:
        report['sanitizer_scope']='ASan and UBSan. External Qt/driver leak scanning disabled; decoded/GPU retirement balances checked explicitly.'
        for mode,flags in [('release',['-DCMAKE_BUILD_TYPE=Release']),('sanitized',['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=buildroot/(mode+'-build');print('Building '+mode,flush=True)
            run(mode+'-configure',['cmake','-S',ROOT/'apps/text-session','-B',build,*flags]);run(mode+'-build',['cmake','--build',build,'--target','text-session-test','sft-loader-test','-j2'])
            binary=build/'text-session-test'
            report['native'][mode]=json.loads(run(mode+'-native',[sys.executable,ROOT/'tools/run-surface-wrapper-control.py','--log-root',out/(mode+'-display'),'--',binary]))
            assert report['native'][mode]['success']
            run(mode+'-sft-loader',[build/'sprites/assets/sft-loader-test'])
            run(mode+'-ctest',['ctest','--test-dir',build,'--output-on-failure','-R','^native-text-session$'])
            report['native'][mode]['ctest_wiring_pass']=True
            report['native'][mode]['sft_regression_pass']=True
            report['native'][mode]['binary_sha256']=hashlib.sha256(binary.read_bytes()).hexdigest()
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
        report['owned_source_variants']={p:(ROOT/p).read_text() for p in paths if p in {'apps/text-session/text_session.cpp','apps/text-session/text_session.hpp','apps/text-session/CMakeLists.txt','renderer/fonts/font_pixels.cpp','renderer/fonts/font_pixels.hpp','tests/text-session-test.cpp','tools/test-rendering-twentysixth-ten.py'}}
    finally:
        report['sources_stable']=all(digest(ROOT/p)==h for p,h in sources.items());report['inputs_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inputs'].items());report['success']=bool(report['success'] and report['sources_stable'] and report['inputs_stable'])
        write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','sources_stable','inputs_stable']}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
