"""Required rendering boundaries for the exact demonstrated player domains.

Original recovery remains independent. A missing natural witness never becomes
an unreachable-function claim. Each execution keeps its actual source phase.
"""
import copy
import hashlib
import json
from pathlib import Path
from coverage_claims import fingerprint
from rendering_pilot import require
from rendering_menu_acceptance import artifact
from rendering_interaction import decode as input_decode
from viewport_cursor import physical_point
from required_boundary_observation import decode as boundary_decode,storage_summary
from required_boundary_source_review import review as review_source_phase
from rendering_required_boundaries import (DOMAIN,INPUTS,PARENT as HISTORY_PARENT,
    check_history,check_progression,resource_journal,check_domain)
from operations_gate import module
from rendering_acceptance import requirements,validate

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/required-boundaries-results-20261110/'
BID='NP.required-boundaries-closeout'
SID='native.required-boundaries-closeout'
GATE='gate.required-boundaries'
PROFILES=('player-default','player-software')
SIZES=([800,600],[640,480],[800,600])
POLICY='docs/native-rendering-required-boundaries.json'
BASIS='research/runtime/rendering-final-decisions-results-20261110/master-admission-01.json'
ORIGINAL='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def local_python_sources():
    import ast
    pending=[ROOT/'tools/required_boundaries_gate.py',ROOT/'tools/check-required-provider-session.py',
             ROOT/'tools/close-rendering-alias-bounded.py',
             ROOT/'tools/join-rendering-normal-acceptance.py'];seen=set()
    while pending:
        p=pending.pop();name=p.relative_to(ROOT).as_posix()
        if name in seen:continue
        seen.add(name)
        for node in ast.walk(ast.parse(p.read_bytes())):
            if (isinstance(node,ast.Call)and isinstance(node.func,ast.Name)and node.func.id=='module'
                    and node.args and isinstance(node.args[0],ast.Constant)and isinstance(node.args[0].value,str)):
                target=ROOT/'tools'/(node.args[0].value+'.py')
                if target.is_file():pending.append(target)
            names=[v.name for v in node.names]if isinstance(node,ast.Import)else([node.module]if isinstance(node,ast.ImportFrom)and node.module else[])
            for n in names:
                for base in (ROOT/'tools',ROOT/'protocols/python'):
                    target=base/Path(*n.split('.')).with_suffix('.py')
                    if target.is_file():pending.append(target)
    return seen


def basis(plan,parent,frozen):
    validate(plan);definitions={(r['configuration'],r['id']):r for r in requirements(plan)}
    reviewed=module('close-rendering-alias-bounded').definition_review(plan,parent)
    amendments={(r['configuration'],r['id']):r for r in(reviewed or{}).get('only_added_candidate_annotations',[])}
    rows=parent['requirements'];keys=[(r['configuration'],r['id'])for r in rows]
    require(parent['success']is True and len(keys)==len(set(keys))==len(definitions)==156
            and set(keys)==set(definitions),'Changed acceptance inventory')
    require(parent['passed_requirements']==sum(r['state']=='passed'for r in rows)==154
            and parent['pending_requirements']==sum(r['state']=='pending'for r in rows)==2,
            'Required-boundary join needs the complete154/156parent')
    require(rows==frozen['requirements'] and parent['scope_amendments']==plan['acceptance_scope_amendments']
            ==frozen['scope_amendments'],'Changed frozen rows or approved overlap amendment')
    require(parent['decisions']=={'supported-player':'pending','original-recovery':'pending'}
            and parent['current_whole_player_proved']is False and parent['current_combined_player_proved']is False
            and parent['supported_player_accepted']is False and parent['original_recovery_complete']is False
            and parent['historical_failures_retired']is False,
            'Unreviewed final verdict or historical-negative disposition')
    for row in rows:
        key=row['configuration'],row['id'];expected=copy.deepcopy(definitions[key])
        if key in amendments:expected['candidate_evidence']=amendments[key]['before']
        require(all(row[k]==v for k,v in expected.items()),'Changed requirement definition')
        require((row['state']=='pending' and not row['admitted_execution_evidence'])if row['id']==GATE
                else row['state']=='passed','Wrong closure frontier')
    require({r['configuration']for r in rows if r['id']==GATE}==set(PROFILES),'Missing boundary profile')
    return rows


