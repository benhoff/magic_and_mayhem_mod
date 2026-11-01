#!/usr/bin/env python3
"""Fail-closed controls for the EP06 evidence frontier."""
import copy
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import rendering_ep06 as ep
from rendering_acceptance import requirements


class Frontier(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        cls.candidates = [dict(id=n, closes_requirement=False,current_source_binding_state='changed') for n in sorted({e for r in requirements(cls.plan) if r['owner_task']=='EP06' and r['kind']=='operation' for e in r['candidate_evidence']})]

    def test_failed_session_individual_calls_never_admit_an_operation(self):
        negative=dict(configuration='player-default',execution={'record':'failed','sha256':'a'*64},
                      copy=dict(checked_original_gpu_calls=848,missing_output_calls=[4010,4016]))
        result=ep.reconcile(self.plan,self.candidates,[],[negative])
        selected=[r for r in result['rows'] if r['failed_session_scoped_proofs']]
        self.assertEqual(len(selected),1)
        self.assertEqual(selected[0]['requirement'],'operation.copy-opaque')
        self.assertFalse(selected[0]['whole_operation_accepted'])
        self.assertEqual(result['whole_operations_accepted'],0)

    def test_exact_denominator_no_attachment_promotions(self):
        result = ep.reconcile(self.plan, self.candidates, [])
        self.assertEqual((result['requirements_total'], result['ep06_obligations'], result['ep09_obligations_untouched']), (156,36,6))
        self.assertEqual(result['whole_operations_accepted'], 0)
        self.assertEqual({r['state'] for r in result['rows']}, {'pending'})
        self.assertTrue(all(r['contract_remaining'] and r['next_proof'] for r in result['rows']))
        self.assertFalse(result['player_accepted'] or result['original_recovery_accepted'])

    def test_unknown_evidence_does_not_disappear(self):
        with self.assertRaises(KeyError):
            ep.reconcile(self.plan, self.candidates[:-1], [])

    def test_zero_branch_and_whole_frames_cannot_prove_keyed_gpu(self):
        fact = dict(admission=dict(pair={'test':'binding'}, world_extents=[[800,600]],provider='llvmpipe'),
                    copy=dict(all_application_output_closed=False,branches={'keyed_gpu':dict(proven=False)}),
                    producer=dict(operations={}),cpu_reads=[])
        self.assertEqual(ep.scoped_facets('copy-keyed', [fact]), [])
        history = ep.scoped_facets('destination-dependency', [fact])
        self.assertEqual(len(history),1)
        self.assertEqual(ep.scoped_facets('fill-full', [fact]), [])
        self.assertEqual(ep.scoped_facets('text-font-dc-lifetime', [fact]), [])
        self.assertEqual(ep.scoped_facets('flags-and-return-values', [fact]), [])

    def test_diagnostic_and_software_admission_remain_mandatory(self):
        with patch.object(ep, 'admit', side_effect=ValueError('Nominal target required')) as admission:
            with self.assertRaises(ValueError):
                ep.nominal_facts(ROOT, None, 'player-software', {})
            admission.assert_called_once_with(None, {}, 'player-software')

    def test_observer_flags_need_typed_false(self):
        base=dict(configuration=dict(operation_census=dict(copy_scope='all',coordinate_exercise=False,changes_actions=False,changes_admission=False)))
        class Reader:
            def digest(self, name): return 'a'*64
        for bad in (None, {}, dict(base['configuration']['operation_census'],changes_actions=0),dict(base['configuration']['operation_census'],coordinate_exercise=True)):
            acquired=copy.deepcopy(base);acquired['configuration']['operation_census']=bad
            with patch.object(ep,'admit',return_value=dict(sources={'x':'a'*64})), patch.object(ep,'receipt',side_effect=[acquired,{}]):
                with self.assertRaises(ValueError): ep.nominal_facts(ROOT,Reader(),'player-default',{'acquisition':{},'comparison':{}})

    def test_physical_inputs_must_match_retained_bytes(self):
        class Reader:
            def digest(self, name): return hashlib.sha256(b'original').hexdigest()
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary);path=root/'raw.bin';path.write_bytes(b'original')
            acquired=dict(artifacts=[dict(path='raw.bin',record='retained',sha256=Reader().digest('retained'))])
            self.assertEqual(len(ep.bind_artifacts(root,Reader(),acquired,[path])),1)
            path.write_bytes(b'changed')
            with self.assertRaises(ValueError):ep.bind_artifacts(root,Reader(),acquired,[path])
            path.unlink();path.symlink_to(root/'other')
            with self.assertRaises(ValueError):ep.bind_artifacts(root,Reader(),acquired,[path])


if __name__ == '__main__':
    unittest.main()
