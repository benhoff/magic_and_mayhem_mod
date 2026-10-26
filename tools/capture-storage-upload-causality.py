#!/usr/bin/env python3
"""Capture original storage uploads and compare source/native/destination/output."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys

from coverage_evidence import evidence_digest, read_bytes, read_json, write_report
from drawing_recorded_session import backend_environment
from storage_upload_causality import admit_namespace, analyze_original

ROOT=Path(__file__).resolve().parents[1]
FLAGS=['campaign-storage','session-producers','engine-cadence','semantic-state',
       'canvas-storage-transitions','wine-submission','wine-resource-map','wine-resource-copy',
       'wine-texture-entry','wine-blit-worker','wine-blit-packet','wine-worker-context',
       'wine-source-descriptor','wine-source-map','wine-source-map-bounded','wine-source-version',
       'wine-gpu-completion','source-byte-witness','egl-physical','egl-swap-completion',
       'egl-source-gpu-witness','egl-destination-gpu-witness','egl-xcb-image-witness']


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('output','report','claims','source-game','prefix-template','owned-prefix'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--profile',choices=['plain','baseline','corrected'],required=True)
    p.add_argument('--isolation-record',type=Path,required=True)
    p.add_argument('--session-close-diagnostics',action='store_true')
    p.add_argument('--storage-initial-worlds',type=int,choices=range(3,17),default=3)
    a=p.parse_args()
    a.report=a.report.absolute()
    out=a.output.absolute()
    if out.exists() or a.report.exists() or not out.is_relative_to(ROOT/'working'):
        p.error('Require fresh owned execution/report paths')
    declaration=json.loads(a.claims.read_bytes());sources=declaration['sources']
    stable=lambda:all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items())
    if not stable():raise ValueError('Prospective source changed')
    out.mkdir(parents=True);(out/'.retain').write_text('Retain original storage diagnosis, native comparisons and failures.\n')
    result=dict(schema=1,success=False,profile=a.profile,sources=sources,claims=declaration['claims'],
        commands=[],original_execution=False,original_pixels_used_as_native_inputs=False,
        full_renderer_accepted=False,historical_reference_changed=False,
        scope='New source-bound original-active storage session with separate public-SHM diagnostic. '
              'Compare native source-only replay to original GL source/default/physical output; '
              'bind actual upload bytes externally. The historical238-frame reference is unchanged. '
              'Queries/readonly attachments perturb timing; no all-writer or private allocator proof.')
    env=backend_environment()
    env={k:v for k,v in env.items() if not k.startswith(('MNM_','WINE'))}
    env.pop('LD_PRELOAD',None)
    env.update(TMPDIR=str(ROOT/'working/tmp'),PYTHONDONTWRITEBYTECODE='1',LP_NUM_THREADS='4')
    def run(label,command,allowed=(0,),timeout=900):
        r=subprocess.run(list(map(str,command)),cwd=ROOT,env=env,capture_output=True,timeout=timeout)
        data=r.stdout+b'\n'+r.stderr;(out/(label+'.log')).write_bytes(data)
        result['commands'].append(dict(label=label,argv=list(map(str,command)),returncode=r.returncode,
                                      diagnostic=data.decode(errors='replace')))
        print(label,r.returncode,flush=True)
        if r.returncode not in allowed:raise ValueError(label+' exit '+str(r.returncode))
        return r
    verified=False
    try:
        result['isolation']=admit_namespace(ROOT,a.isolation_record)
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
        build_script='tools/build-drawing-x11-delivery.py' if a.profile=='plain' else 'tools/build-storage-upload-observer.py'
        spec=importlib.util.spec_from_file_location('upload_build',ROOT/build_script)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        if a.profile=='plain':
            library=module.build(egl=True)
            manifest=json.loads((library.parent/'manifest.json').read_bytes())
        else:library,helper,manifest=module.build()
        result['observer_build']=manifest
        if set(manifest.get('compiled_sources',[]))-sources.keys():
            raise ValueError('Observer compiler dependencies absent from prospective declaration')
        build=ROOT/'working/build/storage-upload-drawing'
        run('configure',['cmake','-S',ROOT/'apps/drawing-session','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','mnm-drawing-observe','mnm-drawing-session-replay',
                     'mnm-drawing-present','-j2'])
        child=a.report.with_name(a.report.stem+'-original.json')
        command=[sys.executable,'-B',ROOT/'tools/capture-drawing-interactive.py',
            '--source-game',a.source_game.resolve(),'--prefix-template',a.prefix_template.resolve(),
            '--owned-prefix',a.owned_prefix.resolve(),'--experiment-parent',out/'original-runs',
            '--observer-mode','physical','--physical-peer',build/'mnm-drawing-observe',
            '--producer-samples','32','--physical-oracle-mib','1024','--physical-transient-mib','512',
            '--x11-delivery-library',library,'--claims',a.claims.resolve(),'--report',child,
            *['--'+flag for flag in FLAGS]]
        command+=['--storage-initial-worlds',str(a.storage_initial_worlds)]
        if a.session_close_diagnostics:command.append('--session-close-diagnostics')
        if a.profile=='corrected':command.append('--egl-correct-stale-upload')
        result['original_capture_invoked']=True
        captured=run('original-capture',['xvfb-run','-a','-s','-screen 0 1280x1024x24',*command],allowed=(0,1),timeout=1200)
        original=read_json(child,root=ROOT)
        directory=Path(original['experiment'])/'capture'
        result['original_execution']=any(directory.glob('egl-physical-request-*.bin'))
        result['original_session_accepted']=original.get('success') is True and original.get('normal_original_quit_verified') is True
        result['original']=dict(record=child.relative_to(ROOT).as_posix(),sha256=evidence_digest(child,root=ROOT))
        if captured.returncode:
            raise ValueError('Original collector refused: '+str(original.get('error')))
        if not all(original.get(k) is True for k in ('success','normal_original_quit_verified',
            'original_sections_unchanged','original_drawing_active','original_manifest_verified_before_after','sources_stable')):
            raise ValueError('Original acquisition incomplete')
        result['causality']=analyze_original(original,directory,require_samples=a.profile!='plain')
        # Explicit bounded physical interchange for the unchanged legacy comparator.
        interchange=out/'original-interchange.json';interchange.write_bytes(read_bytes(child,root=ROOT))
        native=out/'native-interchange.json'
        run('native-comparison',[sys.executable,'-B',ROOT/'tools/compare-drawing-quick-battle.py',
            '--original',interchange,'--native',build/'composition/mnm-drawing-session-replay',
            '--presenter',build/'mnm-drawing-present','--claims',a.claims.resolve(),
            '--output-parent',out/'native-runs','--report',native],allowed=(0,1),timeout=1200)
        comparison=json.loads(native.read_bytes())
        durable=a.report.with_name(a.report.stem+'-native.json')
        write_report(ROOT,durable,native.read_bytes())
        result['native']=dict(record=durable.relative_to(ROOT).as_posix(),sha256=evidence_digest(durable,root=ROOT),
            success=comparison.get('success'),frames=comparison.get('compared_presentations'),
            gpu_mismatching_pixels=comparison.get('mismatching_pixels'),physical_mismatching_pixels=comparison.get('physical_mismatching_pixels'))
        count=result['causality']['frames']
        if (comparison.get('native_exit_code')!=0 or comparison.get('refusals') or
                comparison.get('all_observed_presentations_compared') is not True or
                comparison.get('original_gpu_source_comparisons')!=count or
                comparison.get('original_destination_comparisons')!=count or
                comparison.get('source_destination_comparisons')!=count or
                comparison.get('original_gpu_source_success') is not True or
                comparison.get('native_destination_mismatching_words')!=0 or
                comparison.get('source_destination_mismatching_words')!=0):
            raise ValueError('Native/source/nominal destination cohort disagrees or is incomplete')
        if a.profile=='plain':
            failures=set(result['causality']['physical_mismatch_ordinals'])
            if not failures or failures-set(result['causality']['shrink_ordinals']) or comparison['success'] is not False:
                raise ValueError('Plain observer did not reproduce the first-shrink presentation discrepancy')
            result['classification']='original_presentation_control_without_shm_diagnostic'
        elif a.profile=='baseline':
            if (not result['causality']['every_physical_difference_explained'] or comparison['success'] is not False or
                    {r['ordinal'] for r in comparison['comparisons'] if r['mismatching_pixels']}!=
                    set(result['causality']['explained_ordinals'])):
                raise ValueError('Fresh baseline did not establish the claimed upload boundary')
            result['classification']='observed_original_upload_source_window'
        elif not comparison['success'] or result['causality']['physical_mismatch_ordinals']:
            raise ValueError('Corrected control does not match complete native/original output')
        else:
            result['classification']='explicit_corrected_upload_control'
        result['success']=True
    except Exception as error:
        result['error']=repr(error)
    finally:
        if result.get('success'):
            try:admit_namespace(ROOT,a.isolation_record)
            except Exception as error:result.update(success=False,error=repr(error))
        if verified:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);result['original_manifest_verified_before_after']=True
            except Exception as error:result.update(success=False,manifest_error=repr(error))
        result['sources_stable']=stable();result['success'] &= result['sources_stable']
        result['artifacts']=out.relative_to(ROOT).as_posix()
        write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ('success','profile','classification','error')}))
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
