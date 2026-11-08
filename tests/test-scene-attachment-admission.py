#!/usr/bin/env python3
"""Actual retained cast boundary guards and exact four-row accounting controls."""
from copy import deepcopy
import importlib.util
import hashlib
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('scene_close',ROOT/'tools/close-scene-attachment-combat.py')
close=importlib.util.module_from_spec(spec);spec.loader.exec_module(close)
import scene_attachment_journey as attachment


class ReadyInventoryControls(unittest.TestCase):
    """Replay real saved visits with an unavailable optional live publication."""
    @classmethod
    def setUpClass(cls):
        cls.reader=close.Reader(ROOT);cls.reader.__enter__();cls.cases=[]
        for name in('attachment-default800-04.json','attachment-software800-01.json'):
            acquired=cls.reader.json(close.PREFIX+name);ready=acquired['states'][5]
            artifact=next(a for a in acquired['artifacts']if a['path']==ready['wire_path'])
            raw=cls.reader.read_bytes(artifact['record'])
            assert hashlib.sha256(raw).hexdigest()==artifact['sha256']
            cls.cases.append((ready,raw,acquired['attachment_route']['inventory']))

    @classmethod
    def tearDownClass(cls):cls.reader.__exit__(None,None,None)

    def setUp(self):
        self.scratch=tempfile.TemporaryDirectory(prefix='scene-ready-inventory-')
        self.addCleanup(self.scratch.cleanup)
        self.directory=Path(self.scratch.name)
        self.root_patch=patch.object(attachment.menus,'ROOT',self.directory)
        self.root_patch.start();self.addCleanup(self.root_patch.stop)

    def journey(self,index=0):
        ready,raw,inventory=self.cases[index]
        path=self.directory/(str(index)+'.bin');path.write_bytes(raw)
        journey=attachment.AttachmentJourney.__new__(attachment.AttachmentJourney)
        journey.last=deepcopy(ready);journey.last['wire_path']=path.name
        journey.proof={};journey.read=lambda:None
        journey.node=lambda name:(None,{'name':'original slot — empty'})
        def stop_before_physical_input(name):raise StopIteration(name)
        journey.press=stop_before_physical_input
        return journey,path,inventory

    def test_select_spell_survives_a_transient_none_live_read(self):
        journey,path,inventory=self.journey()
        with self.assertRaises(StopIteration):journey.select_spell()
        for key in('owner','offered','counts','assignments','recipes','ingredients','spells'):
            self.assertEqual(journey.proof['inventory'][key],inventory[key])

    def test_saved_inventory_matches_both_actual_accepted_profiles(self):
        for index in range(2):
            with self.subTest(profile=index):
                journey,path,inventory=self.journey(index)
                actual=journey.ready_spellbox_inventory()
                for key in('owner','offered','counts','assignments','recipes','ingredients','spells'):
                    self.assertEqual(actual[key],inventory[key])

    def test_foreign_ready_visit_identity_is_rejected(self):
        for key in('sequence','generation','ack','status','thread','handoff'):
            with self.subTest(key=key):
                journey,path,inventory=self.journey();journey.last[key]+=2
                with self.assertRaises(ValueError):journey.ready_spellbox_inventory()

    def test_a_non_spellbox_visit_cannot_supply_inventory(self):
        journey,path,inventory=self.journey();journey.last['screen']=22
        with self.assertRaises(ValueError):journey.ready_spellbox_inventory()

    def test_unready_or_missing_visit_cannot_supply_inventory(self):
        journey,path,inventory=self.journey();journey.last['ready']=False
        with self.assertRaises(ValueError):journey.ready_spellbox_inventory()
        journey.last=None
        with self.assertRaises(ValueError):journey.ready_spellbox_inventory()

    def test_missing_saved_wire_cannot_fall_back_to_optional_live_data(self):
        journey,path,inventory=self.journey();path.unlink()
        with self.assertRaises(ValueError):journey.ready_spellbox_inventory()

    def test_busy_saved_publication_is_rejected(self):
        journey,path,inventory=self.journey();raw=bytearray(path.read_bytes())
        struct.pack_into('<I',raw,128,struct.unpack_from('<I',raw,128)[0]|1);path.write_bytes(raw)
        with self.assertRaises(ValueError):journey.ready_spellbox_inventory()

    def test_foreign_saved_wire_signature_is_rejected(self):
        journey,path,inventory=self.journey();raw=bytearray(path.read_bytes());raw[0]^=1;path.write_bytes(raw)
        with self.assertRaises(ValueError):journey.ready_spellbox_inventory()


