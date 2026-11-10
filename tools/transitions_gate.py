"""Finite transition acceptance from owned original events and retained native output."""
import copy
import json
import struct
from pathlib import Path
from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_pilot import require, sha
from rendering_menu_acceptance import artifact
from rendering_progression import validate_callbacks
from rendering_interaction import decode as decode_input
from campaign_transition_review import semantic_handoff, native_handoff, referenced_output, callbacks_match
from campaign_results_review import victory_route, map_identity
from campaign_map_resources import assess as original_resources
from rendering_defeat import admit as defeat_admit, defeat_route
from viewport_cursor import SIZES, complete_domains
from operations_gate import module, pair_of, comparison_pixels, local_python_sources as operation_sources

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/transitions-gate-results-20261110/'
BID='NP.transitions-gate-closeout'
SID='native.transitions-gate-closeout'
GATE='gate.transitions'
PROFILES=('player-default','player-software')
EXTENTS={(640,480),(800,600)}
MAP_LEAVES={'journey.map-transitions.'+v for v in ('leave-map','loading','next-map')}|{'journey.battle-results.victory'}
LEAVES=MAP_LEAVES|{'journey.battle-results.defeat','journey.camera-motion.viewport-boundaries'}
SUPPORT=('campaign-victory-next-map-results-20261109/acceptance-02.json',
         'campaign-progression-closeout-results-20261108/acceptance-01.json',
         'rendering-defeat-results-20261108/acceptance-01.json')
SCOPE=('Only both transition gates: real natural victory/Realm/travel/region2 first and resumed World output, '
       'natural death/defeat/fresh Main, and actual Main Preferences800/640/800 recreation in both default/software profiles. '
       'Recheck original callbacks, populated map retirement/reacquisition, native command-surface incarnations, '
       'complete source returns and normal input/graphics retirement, joined to the accepted complete-output/history proof. '
       'Initial checkpoint Load is input only; no saved-map reload, forfeit or component fixture substitutes progression/results. '
       'Retain all execution/compiler/runtime/oracle source phases and failures. No new game, native execution, pixel replay, '
       'same-World resize, QWidget glyph/font/DC equivalence, physical scanout/driver, complete external-object lifetime, '
       'assembled map/RNG recovery, combined-current player, other gate or final-decision promotion.')


def local_python_sources():
    # Reuse the conservative AST walker; seed every additional eager/dynamic analyzer.
    import ast
    pending=[ROOT/n for n in operation_sources()] + [Path(__file__),
        ROOT/'tools/campaign_results_review.py',ROOT/'tools/rendering_defeat.py',
        ROOT/'tools/viewport_cursor.py',ROOT/'tools/close-campaign-map-transitions.py',
        ROOT/'tools/join-rendering-normal-acceptance.py',ROOT/'tools/close-rendering-world-exit.py']
    seen=set()
    while pending:
        path=pending.pop().resolve();name=path.relative_to(ROOT).as_posix()
        if name in seen:continue
        seen.add(name)
        for node in ast.walk(ast.parse(path.read_bytes())):
            names=[v.name for v in node.names]if isinstance(node,ast.Import)else([node.module]if isinstance(node,ast.ImportFrom)and node.module else[])
            for value in names:
                for base in (ROOT/'tools',ROOT/'protocols/python'):
                    p=base/Path(*value.split('.')).with_suffix('.py')
                    if p.is_file():pending.append(p)
    return seen


def normalized(v):return json.loads(json.dumps(v))


