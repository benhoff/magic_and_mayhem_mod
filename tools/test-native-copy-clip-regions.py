#!/usr/bin/env python3
"""Replay retained copy-plan inputs with current provider observation off/on.

The old exact PE32 probe is a retained test harness, not current production
bridge execution. These controls compare current NativeDraw pixels/results and
plans with accepted bytes and with observation disabled. No original game.
"""
import argparse
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import time
from coverage_evidence import Reader,write_report
from native_prefix_template import borrow
from native_copy_clip_regions import decode,plan_regions
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-clipped-operations-results-20261107/'
CASES={'cpu':['clip-multiple','clip-empty','clip-late-source-failure','fill-clip-2','keyed-clip'],
       'gpu':['clip-single','clip-multiple','clip-late-source-failure','self-keyed-clipped','clip-256']}


def module(name):
    s=importlib.util.spec_from_file_location('clip_control_'+name,ROOT/'tools'/(name+'.py'))
    m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh control report required')
    out=ROOT/'working/clip-observer-controls'/Path(args.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
    result=dict(schema=1,kind='current_provider_retained_harness_clip_controls',success=False,original_game_runs=0,
                no_current_production_bridge_claim=True,scope=__doc__,cases=[],artifacts=[])
    names={'tools/test-native-copy-clip-regions.py','tools/native_copy_clip_regions.py',
           'tools/native_prefix_template.py','tools/experiment_environment.py','tools/run-native-owned.py',
           'tools/build-native-draw.py','runtime/native-draw/factory.c','runtime/native-draw/surfaces.h',
           'runtime/native-draw/gpu_copy_plan.h','runtime/native-draw/clip_region_census.h',
           'protocols/include/mnm/native_copy_clip_regions_v1.h'}
    lease=server=None;env={};verified=False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);verified=True
        ram=Path('/tmp/mnm-rendering-clipping-controls-20261107/validated-provider');require(not ram.exists(),'Fresh RAM provider build required')
        dll=module('build-native-draw').build(ram)
        manifest=json.loads((ram/'manifest.json').read_text());names.update(manifest['sources'])
        result['provider']=dict(sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),sources=manifest['sources'])
        result['sources']={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(names)}
        display=out/'display';display.mkdir()
        server,receipt,_=module('run-native-owned').start_display(display)
        env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
        env.update(DISPLAY=receipt['display'],XAUTHORITY=receipt['authority'],WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=',
                   XDG_CACHE_HOME=str(out/'cache'),TMPDIR='/tmp',LP_NUM_THREADS='4')
        deadline=time.monotonic()+120
        while True:
            try:lease,result['prefix_pool']=borrow(ROOT/'working/fixtures/prefix-seed',out/'wineprefix',env);break
            except ValueError as error:
                if 'Both environment slots are busy or held' not in str(error) or time.monotonic()>=deadline:raise
                time.sleep(2)
        env.update(WINEPREFIX=str(out/'wineprefix'),WINEARCH='win64')
        with Reader(ROOT) as reader:
            for mode,selected in CASES.items():
                parent='research/runtime/native-adapter-master-results-20261027/'+mode+'-controls05.json'
                archive=parent.removesuffix('.json')+'-raw.tar.gz';old=reader.json(parent)
                data=reader.read_bytes(archive)
                result.setdefault('retained_inputs',[]).append(dict(record=parent,sha256=reader.digest(parent),archive=archive,
                    archive_sha256=reader.digest(archive),probe_sha256=old['probe_sha256'],phase='Historical exact harness/input/output bytes; no current bridge/source-freshness claim.'))
                with tarfile.open(fileobj=io.BytesIO(data),mode='r:gz') as tar:
                    def member(name):
                        f=tar.extractfile('controls/'+name);require(f is not None,'Missing retained control member');return f.read()
                    probe=member('probe.exe');require(hashlib.sha256(probe).hexdigest()==old['probe_sha256'],'Changed retained probe')
                    for name in selected:
                        expected=member(name+'/output.bin');input_bytes=member(name+'/input.bin');outputs=[];snapshots=[]
                        for enabled in (False,True):
                            case=out/(mode+'-'+name+('-on' if enabled else '-off'));case.mkdir()
                            (case/'input.bin').write_bytes(input_bytes);(case/'probe.exe').write_bytes(probe);shutil.copyfile(dll,case/'MnmDraw.dll')
                            child_env=dict(env,MNM_NATIVE_COPY_CLIP_REGIONS='1' if enabled else '0')
                            with (case/'execution.log').open('xb') as log:
                                code=subprocess.run(['wine',case/'probe.exe'],cwd=case,env=child_env,stdout=log,stderr=subprocess.STDOUT,timeout=45).returncode
                            require(code==0,'Retained '+mode+' '+name+' provider control failed: '+str(code))
                            actual=(case/'output.bin').read_bytes();require(actual==expected,'Current provider result/plan/full WORD plane differs from accepted bytes')
                            outputs.append(hashlib.sha256(actual).hexdigest())
                            clip_file=case/'native-copy-clip-regions-v1.bin'
                            if enabled:
                                _,rows=decode(clip_file.read_bytes());require(rows,'No provider clip observations')
                                for row in rows:
                                    if row['phase'] in (1,2):
                                        pieces,status=plan_regions(row)
                                        require(len(pieces)==row['plan_pieces'] and status==row['result'],'Independent supplied clip-plan mismatch')
                                snapshots.append(dict(records=len(rows),phases=sorted({r['phase'] for r in rows}),
                                    region_counts=sorted({len(r['regions']) for r in rows}),sha256=hashlib.sha256(clip_file.read_bytes()).hexdigest()))
                            else:require(not clip_file.exists(),'Disabled observation created a file')
                        result['cases'].append(dict(mode=mode,name=name,success=True,observation_off_on_equal=outputs[0]==outputs[1],
                            accepted_output_sha256=hashlib.sha256(expected).hexdigest(),observations=snapshots))
        require(len(result['cases'])==10,'Missing supplied control')
        result.update(success=True,processes=20)
    except Exception as error:result['error']=str(error)
    finally:
        if lease:
            subprocess.run(['wineserver','-k'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10)
            subprocess.run(['wineserver','-w'],env=env,check=True,timeout=10)
            lease.close();result['prefix_pool']['returned']=True
        if server and server.poll() is None:server.terminate();server.wait(timeout=10)
        if verified:
            subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);result['original_manifest_verified_before_after']=True
        result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in result.get('sources',{}).items())
        result['success'] &= result['sources_stable']
        for f in sorted(out.rglob('*')):
            if not f.is_file() or f.is_symlink() or 'wineprefix' in f.relative_to(out).parts or f.name=='Xauthority':continue
            rel=f.relative_to(out).as_posix();body=f.read_bytes();name=args.report.removesuffix('.json')+'-raw-'+hashlib.sha256(rel.encode()).hexdigest()[:20]+'.bin'
            result['artifacts'].append(dict(path=rel,record=name,sha256=write_report(ROOT,name,body),bytes=len(body)))
        h=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
        print(json.dumps(dict(success=result['success'],cases=len(result['cases']),error=result.get('error'),sha256=h)),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
