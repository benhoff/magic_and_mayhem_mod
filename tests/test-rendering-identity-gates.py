#!/usr/bin/env python3
"""Adverse identity admission checks using the actual retained accepted cohort."""
import copy
import importlib.util
import os
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
s = importlib.util.spec_from_file_location('identity_gates', ROOT/'tools/close-rendering-identity-gates.py')
m = importlib.util.module_from_spec(s)
s.loader.exec_module(m)


class IdentityChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.q = r.json(os.environ.get('MNM_IDENTITY_RESULT', m.PREFIX+'closeout-analysis-02.json'))
            assert cls.q['success']
            cls.parent = r.json(m.PARENT)
            cls.plan = r.json('docs/native-rendering-closure.json')
            cls.case = cls.q['cases'][0]
            a = r.json(cls.case['reference']['record'])
            cls.args = [a, *[r.json(cls.case[k]['record']) for k in ('admission','staged_manifest')],
                        *[__import__('json').loads(artifact(r,a,n)) for n in ('native-only-session.json','original-process-result.json')], cls.case['profile']]
            cls.manifest = r.read_bytes(a['launch_manifest']['record'])
            cls.inputs = a['installation_inputs']
            cls.original = cls.case['installation']['original_executable_sha256']
            cls.player = r.read_bytes(cls.case['player']['reference']['record'])

    def bad_case(self, change):
        c = copy.deepcopy(self.case)
        change(c)
        with self.assertRaises(ValueError):m.check_case(c)

    def bad_config(self, change):
        args = copy.deepcopy(self.args)
        change(args)
        with self.assertRaises(ValueError):m.configuration(*args)

    def test_all_actual_cases(self):
        for c in self.q['cases']:m.check_case(c)

    def test_actual_configuration(self):
        self.assertEqual(m.configuration(*self.args), self.case['policy'])

    def test_actual_installation(self):
        self.assertEqual(m.installation(self.manifest,self.inputs,self.original),self.case['installation'])

    def test_actual_player_compiler(self):
        self.assertEqual(m.elf_compiler(self.player,self.case['player']['reference']['sha256']),self.case['player']['compiler_banners'])

    def test_original_executable_substitution(self):
        with self.assertRaises(ValueError):m.installation(self.manifest,self.inputs,'0'*64)

    def test_duplicate_installation_entry(self):
        with self.assertRaises(ValueError):m.installation(self.manifest+self.manifest.splitlines(True)[1],self.inputs,self.original)

    def test_installation_path_escape(self):
        with self.assertRaises(ValueError):m.installation(self.manifest+b'0'*64+b'\t1\t../escape\n',self.inputs,self.original)

    def test_missing_installation_input(self):
        v = dict(self.inputs);v.pop(next(iter(v)))
        with self.assertRaises(ValueError):m.installation(self.manifest,v,self.original)

    def test_changed_input_size(self):
        v = copy.deepcopy(self.inputs);v[next(iter(v))]['bytes']+=1
        with self.assertRaises(ValueError):m.installation(self.manifest,v,self.original)

    def test_changed_input_hash(self):
        v = copy.deepcopy(self.inputs);v[next(iter(v))]['sha256']='0'*64
        with self.assertRaises(ValueError):m.installation(self.manifest,v,self.original)

    def test_actual_low_resolution_override(self):
        with Reader(ROOT) as r:
            c=next(c for c in self.q['cases'] if c['installation']['effective_preference_override'])
            a=r.json(c['reference']['record'])
            self.assertEqual(m.installation(r.read_bytes(a['launch_manifest']['record']),a['installation_inputs'],self.original,c['installation']['effective_preference_override']),c['installation'])

    def test_other_preference_override_refuses(self):
        c=next(c for c in self.q['cases'] if c['installation']['effective_preference_override'])
        bad=copy.deepcopy(c['installation']['effective_preference_override']);bad['path']='other'
        with self.assertRaises(ValueError):m.installation(self.manifest,self.inputs,self.original,bad)

    def test_unverified_preference_override_refuses(self):
        c=next(c for c in self.q['cases'] if c['installation']['effective_preference_override'])
        bad=copy.deepcopy(c['installation']['effective_preference_override']);bad['byte_delta_verified']=False
        with self.assertRaises(ValueError):m.installation(self.manifest,self.inputs,self.original,bad)

    def test_wrong_player_hash(self):
        with self.assertRaises(ValueError):m.elf_compiler(self.player,'0'*64)

    def test_wrong_player_architecture(self):
        b=bytearray(self.player);b[18:20]=b'\x03\0'
        with self.assertRaises(ValueError):m.elf_compiler(bytes(b),m.sha(b))

    def test_installation_payload_check_required(self):
        self.bad_case(lambda c:c.update(installation_payloads_verified=False))

    def test_original_payload_check_required(self):
        self.bad_case(lambda c:c.update(original_payload_verified=False))

    def test_staged_payload_check_required(self):
        self.bad_case(lambda c:c['staged_executable'].update(payload_verified=False))

    def test_player_payload_check_required(self):
        self.bad_case(lambda c:c['player'].update(payload_verified=False))

    def test_compiler_banner_required(self):
        self.bad_case(lambda c:c['player'].update(compiler_banners=[]))

    def test_missing_native_dll(self):
        self.bad_case(lambda c:c['dlls'].pop())

    def test_duplicate_native_dll(self):
        self.bad_case(lambda c:c['dlls'].__setitem__(0,c['dlls'][1]))

    def test_provider_required(self):
        self.bad_case(lambda c:c['policy'].update(observed_renderer=''))

    def test_exact_configuration_required(self):
        self.bad_case(lambda c:c['policy']['configuration'].update(skip_intros=not c['policy']['configuration']['skip_intros']))

    def test_force_stop_refuses(self):
        self.bad_config(lambda a:a[0].update(forced_private_prefix_stop=True))

    def test_launch_software_flag_must_match(self):
        self.bad_config(lambda a:a[0]['command'].append('--software-rendering'))

    def test_cross_session_refuses(self):
        self.bad_config(lambda a:a[1].update(session='other'))

    def test_cross_prefix_refuses(self):
        self.bad_config(lambda a:a[4].update(prefix='other'))

    def test_policy_substitution_refuses(self):
        self.bad_config(lambda a:a[2].update(native_only_policy='other'))

    def test_legacy_embedding_refuses(self):
        self.bad_config(lambda a:a[1].update(legacy_embedding=True))

    def test_unrecorded_physical_provider_refuses(self):
        self.bad_config(lambda a:a[1].update(host_graphics_devices=True))

    def test_namespace_boundary_required(self):
        self.bad_config(lambda a:a[1].update(namespace_pid=False))

    def test_complete_replacement_is_not_inferred(self):
        self.bad_config(lambda a:a[1].update(complete_replacement=True))

    def test_unknown_renderer_refuses(self):
        self.bad_config(lambda a:a[0]['resident'].update(renderer='Unknown physical driver'))

    def test_diagnostic_run_refuses(self):
        self.bad_config(lambda a:a[0]['configuration'].update(diagnostic_only=True))

    def test_cohort_omission_refuses(self):
        with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.q['cohort'],self.q['cases'][:-1])

    def test_conflicting_compiler_closure_refuses(self):
        cases=copy.deepcopy(self.q['cases']);cases[0]['player']['compiler_source_sha256']='0'*64
        with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.q['cohort'],cases)

    def test_cohort_profile_substitution_refuses(self):
        cases=copy.deepcopy(self.q['cases']);cases[0]['profile']='player-software'
        with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.q['cohort'],cases)

    def test_exact_two_rows_and_all_parent_rows(self):
        rows=m.close_rows(self.plan,self.parent,self.q['cohort'],self.q['cases'])
        changed=[(a['configuration'],a['id']) for a,b in zip(rows,self.parent['requirements']) if a!=b]
        self.assertEqual(set(changed),{(p,m.LEAF) for p in ('player-default','player-software')})
        self.assertEqual(rows,self.q['requirements'])
        self.assertEqual(self.q['decisions'],self.parent['decisions'])


if __name__ == '__main__':unittest.main()
