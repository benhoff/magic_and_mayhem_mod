#!/usr/bin/env python3
"""Replay saved source-only startup/HUD producers and compare every CPU/GPU canvas."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--capture-report',type=Path,required=True);args=parser.parse_args()
    claims=json.loads(args.claims.read_text());sources={p:sha(ROOT/p) for p in claims['sources']}
    if sources!=claims['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/other-producer-checkpoints';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=claims['claims'],sources=sources,experiment=str(out.relative_to(ROOT)),original_pixels_used_as_native_inputs=False,original_oracles_read_by_native=False,original_work_bypassed=False,live_replacement=False,
        scope='Fresh assertion-enabled native source-only replay of a separately pinned saved original startup/HUD prefix; every native CPU canvas and GPU mirror compared with its independent original checkpoint. BMP/JPEG/PCX/copy/panel/fade/HUD/minimap/marker and retained font requests included as recorded input. No fresh live capture, glyph backend takeover, unknown unsampled writer attribution or complete rendering replacement.')
    inputs={};commands=[]
    def pin(path):inputs[str(path.resolve().relative_to(ROOT))]=sha(path)
    def run(label,argv,environment=None):
        log=out/(label+'.log');r=subprocess.run([str(a) for a in argv],capture_output=True,text=True,timeout=300,env=environment);log.write_text(r.stdout+r.stderr);commands.append(dict(label=label,argv=[str(a) for a in argv],returncode=r.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)));r.check_returncode()
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        capture=json.loads(args.capture_report.read_text());pin(args.capture_report)
        if not capture['success'] or capture['source_executable_sha256']!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':raise ValueError('Unsupported/failed original capture')
        root=Path(capture['experiment']);directory=root/'capture';inspector=module('saved_producer_inspector','tools/inspect-canvas-producers.py');analysis=inspector.analyze(directory)
        if analysis!=capture['canvas_producers'] or analysis['failures']:raise ValueError('Saved original capture identity changed')
        stream=directory/'canvas-producers.bin';marker=directory/'canvas-producers.done';pin(stream);pin(marker)
        _,records=inspector.decode(stream.read_bytes());images=module('saved_source_paths','tools/test-native-canvas-producers.py')
        for r,payload in records:
            if r[2] in (7,20):pin(images.resolve(root/'game',payload[:-1].decode('ascii')))
        nativeInputs=out/'native-inputs';nativeInputs.mkdir();shutil.copy2(stream,nativeInputs/stream.name);shutil.copy2(marker,nativeInputs/marker.name)
        # The native process receives only request bytes and encoded asset root;
        # original checkpoint files stay outside its input directory.
        build=ROOT/'working/build/other-producer-checkpoints'
        run('configure',['cmake','-S',ROOT/'compat/legacy/canvas-producers','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','mnm-canvas-producers-live','canvas-sequence-test','minimap-producer-test','other-writer-atomic-test','-j4'])
        closure={'tools/test-other-producer-checkpoints.py','tools/original-manifest.sh','tools/inspect-canvas-producers.py','tools/test-native-canvas-producers.py'}
        for dep in build.rglob('*.o.d'):
            for p in dep.read_text().replace('\\\n',' ').split(':',1)[1].split():
                if p.startswith(str(ROOT)+'/') and not p.startswith(str(ROOT/'working')+'/'):closure.add(str(Path(p).resolve().relative_to(ROOT)))
        if closure-sources.keys():raise ValueError('Unbound native compiler dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources']=sorted(closure)
        run('native-unit-tests',['ctest','--test-dir',build,'-R','^native-(canvas-sequence|minimap-producer|other-writer-atomic)$','--output-on-failure'])
        run('atomic-writer-details',[build/'other-writer-atomic-test'])
        report['atomic_writers']=json.loads((out/'atomic-writer-details.log').read_text())
        if not report['atomic_writers']['success'] or report['atomic_writers']['atomic_refusals']!=19:raise ValueError('Missing atomic writer refusal cases')
        environment={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'}
        executable=build/'mnm-canvas-producers-live';report['native_binary_sha256']=sha(executable)
        run('native-cpu-gpu',[executable,nativeInputs/stream.name,root/'game',out/'native'],environment)
        native=json.loads((out/'native/live-report.json').read_text());byId={r['oracle']:r for r in native['checkpoints']};rows=[]
        if not native['success'] or native['original_oracles_read'] or native['original_pixels_used_as_native_inputs'] or native['remaining_surfaces']:raise ValueError('Native completion/ownership failed')
        for original in analysis['checkpoints']:
            checkpoint=byId.pop(original['oracle'],None)
            if not checkpoint or not checkpoint['gpu_equal']:raise ValueError('Missing CPU/GPU checkpoint')
            path=directory/original['path'];pin(path);expected=path.read_bytes();actual=(out/'native'/checkpoint['path']).read_bytes()
            if len(expected)!=original['width']*original['height']*2 or expected!=actual:raise ValueError('Independent checkpoint mismatch: '+str(original['oracle']))
            rows.append(dict(oracle=original['oracle'],canvas=original['canvas'],width=original['width'],height=original['height'],sha256=sha(path),gpu_equal=True))
        if byId:raise ValueError('Unexpected native checkpoints')
        report.update(success=True,checkpoints=len(rows),pixels_compared=sum(r['width']*r['height'] for r in rows),mismatches=0,gpu_mirror_equal=True,queues=native['queues'],presentations=native['presentations'],producer_counts=capture['canvas_producers'].get('counts',{}),completed=rows,native_report_sha256=sha(out/'native/live-report.json'))
    except Exception as error:report['error']=repr(error)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error:report.update(success=False,manifest_error=repr(error))
        report.update(commands=commands,inputs=inputs,inputs_stable=all(sha(ROOT/p)==h for p,h in inputs.items()),sources_stable=all(sha(ROOT/p)==h for p,h in sources.items()));report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        report['artifacts']={str(p.relative_to(ROOT)):dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable']}))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