def policy_review(policy,plan):
    require(policy['schema']==1 and policy['id']=='demonstrated-required-rendering-boundaries-v1'
            and policy['authorization']=='Use demonstrated finite domains'
            and policy['original_sha256']==ORIGINAL==plan['original_sha256'], 'Changed supported policy/build')
    require(policy['profiles']==list(PROFILES) and policy['journeys']==14 and policy['journey_leaves']==47
            and policy['operations']==21 and policy['technical_unknowns_waived']is False,
            'Policy dropped a required journey/operation or waived a technical unknown')
    require(policy['input_domain']=='exact_retained_acquisitions_inputs_and_durations'
            and policy['arbitrary_maps_saves_duration_supported']is False
            and policy['all_installed_assets_supported']is False
            and policy['supported_outcomes']==['victory','defeat','actual-later-map'],
            'Unsupported campaign/map/save/duration expansion')
    require(policy['demonstrated_maps']==dict(cohort_Quick_Battle=[0,2],fullscreen_Quick_Battle=[1],
        campaign_realm='Celtic',campaign_regions=[1,2],scope='Exact recorded cases only; no arbitrary scenario on a named map.'),
        'Changed finite demonstrated map identity')
    require(policy['configuration']==dict(DEBUG_UseMallocedBackBuffer=False,padding=[0,0],alternate_publication=False,
            original_exclusive_fullscreen=False,qt_toolbar_and_F9_fullscreen=True)
            and policy['formats']==dict(live_World='RGB565',native_primary='RGB565',
                explicit_offscreen=['RGB565','RGB555'],live_RGB555_supported=False,
                global_RGB555_unreachable=False),'Unsupported alternate/padded/live-format claim')
    require(policy['platform']==dict(host='Linux x86_64',guest='Wine PE32',presentation='Qt X11 OpenGL',
            observed_renderer='llvmpipe (LLVM 22.1.8, 256 bits)',hardware_renderer_proved=False,
            Win98_ME_proved=False,physical_driver_equivalence=False,physical_scanout_proved=False,
            Windows_SEH_dispatch_proved=False),'Unsupported platform/device/exception promise')
    require(policy['supplier_boundary']==dict(original_coordinator=True,original_geometry=True,
            original_serializers=True,original_helpers=True,required_route=['Main','Realm','World','Report'],
            all_external_suppliers_recovered=False,all_exceptional_lifetimes_recovered=False),
            'Changed retained original supplier boundary')
    require(policy['natural_callers']==dict(installed_routes=43,optional_unresolved_routes=5,
            global_unreachable_proved=False,natural_overlap_witness=False,
            controlled_overlap_extent=[8,6],unknown_callers_fail_closed=True,
            supported_output='complete_independently_compared_finite_histories'),
            'Natural caller uncertainty or approved overlap scope changed')
    return copy.deepcopy(policy)


def current_sources(reader,q):
    require(q['success']is True and q['sources_stable']is True,'Failed or unstable required proof')
    require(all(reader.digest(n)==h for n,h in q['sources'].items()),'Required proof source drift')


def provider_exits(d):
    report=[];region=[]
    for prepare_tag,exit_tag,output in ((10,13,report),(20,22,region)):
        prepares=[(e,r)for e,r in d['pairs']if e[2]==prepare_tag]
        exits=[(e,r)for e,r in d['pairs']if e[2]==exit_tag]
        require(len(prepares)==len(exits)==2,'Missing populated provider reacquisition/exit')
        for i,((pe,pr),(ee,er))in enumerate(zip(prepares,exits)):
            require(pe[0]<pr[0]<ee[0]<er[0]and pe[6]==pr[6]==ee[6]==er[6]
                    and ee[9]==1 and er[9]==0 and er[43]==er[44],
                    'Wrong provider owner/order/initialized/vector retirement')
            if i:require(exits[i-1][1][0]<pe[0],'Reacquisition precedes prior retirement')
            if prepare_tag==10:
                require(pr[88]==ee[88]==er[88]==1 and all(pr[89:93])
                    and pr[92]==ee[92]and not any(er[89:93])
                    and not any(er[j]for j in(50,52,53,54,55)),
                    'Report bitmap/embedded interface/arrays/callback survives or was never populated')
            else:
                require(all(pr[j]for j in(40,50,52,53,54,55))
                    and not any(er[j]for j in(40,50,52,53,54,55)),
                    'Region bitmap/controls/providers survive or were never populated')
            output.append(dict(owner=pr[6],prepare_entry=pe[0],prepared_return=pr[0],
                exit_entry=ee[0],retired_return=er[0],bitmap=pr[92]if prepare_tag==10 else pr[40],
                initialized_cleared=True,populated_members_retired=True))
    return dict(report=report,region=region,scope='Observed bitmap/member retirement plus unchanged local original-body proofs and native owner census; no Windows exceptional dispatch inference.')


