import copy
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_importance import task_work_review


class Display(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root/'impl.py').write_text('pass\n')
        self.labels = json.loads((ROOT/'apps/coverage-ui/importance.json').read_text())['tranche_plan']['work_labels']
        self.behavior = dict(implementation=['impl.py'], tests=[], documents=[], evidence=['TEST.profiles'])
        self.work = dict(implementation='implemented_within_scope', implementation_gaps=[],
            current_state='Supplied bounded mechanism', validation_state='historical',
            validation_summary='Recorded snapshot only', acceptance='pending',
            acceptance_summary='Wider acceptance remains pending', evidence=['TEST.profiles'],
            next_steps=[dict(kind='validation', action='Exercise the separately declared wider case')],
            source_sha256={'impl.py':hashlib.sha256(b'pass\n').hexdigest()},
            code_witnesses=[dict(path='impl.py', line=1, text='pass')])

    def view(self):
        return task_work_review(self.work, self.labels, self.behavior, self.root)

    def unresolved(self):
        self.work['implementation'] = 'unresolved_aggregate'
        self.work['scope_resolution_record'] = 'impl.py'
        self.work['scope_domains'] = [dict(name='Physical driver recovery', disposition='unresolved',
            finding='Delivered storage is distinct from an original physical driver recovery contract',
            validation='No physical loss capture is claimed', suppliers=['TEST.supplier'], paths=['impl.py'],
            required_case='Lose a context with pending work and capture ownership/results',
            closure_condition='Declare and pass the original/native loss or guarded restart matrix')]

    def profile(self):
        name = 'research/runtime/profiles.json'
        record = dict(success=True, sources_stable=True, inputs_stable=True,
            policy='critical-rendering-correct-output-v1',
            whole_renderer_accepted=False, all_critical_behaviors_accepted=False,
            accepted_profiles={backend:{p:dict(outcome='PASS', frames=n, mismatches=0)
                for p,n in [('corrected_storage',222),('quick_battle',323)]}
                for backend in ('software','hardware')},
            legacy_diagnostics={backend:dict(outcome='EXPECTED_DIFFERENCE', blocking=False)
                for backend in ('software','hardware')})
        p=self.root/name;p.parent.mkdir(parents=True);p.write_text(json.dumps(record))
        self.work.update(acceptance_profile_record=name, acceptance_profile_evidence='TEST.profiles')
        self.work['source_sha256'][name]=hashlib.sha256(p.read_bytes()).hexdigest()
        return p,record

    def test_unknown_required_scope_has_a_concrete_case_and_closure(self):
        self.unresolved()
        self.assertEqual(self.view()['scope_domains'][0]['disposition'],'unresolved')
        for key in ('required_case','closure_condition'):
            before=copy.deepcopy(self.work);self.work['scope_domains'][0][key]=''
            with self.assertRaises(ValueError):self.view()
            self.work=before

    def test_recorded_bounded_validation_requires_named_registered_results(self):
        self.work['validation_state'] = 'recorded'
        with self.assertRaises(ValueError): self.view()
        self.work['validation_evidence'] = ['TEST.profiles']
        self.assertEqual(self.view()['validation_state'], 'recorded')
        self.work['validation_evidence'] = ['UNREGISTERED.result']
        with self.assertRaises(ValueError): self.view()

    def test_unknown_required_scope_cannot_be_labelled_implemented(self):
        self.unresolved();self.work['implementation']='implemented_within_scope'
        with self.assertRaises(ValueError):self.view()

    def test_profiles_show_scoped_passes_and_preserve_wider_pending_acceptance(self):
        self.profile();view=self.view()
        self.assertEqual(len(view['acceptance_profiles']),4)
        self.assertEqual(view['acceptance'],'pending')
        self.assertFalse(view['legacy_diagnostics']['software']['blocking'])

    def test_profile_file_must_match_its_recorded_hash_and_evidence(self):
        p,record=self.profile();p.write_text('{}')
        with self.assertRaises(ValueError):self.view()
        p.write_text(json.dumps(record))
        self.work['acceptance_profile_evidence']='UNREGISTERED'
        with self.assertRaises(ValueError):self.view()

    def test_incomplete_failed_or_global_profile_claims_refuse_even_with_updated_hash(self):
        p,record=self.profile()
        for case in ('hardware','frames','mismatch','global','diagnostic','unstable'):
            value=copy.deepcopy(record)
            if case=='hardware':del value['accepted_profiles']['hardware']
            if case=='frames':value['accepted_profiles']['software']['corrected_storage']['frames']=221
            if case=='mismatch':value['accepted_profiles']['software']['corrected_storage']['mismatches']=1
            if case=='global':value['whole_renderer_accepted']=True
            if case=='diagnostic':value['legacy_diagnostics']['software']['blocking']=True
            if case=='unstable':value['sources_stable']=False
            p.write_text(json.dumps(value));self.work['source_sha256'][self.work['acceptance_profile_record']]=hashlib.sha256(p.read_bytes()).hexdigest()
            with self.subTest(case=case),self.assertRaises(ValueError):self.view()


if __name__=='__main__':unittest.main()