def basis(plan,parent,bindings):
    validate(plan);definitions={(v['configuration'],v['id']):v for v in requirements(plan)}
    review=module('close-rendering-alias-bounded').definition_review(plan,parent)
    annotations={(v['configuration'],v['id']):v for v in (review or{}).get('only_added_candidate_annotations',[])}
    rows=parent['requirements'];keys=[(v['configuration'],v['id'])for v in rows]
    require(parent['success']is True and len(rows)==len(set(keys))==len(definitions)==156 and set(keys)==set(definitions),'Changed acceptance inventory')
    require(parent['passed_requirements']==sum(v['state']=='passed'for v in rows)
            and parent['pending_requirements']==sum(v['state']=='pending'for v in rows)
            and parent['passed_requirements']+parent['pending_requirements']==156,'Changed acceptance totals')
    require(parent['decisions']=={'supported-player':'pending','original-recovery':'pending'},'Final decisions changed')
    require(bindings['success']is True and bindings['leaf_ids']==sorted(LEAVES)
            and bindings['scope_amendments']==parent['scope_amendments']==plan['acceptance_scope_amendments'],'Changed scope amendment or transition domain')
    bound={(v['configuration'],v['id']):v for v in bindings['rows']}
    require(len(bindings['rows'])==len(bound)==12 and set(bound)=={(p,n)for p in PROFILES for n in LEAVES},'Missing/duplicate prerequisite')
    for row in rows:
        key=(row['configuration'],row['id']);expected=copy.deepcopy(definitions[key])
        if key in annotations:expected['candidate_evidence']=annotations[key]['before']
        require(all(row[k]==v for k,v in expected.items()),'Changed acceptance owner or condition')
        if row['id']in LEAVES:require(row['state']=='passed'and row==bound[key],'Changed accepted transition prerequisite')
        if row['id']==GATE:require(row['state']=='pending'and row['admitted_execution_evidence']==[],'Transition gate already changed')
    return bound


