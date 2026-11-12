#!/usr/bin/env python3
"""Adversarial provenance/lifetime controls; fixtures do not certify execution."""
import copy,hashlib,importlib.util,json,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_surface_loss import loss_receipt,admit_requests

def fixture():
    request=dict(schema=1,producer=271,transport_version=3,request_id='0123456789abcdef0123456789abcdef',action='destroy-render-target')
    raw=json.dumps(request).encode()
    observed=dict(schema=1,kind='native_offscreen_target_loss',producer=271,transport_version=3,request_id=request['request_id'],
        request_sha256=hashlib.sha256(raw).hexdigest(),begin_host_us='200',injected_diagnostic=True,physical_hardware_reset=False,
        old_generation_continuation=False,provider_owner='dedicated-thread',worker_thread=True,gui_destroyed_target=True,surface_valid_before=True,surface_valid_after=False,
        worlds_before='8',live_surfaces_before=4,pending_cpu_read='0',destroy_begin_host_us='201',destroy_end_host_us='202',
        end_host_us='203',error='OpenGL offscreen target is unavailable',graphics_failure=True)
    retired=dict(observed,retirement_host_us='204',target_restored_for_deletion=True,command_surfaces_after_cleanup='0',
        renderer_surfaces_after_cleanup='0',command_owner_retired=True,world_owner_retired=True,frame_leases_retired=True,
        worker_teardown_complete=True,normal_completion_claimed=False)
    value=dict(kind='native-offscreen-target-loss',request_host_ns=199000,request=request,observation=observed,retirement=retired)
    return value,271,raw,observed,retired

class Controls(unittest.TestCase):
    def test_complete(self):
        result=loss_receipt(*fixture());self.assertTrue(result['actual_platform_target_destroyed']);self.assertFalse(result['old_generation_continuation'])
    def test_primary_gui_owner(self):
        args=fixture()
        for row in (args[3],args[4]):row.update(provider_owner='gui-resident',worker_thread=False,render_owner_gui_thread=True)
        self.assertEqual(loss_receipt(*args)['provider_owner'],'gui-resident')
    def rejected(self,change):
        args=copy.deepcopy(fixture());change(args)
        with self.assertRaises((ValueError,KeyError,TypeError)):loss_receipt(*args)
    def test_foreign_producer(self):self.rejected(lambda a:a[0]['request'].update(producer=272))
    def test_foreign_raw_request(self):self.rejected(lambda a:a[0]['request'].update(action='producer-invalid'))
    def test_fabricated_hash(self):
        def change(a):a[3]['request_sha256']='0'*64;a[4]['request_sha256']='0'*64
        self.rejected(change)
    def test_no_world(self):
        def change(a):a[3]['worlds_before']='0';a[4]['worlds_before']='0'
        self.rejected(change)
    def test_live_cpu_borrow(self):
        def change(a):a[3]['pending_cpu_read']='1';a[4]['pending_cpu_read']='1'
        self.rejected(change)
    def test_wrong_failure_class(self):
        def change(a):a[3]['graphics_failure']=False;a[4]['graphics_failure']=False
        self.rejected(change)
    def test_no_actual_destroy(self):
        def change(a):a[3]['surface_valid_after']=True;a[4]['surface_valid_after']=True
        self.rejected(change)
    def test_not_gui_owner(self):
        def change(a):a[3]['gui_destroyed_target']=False;a[4]['gui_destroyed_target']=False
        self.rejected(change)
    def test_not_worker_owner(self):
        def change(a):a[3]['worker_thread']=False;a[4]['worker_thread']=False
        self.rejected(change)
    def test_old_generation_continuation(self):
        def change(a):a[3]['old_generation_continuation']=True;a[4]['old_generation_continuation']=True
        self.rejected(change)
    def test_physical_reset_claim(self):
        def change(a):a[3]['physical_hardware_reset']=True;a[4]['physical_hardware_reset']=True
        self.rejected(change)
    def test_wrong_native_error(self):
        def change(a):a[3]['error']='INVALID producer';a[4]['error']='INVALID producer'
        self.rejected(change)
    def test_unrestored_deletion_target(self):self.rejected(lambda a:a[4].update(target_restored_for_deletion=False))
    def test_retained_command_owner(self):self.rejected(lambda a:a[4].update(command_owner_retired=False))
    def test_retained_world_owner(self):self.rejected(lambda a:a[4].update(world_owner_retired=False))
    def test_retained_frame_lease(self):self.rejected(lambda a:a[4].update(frame_leases_retired=False))
    def test_retained_command_surface(self):self.rejected(lambda a:a[4].update(command_surfaces_after_cleanup='1'))
    def test_retained_native_storage(self):self.rejected(lambda a:a[4].update(renderer_surfaces_after_cleanup='1'))
    def test_false_normal_credit(self):self.rejected(lambda a:a[4].update(normal_completion_claimed=True))
    def test_wrong_retired_fault(self):self.rejected(lambda a:a[4].update(request_id='a'*32))
    def test_retirement_before_failure(self):self.rejected(lambda a:a[4].update(retirement_host_us='201'))
    def test_request_after_fault(self):self.rejected(lambda a:a[0].update(request_host_ns=250000))
    def test_controller_handoff_differs(self):self.rejected(lambda a:a[0].update(kind='producer-invalid'))

