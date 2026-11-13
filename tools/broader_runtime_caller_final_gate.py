"""Retain the full runtime gate and verify the distinct caller analysis recovery."""
import hashlib,json
from broader_runtime_final_gate import supplement as original_supplement,exact_ledger
from broader_player_runtime_caller_matrix import P,FIRST,REVIEW
from broader_player_acceptance import checked
from rendering_pilot import require


def caller_binding(reader,value):
    comparison=checked(reader,value['value']['comparison'])
    binding=comparison['analysis_schema_binding'];review=checked(reader,REVIEW)
    phase=reader.json(value['spec']['plan']);expected=phase['compiler_sources']|phase['runtime_sources']
    require(binding['phase']==value['acquired']['prospective_plan']
            and binding['recorded_object_root']==review['source_root']
            and binding['build']==phase['current_player_build']
            and binding['resolved_source_count']==len(expected)==1376
            and binding['resolved_evidence_payload_count']==7
            and binding['recorded_payload_changed']is False
            and binding['native_source_waivers']is False
            and binding['source_lookup_waivers']is False
            and binding['original_game_runs_added']==0,
            'Caller analysis root/payload interpretation changed or waived source identity')
    root=binding['root_snapshot'];require(reader.digest(root['record'])==root['sha256']==review['source_root']['sha256'],
            'Caller build-root snapshot changed')
    manifest=reader.json(root['record']);require(set(manifest['sources'])==set(expected)
            and all(v['sha256']==expected[n]for n,v in manifest['sources'].items()),
            'Caller build-root source domain changed')
    payloads=binding['evidence_payload_references'];require(len(payloads)==7,'Caller evidence payload denominator changed')
    keys=set(expected)|set(payloads);mapping={n:comparison['source_preimages'][n]for n in keys}
    for name,snapshot in mapping.items():
        wanted=expected[name]if name in expected else payloads[name]['sha256']
        require(snapshot['sha256']==wanted==reader.digest(snapshot['record'])
                and comparison['sources'][name]==wanted,'Caller source snapshot no longer binds original bytes')
        if name in payloads:require(reader.digest(name)==wanted,'Caller original evidence payload changed')
    encoded=json.dumps(mapping,sort_keys=True,separators=(',',':')).encode()
    require(hashlib.sha256(encoded).hexdigest()==binding['resolved_mapping_sha256'],
            'Caller interpreted source mapping changed')
    require(comparison['caller_state']['verified']and comparison['caller_state']['normal_retirement']
            and comparison['original_manifest_verified_before_after']
            and all(reader.digest(n)==h for n,h in comparison['sources'].items()),
            'Caller state/model/output/source validation incomplete')
    return binding


def supplement(reader,root,rows,values,plan):
    result=original_supplement(reader,root,rows,values,plan)
    callers=[v for v in values if v['spec']['family']=='caller'];require(len(callers)==4,'All four caller contexts required')
    bindings=[dict(case=v['spec']['case']['id'],validation=v['validation'],binding=caller_binding(reader,v))for v in callers]
    review=checked(reader,REVIEW);diagnosis=checked(reader,review['diagnosis'])
    require(diagnosis['request_rows']==103 and diagnosis['conservative_charges']==143
            and diagnosis['comparison_report_created']is False,'Caller setup refusal changed')
    failed=review['failed_coordinator'];require(reader.digest(failed['record'])==failed['sha256']
            and reader.json(failed['record'])['success']is False,'Failed caller coordinator was replaced')
    cohort=review['stopped_cohort'];require(reader.digest(cohort['record'])==cohort['sha256']
            and reader.json(cohort['record'])['success']is False,'Stopped caller cohort was replaced')
    first=next(v for v in callers if v['spec']['case']['id']==FIRST)
    require(first['coordinator']['record']==P+FIRST+'-coordinator-02.json','Caller successor falsely aliases failed01record')
    row=rows[FIRST];before=review['request_row']
    require(all(row[k]==before[k]for k in ['case_id','report','plan','reserved_games','started_ns','acquisition_sha256'])
            and row['state']=='admitted'and row['validation']==first['validation']['record'],
            'Caller capture/reservation was replaced rather than normally admitted')
    previous=checked(reader,review['previous_review']);refusal=previous['previous_failed_retained_analysis'] if 'previous_failed_retained_analysis'in previous else review['previous_failed_retained_analysis']
    require(reader.digest(refusal['record'])==refusal['sha256']
            and reader.json(refusal['record'])['success']is False,'Retained-analysis snapshot refusal lost')
    controls_name=P+'runtime29-caller-repair-controls-01.json';controls=checked(reader,dict(record=controls_name,sha256=reader.digest(controls_name)))
    require(controls['tests']==34 and controls['Native_source_waivers']is False
            and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Caller recovery controls changed')
    result['caller_source_root_retained_analysis']=dict(verified=True,review=REVIEW,diagnosis=review['diagnosis'],failed_coordinator=failed,stopped_cohort=cohort,retained_analysis_refusal=refusal,controls=dict(record=controls_name,sha256=reader.digest(controls_name)),current_cases=bindings,first_acquisition_preserved=True,additional_original_games=0,additional_reservations=0,Native_source_waivers=False,historical_failure_retired=False)
    return result
