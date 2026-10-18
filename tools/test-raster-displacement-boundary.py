#!/usr/bin/env python3
"""Compare strict recovered and finite native displacement; never extend ownership."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--frame-fixture',type=Path,required=True)
    parser.add_argument('--frame-sha256',required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.report.exists():parser.error('Refusing evidence overwrite')
    frame=args.frame_fixture.resolve()
    if not frame.is_relative_to(ROOT/'working') or sha(frame)!=args.frame_sha256:
        parser.error('Unadmitted captured source frame')
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):parser.error('Prospective source changed')
    parent=ROOT/'working/tests/raster-displacement-boundary';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],
                original_sha256=BUILD,commands=[],original_game_executed=False,
                frame_fixture=dict(path=str(frame.relative_to(ROOT)),sha256=sha(frame)),
                scope='Captured61x47 source and original57dbd0 sine initialization, actual original5968a4 versus production splice over16 phases/two formats/DF0/1 and physical-edge/interior controls. Every owned tap matches original output; unowned taps deliberately retain their destination under native finite-canvas policy. Strict recovered admission still refuses atomically. No original outside-allocation equivalence, natural caller or GPU/session pixel claim.')

    def run(label,command):
        command=[str(x) for x in command]
        p=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=600)
        log=out/(label+'.log');log.write_text(p.stdout+p.stderr)
        report['commands'].append(dict(command=command,exit_code=p.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
        p.check_returncode();return p.stdout

    verified=False
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
        image=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(image)!=BUILD:raise ValueError('Unsupported original')
        baseline=Path(run('baseline',['python3','-B',ROOT/'tools/test-raster-splice.py','--claims',args.claims.resolve()]).strip().splitlines()[-1]).resolve()
        if not baseline.is_relative_to(ROOT/'working/tests/raster-splice'):raise ValueError('Unexpected baseline output')
        prior=json.loads(baseline.read_text())
        if not (prior['success'] and prior['sources_stable'] and prior['inputs_stable']):raise ValueError('Baseline raster comparison failed')
        report['baseline']=dict(path=str(baseline.relative_to(ROOT)),sha256=sha(baseline),comparison=prior['comparison'])
        objects=[baseline.parent/(name+'.o') for name in ('raster_pixels.c','pixel_kernel.c','raster_entry.S','raster_bridge.c','raster-splice-state-probe.S')]
        run('boundary-build',['clang++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-fdeclspec',
                             '-Wno-ignored-attributes','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',
                             ROOT/'tests/raster-displacement-boundary-reference.cpp',*objects,'-o',out/'reference'])
        closure=set(prior['compiled_project_sources'])
        for token in shlex.split((out/'reference.d').read_text().replace('\\\n',' ').split(':',1)[1]):
            p=Path(token).resolve()
            if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):closure.add(str(p.relative_to(ROOT)))
        if closure-declaration['sources'].keys():raise ValueError('Undeclared compiled source: '+repr(sorted(closure-declaration['sources'].keys())))
        report['compiled_sources']=sorted(closure)
        run('original-boundary',[out/'reference',image,frame,out/'comparison.json'])
        report['comparison']=json.loads((out/'comparison.json').read_text());report['success']=report['comparison']['success']
    except Exception as error:report['error']=repr(error)
    finally:
        try:
            run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_verified_before_after']=verified
        except Exception as error:report.update(success=False,manifest_error=repr(error))
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in declaration['sources'].items())
        report['frame_stable']=sha(frame)==args.frame_sha256
        report['success'] &= report['sources_stable'] and report['frame_stable']
        (out/'.retain').write_text('Captured displacement edge original/native comparison; preserve report and source frame.\n')
        args.report.parent.mkdir(parents=True,exist_ok=True)
        with args.report.open('x') as stream:json.dump(report,stream,indent=2);stream.write('\n')
        print(args.report,flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