class Budget(unittest.TestCase):
    def row(self,name='pilot',success=True,proved=True,charge=2,starts=2,repair=None):
        return dict(report=name,repair_of=repair,charged_games=charge,result=dict(success=success,original_game_runs=starts),proved=proved,comparison_failed=False)
    def test_first_request_reserves_two(self):self.assertEqual(admit_requests([],None,6,10)['reserved_original_starts'],2)
    def test_successful_pilot(self):self.assertEqual(admit_requests([self.row()],None,6,10)['charged_original_starts_before'],2)
    def test_failed_pilot_blocks_expansion(self):
        with self.assertRaises(ValueError):admit_requests([self.row(success=False,proved=False)],None,6,10)
    def test_explicit_failure_repair(self):self.assertEqual(admit_requests([self.row(success=False,proved=False)],'pilot',6,10)['repair_ancestors'],['pilot'])
    def test_no_silent_start_refund(self):
        with self.assertRaises(ValueError):admit_requests([self.row(success=False,proved=False,starts=0)],'pilot',6,2)
    def test_observed_start_must_be_charged(self):
        with self.assertRaises(ValueError):admit_requests([self.row(charge=0)],None,6,10)
    def test_request_cap(self):
        with self.assertRaises(ValueError):admit_requests([self.row()],None,1,10)
    def test_start_cap(self):
        with self.assertRaises(ValueError):admit_requests([self.row()],None,6,3)
    def test_missing_repair_parent(self):
        with self.assertRaises(ValueError):admit_requests([], 'missing',6,10)
    def test_only_one_repair(self):
        rows=[self.row(success=False,proved=False),self.row('repair',False,False,repair='pilot')]
        with self.assertRaises(ValueError):admit_requests(rows,'repair',6,10)

