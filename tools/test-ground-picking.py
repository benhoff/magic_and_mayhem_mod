#!/usr/bin/env python3
"""Compare recovered ground XYZ/masks against a pinned unchanged PE32 image."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/ray-z-recovery-results-20261109/'
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BIDS=('RE.sprite-point-mask','RE.world-ground-picking','RE.world-hit-classification')
SID='rendering.ray-z-isolated'
MODELS=['reconstruction/rendering/'+n+'.cpp'for n in ('ground_picking','terrain_traversal','terrain_submission','sprite_queue')]
REFERENCE=['tests/ground-picking-reference.cpp','tests/ground-picking-classify.S',*MODELS,'assets/sprite_loader.cpp']


def digest(path):
    with Path(path).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('executable','sprite','terrain','output'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith(PREFIX)or'..'in Path(a.report).parts or(ROOT/a.report).exists():
        p.error('A new immutable ray/Z report path is required')
    out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    (out/'.retain').write_text('Held raw ray/Z comparison inputs, exact executables, logs and semantic traces. Preserve until verified handoff.\n')
    budget=ROOT/'working/ray-z/attempts.sqlite3';budget.parent.mkdir(parents=True,exist_ok=True)
    with sqlite3.connect(budget)as db,db:
        db.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY, report TEXT UNIQUE, original_processes INTEGER NOT NULL DEFAULT 0, success INTEGER)')
        if db.execute('SELECT count(*) FROM attempts').fetchone()[0]>=4:raise ValueError('Four isolated comparison attempts exhausted; old attempts must not be reset')
        attempt=db.execute('INSERT INTO attempts(report)VALUES(?)',(a.report,)).lastrowid
    result=dict(schema=1,kind='original_ground_picking_recovery',success=False,attempt=attempt,
        original_game_runs=0,isolated_original_processes=0,native_processes=0,commands=[],
        original_manifest_verified_before_after=False,original_file_patched=False,
        native_original_code_or_data_access=False,
        bounds=dict(maximum_attempts=4,maximum_seconds_per_helper=180,maximum_trace_bytes=128*1024**2),
        scope='Owned valid-mask/ordinary-terrain four-view XYZ and exact bounded hit-class block only. No original game, live replacement, resource lifetime or whole Region callback claim.')
    artifacts=[];before=False;failure=None
    def run(label,command,**kwargs):
        command=list(map(str,command));proc=subprocess.run(command,cwd=ROOT,capture_output=True,timeout=180,**kwargs)
        for suffix,raw in (('stdout',proc.stdout),('stderr',proc.stderr)):
            path=out/(label+'.'+suffix);path.write_bytes(raw);artifacts.append(path)
        result['commands'].append(dict(label=label,argv=command,exit_code=proc.returncode))
        if proc.returncode:raise RuntimeError(label+' failed: '+proc.stderr.decode(errors='replace')[-2000:])
        if b'runtime error:'in proc.stderr or b'ERROR: AddressSanitizer'in proc.stderr:raise RuntimeError('Sanitizer diagnostic')
        return proc.stdout
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if digest(a.executable)!=BUILD:raise ValueError('Unreviewed original executable')
        with Reader(ROOT)as r:
            reg=r.json('research/runtime/coverage/register.json');behaviors=[next(b for b in reg['behaviors']if b['id']==bid)for bid in BIDS]
            scenario=next(s for s in reg['scenarios']if s['id']==SID)
            names=set().union(*(required_sources(b)for b in behaviors))
            result['sources']={n:r.digest(n)for n in sorted(names)}
            result['claims']=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(scenario)})for b in behaviors]
        result['source_revision']=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
        result['source_executable_sha256']=BUILD
        result['inputs']={str(n.resolve()):digest(n)for n in (a.executable,a.sprite,a.terrain)}
        result['source_preimages']={}
        for n,h in result['sources'].items():
            name=PREFIX+'source-'+h+'.bin';raw=(ROOT/n).read_bytes()
            if write_report(ROOT,name,raw)!=h:raise ValueError('Source changed while freezing')
            result['source_preimages'][n]=dict(record=name,sha256=h)
        common=['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie','-ffunction-sections','-Wl,--gc-sections','-Ireconstruction/rendering']
        deps=run('compiler-dependencies',[*common,'-m32','-MM',*[n for n in REFERENCE if n.endswith('.cpp')]])
        tokens=deps.decode().replace('\\\n',' ').split()
        compiled={(ROOT/n).resolve().relative_to(ROOT).as_posix() for n in tokens if not n.endswith(':')and(ROOT/n).is_file()}
        if compiled-result['sources'].keys():raise ValueError('Undeclared compiler inputs: '+str(sorted(compiled-result['sources'].keys())))
        result['compiled_dependencies']=sorted(compiled)
        helper=out/'ground-picking-reference';unit=out/'ground-picking-sanitized'
        run('compile-reference',[*common,'-m32',*REFERENCE,'-o',helper]);artifacts.append(helper)
        run('compile-sanitized',['g++','-std=c++17','-O1','-g','-Wall','-Wextra','-Werror','-pedantic','-fno-pie','-no-pie','-fsanitize=address,undefined','-fno-omit-frame-pointer','-Ireconstruction/rendering','tests/ground-picking-test.cpp',*MODELS,'-o',unit]);artifacts.append(unit)
        run('sanitized-unit',[unit],env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1'))
        result['sanitizers_passed']=True
        result['tool_versions']=dict(compiler=run('compiler-version',['g++','--version']).decode(),objdump=run('objdump-version',['objdump','--version']).decode(),python=sys.version)
        result['original_entry_anchors']=[]
        for entry,end in [(0x57dcf0,0x57dd58),(0x4f8ac9,0x4f8b6b),(0x4f8c3e,0x4f8c5e),(0x4fd160,0x4fd1c0),(0x4fcfa2,0x4fcfb4),(0x4fd315,0x4fd33c)]:
            raw=run('static-'+hex(entry),['objdump','-w','-d','-Mintel','--start-address='+hex(entry),'--stop-address='+hex(end),a.executable])
            result['original_entry_anchors'].append(dict(entry=entry,end=end,disassembly_sha256=hashlib.sha256(raw).hexdigest()))
        trace_original=out/'original.bin';trace_native=out/'native.bin'
        with sqlite3.connect(budget)as db,db:db.execute('UPDATE attempts SET original_processes=original_processes+1 WHERE id=?',(attempt,))
        result['isolated_original_processes']=1
        original=json.loads(run('unchanged-original',[helper,'original',a.executable,a.sprite,a.terrain,trace_original]));artifacts.append(trace_original)
        result['original']=original
        result['native_processes']=1
        native=json.loads(run('native-poisoned',[helper,'native',a.executable,a.sprite,a.terrain,trace_native]));artifacts.append(trace_native)
        result['native']=native
        for trace in (trace_original,trace_native):
            if trace.stat().st_size>128*1024**2:raise ValueError('Semantic trace budget exceeded')
        fields=('success','point_cases','point_hits','world_cases','ground_hits','ground_misses','native_domain_refusals','class_cases','physical_layer_annotation_cases','hits_with_suppressed_draw_roles','installed_frames','installed_definitions','orientations','z_hits')
        if any(original[n]!=native[n]for n in fields):raise ValueError('Original/native summary differs')
        if digest(trace_original)!=digest(trace_native):raise ValueError('Original/native semantic trace differs')
        if not(native['native_pe_unreadable']and original['success']and native['success']and original['point_cases']>10000 and original['world_cases']>22000 and original['class_cases']==1000 and all(n>=500 for n in original['orientations'])and original['physical_layer_annotation_cases']>0 and original['xyz_max_depth_disagree_cases']>0 and sum(n>0 for n in original['z_hits'])>=4):raise ValueError('Incomplete declared ray/Z domain')
        result['comparison']=dict(success=True,complete_semantic_trace_equal=True,trace_sha256=digest(trace_original),trace_bytes=trace_original.stat().st_size,point_results_compared=original['point_cases'],worlds_compared=original['world_cases'],hit_class_results_compared=original['class_cases'],native_original_image_protection='PROT_NONE for entire original image; native independent process',original_instruction_protection='Executable PE sections RX throughout original execution',classification_original_scope='Exact unchanged96-byte4fd160..4fd1c0 block copied to private RX allocation, followed only by test RET; no whole scene-builder or Region callback execution',static_only_anchors=['4fcfa2 ground sentinels','4fd315 -64*width depth seeds'])
        result['inputs_stable']=all(digest(n)==h for n,h in result['inputs'].items())
        with Reader(ROOT)as r:result['sources_stable']=all(r.digest(n)==h for n,h in result['sources'].items())
        if not(result['sources_stable']and result['inputs_stable']):raise ValueError('Input/source drift')
        result['success']=True
    except Exception as exc:
        failure=exc;result['error']=type(exc).__name__+': '+str(exc)
    finally:
        if before:
            try:
                run('original-after',[ROOT/'tools/original-manifest.sh','verify']);result['original_manifest_verified_before_after']=True
            except Exception as exc:result['success']=False;result['manifest_error']=str(exc);failure=failure or exc
        for path in sorted(out.iterdir()):
            if path.is_file()and path.name!='.retain'and path not in artifacts:artifacts.append(path)
        result['artifacts']=[]
        for path in artifacts:
            raw=path.read_bytes();name=PREFIX+Path(a.report).stem+'-'+path.name+'.bin'
            result['artifacts'].append(dict(path=str(path),record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw)))
        # Required actual inputs survive independently of a resettable game tree.
        for label,path in [('sprite',a.sprite),('terrain',a.terrain)]:
            raw=path.read_bytes();name=PREFIX+'input-'+hashlib.sha256(raw).hexdigest()+'.bin'
            result.setdefault('retained_inputs',[]).append(dict(kind=label,path=str(path),record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw)))
        digest_result=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
        with sqlite3.connect(budget)as db,db:db.execute('UPDATE attempts SET success=? WHERE id=?',(int(result['success']),attempt))
        print(json.dumps(dict(success=result['success'],record=a.report,sha256=digest_result,comparison=result.get('comparison'),error=result.get('error'))),flush=True)
    if failure:raise SystemExit(1)


if __name__=='__main__':main()
