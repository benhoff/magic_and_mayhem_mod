#!/usr/bin/env python3
"""Reject substituted result, map, output and acceptance-domain witnesses."""
import copy
import json
from pathlib import Path
import struct
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from campaign_results_review import (CASES,PARENT,LEAVES,victory_route,map_identity,later_output,close_rows)
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact


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


class Results(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.a=r.json(CASES[0]);cls.parent=r.json(PARENT)
            cls.events=artifact(r,cls.a,'events.bin');cls.preparation=r.json(cls.a['preparation']['record'])
        cls.plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes())
        cls.plan=frozen_campaign_plan(cls.plan,cls.parent)

    def test_frozen_definition_does_not_modify_current_master_plan(self):
        current=json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes());before=copy.deepcopy(current)
        frozen_campaign_plan(current,self.parent)
        self.assertEqual(current,before)

    def test_frozen_definition_refuses_a_changed_acceptance_condition(self):
        from coverage_claims import fingerprint
        current=json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes())
        current['required_gap_register']['journeys'][0]['subrequirements'][0]['remaining']+=' changed condition'
        current['acceptance_record']['bindings']['required_gap_register']['sha256']=fingerprint(current['required_gap_register'])
        with self.assertRaises(ValueError):frozen_campaign_plan(current,self.parent)

    def test_actual_original_victory_and_later_identity(self):
        result=victory_route(self.a['flow'],self.events)
        self.assertTrue(result['verified']);self.assertFalse(result['native_result_policy']['widget_pixels_compared'])
        identity=map_identity(self.a,self.preparation,result)
        self.assertEqual(identity['name'],'The Plains of Greenhenge.')
        self.assertFalse(identity['assembled_section_choices_recovered'])

    def refuse_route(self,mutate):
        f=copy.deepcopy(self.a['flow']);mutate(f)
        with self.assertRaises(ValueError):victory_route(f,self.events)

    def test_defeat_refused(self):self.refuse_route(lambda f:f['states'][4].update(outcome=0))
    def test_quit_refused(self):self.refuse_route(lambda f:f['states'][4]['texts'].__setitem__(19,'You quit the battle.'))
    def test_wrong_result_title(self):self.refuse_route(lambda f:f['states'][4].update(title='Other outcome'))
    def test_stale_result_ack(self):self.refuse_route(lambda f:f['states'][5].update(ack=f['states'][4]['ack']))
    def test_wrong_result_parent(self):self.refuse_route(lambda f:f['states'][5].update(screen=3))
    def test_same_map_refused(self):self.refuse_route(lambda f:f['worlds'][1].update(region=1))
    def test_reload_refused(self):self.refuse_route(lambda f:f['states'][7].update(event='load'))

    def test_changed_original_callback_refused(self):
        rows=[list(r) for r in struct.iter_unpack('<16I',self.events[16:])]
        next(r for r in rows if r[1]==3 and r[3]==6 and r[9]==21)[2]+=1
        raw=self.events[:16]+b''.join(struct.pack('<16I',*r) for r in rows)
        with self.assertRaises(ValueError):victory_route(self.a['flow'],raw)

    def test_changed_map_input_refused(self):
        a=copy.deepcopy(self.a);name=self.preparation['maps'][1]['inputs'][0]['path']
        a['installation_inputs'][name]['sha256']='0'*64
        with self.assertRaises(ValueError):map_identity(a,self.preparation,victory_route(self.a['flow'],self.events))

    def test_missing_campaign_configuration_refused(self):
        a=copy.deepcopy(self.a);a['installation_inputs'].pop('Realms/Celtic/Celtic.cfg')
        with self.assertRaises(ValueError):map_identity(a,self.preparation,victory_route(self.a['flow'],self.events))

    def test_recipe_is_not_actual_admission(self):
        route=victory_route(self.a['flow'],self.events);route['later_entry']['region']=1
        with self.assertRaises(ValueError):map_identity(self.a,self.preparation,route)

    @staticmethod
    def output_fixture():
        flow=dict(worlds=[dict(world=1),dict(after_world=3,world=5,paint=50,entry_mode='campaign-travel',realm='Celtic',region=2)])
        rows=[dict(paint=p,lease=p,reference_binding=p,immutable_lease=True,world=w,pixels=12,
                   actual_sha256='a'*64,expected_sha256='a'*64,mismatching_pixels=0)
              for p,w in ((40,4),(50,5),(60,6))]
        observed=dict(swaps=[40,50,60],paints={r['paint']:(dict(r),None) for r in rows})
        return flow,observed,dict(comparisons=rows)

    def test_first_probe_can_skip_but_every_displayed_frame_is_bound(self):
        result=later_output(*self.output_fixture())
        self.assertEqual(result['first_displayed']['world'],4)
        self.assertEqual(result['resumed']['world'],6)
        self.assertEqual(result['compared_swaps'],3)

    def test_uncompared_later_swap_refused(self):
        f,o,c=self.output_fixture();c['comparisons'].pop()
        with self.assertRaises(ValueError):later_output(f,o,c)

    def test_missing_resumed_world_refused(self):
        f,o,c=self.output_fixture();c['comparisons'].pop();o['swaps'].pop()
        with self.assertRaises(ValueError):later_output(f,o,c)

    def test_changed_later_pixels_refused(self):
        f,o,c=self.output_fixture();c['comparisons'][0]['expected_sha256']='b'*64
        with self.assertRaises(ValueError):later_output(f,o,c)

    def test_swap_multiplicity_is_not_collapsed(self):
        f,o,c=self.output_fixture();o['swaps'].append(60)
        with self.assertRaises(ValueError):later_output(f,o,c)

    def test_wrong_identity_paint_refused(self):
        f,o,c=self.output_fixture();f['worlds'][1]['paint']=51
        with self.assertRaises(ValueError):later_output(f,o,c)

    def cases(self):
        return [dict(identity=str(i),configuration=p,extent=list(s),verified=True,provider='llvmpipe',
                acquisition=dict(record='a'+str(i),sha256='a'*64),comparison=dict(record='c'+str(i),sha256='b'*64))
                for i,(p,s) in enumerate((p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600)))]

    def test_only_requested_four_cells_change(self):
        rows=close_rows(self.plan,self.parent,self.cases());changed=[b for a,b in zip(self.parent['requirements'],rows) if a!=b]
        self.assertEqual(len(changed),4);self.assertEqual({r['id'] for r in changed},set(LEAVES))

    def test_missing_duplicate_wrong_or_failed_domain_refused(self):
        for mutate in (lambda c:c.pop(),lambda c:c[1].update(identity=c[0]['identity']),
                       lambda c:c[1].update(extent=[640,480]),lambda c:c[1].update(verified=False),
                       lambda c:c[1].update(configuration='other')):
            with self.subTest(mutate=mutate):
                c=self.cases();mutate(c)
                with self.assertRaises(ValueError):close_rows(self.plan,self.parent,c)


if __name__=='__main__':unittest.main()