class Join:
    def __init__(self,reader,bindings):
        self.reader=reader;self.records={};self.phases={};self.cache={};self.identities={}
        admission=self.checked(bindings['complete_output_admission']);self.ph=self.checked(bindings['complete_output_proof'])
        require(admission['proof']==bindings['complete_output_proof'] and admission['kind']=='pixels_history_gate_admission'
                and self.ph['kind']=='retained_pixels_history_gate_proof' and self.ph['sources_stable']is True,
                'Wrong complete-output/history proof')
        require(self.ph['sources'] and all(reader.digest(n)==h for n,h in self.ph['sources'].items()),'Complete-output/history host analysis changed')
        self.supports=[self.checked(bindings['supports'][n])for n in SUPPORT]

    def checked(self,ref):
        require(isinstance(ref,dict)and {'record','sha256'}<=ref.keys()and ref['record'].startswith('research/runtime/')
                and '..'not in Path(ref['record']).parts and self.reader.digest(ref['record'])==ref['sha256'],'Retained report identity changed')
        q=self.reader.json(ref['record']);require(q['success']is True,'Failed retained report')
        self.records[ref['record']]=ref
        sources=q.get('sources',q.get('analysis_sources',{}))
        self.phases[ref['record']]=dict(recorded_sources=sources,compiler_sources=q.get('compiler_sources',{}),
            current_drift={n:dict(recorded=h,current=self.reader.digest(n)if(ROOT/n).is_file()else None)for n,h in sources.items()if not(ROOT/n).is_file()or self.reader.digest(n)!=h},current_native_execution_claimed=False)
        return q

    def pair(self,pair,profile,size):
        key=pair['comparison']['record']
        if key in self.cache:
            case=self.cache[key];require(case['pair']==pair and case['profile']==profile and case['size']==size,'Mixed transition pair')
            return case
        a=self.checked(pair['acquisition']);c=self.checked(pair['comparison']);cfg=a['configuration']
        require(cfg['software_flag']is(profile==PROFILES[1]) and c['acquisition']==pair['acquisition']
                and a['sources_stable']is True and a['private_wine_stopped']is True and a['exit_code']==0
                and a['original_manifest_verified_before_after']is True and a['original_pixels_used_as_native_inputs']is False
                and not a.get('forced_private_prefix_stop',False),'Wrong profile, dependent output or incomplete retirement')
        identity=a['session_identity'];require(identity and identity not in self.identities,'Reused transition original session')
        self.identities[identity]=pair
        pixels=comparison_pixels(c)
        matches=[v for v in self.ph['additional_presented_cases']if v['pair']==pair]
        require(len(matches)==1,'Missing/duplicate accepted complete history case');history=matches[0]
        require(history['configuration']==profile and history['size']==size and history['identity']==identity
                and history['actual_world_extents']==c['world_domain']['extents']
                and history['command_history']['sha256']==c['captured_command_stream_sha256'], 'Mixed output/history source binding')
        require(history['completed_output']==dict(swaps=pixels['presentations'],world_swaps=pixels['world_presentations'],pixels=pixels['pixels']), 'Complete native swap denominator differs')
        raw=module('close-campaign-map-transitions').command_history(self.reader,a)
        require(sha(raw)==history['command_history']['sha256'],'Native commands differ from accepted full output')
        lifetimes=module('close-rendering-world-exit').imported_lifetimes(raw);n=len(lifetimes['imports'])
        require(n==history['source_generations']==history['command_history']['imports']==c['original_reference']['worlds']
                and lifetimes['nonce']==history['command_history']['nonce'] and lifetimes['command_end']==history['command_history']['command_end'], 'Incomplete native source incarnation history')
        cleanup=module('join-rendering-normal-acceptance').cleanup_proof(self.reader,a)
        require(cleanup['verified']is cleanup['normal_quit']is True and cleanup['pending_jobs']==cleanup['graphics_owners']==0
                and cleanup['source_imports']==cleanup['source_returns']==n,'Incomplete normal input/source/graphics retirement')
        g=a['retirement']['graphics_retirement']
        require(g['process_exit']is g['resources_released']is True and not g['surviving_owners']and all(v==0 for v in g['counts'].values()),'Graphics owner survived')
        terminal=module('close-rendering-world-exit').terminal_source(artifact(self.reader,a,'world-continuous.bin'),artifact(self.reader,a,'world-terminal-0001.bin'),lifetimes,cleanup_thread(a))
        require(terminal['native_owned_slots']==0,'Source slot survived')
        resources=module('join-rendering-normal-acceptance').resource_history(raw)
        provider=a['resident']['renderer'];require(provider and(not cfg['software_flag']or provider.lower().startswith(('llvmpipe','softpipe'))),'Actual software provider missing')
        out=dict(verified=True,pair=pair,profile=profile,size=size,identity=identity,provider=provider,
                 completed_output=history['completed_output'],command_history=history['command_history'],
                 cleanup=cleanup,terminal_source=terminal,remaining_surfaces=resources['remaining_surfaces'],
                 source_generations=n,current_native_execution=False)
        case=dict(out,a=a,c=c,raw=raw,lifetimes=lifetimes,resources=resources)
        self.cache[key]=case
        return case

    def progression(self,pair,profile,size):
        x=self.pair(pair,profile,size);a,c=x['a'],x['c'];flow=a['flow']
        require(module('close-campaign-map-transitions').domain(a,c,pair['acquisition'])==(profile,size,x['source_generations']),'Progression domain differs')
        events=artifact(self.reader,a,'events.bin');route=victory_route(flow,events);original=semantic_handoff(flow,events)
        require(validate_callbacks(events,flow)==a['original_callbacks']==c['original_menu_actions'],'Original callback projection differs')
        prep=self.checked(a['preparation']);identity=map_identity(a,prep,route)
        resources=original_resources(artifact(self.reader,a,'map-resource-boundaries.bin'),flow)
        resource_join(resources,original,c)
        later=flow['worlds'][1]
        require(later['entry_mode']=='campaign-travel' and later['region']==2 and later['realm']=='Celtic','Saved-map reload cannot substitute actual later-map progression')
        native=native_handoff(x['raw'],later['world'],size,prior_generation=later['after_world'])
        output=referenced_output(c,flow,sha(x['raw']))
        victory=only_case(self.supports[0],pair);maps=only_case(self.supports[1],pair)
        require(route==victory['victory']and identity==victory['map_identity']and original==maps['original']
                and normalized(resources)==victory['resources']==maps['resources']and native==victory['native_handoff']==maps['native']
                and output==maps['output']and x['cleanup']==victory['cleanup']==maps['cleanup']
                and x['terminal_source']==victory['terminal_source']==maps['terminal_source'],'Changed accepted original/native transition proof')
        later_output(c,flow,victory['later_output'])
        old,new=native['previous_world'],native['first_later_world']
        return dict(victory=route,map_identity=identity,populated_resources_verified=True,original_world_pop=original['original_world_pop'],
                    native_handoff=native,later_output=victory['later_output'],retired_before_rebind=old['deleted']<new['created'])

    def defeat(self,pair,profile,size):
        x=self.pair(pair,profile,size);a,c=x['a'],x['c'];n=defeat_admit(a,c,pair,profile,size)
        route=defeat_route(a['flow'],json.loads(artifact(self.reader,a,'progression-input-0.json')),artifact(self.reader,a,'events.bin'),artifact(self.reader,a,'gameplay-events.bin'))
        require(n==x['source_generations']and callbacks_match(route['callbacks'],a['original_callbacks'],a['flow'])and callbacks_match(route['callbacks'],c['original_menu_actions'],a['flow'])
                and route['semantic']==a['semantic_route']==c['semantic_route'],'Defeat callback projection differs')
        accepted=only_case(self.supports[2],pair)
        require(callbacks_match(route['callbacks'],accepted['route']['callbacks'],a['flow'])and {k:v for k,v in route.items()if k!='callbacks'}=={k:v for k,v in accepted['route'].items()if k!='callbacks'}and x['cleanup']==accepted['cleanup']and x['terminal_source']==accepted['terminal_source']
                and x['resources']==accepted['resources'],'Changed natural defeat/retirement proof')
        require(self.reader.digest(accepted['tested_binary']['record'])==accepted['tested_binary']['sha256'],'Tested defeat binary differs')
        return dict(route=route,populated_provider_lifetime_gap=accepted['populated_provider_lifetime_gap'])

    def resize(self,pair,profile):
        x=self.pair(pair,profile,[800,600]);a,c=x['a'],x['c']
        require(a['configuration']['actual_size_schedule']==SIZES and a['configuration']['skip_intros']is False
                and a['configuration']['terrain_lighting']=='native' and not a['configuration'].get('diagnostic_only',False)
                and ('--software-rendering'in a['command'])is(profile==PROFILES[1]),'Wrong actual recreation configuration')
        require(c['proof']['verified']is c['proof']['normal_quit']is True and c['proof']['mismatching_pixels']==0,'Incomplete recreation output')
        require(c['proof']['software']is(profile==PROFILES[1]) and c['proof']['source_generations']==x['source_generations'],'Wrong recreation proof profile/history')
        rows=decode_input(artifact(self.reader,a,'frame-input.bin'))['rows']
        for r in a['viewport_cursor']['rebuilds']:
            for name in ('original_before','original_after'):
                v=r[name];require(rows[v['ordinal']-1]==v,'Original recreation observation differs from raw bytes')
            for name in ('preferences','returned'):
                state=r[name];raw=artifact(self.reader,a,state['wire_path']);w=struct.unpack_from('<40I',raw)
                require(len(raw)==w[3]and list(w[32:40])==[int(state[k])for k in ('sequence','generation','screen','ready','ack','status','thread','handoff')],'Original recreation menu snapshot differs')
                if state['screen']==10:require(list(struct.unpack_from('<7i',raw,44724))==state['preferences'],'Original Preferences selector differs')
        rebuilds=resize_rebuilds(a,c,x['lifetimes'])
        require(rebuilds==c['proof']['rebuilds']and x['cleanup']==c['proof']['cleanup'],'Changed original/native recreation proof')
        return dict(schedule=SIZES,same_original_process=True,same_world=False,rebuilds=rebuilds)


