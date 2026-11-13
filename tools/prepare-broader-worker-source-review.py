#!/usr/bin/env python3
"""Freeze recorder-source restart, historical charges and the exact new finite cohort."""
import copy,json,sqlite3,subprocess,sys
from contextlib import closing
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_worker_source_budget import bound,predecessors
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/';T='research/runtime/rendering-transition-recovery-results-20261107/'
def main():
 test=subprocess.run([sys.executable,'tests/test-broader-worker-source-budget.py'],cwd=ROOT,capture_output=True);require(test.returncode==0,'Source-phase budget controls failed')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));parent=P+'plan-12.json';g=copy.deepcopy(r.json(parent));require(g['maximum_additional_original_games']==126 and g['maximum_combined_original_games']==134,'Unexpected historical bound')
  with closing(sqlite3.connect(ROOT/'working/broader-player/requests.sqlite3'))as db:
   db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select * from requests order by started_ns,case_id')]
  require(len(rows)==15 and sum(v['reserved_games']for v in rows)==30,'Unexpected old request denominator')
  b=r.json(P+'build-current-04.json');old=r.json('research/runtime/surface-loss-restore-results-20261109/bp12-input-plan-01.json');changed=sorted(n for n,h in old['compiler_sources'].items()if b['compiler_sources'][n]!=h);rt=sorted(n for n,h in old['runtime_sources'].items()if b['runtime_sources'][n]!=h);require(changed==['apps/qt-shell/gui_surface_loss_probe.hpp']and not rt,'Unexpected native delta')
  names=['tools/broader_worker_source_budget.py','tools/run-broader-worker-source-case.py','tests/test-broader-worker-source-budget.py','tools/prepare-broader-worker-source-review.py']
  controls=[P+'parallel-command-controls-02.json',P+'parallel-worker-controls-03.json',P+'parallel-history-controls-03.json',T+'bp12-epoch-controls-01.json',P+'native-controls-05.json']
  q=dict(schema=1,kind='broader_recorder_source_continuation_review',success=True,original_game_runs=0,old_source_current_credit=False,old_reserved_starts=30,old_additional_ceiling=126,new_additional_ceiling=128,old_combined_ceiling=134,new_combined_ceiling=136,fresh_positive_cases=90,fresh_positive_original_starts=98,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,queue_limits_increased=False,reviewer_sources={n:r.digest(n)for n in names},current_build=ref(P+'build-current-04.json'),changed_compiler_files=changed,changed_runtime_files=rt,current_controls=[ref(n)for n in controls],old_requests=rows,parent_global_plan=ref(parent),staging=ref(P+'recorder-player-staging-04.json'),offline_replay=ref(P+'recorded-prefix-parallel-replay-01.json'),old_negative_custody=ref(P+'bp12-loss-default640-01-quiescence-custody-01.json'),maintenance=ref(P+'storage-maintenance-checkpoint-03.json'),controls=dict(tests=7,log=(test.stdout+test.stderr).decode()),fresh_roles=dict(world=56,menu=12,component=8,fault=8,strict=4,boundary=2),reason='The B03 GUI loss probe consumed its request before checking host-local ownership. Its retained guard failure is not a successful loss or a recorder failure. Defer consumption until the unchanged completion/ownership predicates hold, retaining the original8second deadline. Fresh real-GL controls defer unpresented and borrowed requests and then prove actual target destruction/deletion and a distinct fresh identity. Preserve all30previous reservations and negative phases; no source waiver or refund. The wholly fresh90case/98start conjunction needs128additional/136combined conservative reservations with zero spare or automatic repair starts. This is an agent-set finite planning ceiling, not user numeric approval. Original recovery and historical root-cause limits remain unchanged')
  n=P+'recorder-source-review-02.json';h=write_report(ROOT,n,(json.dumps(q,indent=2)+'\n').encode())
  g.update(previous_plan=ref(parent),recorder_source_review=dict(record=n,sha256=h),maximum_additional_original_games=128,maximum_combined_original_games=136,maximum_combined_WP10_and_new_original_games=136,minimum_final_additional_reservations=128,maximum_manually_reviewed_additional_repairs=0,request_driver='tools/run-broader-worker-source-case.py',current_build=ref(P+'build-current-04.json'),budget_review=q['reason'],fresh_source_positive_cases=90,fresh_source_original_starts=98)
  require(bound(r,g)==128 and predecessors(r,rows,g)==30,'New source budget did not validate');gn=P+'plan-13.json';gh=write_report(ROOT,gn,(json.dumps(g,indent=2)+'\n').encode())
  # These are comparison-only source policies, independent of recorder execution.
  for old,new in [('native-edge-analysis-plan-03.json','native-edge-analysis-plan-13.json'),('world-native-edge-analysis-plan-02.json','world-native-edge-analysis-plan-13.json'),('boundary-native-edge-analysis-plan-02.json','boundary-native-edge-analysis-plan-13.json')]:
   a=copy.deepcopy(r.json(P+old));require(a['success']and all(r.digest(k)==v for k,v in a['sources'].items()),'Analysis source drift: '+old);a.update(global_plan=dict(record=gn,sha256=gh),previous_analysis_plan=ref(P+old));write_report(ROOT,P+new,(json.dumps(a,indent=2)+'\n').encode())
  print(json.dumps(dict(success=True,review_sha256=h,global_plan=dict(record=gn,sha256=gh),old_reserved=30,fresh_cases=90,fresh_starts=98)),flush=True)
if __name__=='__main__':main()