class Admission(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        root=Path(__file__).resolve().parents[1]
        spec=importlib.util.spec_from_file_location('surface_admission',root/'tools/close-rendering-surface-loss.py')
        cls.module=importlib.util.module_from_spec(spec);spec.loader.exec_module(cls.module)
        from coverage_evidence import Reader
        with Reader(root) as r:
            cls.plan=r.json('docs/native-rendering-closure.json')
            cls.parent=r.json('research/runtime/caller-state-master-results-20261108/admission-01.json')
    def cases(self):
        out=[]
        for software in (False,True):
            for size in ([640,480],[800,600]):
                n=len(out)
                proof=dict(verified=True,cleanup=dict(verified=True),physical_public_restart=True,fresh_positive_input=True,
                    old_rendering_accepted=False,failed_epoch_normal_completion_claimed=False,
                    provider_loss=dict(provider_owner='gui-resident',actual_platform_target_destroyed=True,restored_for_deletion_only=True,old_generation_continuation=False,physical_hardware_reset=False),
                    terminal_samples=8,terminal_stability_ns=650000000,distinct_world_nonces=['old'+str(n),'fresh'+str(n)],
                    distinct_runner_identities=[dict(pid=n*2+1,start_ticks=1),dict(pid=n*2+2,start_ticks=2)])
                out.append(dict(identity=str(n),domain=dict(software=software,size=size),proof=proof,pixels=480000,
                    player_sha256='a'*64,compiler_sources={'fixture.cpp':'b'*64},provider='supplied fixture; no game',
                    acquisition=dict(record='fixture-'+str(n),sha256='c'*64),comparison=dict(record='comparison-'+str(n),sha256='d'*64)))
        return out
    def test_only_two_rows_change(self):
        rows,_=self.module.close_rows(self.plan,self.parent,self.cases())
        changed=[(a,b) for a,b in zip(self.parent['requirements'],rows) if a!=b]
        self.assertEqual(len(changed),2);self.assertTrue(all(a['id']=='operation.surface-loss-restore' and b['state']=='passed' for a,b in changed))
    def reject(self,change):
        cases=self.cases();change(cases)
        with self.assertRaises(ValueError):self.module.close_rows(self.plan,self.parent,cases)
    def test_missing_profile(self):self.reject(lambda v:v.pop())
    def test_duplicate_session(self):self.reject(lambda v:v[1].update(identity=v[0]['identity']))
    def test_wrong_domain(self):self.reject(lambda v:v[1]['domain'].update(size=[640,480]))
    def test_mixed_player(self):self.reject(lambda v:v[1].update(player_sha256='e'*64))
    def test_mixed_compiler(self):self.reject(lambda v:v[1].update(compiler_sources={}))
    def test_no_provider_loss(self):self.reject(lambda v:v[0]['proof'].update(provider_loss={}))
    def test_alternate_provider_is_not_primary(self):self.reject(lambda v:v[0]['proof']['provider_loss'].update(provider_owner='dedicated-thread'))
    def test_no_input(self):self.reject(lambda v:v[0]['proof'].update(fresh_positive_input=False))
    def test_no_ordinary_quit(self):self.reject(lambda v:v[0]['proof'].update(cleanup=dict(verified=False)))
    def test_old_epoch_normal_credit(self):self.reject(lambda v:v[0]['proof'].update(old_rendering_accepted=True))
    def test_no_output(self):self.reject(lambda v:v[0].update(pixels=0))
    def test_same_original_runner(self):self.reject(lambda v:v[0]['proof'].update(distinct_runner_identities=[dict(pid=1,start_ticks=1)]*2))
    def test_short_terminal_interval(self):self.reject(lambda v:v[0]['proof'].update(terminal_stability_ns=1))

class ExtentDispatchControls(unittest.TestCase):
    def dispatch(self,size):
        from unittest import mock
        from rendering_surface_loss_live import SurfaceLossJourney,Base,Wide
        journey=object.__new__(SurfaceLossJourney);journey.size=size
        with mock.patch.object(Base,'normal_quit',autospec=True,return_value='low') as low, mock.patch.object(Wide,'normal_quit',autospec=True,return_value='wide') as wide:
            if size==[640,480]:self.assertEqual(journey.normal_quit(),'low');low.assert_called_once_with(journey);wide.assert_not_called()
            elif size==[800,600]:self.assertEqual(journey.normal_quit(),'wide');wide.assert_called_once_with(journey);low.assert_not_called()
            else:
                with self.assertRaises(ValueError):journey.normal_quit()
                low.assert_not_called();wide.assert_not_called()
    def test_low_preserves_old_handler(self):self.dispatch([640,480])
    def test_high_uses_existing_wide_handler(self):self.dispatch([800,600])
    def test_unsupported_extent_performs_no_action(self):self.dispatch([1024,768])

class ProviderHeaderControls(unittest.TestCase):
    def setUp(self):
        from rendering_transition_assessment import provider_stopped_header
        self.check=provider_stopped_header
        self.raw=b'MNMRDC03'+b'\0'*(1048640-8)
        self.header=[0,0,3,1048640,271,2104,3,4,1,2000,0,0,0,0,0,0]
    def test_actual_interrupted_tail_is_terminal(self):self.assertTrue(self.check(self.header,self.raw))
    def test_active_and_cancelled_terminal_states(self):
        for state,reason in ((1,0),(3,3)):
            row=list(self.header);row[6:8]=state,reason;self.assertTrue(self.check(row,self.raw))
    def test_end_and_unrelated_errors_refused(self):
        for state,reason in ((2,0),(3,1),(3,2),(3,5),(1,4),(0,0)):
            row=list(self.header);row[6:8]=state,reason;self.assertFalse(self.check(row,self.raw))
    def test_cancellation_required(self):self.header[8]=0;self.assertFalse(self.check(self.header,self.raw))
    def test_producer_required(self):self.header[4]=0;self.assertFalse(self.check(self.header,self.raw))
    def test_geometry_required(self):self.header[3]=64;self.assertFalse(self.check(self.header,self.raw))
    def test_reserved_words_required(self):self.header[10]=1;self.assertFalse(self.check(self.header,self.raw))
    def test_backwards_ack_refused(self):self.header[9]=self.header[5]+1;self.assertFalse(self.check(self.header,self.raw))
    def test_truncated_ring_refused(self):self.assertFalse(self.check(self.header,self.raw[:64]))

class SourcePhaseControls(unittest.TestCase):
    def fixture(self):
        names=('tools/rendering_surface_loss_live.py','tools/rendering_surface_loss.py','tests/test-rendering-surface-loss.py',
            'tools/close-rendering-surface-loss.py','tools/rendering_transition_assessment.py')
        values={n:'after-'+str(i) for i,n in enumerate(names)};pairs={n:dict(before='before-'+str(i),after=values[n],before_record='preimage-'+str(i)) for i,n in enumerate(names)}
        values.update({v['before_record']:v['before'] for v in pairs.values()});values.update({'renderer/blit.cpp':'native-unchanged','accepted-low.json':'comparison-sha'})
        class FakeReader:
            def __init__(self,values):self.values=values
            def digest(self,name):return self.values[name]
        acquisition=dict(record='actual-low-acquisition',sha256='acquisition-sha')
        c=dict(success=True,sources_stable=True,acquisition=acquisition,sources={n:v['before'] for n,v in pairs.items()}|{'renderer/blit.cpp':'native-unchanged'},
            proof=dict(verified=True,domain=dict(size=[640,480])),comparison=dict(mismatching_pixels=0))
        review=dict(success=True,kind='native_surface_loss_extent_continuation',original_game_runs=0,unchanged_compiled_player_runtime_oracle=True,
            existing_low_controller_ast_preserved=True,existing_receipt_budget_and_admission_conditions_ast_preserved=True,
            existing_assessment_ast_preserved_except_provider_terminal_header=True,raw_low_proofs_reassessed_unchanged=True,
            current_low_cli_execution_claimed=False,charges_refunded=0,source_changes=pairs,
            retained_low_comparisons=[dict(record='accepted-low.json',sha256='comparison-sha',acquisition=acquisition)])
        phase=dict(success=True,kind='native_surface_loss_prospective_plan',original_game_runs=0,
            bounds=dict(maximum_requests=6,maximum_original_games=10),controller_continuation=review)
        return FakeReader(values),'accepted-low.json',c,phase
    def test_exact_accepted_low_phase(self):
        from rendering_surface_loss import source_phase_review
        value=source_phase_review(*self.fixture());self.assertFalse(value['current_low_cli_execution_claimed'])
    def reject(self,change):
        from rendering_surface_loss import source_phase_review
        args=self.fixture();change(args)
        with self.assertRaises((ValueError,KeyError,TypeError)):source_phase_review(*args)
    def test_native_drift_cannot_be_waived(self):self.reject(lambda a:a[0].values.update({'renderer/blit.cpp':'new-native'}))
    def test_foreign_comparison_refused(self):self.reject(lambda a:a[0].values.update({'accepted-low.json':'different-report'}))
    def test_foreign_acquisition_refused(self):self.reject(lambda a:a[2].update(acquisition={}))
    def test_missing_preimage_refused(self):self.reject(lambda a:a[0].values.update({'preimage-0':'missing-before-bytes'}))
    def test_bound_expansion_refused(self):self.reject(lambda a:a[3]['bounds'].update(maximum_original_games=12))
    def test_unproved_scope_refused(self):self.reject(lambda a:a[3]['controller_continuation'].update(raw_low_proofs_reassessed_unchanged=False))
    def test_current_low_claim_refused(self):self.reject(lambda a:a[3]['controller_continuation'].update(current_low_cli_execution_claimed=True))
    def test_unrelated_delta_domain_refused(self):self.reject(lambda a:a[3]['controller_continuation']['source_changes'].update({'renderer/blit.cpp':{}}))

if __name__=='__main__':unittest.main()