def cleanup_thread(a):
    return a['flow']['states'][0]['thread']if 'flow'in a else a['viewport_cursor']['rebuilds'][0]['original_before']['thread']


def only_case(report,pair):
    matches=[v for v in report['cases']if (v.get('pair')or{k:v[k]for k in ('acquisition','comparison')})==pair]
    require(len(matches)==1 and matches[0]['verified']is True,'Missing/duplicate accepted semantic case')
    return matches[0]


def resource_join(resources,original,c):
    first,later=c['original_menu_actions']['original_world_epochs'];post=resources['retirements'][0]['after'];pop=original['original_world_pop']
    require(resources['world_epochs']==2 and resources['initial_resources']['menu_event']+1==first['first_sequence']
            and resources['later_resources']['menu_event']+1==later['first_sequence']
            and post['menu_event']+2==pop[0]and post['thread']==pop[2]
            and [post['stack_owner'],post['depth']]==pop[11:13],'Original resource retirement not joined to World pop/reacquisition')


def later_output(c,flow,proof):
    old,later=flow['worlds'];baseline=later['after_world'];witness=later['world']
    rows=[r for r in c['comparison']['comparisons']if int(r['world'])>baseline]
    resumed=[r for r in rows if int(r['world'])>witness]
    require(proof['verified']is True and old['world']<baseline<witness and rows and resumed
            and rows[0]==proof['first_displayed']and resumed[0]==proof['resumed']
            and proof['after_world']==baseline and proof['observed_identity_world']==witness
            and proof['compared_swaps']==len(rows)and proof['pixels']==sum(r['pixels']for r in rows)
            and proof['captured_world_generations']==sorted({int(r['world'])for r in rows})
            and int(rows[0]['world'])<=witness and any(int(r['world'])==witness and r['paint']==later['paint']for r in rows),
            'First or resumed actual later-map output missing')


