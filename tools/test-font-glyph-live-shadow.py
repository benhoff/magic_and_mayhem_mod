#!/usr/bin/env python3
"""Fresh bounded glyph-bearing live native producer shadow; original stays active."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--source-game',type=Path,default=ROOT/'working/game-nocd')
    args=parser.parse_args()
    claims=json.loads(args.claims.read_text())
    for name,digest in claims['sources'].items():
        if sha(ROOT/name)!=digest:
            raise ValueError('Prospective source changed: '+name)
    parent=ROOT/'working/tests/font-glyph-live-shadow'
    parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    print(out,flush=True)
    report={'success':False,'sources':claims['sources'],'claims':claims['claims'],
            'live_replacement':False,'original_work_bypassed':False,
            'original_pixels_used_as_native_inputs':False,
            'scope':'Four startup World returns with live native owned menu/loading/HUD history, positive tinted glyph inputs and external whole-canvas completion/GPU mirror comparisons. Original text state and drawing active; no byte-state hook or suppression.'}

    def run(label,command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=ROOT,stdout=log,
                           stderr=subprocess.STDOUT,check=True,timeout=240)

    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'compat/legacy/canvas-producers','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','mnm-canvas-producers-live','-j4'])
        run('capture',['xvfb-run','-a','-s','-screen 0 1280x1024x24','python3',ROOT/'tools/capture-scene-game.py',
            '--canvas-producers','--samples','4','--source-game',args.source_game,
            '--producer-live',build/'mnm-canvas-producers-live','--skip-window-screenshot','--claims',args.claims])
        lines=(out/'capture.log').read_text().splitlines()
        paths=[Path(line) for line in lines if line.endswith('/report.json') and Path(line).is_file()]
        if len(paths)!=1:
            raise ValueError('Expected one retained capture report')
        path=paths[0];capture=json.loads(path.read_text());producer=capture['canvas_producers']
        glyphs=producer['operations'].get('font',0)
        checks=capture['producer_comparison']
        if not capture['success'] or not capture['sources_stable'] or glyphs<=0 or not checks or producer['failures']:
            raise ValueError('Incomplete glyph-bearing live producer shadow')
        if capture['original_work_bypassed'] or capture['original_pixels_used_as_native_inputs']:
            raise ValueError('Expected original-active source-only shadow')
        native=capture['native_producers']
        if not all(c['mismatches']==0 for c in checks) or not all(c['gpu_equal'] for c in native['checkpoints']):
            raise ValueError('Live completion/GPU mirror differs')
        # Keep the closure visible; reject undeclared local compiler dependencies.
        import shlex
        dependencies=set()
        for dependency in build.rglob('*.o.d'):
            for name in shlex.split(dependency.read_text().replace('\\\n',' ')):
                p=Path(name).resolve() if Path(name).is_absolute() else Path(name)
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    dependencies.add(str(p.relative_to(ROOT)))
        if dependencies-set(report['sources']):
            raise ValueError('Unbound native compiler dependencies: '+repr(sorted(dependencies-set(report['sources']))))
        report.update(source_executable_sha256=capture['source_executable_sha256'],
                      capture_report=str(path.relative_to(ROOT)),capture_report_sha256=sha(path),
                      experiment=capture['experiment'],glyph_calls=glyphs,world_returns=4,
                      checkpoints=len(checks),pixels_compared=sum(c['pixels'] for c in checks),
                      mismatches=sum(c['mismatches'] for c in checks),gpu_mirror_equal=True,
                      native_binary_sha256=sha(build/'mnm-canvas-producers-live'),
                      sources={**capture['sources'],**report['sources']},
                      producer_counts=producer['operations'],inputs={str(path.relative_to(ROOT)):sha(path)})
        directory=Path(capture['experiment'])/'capture'
        for p in [directory/'canvas-producers.bin',directory/'canvas-producers.done']:
            report['inputs'][str(p.relative_to(ROOT))]=sha(p)
        report['success']=True
    except Exception as exc:
        report['error']=str(exc)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['inputs_stable']=all(sha(ROOT/n)==h for n,h in report.get('inputs',{}).items())
        report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
