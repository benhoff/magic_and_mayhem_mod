"""Evidence-bound technical answers to the eight rendering scope questions.

An answered requirement is distinct from completed engineering/proof. Only the
already bounded physical-compatibility claim closes here; no player is accepted.
"""
import argparse
from collections import Counter
import json
from pathlib import Path

from coverage_claims import fingerprint
from coverage_audit import pointer
from coverage_evidence import Reader, write_report

IDS = {'SC.alternate-ownership','SC.padded-fullscreen','SC.natural-callers','SC.prior-history',
       'SC.map-save-domain','SC.screen-provider-lifetime','SC.rgb555-live','SC.physical-compatibility'}


def need(ok, message):
    if not ok:raise ValueError(message)


def validate(plan, report):
    spec=plan['scope_review'];contract=plan['completion_contract']
    need(spec['schema']==1 and spec['id']=='rendering-scope-review-v1'
         and spec['applies_to']=='supported-player-scope' and spec['baseline_decisions_preserved'] is True,
         'Unsupported scope review')
    need(report['kind']=='rendering_scope_decision_review' and report['success'] is True
         and report['contract_sha256']==fingerprint(contract)
         and report['gap_register_sha256']==fingerprint(plan['required_gap_register'])
         and report['execution_plan_sha256']==fingerprint(plan['execution_plan']), 'Scope review input binding differs')
    decisions={d['id']:d for d in contract['scope_decisions']}
    rows=report['decisions']
    need(len(rows)==8 and {r['id'] for r in rows}==IDS,'Missing or duplicate scope answer')
    for row in rows:
        original=decisions[row['id']]
        need(row['blocking_condition']==original['blocking_condition'] and row['close_when']==original['close_when'],
             'Original conditional scope rule changed')
        need(row['user_judgment_required_now'] is False and row['answer'].strip()
             and row['technical_remaining'] and row['owner_task']=='EP07', 'Incomplete scope answer')
        need(row['question_state'] in {'answered','technical_investigation_open','support_promise_unselected'},
             'Invalid scope answer state')
        need(row['original_recovery_state']=='unresolved','Player boundary cannot close original recovery')
        expected='resolved' if row['id']=='SC.physical-compatibility' else 'unresolved'
        need(row['player_scope_state']==expected,'Unproved engineering scope was marked closed')
    need(report['no_engine_status_promotion'] is True and report['supported_player_accepted'] is False
         and report['original_recovery_complete'] is False and report['status_promotions']==[],
         'Scope review cannot promote engine acceptance')
    need(report['facts']['qt_fullscreen_already_exposed'] is True
         and report['facts']['rgb555_surfaces_are_not_rgb555_world_transport'] is True
         and report['facts']['natural_absence_is_not_global_unreachability'] is True,
         'Required technical distinctions disappeared')
    need({x['id'] for x in report['acceptance_followups']}==
         {'qt-fullscreen','format-production','owned-history','screen-routes','natural-callers','alternate-chain'},
         'Required engineering followup disappeared')
    counts=report['counts']
    need(counts['scope_questions']==8 and counts['immediate_user_decisions']==0
         and counts['player_scope_closed']==sum(r['player_scope_state']=='resolved' for r in rows)
         and counts['player_scope_unresolved']==sum(r['player_scope_state']=='unresolved' for r in rows)
         and counts['original_recovery_scope_unresolved']==8
         and counts['question_states']==dict(Counter(r['question_state'] for r in rows)), 'Scope counters differ from actual decisions')
    return report


def load(root, reader, plan):
    """Read the reviewed sidecar; frozen contract/gap/plan sections remain unchanged."""
    root=Path(root).resolve();spec=plan['scope_review'];name=spec['record'];p=Path(name)
    need(not p.is_absolute() and '..' not in p.parts and name.startswith('research/runtime/'), 'Unsafe scope result path')
    report=validate(plan,reader.json(name))
    entries={e['id']:e for e in reader.json('research/runtime/coverage/register.json')['evidence']}
    need(spec['evidence'] in entries,'Scope review lacks its registered evidence identity')
    entry=entries[spec['evidence']]
    need(entry['kind']=='static' and entry['record']==name and entry['sha256']==reader.digest(name)
         and all(pointer(report,a['pointer'])==a['equals'] for a in entry['assertions']), 'Scope review evidence identity/assertions differ')
    for path,expected in report['sources'].items():
        p=Path(path)
        need(not p.is_absolute() and '..' not in p.parts and p.parts[0] not in {'original','working','.git'}, 'Unsafe scope witness')
        need(reader.digest(path)==expected,'Scope review source changed: '+path)
    for record in report['records']:
        need(reader.digest(record['record'])==record['sha256'],'Historical scope input changed: '+record['record'])
    return dict(record=name,sha256=reader.digest(name),evidence=spec['evidence'],
        decisions=report['decisions'],facts=report['facts'],acceptance_followups=report['acceptance_followups'],
        counts=report['counts'],scope='Current engineering answers and one bounded player-scope closure. '
            'Frozen historical decisions, original-recovery gaps and all player acceptance requirements remain.')