def resize_rebuilds(a,c,lifetimes):
    rebuilt=[];rs=a['viewport_cursor']['rebuilds'];require(len(rs)==2,'Missing actual display rebuild')
    world={v['generation']:[v['width'],v['height']]for v in c['world_domain']['worlds']};pixels=c['comparison']['comparisons']
    for old,new,r in zip(SIZES[:-1],SIZES[1:],rs,strict=True):
        x,y=r['original_before'],r['original_after'];paint=r['confirmed_paint'];p=int(paint['paint']);generation=y['frame']['generation']
        require(r['before']==old and r['after']==new and r['same_original_process']is True and r['same_world']is False
                and x['thread']==y['thread']and int(x['frame']['nonce'])==int(y['frame']['nonce'])==lifetimes['nonce']
                and x['frame']['generation']<generation and x['high_resolution']==int(old[0]==800)
                and y['high_resolution']==int(new[0]==800)and [paint['frame_width'],paint['frame_height']]==new
                and r['preferences']['screen']==10 and r['returned']['screen']==3
                and r['returned']['sequence']>r['preferences']['sequence']and r['returned']['ack']==r['preferences']['ack']+1
                and world[generation]==new and world[x['frame']['generation']]==old
                and x['frame']['generation']<int(paint['world'])<=generation and world[int(paint['world'])]==new
                and any(v['paint']==p and int(v['world'])==int(paint['world']) and v['actual_sha256']==paint['capture_body_sha256']for v in pixels),'Unbound original Preferences/selector/source/native recreation')
        before=lifetimes['imports'][x['frame']['generation']-1];after=lifetimes['imports'][generation-1]
        require(before['created_sequence']!=after['created_sequence']and before['deleted_sequence']<after['created_sequence']
                and lifetimes['imports'][int(paint['world'])-1]['created_sequence']==after['created_sequence'],'Old native World storage survives recreation')
        rebuilt.append(dict(verified=True,before=old,after=new,first_generation=generation,paint=paint['paint']))
    return rebuilt


