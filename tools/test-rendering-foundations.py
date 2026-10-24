#!/usr/bin/env python3
"""Compare rendering helpers and investigate alternate-canvas publication."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess

from alternate_canvas_lifecycle import inspect
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import evidence_digest, read_bytes, read_json, write_report
from original_x86 import PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RS.minimap-camera-dispatch', 'RS.raster-displacement-table-initializer',
       'RS.alternate-canvas-initializer', 'RS.alternate-canvas-provenance']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', required=True)
    parser.add_argument('--frontier', type=Path, required=True,
                        help='Retained original recursive frontier; restore archived inputs first')
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require new output under this worktree working/tests')
    if (ROOT/args.report).exists(): parser.error('Refusing immutable report overwrite')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Rendering helpers and alternate publication original evidence.\n')
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1', UBSAN_OPTIONS='halt_on_error=1')
    register = read_json(ROOT/'research/runtime/coverage/register.json', root=ROOT)
    behaviors = {b['id']:b for b in register['behaviors']}
    builds = {b['id']:b for b in register['builds']}
    paths = set().union(*(required_sources(behaviors[i]) for i in IDS))
    paths.update(['tools/test-rendering-foundations.py','tools/alternate_canvas_lifecycle.py',
                  'tools/coverage_evidence.py','tools/coverage_claims.py','tools/original_x86.py'])
    sources = {p:evidence_digest(ROOT/p,root=ROOT) for p in sorted(paths)}
    claims = [dict(behavior=i,contract_sha256=behavior_contract(behaviors[i],builds)) for i in IDS]
    report = dict(schema=1,success=False,sources=sources,claims=claims,commands=[],
        original_sha256=BUILD,original_pixels_used_as_native_inputs=False,
        live_replacement=False,original_game_executed=False,
        scope='Exact minimap selector/common marker call ordering; full16-entry displacement initialization and selected x87 state; bounded full graphics initializer with two hardware publication watches; conservative static lifecycle frontier. No live installation, pixel/session acceptance or recovered alternate allocator.')

    def run(label, command):
        command = list(map(str,command));print(label,flush=True)
        log = out/(label+'.log')
        with log.open('x') as stream:
            p = subprocess.run(command,cwd=ROOT,env=env,stdout=stream,stderr=subprocess.STDOUT,timeout=180)
        text = log.read_text()
        report['commands'].append(dict(label=label,argv=command,exit=p.returncode,
            log=log.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(log.read_bytes()).hexdigest(),output=text))
        if p.returncode: raise ValueError(label+': '+text[-4000:])
        return text

    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        image_path=ROOT/'original/Arcane_Nocd/Chaos.exe';image=PE32(read_bytes(image_path))
        if image.sha256 != BUILD: raise ValueError('Original image hash changed')
        old=read_json(ROOT/'research/runtime/alternate-canvas-results-20261020/provenance.json',root=ROOT)
        raw=read_bytes(args.frontier)
        if hashlib.sha256(raw).hexdigest()!=old['prior_frontier']['sha256']:
            raise ValueError('Original frontier identity differs')
        frontier=json.loads(raw)
        if frontier['original_sha256']!=BUILD or not frontier['success']:
            raise ValueError('Unadmitted frontier')
        report['lifecycle']=inspect(image,frontier)
        report['retained_frontier']=dict(path=str(args.frontier.resolve()),sha256=hashlib.sha256(raw).hexdigest())
        flags=['-m32','-mstackrealign','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror']
        programs=[('reference','tests/rendering-foundations-reference.cpp',['-O2']),
                  ('sanitized','tests/rendering-foundations-reference.cpp',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
                  ('publication','tests/alternate-canvas-publication-watch.cpp',['-O2'])]
        compiled=set()
        for name, source, extra in programs:
            dep=out/(name+'.d');binary=out/name
            run(name+'-build',['g++',*flags,*extra,'-MMD','-MF',dep,ROOT/source,'-o',binary])
            for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(token).resolve()
                if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    compiled.add(path.relative_to(ROOT).as_posix())
            if compiled-set(sources):raise ValueError('Undeclared compiled inputs: '+repr(sorted(compiled-set(sources))))
            text=run(name,[binary,image_path]);values=[json.loads(line) for line in text.splitlines() if line.startswith('{')]
            report[name]=values[0] if len(values)==1 else values
        if report['reference'] != report['sanitized']:raise ValueError('Sanitized comparison differs')
        run('native64-build',['g++','-std=c++17','-O1','-Wall','-Wextra','-Werror',
            '-fsanitize=address,undefined','-fno-omit-frame-pointer','-MMD','-MF',out/'native64.d',
            ROOT/'tests/rendering-foundations-native.cpp','-o',out/'native64'])
        for token in shlex.split((out/'native64.d').read_text().replace('\\\n',' ').split(':',1)[1]):
            path=Path(token).resolve()
            if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                compiled.add(path.relative_to(ROOT).as_posix())
        if compiled-set(sources):raise ValueError('Undeclared native64 inputs')
        report['native64']=json.loads(run('native64',[out/'native64']))
        report['compiled_sources']=sorted(compiled)
        report['binaries']={name:dict(path=(out/name).relative_to(ROOT).as_posix(),sha256=hashlib.sha256((out/name).read_bytes()).hexdigest()) for name in [*(p[0] for p in programs),'native64']}
        report['sanitizer_passed']=True
        report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as error:
        report['error']=str(error)
    finally:
        try:
            run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after']=any(c['label']=='manifest-before' and c['exit']==0 for c in report['commands'])
        except (OSError,ValueError,subprocess.SubprocessError) as error:
            report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(evidence_digest(ROOT/p,root=ROOT)==h for p,h in sources.items())
        report['success'] &= report['sources_stable']
        sha=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],record=args.report,sha256=sha,error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