def generate(root):
    from rendering_completion import review
    from rendering_acceptance import verify_evidence
    root=Path(root).resolve()
    prerequisite=review(root,include_scope_review=False)
    with Reader(root) as reader:
        plan=reader.json('docs/native-rendering-closure.json');register=reader.json('research/runtime/coverage/register.json')
        contract=plan['completion_contract'];records=[];payloads={}
        evidence_ids=['OA1.controls-current-20261026','OA1.campaign-current-20261026',
            'ACO7.final-native-controls-20261026','RRG26.natural-native','SP26.master-native-20261026',
            'NPL27.quick-map-005','NPL27.quick-map-207','NPL27.extended05','PLAYERMERGE27.storage01']
        for eid in evidence_ids:
            entry=verify_evidence(root,reader,register,eid);records.append(entry)
            payloads[eid]=reader.json(entry['record'])
        for key,path in [('callers','callers-03.json'),('components','components-03.json')]:
            name='research/runtime/rendering-scope-results-20261030/'+path;p=reader.json(name)
            need(p['success'] is True and p['sources_stable'] is True,'Scope control is incomplete: '+key)
            need(all(reader.digest(n)==h for n,h in p['sources'].items()),'Fresh scope control source differs: '+key)
            payloads[key]=p;records.append(dict(id=key,record=name,sha256=reader.digest(name),scope=p.get('scope',key),
                                               current_source_binding_state='matching_recorded_sources'))
        name='research/runtime/rendering-remaining-results-20261026/natural-history-difference-01.json'
        history=reader.json(name);records.append(dict(id='history-diagnostic',record=name,sha256=reader.digest(name),
            scope=history['scope'],current_source_binding_state='historical_diagnostic_not_current_execution'))
        need(payloads['OA1.controls-current-20261026']['original_allocation_recovered'] is False
             and payloads['OA1.controls-current-20261026']['provider_lifecycle_authored'] is True
             and payloads['OA1.campaign-current-20261026']['observed_natural_publications'] is False,
             'Alternate repair was relabeled recovered natural ownership')
        scan=payloads['callers'];need(scan['global_unreachable_proved'] is False and len(scan['routes'])==5
            and all(r['natural_caller_recovered'] is False for r in scan['routes']), 'Caller scan does not support recorded boundary')
        need(history['comparisons']==dict(native_owned_zero_original_zero=True,original_before_live=True,native_owned_zero_live=False)
            and len(history['frames'])==8 and all(f['differences']==[dict(x=548,y=433,native_zero=0,live=6338,original_before=6372)]
                                                 for f in history['frames']), 'Prior-history counterexample changed')
        need('record=1236 op=0 backend=2 old=6372 next=0' in history['original_watch']['before_log']
             and 'record=1261 op=3 backend=9 old=0 next=6338' in history['original_watch']['before_log'], 'History propagation witness changed')
        providers=payloads['SP26.master-native-20261026']
        need(providers['platform_file_profile_heap_com_codec_endpoints_supplied'] is True
             and providers['windows_seh_dispatch'] is False and providers['physical_driver_equivalence'] is False,
             'Local provider fixtures were relabeled real transition/platform recovery')
        controls=payloads['components']
        need(len(controls['presentations'])==2 and all(p['f9_consumed'] and p['fullscreen_restore']
             and p['logical_input_preserved'] and p['onscreen_comparisons']==17 for p in controls['presentations']), 'Qt fullscreen controls incomplete')
        need([p['masks'] for p in controls['format_rows']]==[[0xf800,0x7e0,31],[0xf800,0x7e0,31],[0x7c00,0x3e0,31]]
             and all(p['remaining_surfaces']==0 and p['remaining_devices']==0 for p in controls['format_rows']), 'Facade format/retirement controls incomplete')
        witnesses=['apps/qt-shell/main.cpp','apps/qt-shell/viewport_presentation.cpp','apps/qt-shell/viewport_presentation.hpp',
            'runtime/native-draw/factory.c','runtime/native-draw/surfaces.h','runtime/scene/canvas_producers.c',
            'compat/legacy/world_frame.cpp','runtime/native-draw/native_only.h','tools/native_only_contract.py',
            'docs/native-rendering-closure.md','docs/native-rendering-execution.md','docs/native-rendering-acceptance.md',
            'research/runtime/rendering-scope-decisions-20261030.md','tools/rendering_scope.py','tests/test-rendering-scope.py']
        witnesses += ['tools/rendering_acceptance.py','tools/rendering_execution.py','tools/rendering_gaps.py',
                      'tools/coverage_audit.py','tools/coverage_claims.py','tools/coverage_evidence.py','tools/evidence_local.py']
        sources=dict(prerequisite['sources']);sources.update({n:reader.digest(n) for n in witnesses})
        source=lambda n:reader.read_text(n)
        need('presentation_->addControls(*toolbar)' in source('apps/qt-shell/main.cpp')
             and 'Qt::Key_F9' in source('apps/qt-shell/viewport_presentation.cpp')
             and 'window_.showFullScreen()' in source('apps/qt-shell/viewport_presentation.cpp'), 'Fullscreen is not wired as reviewed')
        need('masks[0]==0x7c00' in source('runtime/native-draw/surfaces.h')
             and 'WorldInputPolicy::colourRgb555?1u:0u' in source('compat/legacy/world_frame.cpp')
             and 'frame.colourRgb555&&op!=MNM_WORLD_COLOUR_RECT' in source('compat/legacy/world_frame.cpp'),
             'Surface/World format policy changed')
        descriptions={
          'SC.alternate-ownership':('technical_investigation_open','Required used-surface ownership remains engineering work. The native padded owner and opt-in original-process repair are explicit policies; neither recovers the historical allocator or proves all original alternate paths absent.',
              ['Trace the required original allocation/publication/writer/retirement chain, or justify a precise caller/configuration boundary from actual required journeys.','Do not add another allocator or delete the original path merely because bounded watches saw no natural publication.']),
          'SC.padded-fullscreen':('answered','Qt fullscreen is already exposed by the toolbar and F9 in the current shell. It is required presentation validation, not a new product preference. Startup --fullscreen rejection and original exclusive/padded DirectDraw behavior are separate boundaries.',
              ['Exercise the existing Qt toggle with current player output/input/focus and both declared configurations/frame sizes.','Retain original padding/exclusive-driver limits separately; the fresh synthetic Qt results do not close actual-game presentation.']),
          'SC.natural-callers':('technical_investigation_open','All optional handlers already exist. The fresh byte-pinned scan still leaves the five routes without recovered natural callers and retains runtime/indirect uncertainty. No user decision is needed to retain them or investigate their requiredness.',
              ['Obtain required caller/provider observations or a precise justified boundary for each route, raw reader, alias/writer and fill.','Zero observed or static candidates do not establish global absence, dead code or permission to drop a required operation.']),
          'SC.prior-history':('answered','Earlier owned pixel history is required wherever destination-dependent drawing reads it. The retained eight-frame counterexample proves this for that captured interval; it does not establish a current resident-player defect.',
              ['Recover/produce the needed owned prefix and independently verify its origin and continuity.','Establish current-player relevance before calling the retained mismatch a regression; never feed original before pixels to native rendering.']),
          'SC.map-save-domain':('support_promise_unselected','Engineering can select and hash validation inputs now. Retained Quick Battle rows 0 and 2 and the extended profile are finite evidence, not a final campaign/map/save/duration support promise.',
              ['Bind concrete campaign, actual later-map, victory/defeat and save inputs without waiving any journey/operation.','Only a broader final release promise beyond demonstrated finite inputs needs a product recommendation and user judgment.']),
          'SC.screen-provider-lifetime':('answered','Keep the permitted original coordinator; a full native screen-router rewrite is not required. Required real Main/Realm/World/Report routes and their populated resource/callback lifetimes still need engineering recovery and validation.',
              ['Build a route-driven round trip through actual selected screen lifetimes and error/retirement paths.','Existing complete local providers/callers with supplied endpoints do not establish the real transition router or Windows SEH behavior.']),
          'SC.rgb555-live':('technical_investigation_open','RGB565 World transport and RGB555 surface storage are different contracts. Fresh production-facade controls admit explicit RGB555 offscreen storage and retain its words across leases. Do not treat all RGB555 handling as an optional feature.',
              ['Determine actual required game format production and sprite/effect/source use; validate the required complete path.','Live format1 World ingestion remains unproved/unadmitted; default RGB565 admission and this non-strict facade control do not establish its natural absence or strict-player acceptance.']),
          'SC.physical-compatibility':('answered','Resolve the current player boundary as the already specified x86 Linux/Wine/Qt/native-provider claim, with exact actual provider/configuration recorded per result. It makes no legacy-Windows driver-identity or physical device-reset equivalence promise.',
              ['Retain logical provider failure/cleanup/restart checks and actual backend admission as required acceptance work.','Original physical-driver/Windows exceptional behavior and any requested expanded platform/reset promise remain separate original-recovery or product-expansion work.'])}
        rows=[]
        for d in contract['scope_decisions']:
            state,answer,remaining=descriptions[d['id']]
            rows.append(dict(id=d['id'],question_state=state,answer=answer,user_judgment_required_now=False,
                judgment_owner='engineering' if d['id']!='SC.map-save-domain' else 'engineering_inputs_product_promise_later',
                player_scope_state='resolved' if d['id']=='SC.physical-compatibility' else 'unresolved',
                original_recovery_state='unresolved',blocking_condition=d['blocking_condition'],close_when=d['close_when'],
                technical_remaining=remaining,owner_task='EP07',contracts=d['contracts']))
        followups=[dict(id=i,task=t,requirements=req,condition=condition) for i,t,req,condition in [
            ('qt-fullscreen','EP04',['journey.camera-motion.viewport-boundaries','journey.cursor-interaction.picking','gate.transitions','gate.input-media'],
             'Required existing toolbar/F9 route, both configurations; synthetic controls are component evidence only.'),
            ('format-production','EP06',['operation.sprite-direct','operation.sprite-auxiliary','operation.flags-and-return-values','gate.required-boundaries'],
             'Investigate required formats; no unsupported normal path is silently excluded and no live RGB555 promise is invented.'),
            ('owned-history','EP03',['operation.destination-dependency','operation.cpu-write-readback','gate.pixels-history'],
             'Start at a verified owned origin; incomplete late-prefix replay cannot close complete output.'),
            ('screen-routes','EP05',['journey.campaign-entry.loading','journey.map-transitions.next-map','journey.battle-results.return-to-menu'],
             'Actual router and populated owners; no full native rewrite imposed.'),
            ('natural-callers','EP06',['operation.copy-alias-overlap','operation.copy-keyed','gate.required-boundaries'],
             'Retain all handlers and unknowns; actual required caller or justified boundary needed.'),
            ('alternate-chain','EP07',['gate.required-boundaries'],
             'Native owned policy and authored repair do not recover the historical allocator.')]]
        domain=[]
        for eid in ['NPL27.quick-map-005','NPL27.quick-map-207','NPL27.extended05']:
            p=payloads[eid];domain.append(dict(evidence=eid,profile=p['acceptance_profile'],command=p['command'],
                original_worlds=p['original_worlds'],source_binding_state=next(e['current_source_binding_state'] for e in records if e['id']==eid),
                grants_arbitrary_maps_saves_duration=False))
        report=dict(schema=1,kind='rendering_scope_decision_review',success=True,
            subject_commit=plan['completion_contract']['reviewed_source_commit'],review_base_commit='5237da93a1ff2d0f194b9f6d13c073a4f2fa47ca',
            contract_sha256=fingerprint(contract),gap_register_sha256=fingerprint(plan['required_gap_register']),
            execution_plan_sha256=fingerprint(plan['execution_plan']),decisions=rows,acceptance_followups=followups,
            facts=dict(qt_fullscreen_already_exposed=True,rgb555_surfaces_are_not_rgb555_world_transport=True,
                natural_absence_is_not_global_unreachability=True,caller_instructions=scan['graph']['instruction_count'],
                unresolved_computed_target_classes=scan['graph']['computed_flow_reconciliation']['classifications'],
                qt_dpi_controls=controls['presentations'],facade_format_controls=controls['format_rows'],
                historical_history_pixel=history['frames'][0]['differences'][0],history_is_not_current_player_defect=True),
            observed_validation_domain=domain,records=records,sources=sources,
            counts=dict(scope_questions=8,immediate_user_decisions=0,player_scope_closed=1,player_scope_unresolved=7,
                original_recovery_scope_unresolved=8,question_states=dict(Counter(r['question_state'] for r in rows))),
            no_engine_status_promotion=True,supported_player_accepted=False,original_recovery_complete=False,
            original_game_executed=False,engine_execution_performed=False,status_promotions=[],
            scope='Current-source/evidence engineering decisions, fresh byte-only caller scan and separately recorded synthetic Qt/facade controls. '
                  'Answered requirements are not completed proof. Only the pre-existing bounded physical-compatibility claim closes; '
                  'seven technical/product-support scope closures and all player acceptance remain pending.')
        return validate(plan,report)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    args=parser.parse_args();root=Path(__file__).resolve().parents[1]
    with Reader(root) as reader:
        plan=reader.json('docs/native-rendering-closure.json')
    need(args.report==plan['scope_review']['record'] and not (root/args.report).exists(),'Require the new configured scope record')
    report=generate(root)
    sha=write_report(root,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,sha256=sha,counts=report['counts'])))


if __name__=='__main__':main()