def provider_case(reader,acquisition,comparison,profile):
    a,c=checked(reader,acquisition),checked(reader,comparison)
    current_sources(reader,a);current_sources(reader,c)
    require(c['acquisition']==acquisition and a['configuration']['id']==profile
            and a['configuration']['evidence_mode']=='diagnostic' and a['player_accepted']is False
            and a['original_game_runs']==1 and a['private_wine_stopped']and a['prefix_pool']['returned'],
            'Wrong diagnostic provider role/profile/retirement')
    require(a['semantic_route']['natural_outcome']==1 and a['semantic_route']['later_map_entered']is True
            and a['original_callbacks']['verified']is True,'Missing natural provider route')
    save=a['entry_checkpoint'];saved=save['original_save']
    require(save['name']=='checkpoint-transition-ready-800-02'and save['bytes']==saved['bytes']==357718
        and save['sha256']==saved['sha256']=='8862488014bfb67c34c4e080adb46601d4cd1a55f64042e349f73aef95150818'
        and reader.digest(saved['record'])==saved['sha256']and len(reader.read_bytes(saved['record']))==saved['bytes'],
        'Changed exact original checkpoint input')
    d=boundary_decode(artifact(reader,a,'required-boundaries-v1.bin'))
    lifetimes=module('check-required-provider-session').provider_lifetimes(d)
    require(lifetimes==c['lifetimes'],'Provider analysis differs from raw entry/return bytes')
    exits=provider_exits(d)
    from campaign_map_resources import assess
    resources=json.loads(json.dumps(assess(artifact(reader,a,'map-resource-boundaries.bin'),a['flow'])))
    require(resources==c['world_resources'] and resources['verified'] and resources['world_epochs']==2,
            'Provider World retirement/reacquisition differs')
    g=a['retirement']['graphics_retirement']
    require(g['process_exit']and g['resources_released']and not g['surviving_owners']
            and all(v==0 for v in g['counts'].values()) and a['retirement']['forwarded']==0,
            'Surviving or forwarded required provider owner')
    return dict(profile=profile,acquisition=acquisition,comparison=comparison,lifetimes=lifetimes,
        storage=storage_facts(d),world_resources=resources,normal_quit=True,pixel_admission=False,
        entry_checkpoint=a['entry_checkpoint'],original_callbacks=a['original_callbacks'],populated_exits=exits)


def format_control(reader,ref):
    q=checked(reader,ref);current_sources(reader,q)
    require(q['original_sha256']==ORIGINAL and q['original_code_executed']is True
            and q['original_query_unmodified']is True and q['original_manifest_verified_before_after']is True
            and q['physical_driver_equivalence']is False,'Getter control role changed')
    rows=q['rows'];require(len(rows)==4 and [v['kind']for v in rows]==list(range(4))
        and [v['selector']for v in rows]==[0,0,0,1]
        and [v['masks']for v in rows]==[[0xf800,0x7e0,31]]*3+[[0x7c00,0x3e0,31]]
        and all(v['bits']==16 and v['neighbor_bytes_preserved']and v['query_bytes_unchanged']
                and v['remaining_surfaces']==v['remaining_devices']==0 for v in rows),
        'Original getter/native format facade mismatch')
    return dict(evidence=ref,rows=rows,live_RGB555_supported=False)


