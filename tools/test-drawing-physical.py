#!/usr/bin/env python3
"""Compare guarded original X11 witnesses with separately owned native X11 output."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from drawing_corpus import ROOT,BUILD,load_tool
from drawing_presented_session import sha
from drawing_publication import analyze as publications
from drawing_physical import analyze as physicals
from drawing_scene_corpus import raster_inputs


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--capture-report',type=Path,required=True)
    p.add_argument('--composition',type=Path,required=True)
    p.add_argument('--presenter',type=Path,required=True)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing report overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):p.error('Prospective source changed')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],draw02_complete=False,
        original_pixels_used_as_native_inputs=False,original_oracles_read_by_native=False,
        all_global_presentation_routes_observed=False,complete_presented_journey=False,live_replacement=False)
    try:
        capture_path=a.capture_report.resolve();capture=json.loads(capture_path.read_text())
        if capture.get('success') is not True or capture.get('sources_stable') is not True or capture.get('original_manifest_verified_before_after') is not True or capture.get('source_executable_sha256')!=BUILD or capture.get('live_replacement') is not False:
            raise ValueError('Invalid independent original capture')
        root=Path(capture['experiment']);directory=root/'capture'
        pub=publications(directory);original=physicals(directory,root/'physical-report.json',pub)
        if pub!=capture['drawing_publication'] or original!=capture['drawing_physical']:raise ValueError('Original witness artifacts changed')
        inspector=load_tool('inspect-canvas-producers');raw=(directory/'canvas-producers.bin').read_bytes()
        _,records=inspector.decode(raw,complete=False);frames=pub['frames'];records=records[:frames[-1]['source_sequence']]
        if [f['source_sequence'] for f in frames]!=sorted(set(f['source_sequence'] for f in frames)):raise ValueError('Reordered or repeated source boundaries')
        assets=load_tool('check-drawing-publication').source_assets(root/'game',records)
        references=[capture_path,root/'physical-report.json',*sorted(directory.glob('physical-*.*')),
            directory/'canvas-producers.bin',directory/'canvas-producers.done',directory/'drawing-publication-0000.bin',
            directory/'drawing-actions-0000.bin',*[directory/f['path'] for f in frames],*assets.values()]
        inputs={str(f.relative_to(ROOT)):sha(f.read_bytes()) for f in references if f.is_file()}
        parent=ROOT/'working/tests/drawing-physical';parent.mkdir(parents=True,exist_ok=True)
        run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));owned=run/'owned';output=run/'native'
        boundaries=[dict(sequence=f['source_sequence'],queues=sum(r[2]==12 for r,_ in records[:f['source_sequence']]),
            canvas=f['canvas'],width=f['width'],height=f['height']) for f in frames]
        prefix=raw[:64+sum(r[0] for r,_ in records)]
        # The observed raster return field is removed. No oracle planes or
        # expected action/effect outcomes enter either native child process.
        load_tool('test-drawing-presented-session').write_inputs(owned,
            [dict(stage='scene',producer='history',frames=[dict(frame_index=i,boundary=b) for i,b in enumerate(boundaries)])],
            dict(history=raster_inputs(prefix)),[(name,path.read_bytes()) for name,path in assets.items()],
            'fenced-original-startup',['rgba8']*len(frames),
            [dict(index=0,action='replay-owned-startup-history',before_presentation=17)])
        binaries={str(f.resolve()):sha(f.read_bytes()) for f in (a.composition,a.presenter)}
        guards=subprocess.run(['python3','-B',str(ROOT/'tests/test-drawing-physical.py')],capture_output=True,timeout=60)
        (run/'guards.log').write_bytes(guards.stdout+guards.stderr);guards.check_returncode()
        command=['xvfb-run','-a','-s','-screen 0 2048x2048x24','python3',str(ROOT/'tools/replay-drawing-presented-session.py'),
            str(owned),str(output),'--composition',str(a.composition.resolve()),'--presenter',str(a.presenter.resolve()),
            '--display-conversion','normalized-nearest']
        native=subprocess.run(command,env=dict(os.environ,QT_QPA_PLATFORM='xcb',QT_SCALE_FACTOR='1',LIBGL_ALWAYS_SOFTWARE='1'),capture_output=True,timeout=600)
        (run/'native.log').write_bytes(native.stdout+native.stderr);native.check_returncode()
        checks=[]
        for i,(f,witness) in enumerate(zip(frames,original['frames'])):
            expected=(directory/f'physical-oracle-{i+1:04}.rgba').read_bytes();actual=(output/f'frame-{i:06}.bin').read_bytes()
            if len(actual)!=len(expected):raise ValueError('Native physical plane is incomplete')
            mismatches=sum(actual[j:j+4]!=expected[j:j+4] for j in range(0,len(actual),4))
            checks.append(dict(ordinal=i+1,source_sequence=f['source_sequence'],queue=f['queue'],canvas=f['canvas'],
                pixels=witness['pixels'],mismatches=mismatches,original_sha256=sha(expected),native_sha256=sha(actual)))
        if any(sha((ROOT/n).read_bytes())!=h for n,h in {**sources,**inputs}.items()) or any(sha(Path(n).read_bytes())!=h for n,h in binaries.items()):
            raise ValueError('Sources, original witnesses or binaries changed during comparison')
        report.update(success=not any(f['mismatches'] for f in checks),sources_stable=True,inputs=inputs,inputs_stable=True,binaries=binaries,
            original_execution=True,original_draw_thread_fenced=True,physical_presentation=True,boundary='fenced_opaque_primary_point',
            frames=checks,pixels_compared=sum(f['pixels'] for f in checks),mismatches=sum(f['mismatches'] for f in checks),
            admission_guard_methods=int(re.search(rb'Ran ([0-9]+) tests',guards.stderr)[1]),
            capture_report=str(capture_path.relative_to(ROOT)),capture_report_sha256=sha(capture_path.read_bytes()),
            applied_menu_actions=pub['applied_actions'],artifacts=str(run.relative_to(ROOT)))
    except (ValueError,OSError,KeyError,TypeError,subprocess.SubprocessError) as error:
        report.update(error=str(error),sources_stable=False,inputs_stable=False)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    report['original_manifest_verified_before_after']=True
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({k:report[k] for k in ('success','pixels_compared','mismatches','error') if k in report}))
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
