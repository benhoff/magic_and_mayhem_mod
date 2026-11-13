"""Certificates for original World execution with the existing native edge rule."""
from rendering_pilot import require
POLICY='native-retain-outside-displacement-v1'
def assess(reader,c):
 p=c['native_edge_policy'];require(p['id']==POLICY and p['ignored_pixels']==0 and p['all_output_pixels_compared']is True and p['original_padding_equivalence']is False and p['production_native_code_changed']is False,'Unknown/weakened World edge model')
 def load(ref):
  require(reader.digest(ref['record'])==ref['sha256'],'Changed World policy dependency');return reader.json(ref['record'])
 raw=load(p['raw_original_comparison']);build=load(p['oracle_build']);controls=load(p['controls'])
 require(raw['acquisition']==c['acquisition']and raw['sources_stable']and raw['comparison_processes_stopped']and raw['original_manifest_verified_before_after'],'Unbound raw original World result')
 require(build['success']and controls['success']and controls['cases']==26 and controls['guard_sha256']==reader.digest('tools/broader_finite_edge_oracle.hpp')and all(reader.digest(n)==h for n,h in build['sources'].items()),'Untested/stale World policy oracle')
 require(reader.digest(build['binary']['record'])==build['binary']['sha256'],'World policy executable not retained')
 require(c['success']and c['sources_stable']and c['comparison_processes_stopped']and c['original_manifest_verified_before_after']and c['comparison']['success']and c['comparison']['mismatching_pixels']==0,'Incomplete World policy comparison')
 groups=p['reference_groups'];require(1<=len(groups)<=16,'Missing/excessive original World invocations');worlds=taps=0;inputs=set()
 for group in groups:
  identity=(group['source_sha256'],group['output_identity']);require(identity not in inputs,'Duplicate policy reference invocation');inputs.add(identity)
  value=load(group['certificate']);require(value['schema']==1 and value['policy']==POLICY and value['original_padding_equivalence']is False and value['index_sha256']==group['index_sha256']and len(value['worlds'])==group['worlds'],'Unbound per-World policy certificates')
  require([v['generation']for v in value['worlds']]==list(range(1,group['worlds']+1)),'Policy reference generation gap')
  for world in value['worlds']:
   require(all(type(world[k])is int and 1<=world[k]<=2048 for k in('width','height')),'Policy reference extent');size=world['width']*world['height'];seen=set()
   for tap in world['taps']:
    draw,destination,sample,word=(tap[k]for k in('draw','destination','outside_sample','retained_word'))
    require(all(type(v)is int for v in(draw,destination,sample,word))and 1<=draw<=12320 and 0<=destination<size<=sample and 0<=sample-destination<=16 and 0<=word<=65535 and(draw,destination)not in seen,'Invalid covered World edge tap');seen.add((draw,destination))
   taps+=len(world['taps'])
  worlds+=group['worlds']
 require(worlds==p['certified_worlds']and taps==p['retained_taps'],'World policy summary lost certificates')
 reference=c.get('original_reference',{})
 if len(groups)==1 and type(reference.get('worlds'))is int:require(worlds==reference['worlds'],'Policy World count differs from compared history')
 return dict(verified=True,id=POLICY,raw_original_comparison=p['raw_original_comparison'],certified_worlds=worlds,retained_taps=taps,ignored_pixels=0,original_padding_equivalence=False,oracle_sources=build['sources'])