def caller_control(reader,bindings,fullscreen):
    q=checked(reader,bindings['primary']);current_sources(reader,q)
    require(q['original_sha256']==ORIGINAL and q['fragment']==dict(success=True,cases=196608,
        fallbacks=3,all_low16_values=True,high_bits_ignored_for_decision=True,arguments_and_stack=True)
        and q['guards']['callers']==54 and q['guards']['admitted_pairs']==64
        and q['guards']['refusal_controls']>=4258,'Missing exact original continuation/guard controls')
    generic=checked(reader,bindings['generic']);current_sources(reader,generic)
    require(generic['original_sha256']==ORIGINAL and generic['comparison']['complete']is True
        and generic['comparison']['equivalent']is True
        and generic['comparison']['counts']['cases']==35280
        and all(v==35280 for v in generic['comparison']['counts'].values())
        and generic['native_result']['failures']==0 and generic['native_result']['entries']==7
        and generic['native_result']['installed_game']is False,
        'Missing complete original/native generic helper state and pixel comparison')
    require(len(bindings['caller_counts'])==2,'Missing caller-count profile')
    observed=[]
    for ref,case in zip(bindings['caller_counts'],fullscreen):
        count=checked(reader,ref)
        require(count['success']is True and count['sources_stable']is True,'Failed caller counting')
        for name,old_hash in count['sources'].items():
            if reader.digest(name)==old_hash:continue
            require(name=='tools/check-required-primary-caller.py','Caller-count source drift')
            old_path=PREFIX+'primary-caller-controls-03-source-'+hashlib.sha256(name.encode()).hexdigest()[:16]+'.bin'
            require(reader.digest(old_path)==old_hash,'Missing old control reporter source')
            old=reader.read_bytes(old_path);needle=b"result=dict(schema=1,kind='required_indexed_primary_caller_controls',success=False,"
            replacement=b"result=dict(schema=1,kind='required_indexed_primary_caller_controls',original_sha256='"+ORIGINAL.encode()+b"',success=False,"
            require(old.count(needle)==1 and old.replace(needle,replacement)==reader.read_bytes(name),
                    'Control-report metadata correction exceeded original hash exposure')
        require(count['acquisition']==case['acquisition'] and count['comparison']==case['comparison']
                and count['worlds']==case['worlds'] and count['independently_compared_pixels']==case['pixels']
                and count['counts']=={'5244023':0,'5244054':0}
                and count['natural_primary_witness']is False and count['natural_clipped_fallback_witness']is False,
                'Changed or fabricated repaired natural witness')
        observed.append(dict(evidence=ref,counts=count['counts'],natural_primary_witness=False))
    failure=checked(reader,bindings['discovery']);require(failure['success']is True
        and failure['original_sha256']==ORIGINAL and failure['observed_return']==0x500477
        and failure['observed_tag']==14,'Historical natural caller discovery changed')
    return dict(original_control=bindings['primary'],generic_control=bindings['generic'],generic_cases=35280,
        installed_counts=observed,discovery=bindings['discovery'],
        repaired_natural_witness=False,failed_World_operands_recovered=False,
        supported_boundary='Exact finite compared inputs/history only. All43handlers remain installed; unknown signatures fail closed. Five optional helper routes and natural overlap retain their unresolved original domains.',
        global_unreachable_proved=False)


def acquisition_budget(reader,bindings):
    requests=bindings['requests'];require(len(requests)==10 and len({v['record']for v in requests})==10,
        'Dropped or duplicate prospective request')
    rows=[checked(reader,v)for v in requests]
    require(sum(q['original_game_runs']for q in rows)==6 and sum(q['success']for q in rows)==4
        and all(q['original_manifest_verified_before_after']is True for q in rows),
        'Original start budget or manifest bookends differ')
    failed=[dict(reference=v,error=q.get('error'),original_starts=q['original_game_runs'])
            for v,q in zip(requests,rows)if not q['success']]
    return dict(requests=10,original_starts=6,successful_acquisitions=4,failed=failed,
        original_operands_missing=True,failed_phases_promoted=False)