def analyze(reader,parent,bindings=None):
    bindings=bindings or reader.json(PREFIX+'bindings-01.json');bound=basis(reader.json('docs/native-rendering-closure.json'),parent,bindings);join=Join(reader,bindings)
    linked=[];cases={}
    for (profile,leaf),row in sorted(bound.items()):
        attachments=row['admitted_execution_evidence'];expected=1 if leaf=='journey.camera-motion.viewport-boundaries'else 2
        require(len(attachments)==expected,'Missing/extra transition evidence domain');extents=set();keys=[]
        for attachment in attachments:
            pair=pair_of(attachment);sizes=attachment['actual_world_extents'];size=sizes[0]
            if leaf=='journey.camera-motion.viewport-boundaries':
                require({tuple(v)for v in sizes}==EXTENTS and attachment['same_original_process']is True and attachment['same_world']is False,'Changed actual recreation scope')
                fact=join.resize(pair,profile);kind='recreation';extents.update(map(tuple,sizes))
            else:
                require(len(sizes)==1 and tuple(size)in EXTENTS,'Unknown transition extent');extents.add(tuple(size))
                kind='progression'if leaf in MAP_LEAVES else'defeat'
                key=pair['comparison']['record']
                fact=cases[key]['transition']if key in cases else(join.progression(pair,profile,size)if kind=='progression'else join.defeat(pair,profile,size))
            key=pair['comparison']['record'];x=join.cache[key]
            case={k:v for k,v in x.items()if k not in ('a','c','raw','lifetimes','resources')};case.update(kind=kind,transition=fact)
            require(key not in cases or cases[key]==case,'Transition case reused inconsistently');cases[key]=case;keys.append(key)
        require(extents==EXTENTS,'Missing actual640/800 transition domain')
        linked.append(dict(configuration=profile,requirement=leaf,comparisons=keys))
    complete_domains([x['c']['proof']for x in join.cache.values()if 'viewport_cursor'in x['a']])
    require(len(cases)==10 and len(join.identities)==10 and len(linked)==12,'Incomplete transition cohort')
    for kind,count in [('progression',4),('defeat',4),('recreation',2)]:require(sum(v['kind']==kind for v in cases.values())==count,'Wrong transition outcome count')
    return dict(schema=1,kind='retained_transition_gate_proof',success=True,transition_cells=12,linked_requirements=linked,cases=list(cases.values()),
        complete_output_admission=bindings['complete_output_admission'],complete_output_proof=bindings['complete_output_proof'],retained_records=list(join.records.values()),
        retained_execution_source_phases=join.phases,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,current_whole_player_proved=False,scope=SCOPE)


def close_rows(parent,proof_ref,proof):
    require(proof['success']is True and proof['transition_cells']==12 and len(proof['linked_requirements'])==12
            and {(v['configuration'],v['requirement'])for v in proof['linked_requirements']}=={(p,n)for p in PROFILES for n in LEAVES}
            and len(proof['cases'])==10 and len({v['identity']for v in proof['cases']})==10,'Incomplete transition gate proof')
    rows=copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id']==GATE:
            require(row['state']=='pending'and not row['admitted_execution_evidence'],'Transition gate already changed')
            row.update(state='passed',admitted_execution_evidence=[dict(proof=proof_ref,actual_world_extents=[[640,480],[800,600]],
                original_sessions=[v['identity']for v in proof['cases']if v['profile']==row['configuration']],reason=SCOPE)])
    require(sum(v['state']=='passed'for v in rows)==parent['passed_requirements']+2,'Exceeded exactly two transition gates')
    return rows
