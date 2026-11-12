"""Review only post-retirement host-control drift; native inputs must be identical."""
from coverage_claims import fingerprint
from rendering_pilot import require
PREFIX='research/runtime/caller-state-installed-results-20261108/'
HOST={'tools/run-rendering-caller-state.py','tools/prepare-rendering-caller-state.py'}
def native_identity(plan):
 return fingerprint(dict(compiler=plan['compiler_sources'],runtime={n:h for n,h in plan['runtime_sources'].items()if n not in HOST},player=plan['player_binary']['sha256'],oracle_sources=plan['oracle']['native_sources'],oracle_binaries=[b['sha256']for b in plan['oracle']['binaries']]))
def review(reader,record,proof):
 ref=proof['prospective_plan'];require(reader.digest(ref['record'])==ref['sha256'],'Changed executed source plan');old=reader.json(ref['record']);changed=sorted(n for n,h in proof['sources'].items()if reader.digest(n)!=h)
 result=dict(verified=True,native_phase_sha256=native_identity(old),executed_plan=ref,raw_changed_sources=changed,current_whole_execution=not changed)
 if not changed:return result
 require(set(changed)==HOST,'Changed non-controller execution/analysis source')
 name=PREFIX+'input-plan-05.json';current=reader.json(name);repair=current['capacity_repair'];require(reader.digest(repair['record'])==repair['sha256'],'Changed capacity review');r=reader.json(repair['record'])
 require(r['kind']=='caller_state_capacity_failure_and_bounded_repair_plan'and r['success']and r['parent_plan']==ref and set(r['allowed_post_retirement_controls'])==HOST,'Unreviewed retired controller phase')
 require(dict(record=record,sha256=reader.digest(record))in r['retained_parent_proofs'],'Unaccepted prior proof cannot acquire applicability')
 require(native_identity(current)==native_identity(old),'Native/player/oracle source phase changed')
 require({n for n,h in current['runtime_sources'].items()if old['runtime_sources'].get(n)!=h}==HOST,'Controller-only source closure differs')
 for n in HOST:
  for plan in(old,current):
   pre=plan['source_preimages'][n];require(pre['sha256']==plan['runtime_sources'][n]and reader.digest(pre['record'])==pre['sha256'],'Lost exact controller preimage')
  require(proof['sources'][n]==old['runtime_sources'][n]and reader.digest(n)==current['runtime_sources'][n],'Controller drift is outside the reviewed phases')
 result.update(review=repair,current_plan=dict(record=name,sha256=reader.digest(name)),scope='Only two retired host acquisition controls changed. Shorter physical focus/waits and an explicit cumulative budget extension do not alter retained observations, native/compiler/player/oracle inputs, state models, expected bytes, pixel analysis or the completed normal retirement. No current-controller replay or whole current execution is asserted. Raw historical hashes and failures remain unchanged.')
 return result
