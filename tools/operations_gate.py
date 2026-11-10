"""Join finite admitted operation proofs without widening their execution scopes."""
import ast
import copy
from functools import lru_cache
import importlib.util
import json
from pathlib import Path
import sys

from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_pilot import require
from rendering_menu_acceptance import artifact
from fault_lifetime_gate import Join as FaultJoin

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/operations-gate-results-20261109/'
BID = 'NP.operations-gate-closeout'
SID = 'native.operations-gate-closeout'
GATE = 'gate.operations'
PROFILES = ('player-default', 'player-software')
SIZES = {(640, 480), (800, 600)}
LEAVES = frozenset('operation.'+v for v in (
    'copy-opaque', 'copy-keyed', 'copy-clipped', 'copy-stretched', 'copy-alias-overlap',
    'fill-full', 'fill-clipped', 'partial-write', 'cpu-write-readback', 'text-font-dc-lifetime',
    'image-cropping', 'sprite-indexed', 'sprite-direct', 'sprite-auxiliary', 'palette-remap',
    'palette-cycling', 'destination-dependency', 'flags-and-return-values',
    'surface-loss-restore', 'transition-recovery', 'failure-cleanup'))
FAULTS = frozenset('operation.'+v for v in ('surface-loss-restore','transition-recovery','failure-cleanup'))
SUPPORT = {
    'copy-opaque': 'rendering-copy-results-20261106/closeout-analysis-01.json',
    'image-cropping': 'rendering-copy-results-20261106/closeout-analysis-01.json',
    'copy-keyed': 'rendering-four-row-results-20261107/closeout-analysis-02.json',
    'palette-remap': 'rendering-four-row-results-20261107/closeout-analysis-02.json',
    'copy-clipped': 'rendering-clipped-operations-results-20261107/master-admission-02.json',
    'copy-stretched': 'rendering-copy-natural-results-20261108/dashboard-01.json',
    'fill-full': 'rendering-fill-operations-results-20261107/closeout-integrated-05.json',
    'fill-clipped': 'rendering-clipped-fill-results-20261108/acceptance-01.json',
    'partial-write': 'rendering-functional-ten-results-20261107/closeout-analysis-01.json',
    'cpu-write-readback': 'rendering-operations-results-20261107/closeout-analysis-final-02.json',
    'text-font-dc-lifetime': 'font-dc-lifetime-results-20261108/acceptance-01.json',
    'sprite-indexed': 'rendering-destination-results-20261106/closeout-analysis-05.json',
    'destination-dependency': 'rendering-destination-results-20261106/closeout-analysis-05.json',
    'sprite-direct': 'rendering-blend-direct-results-20261106/closeout-analysis-02.json',
    'sprite-auxiliary': 'rendering-five-results-20261107/closeout-analysis-03.json',
    'palette-cycling': 'palette-cycling-results-20261108/acceptance-01.json',
    'flags-and-return-values': 'caller-state-installed-results-20261108/admission-01.json',
}
SCOPE = ('Finite conjunction of all 21 admitted operation requirements in each of the default and software profiles. '
         'Recheck exact row provenance, profile/extent/identity, independent output and operation-specific witnesses, '
         'normal or declared failed/fresh retirement, and the owner-approved controlled opaque-overlap exception. '
         'Retain every original execution/source/compiler/oracle phase, failure and narrower output/history limit. '
         'No new game, native execution, pixel replay, whole-player/current-combined execution, physical driver/reset, '
         'natural alias reachability, other gate or final-decision promotion.')