def analyze(reader,parent,bindings=None,policy=None):
    bindings=bindings or reader.json(PREFIX+'closeout-bindings-02.json')
    plan=reader.json('docs/native-rendering-closure.json');basis(plan,parent,bindings['parent'])
    policy=policy_review(policy or reader.json(POLICY),plan)
    require(bindings['parent_reference']==policy['evidence']['frozen_parent']
        and bindings['parent']==checked(reader,bindings['parent_reference'])
        and policy['evidence']['selected_domain']==DOMAIN
        and policy['evidence']['closeout_bindings']==PREFIX+'closeout-bindings-02.json',
        'Frozen parent or authored input binding changed')
    domain=check_domain(reader.json(DOMAIN),plan)
    require(len(bindings['fullscreen'])==len(bindings['providers'])==2,'Missing/extra profile cases')
    budget=acquisition_budget(reader,bindings)
    fullscreen=[fullscreen_case(reader,*refs,p)for p,refs in zip(PROFILES,bindings['fullscreen'])]
    require(len(fullscreen)==2 and all(v['renderer']==policy['platform']['observed_renderer']for v in fullscreen),
            'Actual fullscreen provider differs from supported platform')
    providers=[provider_case(reader,*refs,p)for p,refs in zip(PROFILES,bindings['providers'])]
    require(len(providers)==2,'Missing provider profile')
    original_inputs={n:reader.json(path)for n,path in INPUTS.items()}
    from rendering_required_boundaries import check_callers
    old_callers=check_callers(original_inputs['callers'],original_inputs['routing'])
    history=check_history(original_inputs['history'],reader.json(HISTORY_PARENT))
    progression=check_progression(original_inputs['progression']);journals=[]
    for case in original_inputs['progression']['cases']:
        a=checked(reader,case['acquisition'])
        journals.append(resource_journal(reader,a,case['resources']))
    fmt=format_control(reader,bindings['format']);callers=caller_control(reader,bindings,fullscreen)
    local=original_inputs['providers']
    require(local['success']is True and local['platform_file_profile_heap_com_codec_endpoints_supplied']is True
            and local['windows_seh_dispatch']is False and local['physical_driver_equivalence']is False,
            'Original provider fixtures widened to platform recovery')
    answers={
      'SC.alternate-ownership':'Selected DEBUG=false/zero-padding/no-alternate configuration is bound to every observed World and matching active backing/stride. Native owners and historical opt-in alternate repair remain separate.',
      'SC.padded-fullscreen':'Both profiles execute twelve physical Qt toolbar/F9 transitions with independent complete output, affine cursor/hover picking, focus, actual800/640/800 recreation and normal owner retirement. Original exclusive/padded DirectDraw is outside this exact configuration.',
      'SC.natural-callers':'All43routes remain installed. Complete owned-history comparisons and guarded raw-read/copy operations establish the exact demonstrated inputs; no arbitrary timing/input or global unused-route absence is promised. The natural500477 discovery and exact fragment repair are retained without a repaired natural witness. ALIAS09 keeps only its approved8x6controlled mechanism and unresolved natural original domain.',
      'SC.prior-history':'Twelve admitted historical owned-history cases plus two current coherent complete-source fullscreen histories are joined. The historical eight-frame counterexample remains unchanged and supplies no current defect/whole-player claim.',
      'SC.map-save-domain':'The owner-selected exact acquisition/input/save/duration domain preserves all14journeys/47leaves/21operations, victory/defeat and actual Celtic1to2progression. Original checkpoint identity is pinned in both provider routes; installed assets do not grant arbitrary support.',
      'SC.screen-provider-lifetime':'Actual populated Main controls/callbacks, Realm/coordinator callbacks, World epochs and Report labels/button/callback acquire, release and reacquire in both profiles. Existing local body and fault/lifetime gates retain their exact scope. Realm indirect tails, external suppliers and Windows exception dispatch are not relabeled recovered.',
      'SC.rgb555-live':'Live World and primary are RGB565, checked at each original initializer and each observed World. The original getter with production facade independently selects explicit RGB555offscreen storage; no live555sprites/effects or global format1unreachability is promised.',
      'SC.physical-compatibility':'Actual Linux x86_64/Wine PE32/Qt X11/OpenGL with llvmpipe in both requested modes; original coordinator/geometry/serializers/helpers remain explicit suppliers. No hardware,Win98/ME,physical scanout,driver reset or Windows SEH equivalence.'}
    require(set(answers)=={v['id']for v in plan['completion_contract']['scope_decisions']},'Incomplete boundary answers')
    decisions=[dict(id=d['id'],blocking_condition=d['blocking_condition'],close_when=d['close_when'],owner_task='EP07',player_scope_state='resolved',
        original_recovery_state='unresolved',question_state='answered',answer=answers[d['id']],
        technical_remaining='Original domains beyond the exact demonstrated inputs/configuration/suppliers remain unresolved; expansion requires affected output,caller,provider and lifetime evidence.',
        user_judgment_required_now=False)for d in plan['completion_contract']['scope_decisions']]
    return dict(schema=1,kind='required_rendering_boundary_closeout',success=True,gate_ready=True,
        decisions=decisions,policy=policy,support_domain=domain,fullscreen=fullscreen,providers=providers,
        format=fmt,callers=callers,historical_callers=old_callers,owned_history=history,acquisition_budget=budget,
        progression=progression,resource_journals=journals,
        total_new_compared_pixels=sum(v['pixels']for v in fullscreen),
        total_new_compared_Worlds=sum(v['worlds']for v in fullscreen),
        historical_failures_retired=False,current_whole_player_proved=False,
        supported_player_accepted=False,original_recovery_complete=False,
        original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0)


