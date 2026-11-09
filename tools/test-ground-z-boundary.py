#!/usr/bin/env python3
"""Acquire only missing Z31 original cases; reuse unchanged original matrix."""
import argparse,hashlib,json,os,sqlite3,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/ray-z-recovery-results-20261109/'
BID='RE.world-ground-picking-upper-layer';SID='rendering.ground-z-upper-boundary'
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
MODELS=['reconstruction/rendering/'+n+'.cpp'for n in('ground_picking','terrain_traversal','terrain_submission','sprite_queue')]
def sha(p):
    with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in('executable','sprite','terrain','output'):p.add_argument('--'+n,type=Path,required=True)
    p.add_argument('--retained-comparison',required=True);p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith(PREFIX)or'..'in Path(a.report).parts or(ROOT/a.report).exists():p.error('New immutable boundary report required')
    out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Ray/Z upper-boundary semantic traces and exact binaries; preserve.\n')
    budget=ROOT/'working/ray-z/attempts.sqlite3'
    with sqlite3.connect(budget)as db,db:
        if db.execute('SELECT count(*)FROM attempts').fetchone()[0]>=4:raise ValueError('Shared four-attempt isolated recovery cap exhausted')
        attempt=db.execute('INSERT INTO attempts(report)VALUES(?)',(a.report,)).lastrowid
    value=dict(schema=1,kind='original_ground_z_upper_boundary',success=False,attempt=attempt,original_game_runs=0,isolated_original_processes=0,new_native_processes=0,original_manifest_verified_before_after=False,commands=[],original_file_patched=False,scope='Only four fresh original upper-layer worlds. The unchanged original lower/mask/class matrix is reused through exact inputs, full source/binary hashes and fresh current native semantic output. No game, live replacement or whole Region callback claim.')
    artifacts=[];before=False;failure=None
    def run(label,args):
        args=list(map(str,args));q=subprocess.run(args,cwd=ROOT,capture_output=True,timeout=180)
        for suffix,raw in(('stdout',q.stdout),('stderr',q.stderr)):
            n=out/(label+'.'+suffix);n.write_bytes(raw);artifacts.append(n)
        value['commands'].append(dict(label=label,argv=args,exit_code=q.returncode))
        if q.returncode:raise RuntimeError(label+': '+q.stderr.decode(errors='replace')[-1600:])
        return q.stdout
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if sha(a.executable)!=BUILD:raise ValueError('Unsupported PE')
        with Reader(ROOT)as r:
            parent=r.json(a.retained_comparison)
            if not(parent['success']and parent['sources_stable']and parent['inputs_stable']and parent['comparison']['complete_semantic_trace_equal']and parent['original_manifest_verified_before_after']and parent['source_executable_sha256']==BUILD):raise ValueError('Incomplete retained original proof')
            if not all(r.digest(n)==h for n,h in parent['sources'].items()):raise ValueError('Matrix code changed; no preservation waiver allowed')
            if not all(sha(n)==parent['inputs'][str(n.resolve())]for n in(a.executable,a.sprite,a.terrain)):raise ValueError('Retained input changed')
            binary=next(v for v in parent['artifacts']if Path(v['path']).name=='ground-picking-reference')
            original_trace=next(v for v in parent['artifacts']if Path(v['path']).name=='original.bin')
            if r.digest(binary['record'])!=binary['sha256']or sha(binary['path'])!=binary['sha256']:raise ValueError('Retained exact native binary changed')
            retained=r.read_bytes(original_trace['record'])
            if hashlib.sha256(retained).hexdigest()!=parent['comparison']['trace_sha256']:raise ValueError('Retained original trace changed')
            reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors']if v['id']==BID);scenario=next(v for v in reg['scenarios']if v['id']==SID)
            sources={n:r.digest(n)for n in sorted(required_sources(b))};value['sources']=sources
            value['claims']=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(scenario)})]
            value['retained_original_matrix']=dict(record=a.retained_comparison,sha256=r.digest(a.retained_comparison),original_trace=original_trace,native_binary=binary,all_parent_sources_unchanged=True)
        value['source_executable_sha256']=BUILD;value['inputs']={str(n.resolve()):sha(n)for n in(a.executable,a.sprite,a.terrain)}
        value['source_preimages']={}
        for n,h in sources.items():
            record=PREFIX+'source-'+h+'.bin';assert write_report(ROOT,record,(ROOT/n).read_bytes())==h;value['source_preimages'][n]=dict(record=record,sha256=h)
        common=['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie','-Ireconstruction/rendering']
        dependencies=run('dependencies',[*common,'-MM','tests/ground-z-boundary-reference.cpp',*MODELS]).decode().replace('\\\n',' ').split()
        compiled={(ROOT/n).resolve().relative_to(ROOT).as_posix()for n in dependencies if not n.endswith(':')and(ROOT/n).is_file()}
        if compiled-sources.keys():raise ValueError('Undeclared compiler inputs: '+str(sorted(compiled-sources.keys())))
        value['compiled_dependencies']=sorted(compiled)
        helper=out/'ground-z-boundary-reference';run('compile',[*common,'tests/ground-z-boundary-reference.cpp',*MODELS,'-o',helper]);artifacts.append(helper)
        original=out/'upper-original.bin';native=out/'upper-native.bin';current=out/'current-native-matrix.bin'
        with sqlite3.connect(budget)as db,db:db.execute('UPDATE attempts SET original_processes=original_processes+1 WHERE id=?',(attempt,))
        value['isolated_original_processes']=1
        value['original']=json.loads(run('original-upper',[helper,'original',a.executable,original]));artifacts.append(original)
        value['new_native_processes']+=1
        value['native']=json.loads(run('native-upper',[helper,'native',a.executable,native]));artifacts.append(native)
        if not(value['original']['success']and value['native']['native_pe_unreadable']and value['original']['world_cases']==value['native']['world_cases']==4 and value['original']['ground_z']==value['native']['ground_z']==31 and sha(original)==sha(native)):raise ValueError('Upper-layer original/native difference')
        value['new_native_processes']+=1
        matrix=json.loads(run('current-native-matrix',[binary['path'],'native',a.executable,a.sprite,a.terrain,current]));artifacts.append(current)
        if current.read_bytes()!=retained or not matrix['native_pe_unreadable']:raise ValueError('Current native output differs from retained original matrix')
        fields=('point_cases','point_hits','world_cases','ground_hits','ground_misses','native_domain_refusals','class_cases','physical_layer_annotation_cases','hits_with_suppressed_draw_roles','installed_frames','installed_definitions','orientations','z_hits')
        if any(matrix[n]!=parent['original'][n]for n in fields):raise ValueError('Retained matrix summary mismatch')
        value['current_native_matrix']=matrix
        value['comparison']=dict(success=True,upper_semantic_trace_equal=True,upper_trace_sha256=sha(original),upper_trace_bytes=original.stat().st_size,new_original_worlds=4,retained_original_matrix_replayed=False,current_native_matrix_equal=True,point_results_compared=matrix['point_cases'],worlds_compared=matrix['world_cases']+4,hit_class_results_compared=matrix['class_cases'],current_native_matrix_sha256=sha(current),original_upper_view_z=[31,31,31,31])
        value['inputs_stable']=all(sha(n)==h for n,h in value['inputs'].items())
        with Reader(ROOT)as r:value['sources_stable']=all(r.digest(n)==h for n,h in sources.items())and all(r.digest(n)==h for n,h in parent['sources'].items())
        if not(value['inputs_stable']and value['sources_stable']):raise ValueError('Source/input drift')
        value['success']=True
    except Exception as e:failure=e;value['error']=str(e)
    finally:
        if before:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);value['original_manifest_verified_before_after']=True
            except Exception as e:value['success']=False;failure=failure or e;value['manifest_error']=str(e)
        for n in out.iterdir():
            if n.is_file()and n.name!='.retain'and n not in artifacts:artifacts.append(n)
        value['artifacts']=[]
        for n in artifacts:
            record=PREFIX+Path(a.report).stem+'-'+n.name+'.bin';raw=n.read_bytes();value['artifacts'].append(dict(path=str(n),record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw)))
        result=write_report(ROOT,a.report,(json.dumps(value,indent=2)+'\n').encode())
        with sqlite3.connect(budget)as db,db:db.execute('UPDATE attempts SET success=?WHERE id=?',(int(value['success']),attempt))
        print(json.dumps(dict(success=value['success'],record=a.report,sha256=result,comparison=value.get('comparison'),error=value.get('error'))),flush=True)
    if failure:raise SystemExit(1)
if __name__=='__main__':main()
