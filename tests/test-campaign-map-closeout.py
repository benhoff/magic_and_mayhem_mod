#!/usr/bin/env python3
"""Refuse incomplete domains and unintended acceptance promotions."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('map_closeout',ROOT/'tools/close-campaign-map-transitions.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


def frozen_campaign_plan(current,parent):
    # Restore only recorded candidate leads in an in-memory requirements fixture.
    from coverage_claims import fingerprint
    from rendering_acceptance import requirements,validate
    plan=copy.deepcopy(current)
    if fingerprint(requirements(plan))!=parent['dossier_identity']:
        old={v['id']:v['candidate_evidence'] for v in parent['requirements'] if v['configuration']=='player-default'}
        groups=plan['required_gap_register']
        for kind,rows in [('journey',groups['journeys']),('operation',groups['operations']),('gate',groups['gates'])]:
            for row in rows:
                key=kind+'.'+row['id']+('.'+row['subrequirements'][0]['id'] if kind=='journey' else '')
                before=old[key]
                if not set(before)<=set(row.get('evidence_leads',[])):
                    raise ValueError('Frozen campaign candidate evidence was removed')
                row['evidence_leads']=copy.deepcopy(before)
        plan['acceptance_record']['bindings']['required_gap_register']['sha256']=fingerprint(plan['required_gap_register'])
    validate(plan)
    if fingerprint(requirements(plan))!=parent['dossier_identity']:
        raise ValueError('Frozen campaign acceptance conditions or owners changed')
    return plan


class Closeout(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.parent=r.json(m.PARENT)
        cls.plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        cls.plan=frozen_campaign_plan(cls.plan,cls.parent)

    def cases(self):
        return [dict(identity=str(i),configuration=p,extent=s,verified=True,provider='llvmpipe',
                     acquisition=dict(record='a'+str(i),sha256='a'*64),
                     comparison=dict(record='c'+str(i),sha256='b'*64))
                for i,(p,s) in enumerate((p,s) for p in ('player-default','player-software')
                                        for s in ([640,480],[800,600]))]

    def test_only_four_rows_change_with_two_sizes_per_row(self):
        rows=m.close_rows(self.plan,self.parent,self.cases())
        changed=[(a,b) for a,b in zip(self.parent['requirements'],rows) if a!=b]
        self.assertEqual(len(changed),4)
        self.assertEqual({b['id'] for a,b in changed},set(m.LEAVES))
        self.assertTrue(all(len(b['admitted_execution_evidence'])==2 for a,b in changed))

    def refusal(self,mutate):
        cases=self.cases();mutate(cases)
        with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,cases)

    def test_missing_domain(self):self.refusal(lambda c:c.pop())
    def test_duplicate_size(self):self.refusal(lambda c:c[1].update(extent=[640,480]))
    def test_duplicate_session(self):self.refusal(lambda c:c[1].update(identity=c[0]['identity']))
    def test_failed_domain(self):self.refusal(lambda c:c[1].update(verified=False))
    def test_other_resolution(self):self.refusal(lambda c:c[1].update(extent=[1024,768]))
    def test_other_configuration(self):self.refusal(lambda c:c[1].update(configuration='other'))
    def test_changed_parent_counts(self):
        parent=copy.deepcopy(self.parent);parent['passed_requirements']=114
        with self.assertRaises(ValueError):m.close_rows(self.plan,parent,self.cases())

    @staticmethod
    def domain_fixture():
        ref=dict(record='a',sha256='a'*64)
        common=dict(success=True,sources_stable=True,original_manifest_verified_before_after=True,
                    artifact_retention_complete=True,original_pixels_used_as_native_inputs=False)
        a=common|dict(kind='public_native_progression_acquisition',configuration=dict(
            requested_size='800x600',id='player-software',software_flag=True,skip_intros=False,
            diagnostic_only=False,evidence_mode='full',capture_policy=dict(pixel_comparison_eligible=True),
            terrain_lighting='native',native_lighting_capture=False,
            map_resource_observation='bounded-world-resource-boundaries-v1'),exit_code=0,
            private_wine_stopped=True,compiler_closure_stable=True,player_binary_stable=True,
            retirement=dict(state='completed',forwarded=0),player_build=dict(binary='exact'),
            resident=dict(renderer='llvmpipe test',imports='4',verified_returns='4',completed='4',
                          pending=False,stage_profile=dict(capture=dict(accepted_worlds=4,written_worlds=4,
                          complete=True,finished=True,truncated=False,error='',queue_bytes=0))))
        c=common|dict(kind='public_native_progression_comparison',acquisition=ref,
            comparison_processes_stopped=True,player_build_binding=dict(verified=True,build=a['player_build']),
            world_domain=dict(verified=True,format='RGB565',extents=[[800,600]]),original_reference=dict(worlds=4))
        return a,c,ref

    def test_complete_domain(self):
        self.assertEqual(m.domain(*self.domain_fixture()),('player-software',[800,600],4))

    def test_domain_negative_controls(self):
        mutations=[lambda a,c:a['configuration'].update(diagnostic_only=True),
            lambda a,c:a['configuration'].update(evidence_mode='diagnostic'),
            lambda a,c:a['configuration'].update(skip_intros=True),
            lambda a,c:a['configuration'].update(map_resource_observation='absent'),
            lambda a,c:c['world_domain'].update(extents=[[640,480]]),
            lambda a,c:a['resident'].update(renderer='hardware unknown'),
            lambda a,c:a['resident'].update(verified_returns='3'),
            lambda a,c:a['resident']['stage_profile']['capture'].update(truncated=True),
            lambda a,c:a['resident']['stage_profile']['capture'].update(queue_bytes=1),
            lambda a,c:a.update(sources_stable=False),
            lambda a,c:c.update(original_pixels_used_as_native_inputs=True),
            lambda a,c:c.update(acquisition=dict(record='different',sha256='a'*64)),
            lambda a,c:a['retirement'].update(forwarded=1),
            lambda a,c:c['player_build_binding'].update(build=dict(binary='different'))]
        for mutate in mutations:
            with self.subTest(mutation=mutate):
                a,c,ref=self.domain_fixture();mutate(a,c)
                with self.assertRaises(ValueError):m.domain(a,c,ref)


class ControllerPhase(unittest.TestCase):
    def fixture(self):
        p='tools/run-rendering-progression.py';sources={p:'a'*64,'runtime/menu/observer.c':'c'*64};current=dict(sources);current[p]='b'*64
        a=dict(record='acquisition',sha256='d'*64);c=dict(record='comparison',sha256='e'*64)
        review=dict(kind='campaign_controller_source_applicability',success=True,original_game_runs=0,historical_hashes_rewritten=False,current_policy_execution_claimed=False,acquisition=a,comparison=c,execution_sources=sources,current_sources=current,changed_sources={p:dict(before=sources[p],after=current[p],reason='Exact owner binding only; preserve old native execution')})
        controls=dict(success=True,sources_stable=True,original_game_executed=False,sources=current)
        return sources,current,review,a,c,controls
    def test_review_preserves_native_execution_without_new_policy_execution_credit(self):
        q=m.controller_source_review(*self.fixture());self.assertTrue(q['verified']);self.assertFalse(q['current_input_policy_execution_claimed'])
    def test_native_runtime_and_capture_changes_never_use_controller_review(self):
        for path in ('runtime/menu/observer.c','renderer/opengl.cpp','tools/rendering_capture.py','tools/check-rendering-progression.py','unknown.py'):
            args=list(copy.deepcopy(self.fixture()));args[0][path]='1'*64;args[1][path]='2'*64
            with self.subTest(path=path),self.assertRaises(ValueError):m.controller_source_review(*args)
    def test_other_acquisition_or_comparison_refuses(self):
        for key in ('acquisition','comparison'):
            args=list(copy.deepcopy(self.fixture()));args[2][key]=dict(record='other',sha256='f'*64)
            with self.subTest(key=key),self.assertRaises(ValueError):m.controller_source_review(*args)
    def test_missing_or_changed_exact_source_review_refuses(self):
        for key in ('execution_sources','current_sources','changed_sources'):
            args=list(copy.deepcopy(self.fixture()));args[2][key]={}
            with self.subTest(key=key),self.assertRaises(ValueError):m.controller_source_review(*args)
    def test_failed_or_stale_controls_refuse(self):
        for change in (dict(success=False),dict(sources_stable=False),dict(original_game_executed=True),dict(sources={})):
            args=list(copy.deepcopy(self.fixture()));args[-1].update(change)
            with self.subTest(change=change),self.assertRaises(ValueError):m.controller_source_review(*args)
    def test_rewritten_history_or_new_input_policy_claim_refuses(self):
        for change in (dict(historical_hashes_rewritten=True),dict(current_policy_execution_claimed=True),dict(original_game_runs=1),dict(kind='unreviewed')):
            args=list(copy.deepcopy(self.fixture()));args[2].update(change)
            with self.subTest(change=change),self.assertRaises(ValueError):m.controller_source_review(*args)
    def test_changed_source_hashes_and_reason_are_mandatory(self):
        for change in (dict(before='0'*64),dict(after='0'*64),dict(reason='')):
            args=list(copy.deepcopy(self.fixture()));args[2]['changed_sources']['tools/run-rendering-progression.py'].update(change)
            with self.subTest(change=change),self.assertRaises(ValueError):m.controller_source_review(*args)


if __name__=='__main__':unittest.main()