def close_rows(parent,ref,proof):
    validate_proof(proof)
    rows=copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id']==GATE:
            require(row['state']=='pending'and not row['admitted_execution_evidence'],'Boundary already changed')
            p=row['configuration'];case=next(v for v in proof['fullscreen']if v['profile']==p)
            row.update(state='passed',admitted_execution_evidence=[dict(gate_proof=ref,
                configuration=p,fullscreen_acquisition=case['acquisition'],fullscreen_comparison=case['comparison'],
                scope='Exact demonstrated finite domains, eight evidenced player-scope dispositions; no final whole-player or original-recovery acceptance.')])
    require(sum(a!=b for a,b in zip(parent['requirements'],rows))==2,'Changed rows beyond required boundaries')
    return rows


def validate_proof(q):
    ids={'SC.alternate-ownership','SC.padded-fullscreen','SC.natural-callers','SC.prior-history',
         'SC.map-save-domain','SC.screen-provider-lifetime','SC.rgb555-live','SC.physical-compatibility'}
    require(q['success']is True and q['gate_ready']is True and len(q['decisions'])==8
        and {v['id']for v in q['decisions']}==ids
        and all(v['player_scope_state']=='resolved'and v['original_recovery_state']=='unresolved'
                and v['answer']and v['technical_remaining']and v['close_when']
                and v['user_judgment_required_now']is False for v in q['decisions']),
        'Incomplete required-boundary dispositions')
    require(all(q[k]is False for k in ('historical_failures_retired','current_whole_player_proved',
        'supported_player_accepted','original_recovery_complete'))and all(q[k]==0 for k in
        ('original_game_runs','new_native_executions','new_pixel_comparisons')),
        'Retained join widened execution,negative disposition or final acceptance')
    require(len(q['fullscreen'])==len(q['providers'])==2
        and all({v['profile']for v in q[k]}==set(PROFILES)for k in ('fullscreen','providers')),
        'Incomplete or duplicate profile proof')
    require(q['total_new_compared_pixels']==sum(v['pixels']for v in q['fullscreen'])>0
        and q['total_new_compared_Worlds']==sum(v['worlds']for v in q['fullscreen'])>0
        and all(v['input']['verified']and len(v['input']['transitions'])==12
                and len(v['input']['rebuilds'])==2 and v['cleanup']['verified']for v in q['fullscreen'])
        and all(v['normal_quit']and v['pixel_admission']is False for v in q['providers']),
        'Missing input/output/lifetime or diagnostic scope')
    require(q['callers']['repaired_natural_witness']is False and q['callers']['global_unreachable_proved']is False
        and q['callers']['failed_World_operands_recovered']is False
        and q['acquisition_budget']['original_starts']==6 and q['acquisition_budget']['requests']==10
        and q['acquisition_budget']['failed_phases_promoted']is False,
        'Natural witness or failed phase was fabricated')
    return True


