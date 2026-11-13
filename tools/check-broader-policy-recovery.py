#!/usr/bin/env python3
"""Compare every recovery pixel with original rasters plus explicit native edges."""
import argparse,ast,hashlib,json,subprocess,sys,types
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
SOURCE='tools/check-rendering-recovery-closeout.py'
POLICY='native-retain-outside-displacement-v1'

def bound_code(raw,filename):
    tree=ast.parse(raw,filename=filename)
    output=ast.parse("ROOT / ('working/surface-loss/comparisons' if surface_loss else 'working/recovery-closeout08/comparisons')",mode='eval').body
    oracle=ast.parse("ROOT / 'working/original-reference'",mode='eval').body
    outputs=oracles=0
    class Bind(ast.NodeTransformer):
        def visit_Assign(self,node):
            nonlocal outputs
            if any(isinstance(t,ast.Name)and t.id=='output_root'for t in node.targets):
                require(len(node.targets)==1 and ast.dump(node.value)==ast.dump(output),'Unexpected output initializer');outputs+=1;node.value=ast.copy_location(ast.Name(id='POLICY_OUTPUT_ROOT',ctx=ast.Load()),node.value)
            return self.generic_visit(node)
        def visit_BinOp(self,node):
            nonlocal oracles
            if ast.dump(node)==ast.dump(oracle):
                oracles+=1;return ast.copy_location(ast.Name(id='POLICY_REFERENCE',ctx=ast.Load()),node)
            return self.generic_visit(node)
    tree=Bind().visit(tree);require(outputs==oracles==1,'Missing/duplicate exact oracle or output binding')
    return compile(ast.fix_missing_locations(tree),filename,'exec'),hashlib.sha256(ast.dump(tree,include_attributes=False).encode()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('acquisition','raw-comparison','oracle-build','controls','report'):p.add_argument('--'+name,required=True)
    p.add_argument('--kind',choices=('loss','transition'),required=True);args=p.parse_args()
    output=Path('/tmp/mnm-broader-policy-comparisons-20261110');output.mkdir(exist_ok=True)
    require(subprocess.check_output(['findmnt','-T',str(output),'-n','-o','FSTYPE'],text=True).strip()=='tmpfs','Policy comparison scratch must be RAM')
    with Reader(ROOT)as r:
        raw=r.json(args.raw_comparison);a=r.json(args.acquisition);build=r.json(args.oracle_build);controls=r.json(args.controls)
        require(raw['acquisition']==dict(record=args.acquisition,sha256=r.digest(args.acquisition)) and raw['sources_stable'] and raw['comparison_processes_stopped'] and raw['original_manifest_verified_before_after'] and len(raw['epochs'])==2,'Raw original comparison not retained and stopped')
        require(build['success'] and all(r.digest(n)==h for n,h in build['sources'].items()) and controls['success'] and controls['cases']==26 and controls['original_game_runs']==0 and controls['guard_sha256']==r.digest('tools/broader_finite_edge_oracle.hpp'),'Unverified policy oracle/control identity')
        binary=Path(build['binary']['path']);require(hashlib.sha256(binary.read_bytes()).hexdigest()==build['binary']['sha256']==r.digest(build['binary']['record']),'Policy oracle binary differs')
        refs={key:dict(record=name,sha256=r.digest(name))for key,name in [('raw_original_comparison',args.raw_comparison),('oracle_build',args.oracle_build),('controls',args.controls)]}
        names=['tools/check-broader-policy-recovery.py','tools/check-rendering-recovery-closeout.py','tools/broader_finite_edge_oracle.hpp','tools/broader-world-policy-reference.cpp','tests/broader-finite-edge-oracle-test.cpp']
        sources={n:r.digest(n)for n in names};producer=r.read_bytes(SOURCE)
    code,ast_sha=bound_code(producer,str(ROOT/SOURCE));writer=coverage_evidence.write_report
    def retain(root,path,body):
        if str(path)==args.report:
            q=json.loads(body);out=output/Path(args.report).stem;certificates=[]
            for index in (0,1):
                logs=sorted((out/('epoch-'+str(index))/'reference').glob('*.finite-edge.json'))
                require(len(logs)==int(q['epochs'][index]['original_reference']['worlds']),'Missing per-World policy certificates')
                taps=0;affected=[]
                for f in logs:
                    value=json.loads(f.read_bytes());require(value['schema']==1 and value['policy']==POLICY and value['original_padding_equivalence_claimed'] is False,'Unknown policy certificate')
                    if value['taps']:affected.append(dict(world=int(f.name.split('-')[1].split('.')[0]),taps=value['taps']))
                    taps+=len(value['taps'])
                q['epochs'][index]['original_reference'].update(reference_model='original-rasters-with-explicit-native-finite-edge',original_equivalence_for_padding=False)
                certificates.append(dict(epoch=index,world_certificates=len(logs),retained_taps=taps,affected_worlds=affected))
            q['native_edge_policy']=dict(id=POLICY,**refs,certificates=certificates,raw_original_mismatching_pixels=sum(e['comparison']['mismatching_pixels']for e in raw['epochs']),all_viewport_pixels_compared=True,ignored_pixels=0,raw_original_result_preserved=True,original_equivalence_for_retained_taps=False,production_renderer_changed=False,original_game_runs_added=0,original_reference_executions_added=True,scope='Original raster execution with documented native retained-destination edges applied after each draw; padding-read equivalence and original allocation recovery remain unclaimed.')
            q['comparison_storage']=dict(path=str(out),temporary=True,policy='task-owned-tmpfs-derived-policy-comparison-v1',bound_ast_sha256=ast_sha,original_producer=SOURCE,only_output_directory_and_oracle_executable_bound=True)
            with Reader(ROOT)as r:stable=all(r.digest(n)==h for n,h in sources.items())
            q['sources'].update(sources);q['sources_stable']&=stable;q['success']&=stable;body=(json.dumps(q,indent=2)+'\n').encode()
        return writer(root,path,body)
    coverage_evidence.write_report=retain
    module=types.ModuleType('broader_policy_recovery');module.__file__=str(ROOT/SOURCE);module.POLICY_OUTPUT_ROOT=output;module.POLICY_REFERENCE=binary
    try:
        exec(code,module.__dict__);sys.argv=[str(ROOT/SOURCE),'--acquisition',args.acquisition,'--report',args.report];return module.main(surface_loss=args.kind=='loss')
    finally:coverage_evidence.write_report=writer
if __name__=='__main__':raise SystemExit(main())