class ReadyPhaseControls(unittest.TestCase):
    def versions(self):
        after=(ROOT/close.DRIVER).read_bytes()
        self.assertEqual(after.count(close.READY_AFTER),1)
        return after.replace(close.READY_AFTER,close.READY_BEFORE),after

    def test_only_the_exact_saved_inventory_correction_is_accepted(self):
        close.ready_only_change(*self.versions())

    def test_a_changed_target_cannot_hide_in_the_inventory_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.ready_only_change(before,after.replace(b'(575, 575)',b'(477, 575)'))

    def test_a_changed_physical_hold_cannot_hide_in_the_inventory_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.ready_only_change(before,after.replace(b'0xff1b,.6',b'0xff1b,.2'))

    def test_a_missing_ready_visit_guard_is_rejected(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.ready_only_change(before,after.replace(b"self.last['screen'] == 7",b'True'))

    def test_ready_review_never_waives_native_code(self):
        reader=type('Reader',(),{'digest':lambda self,n:'current'})()
        with self.assertRaises(ValueError):close.checked_sources(reader,{'runtime/scene/canvas_producers.c':'old'},None,{'before':{'sha256':'old'}})

    def test_an_unrelated_driver_preimage_is_rejected(self):
        reader=type('Reader',(),{'digest':lambda self,n:'current'})()
        with self.assertRaises(ValueError):close.checked_sources(reader,{close.DRIVER:'foreign'},None,{'before':{'sha256':'old'}})


class ActualCastControls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader=close.Reader(ROOT);cls.reader.__enter__()
        cls.acquired=cls.reader.json(close.PREFIX+'attachment-default800-04.json')
        cls.compared=cls.reader.json(close.PREFIX+'attachment-default800-comparison-01.json')

    @classmethod
    def tearDownClass(cls):cls.reader.__exit__(None,None,None)

    def rejected(self,change):
        a,c=deepcopy(self.acquired),deepcopy(self.compared);change(a,c)
        with self.assertRaises((RuntimeError,AssertionError,ValueError)):
            close.attachment_transition(self.reader,a,c)

    def test_actual_original_self_cast_reaches_mode1_child(self):
        q=close.attachment_transition(self.reader,self.acquired,self.compared)
        self.assertEqual([q['source_mode_changes'][i]['mode']for i in(0,-1)],[0,1])
        self.assertFalse(q['actor_allocation_identity_proved'])

    def test_foreign_spell_rejected(self):
        self.rejected(lambda a,c:c['scene_attachment_route']['original_cast'].__setitem__(4,84))

    def test_foreign_self_target_rejected(self):
        self.rejected(lambda a,c:c['scene_attachment_route']['original_cast'].__setitem__(10,1))

    def test_no_original_mana_debit_rejected(self):
        self.rejected(lambda a,c:c['scene_attachment_route']['original_cast'].__setitem__(9,c['scene_attachment_route']['original_cast'][8]))

    def test_missing_source_identity_rejected(self):
        self.rejected(lambda a,c:c['scene_attachment_route']['original_cast'].__setitem__(15,0))

    def test_invented_original_event_rejected(self):
        self.rejected(lambda a,c:c['scene_attachment_route']['original_cast'].__setitem__(0,999999))

    def test_reversed_source_interval_rejected(self):
        self.rejected(lambda a,c:a['attachment_route'].__setitem__('first_attachment_generation',a['attachment_route']['target']['source_generation']))

    def test_active_only_interval_does_not_prove_transition(self):
        self.rejected(lambda a,c:a['attachment_route']['target'].__setitem__('source_generation',a['attachment_route']['first_attachment_generation']-1))


class RowAccountingControls(unittest.TestCase):
    """Synthetic metadata exercises accounting only; actual case joins earn credit."""
    @classmethod
    def setUpClass(cls):
        with close.Reader(ROOT)as r:cls.parent=r.json(close.PARENT)

    def cases(self):
        return [dict(purpose=p,software=s,actual_extent=[w,h],identity=str((p,s,w,h)),
                     success=True,attachment=p=='attachment',combat=p=='combat',pair={},provider='test metadata')
                for p in('attachment','combat')for s in(False,True)for w,h in((640,480),(800,600))]

    def test_exact_four_pending_rows_and_all_parent_rows_preserved(self):
        rows=close.close_rows(self.parent,self.cases())
        self.assertEqual(sum(r['state']=='passed'for r in rows),self.parent['passed_requirements']+4)
        for before,after in zip(self.parent['requirements'],rows):
            if before['id']not in close.LEAVES:self.assertEqual(before,after)
        self.assertEqual(self.parent['decisions'],{'supported-player':'pending','original-recovery':'pending'})

    def test_missing_actual_size_rejected(self):
        with self.assertRaises(ValueError):close.close_rows(self.parent,self.cases()[:-1])

    def test_duplicate_session_rejected(self):
        cases=self.cases();cases[-1]['identity']=cases[0]['identity']
        with self.assertRaises(ValueError):close.close_rows(self.parent,cases)

    def test_relabelled_domain_rejected(self):
        cases=self.cases();cases[-1]['actual_extent']=[640,480]
        with self.assertRaises(ValueError):close.close_rows(self.parent,cases)

    def test_unproved_output_rejected(self):
        cases=self.cases();cases[-1]['combat']=False
        with self.assertRaises(ValueError):close.close_rows(self.parent,cases)

    def test_already_closed_task_row_rejected(self):
        parent=deepcopy(self.parent)
        next(r for r in parent['requirements']if r['id']in close.LEAVES)['state']='passed'
        with self.assertRaises(ValueError):close.close_rows(parent,self.cases())

    def test_partial_report_leaves_incomplete_profile_pending(self):
        rows=close.close_rows(self.parent,self.cases()[:-1],partial=True)
        self.assertEqual(sum(r['state']=='passed'for r in rows),self.parent['passed_requirements']+3)
        pending=[r for r in rows if r['id']in close.LEAVES and r['state']=='pending']
        self.assertEqual([(r['configuration'],r['id'])for r in pending],[('player-software','journey.creatures-effects.combat')])

    def test_one_size_cannot_close_an_attachment_profile(self):
        cases=[c for c in self.cases()if not(c['purpose']=='attachment'and c['software']and c['actual_extent']==[640,480])]
        rows=close.close_rows(self.parent,cases,partial=True)
        self.assertEqual(sum(r['state']=='passed'for r in rows),self.parent['passed_requirements']+3)
        pending=next(r for r in rows if r['id']=='journey.creatures-effects.attachment'and r['configuration']=='player-software')
        self.assertEqual(pending['state'],'pending')

    def test_later_complete_case_preserves_every_earlier_partial_row(self):
        cases=self.cases();parent=deepcopy(self.parent)
        parent['requirements']=close.close_rows(parent,cases[:-1],partial=True)
        parent['passed_requirements']+=3;parent['pending_requirements']-=3
        rows=close.close_rows(parent,cases,partial=True)
        self.assertEqual(sum(r['state']=='passed'for r in rows),parent['passed_requirements']+1)
        for old,new in zip(parent['requirements'],rows):
            if old['state']=='passed':self.assertEqual(old,new)


class BudgetPhaseControls(unittest.TestCase):
    """Only two numerical host admission expressions may change source phase."""
    def versions(self):
        before=(ROOT/close.COLLECTOR).read_bytes()
        # Tests remain usable after the reviewed numerical transition.
        for old,new in close.BUDGET_EDITS:
            if old not in before:
                before=before.replace(b"plan['maximum_actual_games'] in (5,6,8,9,10)",old)if old==close.BUDGET_EDITS[0][0]else before
                before=before.replace(b"plan['maximum_actual_games'] in (5,6,8,9)",old)if old==close.BUDGET_EDITS[0][0]else before
                before=before.replace(b"plan['maximum_actual_games'] in (5,6,8)",old)if old==close.BUDGET_EDITS[0][0]else before
                before=before.replace(new,old)
        after=before
        for old,new in close.BUDGET_EDITS:after=after.replace(old,new)
        return before,after

    def test_exact_two_expression_budget_transition(self):close.budget_only_change(*self.versions())

    def test_exact_eight_start_transition(self):
        before,after=self.versions()
        close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8)"),8)

    def test_six_start_source_cannot_claim_eight(self):
        with self.assertRaises(ValueError):close.budget_only_change(*self.versions(),maximum=8)

    def test_eight_start_source_requires_explicit_eight_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8)"))

    def test_exact_ninth_start_transition(self):
        before,after=self.versions()
        close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8,9)"),9)

    def test_eight_start_source_cannot_claim_nine(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8)"),9)

    def test_ninth_start_source_requires_explicit_ninth_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8,9)"),8)

    def test_an_unreviewed_eleventh_start_bound_is_rejected(self):
        with self.assertRaises(ValueError):close.budget_only_change(*self.versions(),maximum=11)

    def test_exact_tenth_start_transition(self):
        before,after=self.versions()
        close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8,9,10)"),10)

    def test_ninth_source_cannot_claim_ten(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8,9)"),10)

    def test_tenth_source_requires_explicit_tenth_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,after.replace(close.BUDGET_EDITS[0][1],b"plan['maximum_actual_games'] in (5,6,8,9,10)"),9)

    def test_native_or_input_change_cannot_hide_in_budget_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,after.replace(b'journey.run()',b'journey.other_route()'))

    def test_unreviewed_extra_collector_edit_rejected(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,after+b'\n# another edit\n')

    def test_missing_original_preimage_rejected(self):
        with self.assertRaises(ValueError):close.budget_only_change(b'',self.versions()[1])

    def test_only_one_budget_expression_changed_rejected(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.budget_only_change(before,before.replace(*close.BUDGET_EDITS[0]))

    def test_review_does_not_waive_native_hash(self):
        reader=type('Reader',(),{'digest':lambda self,n:'current'})()
        phase={'before':{'sha256':'old'}}
        with self.assertRaises(ValueError):close.checked_sources(reader,{'runtime/scene/canvas_producers.c':'old'},phase)

    def test_review_does_not_waive_pixel_analyzer_hash(self):
        reader=type('Reader',(),{'digest':lambda self,n:'current'})()
        with self.assertRaises(ValueError):close.checked_sources(reader,{'tools/scene_actor_output.py':'old'},{'before':{'sha256':'old'}})

    def test_wrong_collector_preimage_hash_rejected(self):
        reader=type('Reader',(),{'digest':lambda self,n:'current'})()
        with self.assertRaises(ValueError):close.checked_sources(reader,{close.COLLECTOR:'foreign'},{'before':{'sha256':'old'}})


class PostProofControls(unittest.TestCase):
    def versions(self):
        after=(ROOT/close.DRIVER).read_bytes()
        before=after.replace(close.POSTPROOF_AFTER,close.POSTPROOF_BEFORE)
        return before,after

    def test_exact_postproof_turn_removal(self):close.postproof_only_change(*self.versions())

    def test_three_observations_cannot_replace_four(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.postproof_only_change(before,after.replace(b'range(4)',b'range(3)'))

    def test_recipe_change_is_not_postproof_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.postproof_only_change(before,after.replace(b'== 83',b'== 84'))

    def test_exit_timing_change_is_not_postproof_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.postproof_only_change(before,after.replace(b"0xff1b,.6",b"0xff1b,.1"))

    def test_extra_edit_is_not_postproof_review(self):
        before,after=self.versions()
        with self.assertRaises(ValueError):close.postproof_only_change(before,after+b'\n# unrelated edit\n')

    def test_postproof_review_never_waives_runtime_hash(self):
        reader=type('Reader',(),{'digest':lambda self,n:'current'})()
        with self.assertRaises(ValueError):close.checked_sources(reader,{'runtime/scene/world_raster_callers.h':'old'},None,postproof={'before':{'sha256':'old'}})

    def test_reviewed_inventory_source_is_separate(self):
        reader=type('Reader',(),{'digest':lambda self,n:'current'})()
        close.checked_sources(reader,{close.DRIVER:'old'},None,postproof={'before':{'sha256':'old'}})
        with self.assertRaises(ValueError):close.checked_sources(reader,{close.DRIVER:'foreign'},None,postproof={'before':{'sha256':'old'}})

    def test_four_views_finish_at_the_fourth_view_with_three_turns(self):
        import tempfile
        from pathlib import Path
        journey=attachment.AttachmentJourney.__new__(attachment.AttachmentJourney)
        journey.size=(640,480);journey.proof={'cast_requests':0};journey.out=Path('unused');journey.view=0;journey.active=False
        keys=[];captures=[]
        def frame():
            a=[0]*200;a[4]=100;a[12]=3 if journey.active else 1;a[14]=int(journey.active);a[185]=int(journey.active)
            return dict(generation=20+len(keys),view=journey.view,extent=(640,480),actors=[tuple(a)],draws=[(0,0,0,0,0,20,20,0),(0,1,0,0,0,20,20,0)])
        def until(predicate,seconds):
            value=frame();self.assertTrue(predicate(value));return value
        def click(point,button=1):
            if button==3:journey.active=True
        def key(label,symbol,hold=.2):keys.append((label,symbol));journey.view=(journey.view-1)%4
        journey.until_frame=until;journey.frame_click=click;journey.game_key=key;journey.key=lambda *args:None
        journey.capture=lambda label:captures.append((label,journey.view))
        class Sources:
            def __init__(self,*args):pass
            def target(self,*args):return {'point':[10,10]}
            def close(self):pass
        with tempfile.TemporaryDirectory()as directory:
            journey.session=Path(directory)
            (journey.session/'input-admission.bin').write_bytes(b'MNMCAI01'+struct.pack('<II',3,144))
            selected=dict(flags=1,spell_count=1,receiver_slot_plus_one=1,selected_slot=0,mode=2)
            with patch.object(attachment,'LiveSources',Sources),patch.object(attachment,'input_observations',lambda raw:{'rows':[selected]}),patch.object(attachment.time,'sleep'):
                journey.cast_and_observe()
        self.assertEqual([v['view']for v in journey.proof['views']],[0,3,2,1])
        self.assertEqual([v[1]for v in keys],[ord('.')]*3)
        self.assertEqual(captures[-1],('world-active-attachment',1))
        self.assertTrue(journey.proof['success'])


if __name__=='__main__':unittest.main()
