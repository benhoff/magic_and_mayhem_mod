#!/usr/bin/env python3
"""Join reviewed execution research without granting acceptance or scope closure."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements
from rendering_completion import PLAN
from rendering_execution import review

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-execution-research-results-20261031/'

# Concrete implementation/observation contracts. None are accepted outcomes.
FOLLOWUPS = {
    'SC.alternate-ownership': ('EP06', 'Trace the required natural pointer readers at 4fd438/57df56/57e2ff/5818a2/581c32 and source readers 594514/594568 with allocation, publication, generation and release identity. Extend alias discovery only for an observed required path; five static reads and zero explicit stores do not prove absence.'),
    'SC.padded-fullscreen': ('EP04', 'Use existing toolbar/F9 in each configuration at 640x480 and 800x600, bind actual paint/swap and input acknowledgement before/after transition and retain the same native lease/history. Do not substitute original exclusive-mode failure or opt-in padding for the Qt route.'),
    'SC.natural-callers': ('EP06', 'Use the complete pilot capture to join required copy/fill/writer effects. Reuse isolated controls only within their exact caller and input scopes. Treat unobserved GPU self/keyed/clipped/clipper paths as unknown; do not require fabricated positive counts or claim absence.'),
    'SC.prior-history': ('EP03', 'Acquire the native-owned initial state and all prerequisite publications before the first accepted composed frame. Join every required source/history update and fail on dropped generations, unowned original-before inputs, late capture or discontinuity.'),
    'SC.map-save-domain': ('EP05', 'Bind offered UI selection to realm/recipe, actual ordered MAP placements and RNG/assembled grid; retain exact saves and restore checkpoints. Capture legitimate later-map progression and separate victory/defeat outcomes. Choose a finite tested domain first; ask about a broader support promise only after presenting coverage and costs.'),
    'SC.screen-provider-lifetime': ('EP05', 'Join retained original coordinator Main/Realm/World/Report callbacks to object+vptr+generation and populated ownership acquire/release/failure records. The 22 static vtable candidates and bounded tick/action log are discovery frontiers, not complete destructor/router proof.'),
    'SC.rgb555-live': ('EP06', 'Retain the exact primary GetPixelFormat caller and selector/masks at World entry/rebuild. Unmodified original query selects 565 on default native primary and 555 on explicit 555 storage. Investigate any required alternate producer/effect path; direct 555 surface storage alone does not establish supported live transport.'),
    'SC.physical-compatibility': ('EP07', 'Preserve the previously reviewed bounded Linux/Wine/native-provider player boundary. Record exact current provider and fault semantics; do not extend it to Win98/ME, physical scanout/reset or legacy-driver equality.'),
}


def events(data):
    if data[:8] != b'MNMMENU1' or len(data)<16 or struct.unpack_from('<2I',data,8)!=(1,64) or (len(data)-16)%64:
        raise ValueError('Unexpected original menu observer format')
    result=[struct.unpack_from('<16I',data,i) for i in range(16,len(data),64)]
    if [r[0] for r in result] != list(range(1,len(result)+1)):
        raise ValueError('Menu observer sequence discontinuity')
    return dict(records=len(result), bounded_capacity=256, capacity_reached=len(result)==256,
        classes=dict(Counter(str(r[3]) for r in result)), event_kinds=dict(Counter(str(r[1]) for r in result)),
        observed_vptrs=sorted({hex(r[5]) for r in result}),
        action_calls=[dict(sequence=r[0],event=r[1],class_id=r[3],object=hex(r[4]),vptr=hex(r[5]),
            argument=r[9],result=r[10],current=hex(r[11]),depth=r[12]) for r in result if r[1] in (2,3)],
        complete_router_or_destructor_proof=False)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if not args.report.startswith(PREFIX) or (ROOT/args.report).exists():
        parser.error('Require a new owned report')
    prerequisite=review(ROOT)
    with Reader(ROOT) as reader:
        plan=reader.json(PLAN)
        inputs=['inputs-02','build-02','original-01','contracts-01','format-query-01',
            'catalog-01','save-domain-02','music-control-inputs-01','quick-row0-06','storage-01',
            'software-row2-01','software-row2-01-delegate','default-media-01','default-media-02','music-volume15-01',
            'quick-row0-06-retention03','storage-01-retention','software-row2-01-retention',
            'default-media-01-retention','default-media-02-retention','music-volume15-01-retention']
        payloads={n:reader.json(PREFIX+n+'.json') for n in inputs}
        records=[dict(record=PREFIX+n+'.json',sha256=reader.digest(PREFIX+n+'.json'),
                      success=payloads[n]['success']) for n in inputs]
        for n in inputs:
            if n not in ('default-media-01','default-media-02') and payloads[n]['success'] is not True:
                raise ValueError('Research prerequisite unsuccessful: '+n)
        names=['tools/reconcile-rendering-execution-research.py','tools/research-rendering-input-domain.py',
            'tools/collect-rendering-execution-research.py','tools/research-native-software-session.py',
            'apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/gl_viewport.cpp',
            'apps/qt-shell/live_battle_menu_controller.cpp','runtime/menu/observer.c',
            'runtime/menu/battle.h','assets/config_loader.cpp','tools/native_music_launch.py',
            'tools/rendering_acceptance.py','tools/rendering_execution.py']
        sources={n:reader.digest(n) for n in names}
        code=subprocess.check_output(['git','diff','648ce219','--','apps','renderer','runtime','reconstruction','assets','audio','protocols'],cwd=ROOT)
        if code:
            raise ValueError('Product sources changed from reviewed source')
        leads=prerequisite['evidence_applicability_seed']
        rows=[]
        for row in requirements(plan):
            candidates=[e for e in leads if e['id'] in row['candidate_evidence']]
            rows.append(dict(row, lead_review=[dict(id=e['id'],record=e['record'],sha256=e['sha256'],
                current_source_binding_state=e['current_source_binding_state'],changed_sources=e['changed_sources'],
                scope=e['scope'],reuse='historical bounded reference; rerun affected changed dependencies' if e['changed_sources'] else 'candidate for scoped reuse only',
                adequate_to_close_requirement=False) for e in candidates],
                state='pending',accepted=False,
                research_disposition='Source and recorded assertion review only. '+row['pending_reason'],
                required_configuration=row['configuration']))
        catalog=payloads['catalog-01']['catalog']; selections=[]; menu=[]
        names_data=(ROOT/'working/game-nocd/Realms/Multiplayer/MultiplayerregionNames.txt').read_bytes()
        names_record=PREFIX+'join-multiplayer-names-01.bin'
        names_identity=dict(record=names_record,sha256=write_report(ROOT,names_record,names_data))
        offered_names=names_data.decode('latin1').splitlines()
        multiplayer=next(c for c in catalog if c['realm']=='Multiplayer')
        for name in ['quick-row0-06','software-row2-01-delegate']:
            flow=payloads[name]['flow'];display_name=flow['selected_map_name']
            ordinal=flow['selected_map_row']+1
            if flow['selected_map_id']!=str(ordinal) or offered_names[ordinal-1]!=display_name:
                raise ValueError('Actual selection disagrees with installed ordered names')
            recipe=next(r for r in multiplayer['recipes'] if r['id']==ordinal)
            candidates=sorted({r[0] for r in recipe['specific']} | {r[0] for r in recipe['random']})
            installed=payloads['inputs-02']['installation_inputs']
            candidate_inputs={}
            for section in candidates:
                expected=(recipe['path'].replace('\\','/')+'/'+recipe['prefix']+f'{section:02}.map').lower()
                found=[n for n in installed if n.lower()==expected]
                if len(found)!=1:raise ValueError('Unresolved candidate MAP '+expected)
                candidate_inputs[found[0]]=installed[found[0]]
            matches=[dict(realm='Multiplayer',config=multiplayer['input'],recipe=recipe,
                ordered_names=names_identity,candidate_map_inputs=candidate_inputs,
                binding='Observed one-based selection, exact ordered installed name and corresponding recipe number; actual random choices/rotations/placements not captured.')]
            selections.append(dict(observation=name,row=flow['selected_map_row'],selection_id=flow['selected_map_id'],
                display_name=display_name,catalog_name_matches=matches,
                generated_world_identity_established=False))
        for name in ['quick-row0-06-retention03','storage-01-retention','software-row2-01-retention']:
            for artifact in payloads[name]['retained_artifacts']:
                if artifact['path']=='session/events.bin':
                    menu.append(dict(observation=name,record=artifact['record'],sha256=artifact['sha256'],
                        summary=events(reader.read_bytes(artifact['record']))))
        from native_music_launch import validate_activity
        music_raw=next(a for a in payloads['music-volume15-01-retention']['retained_artifacts']
                       if a['path']=='session/native-music-report.json')
        music_activity=validate_activity(reader.json(music_raw['record']))
        report=dict(schema=1,kind='rendering_execution_research_join',success=True,sources=sources,
            product_source_commit=subprocess.check_output(['git','rev-parse','648ce219'],text=True).strip(),
            product_sources_unchanged=True,planning_commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
            records=records,requirements=rows,evidence_leads=leads,
            counts=dict(requirement_configuration_rows=len(rows),evidence_leads=len(leads),
                lead_source_states=dict(Counter(e['current_source_binding_state'] for e in leads)),accepted_rows=0),
            selection_research=selections,observed_menu_routes=menu,
            music_control=dict(record=music_raw['record'],sha256=music_raw['sha256'],
                validation='Existing native_music_launch.validate_activity, without initial-silence allowance',
                activity=music_activity,isolated_setting='SOUND/MusicVolume 0 -> 15',
                default_media_accepted=False,
                conclusion='Zero-volume input generated no plays in two runs; explicit full-volume control produced actual playback and retired normally. Do not classify the earlier expectation failures as a demonstrated renderer defect.'),
            scope_followups=[dict(id=k,owner_task=v[0],required_next_work=v[1],
                question_requires_user_now=False,scope_resolution_claimed=False) for k,v in FOLLOWUPS.items()],
            pilot_implementation_contract=dict(
                entry='Actual run-x86.sh/run-native.sh admission; private-owner research is not public entrypoint proof. Resolve storage placement/reservation without weakening the 90 GiB floor or isolation.',
                presentation='Bind actual QOpenGLWidget frameSwapped or raster paint completion to exact surface generation/lease and GPU completion fence. LiveCommandRenderer.framePresented follows queued update and Qt may coalesce it; it cannot certify actual presented-frame identity.',
                ownership='Own initial history before first frame; retain contiguous prerequisite publications/actions and explicit frame/generation identities. A source capture or all emitted COPYs is narrower.',
                oracle='Independent original drawing on identical owned inputs/actions/history. Never feed original-before pixels to the native renderer. Preserve caller effects and intentional native UI policy separately.',
                admission='Implement collector-specific positive validation, real backend/case and exact receipts in artifact 5. Reject missing frame, wrong generation, substituted inputs, reused original output or unknown acknowledgement.',
                stop='Do not expand acceptance sessions until one pilot has an adequate current receipt; rerun it after shared pipeline changes.'),
            engine_acceptance_assessed=False,supported_player_accepted=False,original_recovery_complete=False,
            static_plan_states_changed=False,status_promotions=[],
            scope='Completed bounded research and implementation handoff; actual required acceptance execution, final domain, complete ownership/history and original recovery remain pending.')
        sha=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,sha256=sha,counts=report['counts'])))


if __name__=='__main__':main()
