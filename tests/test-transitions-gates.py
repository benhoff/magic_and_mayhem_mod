#!/usr/bin/env python3
"""Actual-data adversarial controls for the finite transition conjunction."""
import copy,json,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from campaign_results_review import victory_route
from rendering_defeat import defeat_route
import transitions_gate as gate

class Proxy:
    def __init__(self,r,reports):self.reader,self.reports=r,reports;self.root=r.root
    def digest(self,n):return self.reader.digest(n)
    def json(self,n):return copy.deepcopy(self.reports[n])if n in self.reports else self.reader.json(n)
    def read_bytes(self,n):return self.reader.read_bytes(n)

class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.r=Reader(ROOT);cls.parent=cls.r.json(gate.PREFIX+'basis-01.json');cls.bindings=cls.r.json(gate.PREFIX+'bindings-01.json');cls.plan=cls.r.json('docs/native-rendering-closure.json')
        cls.proof=gate.analyze(cls.r,cls.parent,cls.bindings)
        cls.ph=cls.r.json(cls.bindings['complete_output_proof']['record'])
        cls.victory=next(c for c in cls.proof['cases']if c['kind']=='progression');cls.defeat=next(c for c in cls.proof['cases']if c['kind']=='defeat');cls.resize=next(c for c in cls.proof['cases']if c['kind']=='recreation')
        cls.a=cls.r.json(cls.victory['pair']['acquisition']['record']);cls.c=cls.r.json(cls.victory['pair']['comparison']['record']);cls.events=artifact(cls.r,cls.a,'events.bin')
        cls.da=cls.r.json(cls.defeat['pair']['acquisition']['record']);cls.devents=artifact(cls.r,cls.da,'events.bin');cls.gameplay=artifact(cls.r,cls.da,'gameplay-events.bin');cls.helper=json.loads(artifact(cls.r,cls.da,'progression-input-0.json'))
        cls.join=gate.Join(cls.r,cls.bindings);cls.rx=cls.join.pair(cls.resize['pair'],cls.resize['profile'],[800,600])
    @classmethod
    def tearDownClass(cls):cls.r.close()
    def bad_basis(self,change):
        p=copy.deepcopy(self.parent);change(p)
        with self.assertRaises(ValueError):gate.basis(self.plan,p,self.bindings)
    def bad_pixels(self,change):
        c=copy.deepcopy(self.c);change(c)
        with self.assertRaises(ValueError):gate.comparison_pixels(c)
    def bad_victory(self,change):
        f=copy.deepcopy(self.a['flow']);change(f)
        with self.assertRaises(ValueError):victory_route(f,self.events)
    def bad_defeat(self,change):
        f,h=copy.deepcopy(self.da['flow']),copy.deepcopy(self.helper);change(f,h)
        with self.assertRaises(ValueError):defeat_route(f,h,self.devents,self.gameplay)
    def bad_resize(self,change):
        a,c,l=copy.deepcopy(self.rx['a']),copy.deepcopy(self.rx['c']),copy.deepcopy(self.rx['lifetimes']);change(a,c,l)
        with self.assertRaises(ValueError):gate.resize_rebuilds(a,c,l)
    def bad_history(self,change):
        p=copy.deepcopy(self.ph);change(p);r=Proxy(self.r,{self.bindings['complete_output_proof']['record']:p})
        with self.assertRaises(ValueError):gate.Join(r,self.bindings).pair(self.victory['pair'],self.victory['profile'],self.victory['size'])
    def test_exact_twelve_cells(self):self.assertEqual(self.proof['transition_cells'],12)
    def test_distinct_ten_owned_sessions(self):self.assertEqual(len({v['identity']for v in self.proof['cases']}),10)
    def test_four_progression_four_defeat_two_rebuild(self):self.assertEqual({k:sum(v['kind']==k for v in self.proof['cases'])for k in ['progression','defeat','recreation']},{'progression':4,'defeat':4,'recreation':2})
    def test_no_new_execution(self):self.assertEqual([self.proof[n]for n in ['original_game_runs','new_native_executions','new_pixel_comparisons']],[0,0,0])
    def test_historical_native_phases(self):self.assertTrue(all(not v['current_native_execution_claimed']for v in self.proof['retained_execution_source_phases'].values()))
    def test_only_two_gate_rows(self):
        rows=gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),self.proof)
        self.assertEqual(sum(v['state']=='passed'for v in rows),self.parent['passed_requirements']+2)
        self.assertEqual([v for v in rows if v['id']!=gate.GATE],[v for v in self.parent['requirements']if v['id']!=gate.GATE])
    def test_missing_row(self):self.bad_basis(lambda p:p['requirements'].pop())
    def test_duplicate_row(self):self.bad_basis(lambda p:p['requirements'].__setitem__(1,p['requirements'][0]))
    def test_wrong_counts(self):self.bad_basis(lambda p:p.update(passed_requirements=156))
    def test_changed_gate_condition(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(pending_reason='skip loading'))
    def test_changed_owner(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']in gate.LEAVES).update(owner_task='other'))
    def test_missing_leaf_proof(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']in gate.LEAVES).update(admitted_execution_evidence=[]))
    def test_pending_leaf(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']in gate.LEAVES).update(state='pending'))
    def test_final_decision(self):self.bad_basis(lambda p:p['decisions'].update({'supported-player':'passed'}))
    def test_changed_amendment(self):self.bad_basis(lambda p:p['scope_amendments'].clear())
    def test_already_accepted_gate(self):self.bad_basis(lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(state='passed'))
    def test_hash_mismatch(self):
        with self.assertRaises(ValueError):self.join.checked(dict(self.victory['pair']['acquisition'],sha256='0'*64))
    def test_missing_pixel_row(self):self.bad_pixels(lambda c:c['comparison']['comparisons'].pop())
    def test_changed_native_pixel(self):self.bad_pixels(lambda c:c['comparison']['comparisons'][0].update(actual_sha256='0'*64))
    def test_native_pixels_in_oracle(self):self.bad_pixels(lambda c:c['original_reference'].update(native_player_pixels_read=True))
    def test_destination_capture_in_oracle(self):self.bad_pixels(lambda c:c['original_reference'].update(original_destination_capture_read=True))
    def test_missing_initial_history(self):self.bad_pixels(lambda c:c['original_reference'].update(continuous_from_owned_initial_history=False))
    def test_surface_survives(self):self.bad_pixels(lambda c:c['comparison'].update(remaining_surfaces=1))
    def test_empty_output(self):self.bad_pixels(lambda c:c['comparison'].update(comparisons=[]))
    def test_wrong_pixel_count(self):self.bad_pixels(lambda c:c['comparison'].update(pixels_compared=0))
    def test_wrong_world_count(self):self.bad_pixels(lambda c:c['comparison'].update(world_presentations=0))
    def test_no_natural_victory(self):self.bad_victory(lambda f:f['states'][4].update(outcome=0))
    def test_forfeit_substitution(self):self.bad_victory(lambda f:f['states'][4]['texts'].__setitem__(19,'You quit the battle.'))
    def test_stale_victory_ack(self):self.bad_victory(lambda f:f['states'][5].update(ack=f['states'][4]['ack']))
    def test_missing_campaign_travel(self):self.bad_victory(lambda f:f['states'][7].update(event='load-save'))
    def test_reentered_same_region(self):self.bad_victory(lambda f:f['states'][8].update(region=1))
    def test_stale_later_world(self):self.bad_victory(lambda f:f['worlds'][1].update(world=f['worlds'][0]['world']))
    def test_victory_substitutes_defeat(self):self.bad_defeat(lambda f,h:f['states'][-2].update(title='Victory!'))
    def test_defeat_resumed_checkpoint(self):self.bad_defeat(lambda f,h:h.update(resume_checkpoint=True))
    def test_defeat_simulation_write(self):self.bad_defeat(lambda f,h:h.update(simulation_writes=True))
    def test_no_original_player_death(self):self.bad_defeat(lambda f,h:h['terminal_hero'].update(health=1))
    def test_fabricated_health_chain(self):self.bad_defeat(lambda f,h:h['health_observations'][0].update(health=-9))
    def test_wrong_defeat_return(self):self.bad_defeat(lambda f,h:f['states'][-1].update(screen=4))
    def test_defeat_reuses_generation(self):self.bad_defeat(lambda f,h:f['states'][-1].update(generation=f['states'][-2]['generation']))
    def test_missing_rebuild(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'].pop())
    def test_component_rebuild(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0].update(same_original_process=False))
    def test_same_world_false_claim(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0].update(same_world=True))
    def test_wrong_resolution_selector(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0]['original_after'].update(high_resolution=1))
    def test_stale_resize_source(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0]['original_after']['frame'].update(generation=a['viewport_cursor']['rebuilds'][0]['original_before']['frame']['generation']))
    def test_mixed_resize_nonce(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0]['original_after']['frame'].update(nonce='0'))
    def test_missing_preferences(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0]['preferences'].update(screen=3))
    def test_stale_preferences_ack(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0]['returned'].update(ack=a['viewport_cursor']['rebuilds'][0]['preferences']['ack']))
    def test_unpresented_resize(self):self.bad_resize(lambda a,c,l:a['viewport_cursor']['rebuilds'][0]['confirmed_paint'].update(paint='999999999'))
    def test_native_incarnation_not_retired(self):
        def mutate(a,c,l):
            g=a['viewport_cursor']['rebuilds'][0]['original_before']['frame']['generation'];l['imports'][g-1]['deleted_sequence']=10**12
        self.bad_resize(mutate)
    def test_output_proof_source_drift(self):self.bad_history(lambda p:p['sources'].update({'tools/rendering_pilot.py':'0'*64}))
    def test_incomplete_gate_proof(self):
        p=copy.deepcopy(self.proof);p['linked_requirements'].pop()
        with self.assertRaises(ValueError):gate.close_rows(self.parent,{},p)
    def test_duplicate_gate_identity(self):
        p=copy.deepcopy(self.proof);p['cases'][1]['identity']=p['cases'][0]['identity']
        with self.assertRaises(ValueError):gate.close_rows(self.parent,{},p)
    def bad_resources(self,change):
        saved=self.r.json(self.bindings['supports'][gate.SUPPORT[1]]['record'])['cases'][0];resources=copy.deepcopy(saved['resources']);original=copy.deepcopy(saved['original']);c=self.r.json(saved['comparison']['record']);change(resources,original,c)
        with self.assertRaises(ValueError):gate.resource_join(resources,original,c)
    def test_missing_populated_map_epoch(self):self.bad_resources(lambda r,o,c:r.update(world_epochs=1))
    def test_stale_initial_resource_epoch(self):self.bad_resources(lambda r,o,c:r['initial_resources'].update(menu_event=0))
    def test_stale_later_resource_epoch(self):self.bad_resources(lambda r,o,c:r['later_resources'].update(menu_event=0))
    def test_retirement_after_wrong_pop(self):self.bad_resources(lambda r,o,c:r['retirements'][0]['after'].update(menu_event=0))
    def test_wrong_retirement_owner(self):self.bad_resources(lambda r,o,c:r['retirements'][0]['after'].update(stack_owner=0))
    def test_missing_full_output_case(self):self.bad_history(lambda p:p.update(additional_presented_cases=[v for v in p['additional_presented_cases']if v['pair']!=self.victory['pair']]))
    def test_mixed_full_output_session(self):
        self.bad_history(lambda p:next(v for v in p['additional_presented_cases']if v['pair']==self.victory['pair']).update(identity='another'))
    def test_changed_full_output_swap_count(self):
        self.bad_history(lambda p:next(v for v in p['additional_presented_cases']if v['pair']==self.victory['pair'])['completed_output'].update(swaps=0))
    def test_changed_full_command_hash(self):
        self.bad_history(lambda p:next(v for v in p['additional_presented_cases']if v['pair']==self.victory['pair'])['command_history'].update(sha256='0'*64))

if __name__=='__main__':unittest.main()
