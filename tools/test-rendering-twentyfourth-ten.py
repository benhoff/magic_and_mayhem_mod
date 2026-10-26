#!/usr/bin/env python3
"""Prospective, SQLite-backed controls for ten bounded native surface components."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import read_bytes, write_report

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS=['RE.surface-wrapper.'+name for name in
     ['opaque-guarded','opaque-recovering','keyed-recovering','keyed-rectangle',
      'keyed-explicit','partial-fill','full-fill']]+[
     'NP.surface-bitmap.owned-cursor','NP.surface-bitmap.indexed-recovery','NP.vertical-bar.owned-surface']


def digest(path):
    path=Path(path).resolve()
    data=read_bytes(path,root=ROOT) if path.is_relative_to(ROOT) else path.read_bytes()
    return hashlib.sha256(data).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--main-root',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--reuse-builds',type=Path)
    args=parser.parse_args()
    out=args.output.resolve(); report_path=args.report.absolute()
    if out.exists() or report_path.exists() or not out.is_relative_to(ROOT/'working/tests') or not report_path.is_relative_to(ROOT/'research/runtime'):
        parser.error('Require new owned execution and immutable research paths')
    build_root=args.reuse_builds.resolve() if args.reuse_builds else out
    if args.reuse_builds and (not build_root.is_relative_to(ROOT/'working/tests') or not (build_root/'.retain').is_file()):
        parser.error('Reuse only a retained build root in this owned worktree')
    out.mkdir(parents=True); (out/'.retain').write_text('Retain N24 raw fixtures, logs, negative results and current input binaries.\n')
    (out/'tmp').mkdir()
    env=dict(os.environ,TMPDIR=str(out/'tmp'),QT_QPA_PLATFORM='xcb',QT_FORCE_STDERR_LOGGING='1',QT_ASSUME_STDERR_HAS_CONSOLE='1',QT_LOGGING_RULES='*.debug=false',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2',
             ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    bs={b['id']:b for b in register['behaviors']}; builds={b['id']:b for b in register['builds']}
    paths=set().union(*(required_sources(bs[bid]) | set(bs[bid]['documents']) for bid in IDS))
    candidates={}
    for name in subprocess.check_output(['rg','--files'],cwd=ROOT,text=True).splitlines():
        p=ROOT/name
        if p.suffix in {'.hpp','.h','.inc','.cpp','.c','.S'}:candidates.setdefault(p.name,[]).append(p)
    pending=list(paths); visited=set()
    while pending:
        name=pending.pop()
        if name in visited:continue
        visited.add(name); source=ROOT/name
        if source.suffix not in {'.hpp','.h','.inc','.cpp','.c','.S'}:continue
        for include in re.findall(r'^\s*#\s*include\s+"([^"]+)"',source.read_text(),re.M):
            direct=(source.parent/include).resolve()
            for target in [direct] if direct.is_file() else candidates.get(Path(include).name,[]):
                if target.is_relative_to(ROOT) and not target.is_relative_to(ROOT/'working'):
                    relative=target.relative_to(ROOT).as_posix()
                    if relative not in paths:paths.add(relative);pending.append(relative)
    sources={p:digest(ROOT/p) for p in sorted(paths)}
    claims=[dict(behavior=bid,contract_sha256=behavior_contract(bs[bid],builds)) for bid in IDS]
    report=dict(schema=1,success=False,sources=sources,claims=claims,original_claims=claims[:5],
                commands=[],native={},original={},original_sha256=BUILD,
                original_game_executed=False,live_replacement=False,artifacts=out.relative_to(ROOT).as_posix(),
                source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                scope='Ten bounded component implementations under five partial surface parents. Native Release and ASan/UBSan controls plus fresh unchanged original copy-argument geometry and bar-supplier comparisons. No real driver, ABI pointer identity, original copy failure/retry, whole-session, hardware, performance or live replacement claim. Fill, resource/recovery and bar allocation composition do not receive original comparison credit.')
    original=args.main_root.resolve()/'original/Arcane_Nocd/Chaos.exe'
    report['inputs']={}
    def run(label,argv,timeout=1200):
        argv=[str(v) for v in argv]
        result=subprocess.run(argv,cwd=ROOT,env=env,capture_output=True,timeout=timeout)
        output=(result.stdout+b'\n'+result.stderr).decode(errors='replace')
        (out/(label+'.log')).write_text(output)
        report['commands'].append(dict(label=label,argv=argv,exit_code=result.returncode,
            output_sha256=hashlib.sha256(output.encode()).hexdigest(),output_tail=output[-5000:]))
        result.check_returncode()
        return result.stdout.decode()
    def verify(label):run(label,[args.main_root/'tools/original-manifest.sh','verify'])
    def strip_identity(trace):return {k:v for k,v in trace.items() if k not in {'source_identity','destination_identity'}}
    try:
        verify('original-before')
        report['inputs'][str(original)]=digest(original)
        if report['inputs'][str(original)]!=BUILD:raise ValueError('Unsupported No-CD image')
        report['sanitizer_environment']={k:env[k] for k in ['ASAN_OPTIONS','UBSAN_OPTIONS']}
        report['sanitizer_scope']='Address/undefined behavior checks; external Qt/driver leak scanning is disabled. Owned surface/palette/DC counts are checked explicitly.'
        report['toolchain']=dict(cmake=run('cmake-version',['cmake','--version']).strip(),
                                compiler=run('compiler-version',['g++','--version']).strip())
        spec=importlib.util.spec_from_file_location('n24_rectangle_inputs',ROOT/'tools/compare-surface-rectangles.py')
        rectangles=importlib.util.module_from_spec(spec);spec.loader.exec_module(rectangles)
        cases=rectangles.cases()
        inputs=out/'copy-inputs.txt'
        inputs.write_text(''.join(' '.join(str(v) for v in [c['id'],c['entry'],c['fullscreen'],c['no_wait'],
            c['window'],c['missing'],*c['source'],c['x'],c['y'],*c['destination']])+'\n' for c in cases))
        report['inputs'][str(inputs)]=digest(inputs)
        for mode,flags in [('release',['-DCMAKE_BUILD_TYPE=Release']),('sanitized',[
            '-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie',
            '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=build_root/(mode+'-build')
            print('Building '+mode,flush=True)
            run(mode+'-configure',['cmake','-S',ROOT/'renderer','-B',build,*flags])
            run(mode+'-build',['cmake','--build',build,'--target','surface-wrapper-plan-test','owned-surface-wrappers-test','-j2'],1800)
            traces=json.loads(run(mode+'-plans',[build/'surface-wrapper-plan-test',inputs]))
            (out/(mode+'-plans.interchange.json')).write_text(json.dumps(traces,separators=(',',':'))+'\n')
            native=json.loads(run(mode+'-native',[sys.executable,ROOT/'tools/run-surface-wrapper-control.py','--log-root',out/(mode+'-displays'),'--',build/'owned-surface-wrappers-test']))
            if not native['success']:raise ValueError('Native surface controls failed')
            report['native'][mode]=native
            report['native'][mode]['binary_sha256']=digest(build/'owned-surface-wrappers-test')
            report['native'][mode]['plan_binary_sha256']=digest(build/'surface-wrapper-plan-test')
            dependencies=set()
            for dep in build.rglob('*.o.d'):
                text=dep.read_text().replace('\\\n',' ')
                for token in text.split()[1:]:
                    target=Path(token).resolve()
                    if target.is_absolute() and target.is_file() and target.is_relative_to(ROOT) and not target.is_relative_to(ROOT/'working'):
                        dependencies.add(target.relative_to(ROOT).as_posix())
            if dependencies-set(sources):raise ValueError('Undeclared compiler inputs: '+str(sorted(dependencies-set(sources))))
            report['native'][mode]['declared_compiler_inputs']=sorted(dependencies)
        print('Comparing unchanged original copy wrappers',flush=True)
        binary=out/'original-copy-reference'
        run('original-copy-build',['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',
                                  ROOT/'tests/surface-rectangle-reference.cpp','-o',binary])
        original_traces=json.loads(run('original-copy-cases',[binary,original,inputs]))
        if len(original_traces)!=len(cases):raise ValueError('Original fixture omitted cases')
        (out/'original-plans.interchange.json').write_text(json.dumps(original_traces,separators=(',',':'))+'\n')
        for mode in ['release','sanitized']:
            traces=json.loads((out/(mode+'-plans.interchange.json')).read_text())
            if len(traces)!=len(cases) or any(a!=strip_identity(b) for a,b in zip(traces,original_traces)):
                raise ValueError('Native '+mode+' plan differs from unchanged original geometry')
        report['original']['copy']=dict(cases=len(cases),entries=sorted({c['entry'] for c in cases}),
            exact_geometry_flags_mutation=True,binary_sha256=digest(binary),
            inputs_sha256=digest(inputs),original_outputs_sha256=digest(out/'original-plans.interchange.json'),
            identity_fields_excluded=['source_identity','destination_identity'])
        # Compare the unchanged original bar supplier; native surface construction
        # remains a separately validated policy and never consumes original pixels.
        image=original.read_bytes();pe=struct.unpack_from('<I',image,60)[0]
        table=pe+24+struct.unpack_from('<H',image,pe+20)[0];code=None
        for k in range(struct.unpack_from('<H',image,pe+6)[0]):
            rva,size,raw=struct.unpack_from('<3I',image,table+k*40+12)
            if rva<=0x18f890 and 0x18f9b9<=rva+size:code=image[raw+0x18f890-rva:raw+0x18f9b9-rva];break
        if code is None or code[:8]!=bytes.fromhex('83ec08538bd95556') or code[-8:]!=bytes.fromhex('5f5e5d5b83c408c3'):
            raise ValueError('Original bar supplier signature differs')
        (out/'vertical-bar-original.inc').write_text('static const unsigned char original_bar[]={'+','.join(str(v) for v in code)+'};\n')
        run('original-bar-build',['gcc','-m32','-O2','-std=c11','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',
            '-I',out,ROOT/'tests/vertical-bar-reference-test.c','-o',out/'original-bar-reference'])
        report['original']['bar_supplier']=json.loads(run('original-bar-cases',[out/'original-bar-reference']))
        report['original']['bar_supplier']['code_sha256']=hashlib.sha256(code).hexdigest()
        report['original']['bar_supplier']['binary_sha256']=digest(out/'original-bar-reference')
        report['success']=True
    except Exception as error:
        report['error']=repr(error)
        # Preserve only this batch's unique source versions and base revision,
        # rather than a redundant full checkout snapshot.
        report['owned_source_variants']={p:(ROOT/p).read_text() for p in paths
            if p.startswith(('reconstruction/rendering/surface_wrappers','reconstruction/rendering/owned_surface_wrappers',
                             'tests/surface-wrapper-plan-test','tests/owned-surface-wrappers-test',
                             'tools/test-rendering-twentyfourth-ten')) or p=='renderer/CMakeLists.txt'}
    finally:
        try:
            verify('original-after');report['original_manifest_verified_before_after']=True
        except Exception as error:
            report['original_manifest_verified_before_after']=False;report['manifest_error']=repr(error)
        report['sources_stable']=all(digest(ROOT/p)==h for p,h in sources.items())
        report['inputs_stable']=all(digest(p)==h for p,h in report['inputs'].items())
        report['success']=bool(report['success'] and report['sources_stable'] and report['inputs_stable']
                               and report['original_manifest_verified_before_after'])
        write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps({k:report.get(k) for k in ['success','error','sources_stable','inputs_stable']}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