@lru_cache(maxsize=None)
def module(name):
    spec = importlib.util.spec_from_file_location('operations_'+name.replace('-','_'), ROOT/'tools'/(name+'.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def modules():
    return [module(n) for n in (
        'close-rendering-opaque-cropped-copy', 'close-rendering-keyed-palette',
        'close-rendering-blend-direct', 'close-rendering-animation-auxiliary',
        'close-rendering-operations', 'close-rendering-full-fill', 'check-rendering-clipped-fill',
        'close-rendering-natural-stretch', 'close-rendering-caller-state',
        'close-rendering-alias-bounded', 'check-native-surface-overlap')]


def local_python_sources():
    """Conservative local import/initialization closure, independent of process state."""
    pending=[Path(__file__),*(Path(m.__file__) for m in modules())]
    paths=set()
    while pending:
        path=pending.pop().resolve()
        if not path.is_relative_to(ROOT) or path.suffix!='.py' or not path.is_file():continue
        name=path.relative_to(ROOT).as_posix()
        if name in paths:continue
        paths.add(name)
        tree=ast.parse(path.read_bytes())
        for node in ast.walk(tree):
            imported=[]
            if isinstance(node,ast.Import):imported=[v.name for v in node.names]
            elif isinstance(node,ast.ImportFrom)and node.module:imported=[node.module]
            for value in imported:
                relative=Path(*value.split('.')).with_suffix('.py')
                for base in (ROOT/'tools',ROOT/'protocols/python'):
                    candidate=base/relative
                    if candidate.is_file():pending.append(candidate)
            if isinstance(node,ast.Call)and node.args and isinstance(node.args[0],ast.Constant)and isinstance(node.args[0].value,str):
                func=node.func
                if (isinstance(func,ast.Name)and func.id=='module')or(isinstance(func,ast.Attribute)and func.attr=='module'):
                    pending.append(ROOT/'tools'/(node.args[0].value+'.py'))
                    if len(node.args)>1 and isinstance(node.args[1],ast.Constant)and isinstance(node.args[1].value,str):
                        pending.append(ROOT/node.args[1].value)
    return paths


def basis(plan, parent, bindings):
    validate(plan)
    definitions = {(v['configuration'],v['id']):v for v in requirements(plan)}
    review=module('close-rendering-alias-bounded').definition_review(plan,parent)
    annotations={(v['configuration'],v['id']):v for v in (review or {}).get('only_added_candidate_annotations',[])}
    rows = parent['requirements']; keys = [(v['configuration'],v['id']) for v in rows]
    require(parent['success'] is True and len(rows)==len(set(keys))==len(definitions)==156
            and set(keys)==set(definitions), 'Missing, duplicate or changed acceptance identity')
    require(parent['passed_requirements']==sum(v['state']=='passed' for v in rows)
            and parent['pending_requirements']==sum(v['state']=='pending' for v in rows)
            and parent['passed_requirements']+parent['pending_requirements']==156, 'Parent counts differ')
    require(parent['decisions']=={'supported-player':'pending','original-recovery':'pending'}, 'Final decisions changed')
    require(bindings['success'] is True and bindings['operation_ids']==sorted(LEAVES)
            and len(bindings['rows'])==42, 'Frozen operation binding domain differs')
    bound={(v['row']['configuration'],v['row']['id']):v for v in bindings['rows']}
    require(len(bound)==42 and set(bound)=={(p,n) for p in PROFILES for n in LEAVES}, 'Duplicate/missing operation binding')
    require(set(annotations)=={(p,'operation.copy-alias-overlap')for p in PROFILES},'Unreviewed candidate annotations')
    for key,value in annotations.items():
        require(value==dict(configuration=key[0],id=key[1],before=bound[key]['row']['candidate_evidence'],after=definitions[key]['candidate_evidence']),
                'Changed candidate annotation boundary')
    require(parent.get('scope_amendments',[])==plan.get('acceptance_scope_amendments',[])==bindings['scope_amendments'],
            'Approved scope amendments changed')
    for row in rows:
        expected=copy.deepcopy(definitions[row['configuration'],row['id']])
        if (row['configuration'],row['id'])in annotations:
            annotation=annotations[row['configuration'],row['id']]
            require(annotation['after']==expected['candidate_evidence'],'Changed reviewed candidate annotation')
            expected['candidate_evidence']=annotation['before']
        require(all(row[k]==v for k,v in expected.items()), 'Acceptance condition or owner changed')
        if row['id'] in LEAVES:
            require(row['state']=='passed' and row==bound[row['configuration'],row['id']]['row'],
                    'Operation is missing or differs from its accepted source phase')
        elif row['id']==GATE: require(row['state']=='pending' and row['admitted_execution_evidence']==[], 'Operations gate already changed')
    return bound


def pair_of(value):
    value=value.get('execution',value)
    require({'acquisition','comparison'}<=value.keys(), 'Operation attachment lacks an execution pair')
    return {k:value[k] for k in ('acquisition','comparison')}


def case_pair(case):
    return case.get('pair',case.get('execution')) or {k:case[k] for k in ('acquisition','comparison')}


def case_domain(case):
    if 'domain' in case: return case['domain'][0],case['domain'][1]
    require(type(case['software']) is bool,'Case software profile is not boolean')
    return PROFILES[int(case['software'])],case.get('actual_extent',case.get('size'))


def comparison_pixels(c):
    """Only the existing complete-viewport comparison family enters this branch."""
    require(c['success'] is True and c['sources_stable'] is True
            and c['comparison_processes_stopped'] is True and c['original_manifest_verified_before_after'] is True
            and c['original_pixels_used_as_native_inputs'] is False, 'Incomplete independent viewport comparison')
    ref=c['original_reference'];p=c['comparison'];rows=p['comparisons']
    require(ref['native_player_pixels_read'] is False and ref['original_destination_capture_read'] is False,
            'Dependent output oracle')
    if c['kind']!='public_native_caller_state_comparison':
        require(ref['continuous_from_owned_initial_history'] is True,'Independent owned initial history missing')
    require(p['success'] is True and p['mismatching_pixels']==0 and p['remaining_surfaces']==0 and rows
            and p['pixels_compared']==sum(v['pixels'] for v in rows)
            and p['world_presentations']==sum(int(v['world'])>0 for v in rows)>0
            and all(v['pixels']>0 and v['mismatching_pixels']==0 and v['actual_sha256']==v['expected_sha256'] for v in rows),
            'Incomplete, inconsistent or mismatching viewport output')
    domain=c['world_domain']
    require(domain['verified'] is True and domain['worlds'] and domain['extents']
            and {tuple([v['width'],v['height']]) for v in domain['worlds']}=={tuple(v)for v in domain['extents']},
            'World extent/output domain differs')
    return dict(kind='complete-retained-viewport',pixels=p['pixels_compared'],presentations=len(rows),
                world_presentations=p['world_presentations'],extents=domain['extents'])


class Join:
    def __init__(self, reader):
        self.reader=reader;self.records={};self.phases={};self.outputs={};self.identities={};self.case_cache={}

    def checked(self, ref):
        require(isinstance(ref,dict) and {'record','sha256'}<=ref.keys() and
                ref['record'].startswith('research/runtime/') and '..' not in Path(ref['record']).parts
                and self.reader.digest(ref['record'])==ref['sha256'],'Retained report identity changed')
        q=self.reader.json(ref['record']);require(q['success'] is True,'Failed retained proof')
        self.records[ref['record']]=dict(record=ref['record'],sha256=ref['sha256'])
        if ref['record'] not in self.phases:
            sources=q.get('sources',q.get('analysis_sources',{}))
            self.phases[ref['record']]=dict(recorded_sources=sources,
                current_drift={n:dict(recorded=h,current=self.reader.digest(n) if (ROOT/n).is_file() else None)
                    for n,h in sources.items()if not (ROOT/n).is_file() or self.reader.digest(n)!=h},
                compiler_sources=q.get('compiler_sources',{}),current_native_execution_claimed=False)
        return q

    def normal_pair(self,pair,profile,size,leaf):
        key=pair['comparison']['record']
        if key in self.outputs:
            cached=self.outputs[key]
            require(cached['pair']==pair and cached['profile']==profile and cached['size']==size,'Mixed execution binding')
            return cached
        a=self.checked(pair['acquisition']);c=self.checked(pair['comparison']);cfg=a['configuration']
        software=cfg.get('software_flag',cfg.get('software'))
        require(type(software) is bool and software is (profile==PROFILES[1]), 'Actual profile differs')
        require(a['sources_stable'] is True and a['private_wine_stopped'] is True
                and a['original_manifest_verified_before_after'] is True and a['original_pixels_used_as_native_inputs'] is False
                and not a.get('forced_private_prefix_stop',False) and not cfg.get('compiled_test_driver_invoked',False),
                'Unretired, dependent or compiled-driver acquisition')
        require(c['acquisition']==pair['acquisition'],'Comparison belongs to another acquisition')
        retired=a['retirement'];g=retired['graphics_retirement']
        require(retired['state']=='completed' and g['process_exit'] is True and g['resources_released'] is True
                and not g['surviving_owners'] and all(v==0 for v in g['counts'].values()), 'Graphics or process owners survived')
        identity=a.get('session_identity',a.get('session'))
        require(isinstance(identity,str) and identity and
                (identity not in self.identities or self.identities[identity]==pair['acquisition']), 'Reused/missing original session identity')
        self.identities[identity]=pair['acquisition']
        resident=a.get('resident')
        if resident is None:
            require(a['kind']=='native_grimoire_copy_branch_acquisition','Unknown missing resident report')
            resident=json.loads(artifact(self.reader,a,'resident-world-report.json'))
        renderer=resident['renderer']
        require(isinstance(renderer,str) and renderer and (not software or renderer.lower().startswith(('llvmpipe','softpipe'))),
                'Actual software provider missing')
        if c['kind']=='natural_stretch_complete_output_proof':
            require(leaf=='operation.copy-stretched' and cfg['actual_size']==size
                    and c['sources_stable'] is True and c['natural_stretch_calls']>0,
                    'Wrong natural stretch proof')
            require(c['configuration']['actual_size']==size and c['proofs']
                    and all(v['target_size']==size for v in c['proofs']), 'Stretch destination extent differs')
            output=dict(kind='complete-selected-stretch-planes',extents=[size],calls=c['natural_stretch_calls'])
        elif c['kind']=='public_native_clipping_comparison':
            require(leaf in ('operation.copy-clipped','operation.fill-clipped'),'Wrong bounded clipping supplier')
            clipping(c,size)
            output=dict(kind='complete-selected-clipping-planes',extents=[size],witnesses=len(c['case']['raw_pixel_witnesses']))
        else:
            require(c['kind'] in ('public_native_normal_comparison','public_native_caller_state_comparison'), 'Unknown output proof family')
            require(a['exit_code']==0 and resident['normal_quit_retired'] is True and not resident['pending'],
                    'Normal execution did not retire normally')
            output=comparison_pixels(c);require(output['extents']==[size],'Attachment extent differs from actual Worlds')
        result=dict(pair=pair,profile=profile,size=size,identity=identity,output=output)
        self.outputs[key]=result
        return result


def clipping(q,size):
    require(q['success'] is True and q['analysis_completed'] is True and q['analysis_sources_stable'] is True
            and q['complete_output_verified'] is True and q['native_clip_regions_verified'] is True
            and q['positive_clipped_calls']>0,'Incomplete clipping analysis')
    c=q['case'];p=c['proof'];w=c['raw_pixel_witnesses']
    require(c['actual_size']==size and c['source_domain']['verified'] and p['primary_identity_verified']
            and p['normal_lifetime_verified'] and p['actual_clip_region_verified'] and p['caller_clip_plan_verified']
            and c['region_scope']['owner_records_complete'] and c['region_bound_callers']>0,
            'Incomplete clipping ownership/input')
    require(len(w)>=2 and {v['caller'][15] for v in w}=={1,2}
            and all(v['actual_clip_region_snapshot_retained'] and v['clip_plan'] and v['changed_words']>0
                    and v['preserved_exterior_words']>0 and v['full_input_and_actual_output_verified'] for v in w),
            'Incomplete CPU/GPU clipping output/exterior')


def witnesses(leaf,c):
    """Recheck retained semantic witnesses; no original/native code executes."""
    name=leaf.removeprefix('operation.')
    if name in ('copy-opaque','image-cropping'):
        require(c['proof']['verified'] is True,'Missing copy proof')
        module('close-rendering-opaque-cropped-copy').check_witnesses(c['proof']['witnesses'],('whole','crop') if c['scenario']=='normal-storage' else ('whole',))
    elif name=='copy-keyed':module('close-rendering-keyed-palette').check_keyed(c['keyed']['witnesses'])
    elif name=='palette-remap':module('close-rendering-keyed-palette').check_palettes(c['palette']['witnesses'])
    elif name in ('sprite-indexed','destination-dependency'):
        p=c['proof'];require(p['verified'] and p['indexed_verified'] and p['destination_history_verified']
            and p['source_generations']>0 and p['indexed_calls']>0 and p['destination_calls']>0
            and p['world_presentations']==len(p['presented_worlds'])>0 and p['retained_pixels']>0,
            'Incomplete indexed/destination history proof')
        require(all(v['pixels']>0 and v['world']>0 for v in p['presented_worlds']), 'Missing owned source history')
    elif name=='sprite-direct':module('close-rendering-blend-direct').check_samples(c['samples'])
    elif name=='sprite-auxiliary':module('close-rendering-animation-auxiliary').check_output(c['witnesses'])
    elif name=='fill-full':module('close-rendering-full-fill').check_case(c)
    elif name=='cpu-write-readback':module('close-rendering-operations').check_case(c)
    elif name=='fill-clipped':module('check-rendering-clipped-fill').check_case(c)
    elif name=='partial-write':
        w=c['witnesses'];require(c['verified'] and len(w)==3 and len({v['generation']for v in w})==3
            and all(v['changed_words']>0 and v['preserved_words']>0 and v['preserved_nonzero_words']>0
                    and v['actual_sha256']==v['expected_sha256'] and v['before_exterior_sha256']==v['after_exterior_sha256']
                    and v['source_created']<v['sequence'] and v['target_created']<v['sequence'] and v['caller'][13]==0 for v in w),
            'Incomplete partial-write interior/exterior or caller state')
    elif name=='copy-stretched':
        require(c['verified'] is True and c['calls']>0 and c['pixels']>0 and c['mismatches']==0
                and c['normal_retirement'] is True and c['actual_gpu_planes'] is True and c['native_policy'] is True
                and c['original_driver_comparison'] is False,'Incomplete natural stretch output')
    elif name=='flags-and-return-values':
        require(c['verified'] is True and c['state_cases']>0 and c['pixels']>0 and c['mismatches']==0
                and c['normal_retirement'] is True and c['source_accounted'] is True
                and c['independent_expected_bytes'] is True,'Incomplete installed caller state')
    elif name=='text-font-dc-lifetime':
        f,n=c['font_dc'],c['native_fonts']
        require(f['verified'] and f['font_generations']>0 and f['glyph_commands']>0 and f['normal_native_retirement']
                and n['verified'] and n['normal_retirement'] and n['retired_registrations']>0
                and n['visible_caption_paints']>0 and n['caption_snapshot_joins']
                and f['dc']['complete_observed_chain'] is True and not f['dc']['missing_links']
                and f['dc']['dc_borrows']==f['dc']['dc_release_attempts']>0
                and c['pixels']>0 and c['presented_comparisons']>0,'Incomplete font/DC generations or native captions')
        require(not f['original_gdi_text_claimed'] and not n['original_ui_pixel_equivalence'], 'Font policies were widened')
    elif name=='palette-cycling':
        require(c['success'] and c['checked_all_world_bindings']>0 and c['refreshed_nodes'] and len(c['native'])==3
                and len({v['generation']for v in c['native']})==3
                and all(v['actual_rgba']==v['original_rgba'] and v['lease']>0 and v['paint']>0 for v in c['native'])
                and c['complete_output']['mismatching_pixels']==0 and c['complete_output']['worlds']>0,
                'Incomplete palette refresh/delivery')
    elif name=='copy-clipped':require(c['clip_callers']>0 and c['pixel_witnesses']>=2,'Missing admitted clipping case')
    else:raise ValueError('Unknown operation witness family: '+leaf)


def analyze(reader,parent,bindings=None):
    bindings=bindings or reader.json(PREFIX+'bindings-01.json')
    plan=reader.json('docs/native-rendering-closure.json');bound=basis(plan,parent,bindings)
    join=Join(reader);links=[];faults=FaultJoin(reader);fault_cache={};alias=None
    for row in parent['requirements']:
        leaf=row['id']
        if leaf not in LEAVES:continue
        binding=bound[row['configuration'],leaf];origin=join.checked(binding['inventory'])
        require(row in origin['requirements'] and origin['dossier_identity']==parent['dossier_identity'], 'Operation lost its exact admission provenance')
        if leaf=='operation.copy-alias-overlap':
            amendment=module('close-rendering-alias-bounded').policy(plan)
            require(row['acceptance_scope']=='bounded_controlled_opaque_overlap'
                and row['scope_amendment']==dict(id=amendment['id'],sha256=fingerprint(amendment))
                and row['admitted_execution_evidence']==[] and row['actual_player_overlap_domains']==0
                and row['natural_player_reachability']=='pending' and row['tested_surface_extent']==[8,6]
                and row['retained_obligation']['state']=='pending'
                and row['retained_obligation']['gate']=='gate.required-boundaries', 'Controlled overlap scope/obligation changed')
            if alias is None:alias=module('close-rendering-alias-bounded').read_proof(reader)
            require(row['admitted_mechanism_evidence']==[alias['evidence']] and alias['verified'] and alias['mismatches']==0,
                    'Controlled overlap proof differs')
            links.append(dict(configuration=row['configuration'],requirement=leaf,admission=binding['inventory'],
                controlled_mechanism=alias,natural_player_reachability='pending',retained_obligation=copy.deepcopy(row['retained_obligation'])))
            continue
        attachments=row['admitted_execution_evidence'];domains=set();pairs=[]
        require(attachments and len({fingerprint(pair_of(v))for v in attachments})==len(attachments), 'Missing/duplicate operation pair')
        support=None
        if leaf not in FAULTS:
            support=join.checked(binding['support'])
            require(binding['support']['record']=='research/runtime/'+SUPPORT[leaf.removeprefix('operation.')], 'Wrong operation support family')
        for attachment in attachments:
            pair=pair_of(attachment);size=attachment.get('actual_world_extents',attachment.get('actual_destination_extents'))
            require(isinstance(size,list)and len(size)==1 and tuple(size[0])in SIZES,'Missing/unsupported actual extent')
            size=size[0];domains.add(tuple(size))
            if leaf in FAULTS:
                key=(pair['comparison']['record'],leaf=='operation.surface-loss-restore')
                if key not in fault_cache:fault_cache[key]=faults.case(pair,row['configuration'],leaf)
                case=fault_cache[key];require(case['configuration']==row['configuration']and case['size']==size,'Mixed fault scope')
                output=dict(kind='failed-prefix-and-fresh-viewport',**case)
            else:
                candidates=support.get('partial_cases' if leaf=='operation.partial-write' else 'cases',support.get('executions',[]))
                selected=[v for v in candidates if case_pair(v)==pair]
                require(len(selected)==1,'Missing or duplicate semantic operation case')
                case=selected[0];profile,extent=case_domain(case)
                require(profile==row['configuration'] and extent==size,'Semantic case profile/extent differs')
                witnesses(leaf,case)
                output=join.normal_pair(pair,profile,size,leaf)
                require(case['identity']==output['identity'],'Semantic proof belongs to another original session')
                if leaf=='operation.fill-clipped':
                    require(attachment['native_replay']==case['witness']['native_replay']['artifacts'],'Clipped-fill replay binding changed')
                    for v in attachment['native_replay'].values():require(reader.digest(v['record'])==v['sha256'],'Retained native plane/replay changed')
                if leaf=='operation.palette-cycling':
                    require(attachment['numerical']==case['numerical'],'Palette numerical proof changed')
                    numerical=join.checked(case['numerical']);require(numerical['success'] is True,'Failed numerical palette proof')
            pairs.append(dict(execution=pair,actual_world_extents=[size],output=output))
        require(domains==SIZES,'Operation does not cover both actual extents')
        links.append(dict(configuration=row['configuration'],requirement=leaf,admission=binding['inventory'],
                          support=binding.get('support'),executions=pairs,scope=[v['reason']for v in attachments]))
    require(len(links)==42 and {(v['configuration'],v['requirement'])for v in links}=={(p,n)for p in PROFILES for n in LEAVES},'Incomplete 42-cell operation conjunction')
    join.records.update(faults.records);join.phases.update(faults.phases)
    return dict(schema=1,kind='retained_operations_gate_proof',success=True,linked_requirements=links,
        operation_ids=sorted(LEAVES),operation_cells=42,distinct_primary_comparisons=len(join.outputs)+len(fault_cache),
        retained_records=[join.records[n]for n in sorted(join.records)],retained_execution_source_phases=join.phases,
        original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,current_whole_player_proved=False,
        natural_player_overlap_domains=0,scope_amendments=copy.deepcopy(parent['scope_amendments']),scope=SCOPE)


def close_rows(parent,proof_ref,proof):
    require(proof['success'] is True and proof['operation_cells']==42 and proof['operation_ids']==sorted(LEAVES) and len(proof['linked_requirements'])==42,'Incomplete operation gate proof')
    rows=copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id']!=GATE:continue
        selected=[v for v in proof['linked_requirements']if v['configuration']==row['configuration']]
        require(row['state']=='pending' and len(selected)==21 and {v['requirement']for v in selected}==LEAVES,
                'Incomplete profile operation gate')
        row.update(state='passed',admitted_execution_evidence=[dict(gate_proof=proof_ref,
            operation_requirements=sorted(LEAVES),joined_operation_cells=21,
            natural_overlap_obligation_retained=True,reason=SCOPE)])
    require(sum(v['state']=='passed'for v in rows)==parent['passed_requirements']+2 and
            [v for v in rows if v['id']!=GATE]==[v for v in parent['requirements']if v['id']!=GATE],
            'Operation gate changed unrelated requirements')
    return rows