def checked(reader,ref):
    require(isinstance(ref,dict) and {'record','sha256'}<=ref.keys()
            and ref['record'].startswith('research/runtime/')
            and '..' not in Path(ref['record']).parts and reader.digest(ref['record'])==ref['sha256'],
            'Changed required-boundary evidence identity')
    return reader.json(ref['record'])


def producer_routes(raw):
    import struct
    from native_pixel_counters import ROUTES
    require(len(raw)==2320,'Truncated producer census')
    words=struct.unpack('<580I',raw)
    require(words[:4]==(0x50504e4d,2,2320,6),'Wrong installed-route census')
    expected={family*32+route for family,labels in enumerate(ROUTES.values())for route in range(len(labels))}
    require(len(expected)==43,'Changed installed route domain')
    rows=[words[i:i+3]for i in range(4,len(words),3)]
    require(all((row[0]==1 and row[2]==0)if i in expected else row==(0,0,0)
                for i,row in enumerate(rows)),'Required handler absent,unknown route or original forwarding')
    return dict(installed_routes=43,exercised_routes=sum(row[1]>0 for row in rows),
        native_calls=sum(row[1]for row in rows),forwarded=0,global_absence_proved=False)


def storage_facts(d):
    require(d['worlds'] and all(r[24:30]==(0,0,0,0,0,0)
            and (r[30]&65535,r[30]>>16,r[31]&65535)==(0xf800,0x7e0,31)
            and r[6]==r[34] and r[9]==r[35] for r in d['worlds']),
            'Unsupported alternate/format/active backing state')
    queries=[(e,r)for e,r in d['pairs'] if e[2]==1]
    require(queries and all(e[6]==r[6]==0x6a4fe0 and r[19]==0 for e,r in queries),
            'Live initializer did not query RGB565 native primary')
    return dict(storage_summary(d),format_query_calls=len(queries),
        every_world_checked=True,active_backing_matched=True,
        supported_format='RGB565 live World and native primary',
        alternate_configuration='DEBUG/UseMallocedBackBuffer false; zero padding and no published alternate backing',
        global_alternate_absence=False,global_rgb555_unreachability=False,
        rgb555_live_supported=False,explicit_offscreen_rgb555_is_separate=True)


def fullscreen_inputs(a,c,rows):
    transitions=a['required_boundaries']['transitions']
    require(len(transitions)==12 and len(a['required_boundaries']['rebuilds'])==2,
            'Incomplete physical fullscreen/recreation schedule')
    compared=c['comparison']['comparisons'];by_world={}
    for paint in compared:
        require(paint['mismatching_pixels']==0 and paint['actual_sha256']==paint['expected_sha256'],
                'Displayed output lacks exact independent comparison')
        by_world.setdefault(paint['world'],[]).append(paint)
    pointers=[v for v in a['actions'] if v['action']=='pointer']
    witnesses=[]
    for i,t in enumerate(transitions):
        size=SIZES[i//4];before,after=t['before'],t['after']
        require(t['size']==size and t['fullscreen'] is (i%2==0) and t['toolbar'] is (i%4==0),
                'Fullscreen domain/schedule changed')
        selected=[p for p in pointers if int(before['host_us'])<=p['host_ns']//1000<=int(after['host_us'])]
        require(len(selected)==1,'Missing/ambiguous physical fullscreen pointer')
        logical=physical_point(selected[0],t['paint'],size)
        require(logical==[size[0]//2,size[1]//2] and t['geometry_after']['focus']['host_focused'],
                'Wrong logical point or lost host focus')
        require([t['paint']['width'],t['paint']['height']]==t['geometry_after']['viewport'][2:]
                and (not t['fullscreen'] or t['geometry_after']['viewport'][2:]==[1280,1024]),
                'Fullscreen extent differs')
        base=rows[before['frame_input_row']-1]
        candidates=[]
        for row in rows[before['frame_input_row']:after['frame_input_row']]:
            cursor,hit,frame=row.get('cursor'),row.get('hit'),row.get('frame')
            if not(cursor and hit and frame and row['kind']==1):continue
            if not(cursor['point']==logical and cursor['desktop'] is False
                    and cursor['poll']>base['cursor']['poll'] and cursor['input_sequence']>base['cursor']['input_sequence']):continue
            if not(hit['slot']==hit['type']==hit['owner']==0 and hit['active']==1 and hit['health']>0):continue
            if frame['nonce']!=base['frame']['nonce'] or row['thread']!=base['thread']:continue
            paints=[p for p in by_world.get(frame['generation'],[]) if p['pixels']==t['paint']['width']*t['paint']['height']]
            if paints:candidates.append(dict(ordinal=row['ordinal'],generation=frame['generation'],
                paint=paints[-1]['paint'],poll=cursor['poll'],logical=logical,hit=hit))
        require(candidates,'No fresh displayed original logical cursor and wizard hover-picking witness')
        if i%2:
            old=transitions[i-1]['geometry_before']
            require(t['geometry_after']['origin']==old['origin'] and t['geometry_after']['viewport']==old['viewport'],
                    'Window geometry not restored')
        witnesses.append(dict(name=t['name'],fullscreen=t['fullscreen'],size=size,witness=candidates[-1]))
    rebuilds=[]
    for old,new,r in zip(SIZES,SIZES[1:],a['required_boundaries']['rebuilds']):
        x,y=r['original_before'],r['original_after']
        require(r['before']==old and r['after']==new and x['thread']==y['thread']
                and x['frame']['nonce']==y['frame']['nonce']
                and x['frame']['generation']<y['frame']['generation']
                and x['high_resolution']==int(old[0]==800) and y['high_resolution']==int(new[0]==800),
                'Resolution recreation lacks original source/input continuity')
        rebuilds.append(dict(before=old,after=new,original_before=x['ordinal'],original_after=y['ordinal']))
    return dict(verified=True,transitions=witnesses,rebuilds=rebuilds,
        picking='Fresh original hover hit-test of the tracked living slot0 wizard after physical affine coordinate delivery; selection-toggle coverage retains its separately admitted scope.')


def fullscreen_case(reader,acquisition,comparison,profile):
    a,c=checked(reader,acquisition),checked(reader,comparison)
    require(c['acquisition']==acquisition and a['success'] and c['success']
            and a['sources_stable'] and c['sources_stable'] and c['proof']['verified'],
            'Incomplete fullscreen acquisition or independent comparison')
    require(a['configuration']['software_flag'] is (profile=='player-software')
            and a['configuration']['skip_intros'] is False and a['configuration']['terrain_lighting']=='native'
            and a['original_game_runs']==1 and a['private_wine_stopped'] and a['prefix_pool']['returned'],
            'Wrong fullscreen profile/input/retirement')
    require(a['proof']['map']['map']==1,'Changed actual fullscreen Quick Battle input')
    _,source_exceptions=review_source_phase(reader,a['sources'])
    routes=producer_routes(artifact(reader,a,'native-pixel-producers.bin'))
    require(a['retirement']['forwarded']==0,'Original pixels forwarded')
    require(c['comparison']['mismatching_pixels']==0 and c['comparison']['remaining_surfaces']==0
            and c['original_reference']['continuous_from_owned_initial_history']
            and c['original_reference']['native_player_pixels_read'] is False
            and c['original_reference']['original_destination_capture_read'] is False,
            'Incomplete or dependent complete-output comparison')
    d=boundary_decode(artifact(reader,a,'required-boundaries-v1.bin'))
    require(d['worlds_observed']==int(a['resident']['completed'])==c['original_reference']['worlds'],
            'Every-World passive observation is not joined to complete compared history')
    cleanup=module('join-rendering-normal-acceptance').cleanup_proof(reader,a)
    require(cleanup['verified'] and cleanup['normal_quit'],'Surviving source/input/cursor/graphics owners')
    facts=fullscreen_inputs(a,c,input_decode(artifact(reader,a,'frame-input.bin'))['rows'])
    return dict(profile=profile,acquisition=acquisition,comparison=comparison,configuration=a['configuration'],
        renderer=a['resident']['renderer'],input=facts,storage=storage_facts(d),cleanup=cleanup,source_applicability=source_exceptions,
        worlds=d['worlds_observed'],pixels=c['comparison']['pixels_compared'],
        independent_paints=c['comparison']['world_presentations'],source_phase_preserved=True,
        producer_routes=routes)
