#!/usr/bin/env python3
"""Synthetic completeness gaps, claim guards, build identity and evidence freshness."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_audit import audit, markdown, pointer

spec = importlib.util.spec_from_file_location('binary_inventory', ROOT/'tools/inventory-binary.py')
inventory_tool = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inventory_tool)
spec = importlib.util.spec_from_file_location('coverage_sources', ROOT/'tools/index-coverage-sources.py')
source_tool = importlib.util.module_from_spec(spec)
spec.loader.exec_module(source_tool)


class CoverageTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        for path in ('code.cpp', 'test.py', 'finding.md'):
            (self.root/path).write_text('fixture\n')
        self.binary_hash = 'a'*64
        self.inventory = {'schema': 1, 'build': 'fixture', 'source_sha256': self.binary_hash,
            'sections': [{'start': 4096, 'end': 4196, 'executable': True}],
            'functions': [{'entry': 4096, 'name': 'mapped', 'ranges': [[4096,4106]], 'data_references': [], 'sha256':'c'*64},
                          {'entry': 4128, 'name': 'unknown', 'ranges': [[4128,4138]], 'data_references': [8192], 'sha256':'d'*64}],
            'unassigned_executable_ranges': [[4106,4128],[4138,4196]],
            'imports': [{'iat_va': 8192, 'dll': 'fixture.dll', 'name': 'Example', 'ordinal': None}],
            'flows': [{'site': 4097, 'owner': 4096, 'kind': 'call', 'computed': True, 'targets': []},
                      {'site': 4098, 'owner': 4096, 'kind': 'jump', 'computed': True, 'targets': [4128]}]}
        self.record = {'source_sha256': self.binary_hash, 'success': True,
                       'sources': {'code.cpp': hashlib.sha256((self.root/'code.cpp').read_bytes()).hexdigest()}}
        self.write_json('inventory.json', self.inventory)
        self.write_json('record.json', self.record)
        ev = {'id': 'original', 'kind': 'isolated_original', 'scope': 'selected arithmetic',
              'build': 'fixture', 'record': 'record.json', 'sha256': self.digest('record.json'),
              'sources_pointer': '/sources', 'original_hash_pointer': '/source_sha256',
              'assertions': [{'pointer':'/success','equals':True}]}
        native = {**ev, 'id': 'native', 'kind': 'native_integration'}
        behavior = {'id':'MV.test','title':'Selected movement','subsystem':'movement','kind':'recovered','confidence':'high_within_scope',
            'scope':'One controlled input domain', 'original':[{'build':'fixture','address':4096,'scope':'entry'}],
            'documents':['finding.md'], 'implementation':['code.cpp'], 'tests':['test.py'],
            'evidence':['original','native'], 'status':{'understanding':'scoped','implementation':'scoped',
            'comparison':'recorded','integration':'headless','replacement':'none'}}
        self.register = {'schema':1, 'builds':[{'id':'fixture','source_sha256':self.binary_hash,
            'inventory':'inventory.json','inventory_sha256':self.digest('inventory.json')}],
            'behaviors':[behavior], 'evidence':[ev,native], 'classifications':[], 'api_bindings':[], 'recovered_ranges':[],
            'dispatch_tables':[{'id':'events','original':{'build':'fixture','address':4096,'scope':'dispatch'},
                'document':'finding.md','complete':False,'scope':'Selected events',
                'entries':[{'value':0,'behaviors':['MV.test']},{'value':1,'behaviors':[]}]}],
            'scenarios':[{'id':'headless','level':'headless','scope':'synthetic session',
                         'behaviors':['MV.test'],'evidence':['native'],'tests':['test.py']}]}

    def write_json(self, name, value):
        (self.root/name).write_text(json.dumps(value))

    def digest(self, name):
        return hashlib.sha256((self.root/name).read_bytes()).hexdigest()

    def report(self):
        return audit(self.root, self.register)

    def assert_invalid(self, needle):
        report = self.report()
        self.assertTrue(any(needle in e for e in report['errors']), report['errors'])

    def refresh_record(self):
        self.write_json('record.json', self.record)
        for ev in self.register['evidence']:
            ev['sha256'] = self.digest('record.json')

    def test_unknowns_are_retained_without_failing(self):
        report = self.report()
        self.assertEqual(report['errors'], [])
        self.assertEqual(report['counts']['unclassified_functions'],1)
        self.assertEqual(report['counts']['unassigned_executable_ranges'],2)
        self.assertEqual(report['counts']['unresolved_indirect_flows'],1)
        self.assertEqual(report['counts']['computed_flows_with_inferred_targets'],1)
        self.assertEqual(report['counts']['unmapped_imports'],1)
        self.assertEqual(report['counts']['unmapped_callback_candidates'],1)
        self.assertEqual(report['counts']['unmapped_dispatch_entries'],1)
        self.assertIn('MV.test', markdown(report))

    def test_calls_into_undiscovered_code_are_visible(self):
        self.inventory['flows'].append({'site':4099,'owner':4096,'kind':'call','computed':False,'targets':[4115]})
        self.write_json('inventory.json',self.inventory)
        self.register['builds'][0]['inventory_sha256']=self.digest('inventory.json')
        report=self.report()
        self.assertEqual(report['errors'],[])
        self.assertEqual(report['counts']['calls_to_uninventoried_targets'],1)

    def test_changed_source_retains_historical_claim_and_flags_staleness(self):
        (self.root/'code.cpp').write_text('new implementation\n')
        report = self.report()
        self.assertEqual(report['errors'],[])
        self.assertEqual(report['counts']['stale_evidence'],2)
        self.assertEqual(report['behaviors'][0]['evidence_freshness'],'stale')

    def test_evidence_tamper(self):
        (self.root/'record.json').write_text('{}')
        self.assert_invalid('evidence record hash mismatch')

    def test_wrong_original_build(self):
        self.record['source_sha256']='b'*64
        self.refresh_record()
        self.assert_invalid('different original binary')

    def test_failed_comparison(self):
        self.record['success']=False
        self.refresh_record()
        self.assert_invalid('failed evidence assertion')

    def test_inventory_tamper(self):
        self.inventory['source_sha256']='b'*64
        self.write_json('inventory.json',self.inventory)
        self.assert_invalid('inventory hash mismatch')
        self.assert_invalid('original binary hash mismatch')

    def test_inventory_cannot_hide_unassigned_ranges(self):
        self.inventory['unassigned_executable_ranges']=[]
        self.write_json('inventory.json',self.inventory)
        self.register['builds'][0]['inventory_sha256']=self.digest('inventory.json')
        self.assert_invalid('function-body complement')

    def test_missing_evidence(self):
        self.register['behaviors'][0]['evidence'].append('absent')
        self.assert_invalid('dangling link')

    def test_missing_source(self):
        (self.root/'code.cpp').unlink()
        self.assert_invalid('Missing or escaping')

    def test_path_escape(self):
        self.register['behaviors'][0]['documents']=['../outside.md']
        self.assert_invalid('Missing or escaping')

    def test_missing_status_dimension(self):
        del self.register['behaviors'][0]['status']['replacement']
        self.assert_invalid('status dimensions')

    def test_duplicate_behavior(self):
        self.register['behaviors'].append(copy.deepcopy(self.register['behaviors'][0]))
        self.assert_invalid('Duplicate behavior')

    def test_invalid_anchor(self):
        self.register['behaviors'][0]['original'][0]['address']=4110
        self.assert_invalid('not in discovered function')

    def test_explicit_recovered_range_retains_discovery_gap(self):
        self.record['anchors']={'0x100e':'83ec38'}
        self.refresh_record()
        self.register['recovered_ranges']=[{'id':'missed','build':'fixture','start':4110,'end':4120,
            'document':'finding.md','evidence':'original','anchors_pointer':'/anchors'}]
        self.register['behaviors'][0]['original'][0].update(address=4110,recovered_range='missed')
        report=self.report()
        self.assertEqual(report['errors'],[])
        self.assertEqual(report['counts']['behavior_anchors_outside_discovered_functions'],1)
        self.assertEqual(report['counts']['unassigned_executable_ranges'],2)
        self.register['behaviors'][0]['original'][0]['address']=4121
        self.assert_invalid('outside linked recovered range')

    def test_data_table_is_not_function_coverage(self):
        self.register['dispatch_tables'][0]['original'].update(address=4110,relation='data')
        self.assertEqual(self.report()['errors'],[])
        self.assertEqual(self.report()['counts']['unclassified_functions'],1)

    def test_implementation_without_tests(self):
        self.register['behaviors'][0]['tests']=[]
        self.assert_invalid('implementation missing code/tests')

    def test_partial_implementation_without_tests_is_visible(self):
        behavior = self.register['behaviors'][0]
        behavior['tests'] = []
        behavior['status']['implementation'] = 'partial'
        report = self.report()
        self.assertEqual(report['errors'], [])
        self.assertEqual(report['findings']['behaviors_without_tests'], ['MV.test'])

    def test_frozen_source_binding_preserves_historical_hash(self):
        self.record['sources'] = {'working/frozen/code.cpp': self.digest('code.cpp'),
                                  'working/game.exe': self.binary_hash}
        self.refresh_record()
        for ev in self.register['evidence']:
            ev['source_bindings'] = {'working/frozen/code.cpp': 'code.cpp'}
        self.assertEqual(self.report()['errors'], [])
        (self.root/'code.cpp').write_text('changed\n')
        report = self.report()
        self.assertEqual(report['errors'], [])
        self.assertEqual(report['counts']['stale_evidence'], 2)
        self.assertEqual(report['findings']['stale_evidence'][0]['changed_sources'], ['code.cpp'])

    def test_recorded_inputs_are_pinned_without_consuming_original_artifacts(self):
        self.record['sources']['working/game.exe'] = self.binary_hash
        self.refresh_record()
        for ev in self.register['evidence']:
            ev['recorded_inputs'] = ['working/game.exe']
        self.assertEqual(self.report()['errors'], [])
        self.assertEqual(self.report()['counts']['stale_evidence'], 0)
        (self.root/'code.cpp').write_text('changed source\n')
        self.assertEqual(self.report()['counts']['stale_evidence'], 2)

    def test_code_keys_cannot_be_reclassified_as_recorded_inputs(self):
        self.register['evidence'][0]['recorded_inputs'] = ['code.cpp']
        self.assert_invalid('code provenance cannot be declared an input')

    def test_recorded_input_still_requires_a_valid_recorded_fingerprint(self):
        self.register['evidence'][0]['recorded_inputs'] = ['missing.exe']
        self.assert_invalid('recorded input lacks fingerprint')

    def test_bound_code_cannot_be_hidden_under_an_artifact_key(self):
        self.record['sources'] = {'source.bin': self.digest('code.cpp')}
        self.refresh_record()
        self.register['evidence'][0].update(source_bindings={'source.bin': 'code.cpp'}, recorded_inputs=['source.bin'])
        self.assert_invalid('bound code provenance cannot be declared an input')

    def test_source_bindings_cannot_invent_or_duplicate_fingerprints(self):
        ev = self.register['evidence'][0]
        ev['source_bindings'] = {'unrecorded': 'code.cpp'}
        self.assert_invalid('binding lacks recorded source fingerprint')
        self.record['sources']['another.cpp'] = self.digest('code.cpp')
        self.refresh_record()
        ev['source_bindings'] = {'code.cpp': 'code.cpp', 'another.cpp': 'code.cpp'}
        self.assert_invalid('duplicate bound source path')
        ev['source_bindings'] = {}
        self.assert_invalid('empty source bindings')

    def pin_source_index(self):
        self.write_json('source-index.json', source_tool.index(self.root, self.register))
        self.register['code_index'] = {'path': 'source-index.json', 'sha256': self.digest('source-index.json')}

    def test_source_census_reports_new_removed_changed_and_unlinked_files(self):
        (self.root/'assets').mkdir()
        for name in ('linked.cpp', 'unlinked.hpp', 'removed.cpp'):
            (self.root/'assets'/name).write_text('snapshot\n')
        self.register['behaviors'][0]['implementation'].append('assets/linked.cpp')
        self.pin_source_index()
        self.assertEqual(self.report()['errors'], [])
        self.assertEqual(self.report()['counts']['indexed_sources'], 3)
        (self.root/'assets/linked.cpp').write_text('edited\n')
        (self.root/'assets/removed.cpp').unlink()
        (self.root/'assets/new.cpp').write_text('new\n')
        report = self.report()
        self.assertEqual(report['errors'], [])
        self.assertEqual(report['findings']['changed_indexed_sources'], ['assets/linked.cpp'])
        self.assertEqual(report['findings']['removed_indexed_sources'], ['assets/removed.cpp'])
        self.assertEqual(report['findings']['new_sources_without_index'], ['assets/new.cpp'])
        self.assertEqual(report['findings']['sources_without_behavior_links'], ['assets/removed.cpp', 'assets/unlinked.hpp'])

    def test_source_index_links_cannot_drift_from_register(self):
        (self.root/'assets').mkdir()
        (self.root/'assets/unlinked.cpp').write_text('fixture\n')
        self.pin_source_index()
        self.register['behaviors'][0]['implementation'].append('assets/unlinked.cpp')
        self.assert_invalid('source index/register link drift')

    def test_legacy_source_census_remains_readable_but_cannot_hide_web_sources(self):
        catalog = source_tool.index(self.root, self.register)
        catalog['suffixes'] = sorted(source_tool.SUFFIXES - {'.html', '.css', '.js'})
        self.write_json('source-index.json', catalog)
        self.register['code_index'] = {'path':'source-index.json', 'sha256':self.digest('source-index.json')}
        self.assertEqual(self.report()['errors'], [])
        for path in ('apps/coverage-ui/index.html', 'apps/coverage-ui/app.js', 'apps/coverage-ui/style.css'):
            target = self.root/path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text('fixture')
        self.assertEqual(self.report()['findings']['new_sources_without_index'],
                         ['apps/coverage-ui/app.js','apps/coverage-ui/index.html','apps/coverage-ui/style.css'])
        self.pin_source_index()
        self.assertEqual(self.report()['findings']['new_sources_without_index'], [])
        for path in ('apps/coverage-ui/index.html', 'apps/coverage-ui/app.js', 'apps/coverage-ui/style.css'):
            (self.root/path).write_text('edited')
        self.assertEqual(len(self.report()['findings']['changed_indexed_sources']), 3)

    def test_frontend_source_cannot_be_reclassified_as_recorded_input(self):
        for path in ('apps/example/app.js', 'apps/example/style.css', 'apps/example/index.html'):
            with self.subTest(path=path):
                target = self.root/path
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text('frontend')
                self.record['sources'][path] = self.digest(path)
                self.refresh_record()
                self.register['evidence'][0]['recorded_inputs'] = [path]
                self.assert_invalid('code provenance cannot be declared an input')

    def test_source_census_retains_generated_protocols_builds_and_tests(self):
        for path in ('assets/CMakeLists.txt', 'protocols/include/mnm/frame.h', 'tests/fixture.py',
                     'protocols/schemas/frame.json', 'runtime/render/fixture.inc',
                     'tools/ghidra/Example.java', 'apps/example/main.cpp', 'assets/README.md',
                     'original/ignored.cpp', 'working/ignored.cpp', 'tools/__pycache__/ignored.py'):
            target = self.root/path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text('fixture\n')
        catalog = source_tool.index(self.root, self.register)
        self.assertEqual(len(catalog['files']), 7)
        self.assertEqual({item['role'] for item in catalog['files']}, {'build', 'protocol', 'test', 'tool', 'code'})
        self.assertTrue(all(not item['references'] for item in catalog['files']))

    def test_changed_focused_register_requires_reconciliation(self):
        self.write_json('focused.json', {'scope': 'fixture'})
        self.register['register_imports'] = [{'path': 'focused.json', 'sha256': self.digest('focused.json'),
                                              'scope': 'Reviewed consolidation snapshot'}]
        self.assertEqual(self.report()['errors'], [])
        self.write_json('focused.json', {'scope': 'new evidence'})
        report = self.report()
        self.assertEqual(report['errors'], [])
        self.assertEqual(report['findings']['changed_focused_registers'], ['focused.json'])

    def test_summary_groups_subsystems_and_retains_titles(self):
        text = markdown(self.report())
        self.assertIn('## Movement checklist', text)
        self.assertIn('Selected movement', text)

    def test_native_policy_cannot_claim_equivalence(self):
        self.register['behaviors'][0]['kind']='native_policy'
        self.assert_invalid('native policy must not claim')

    def test_comparison_must_fingerprint_implementation(self):
        self.register['behaviors'][0]['implementation']=['test.py']
        self.assert_invalid('does not fingerprint')

    def test_replacement_needs_live_bypass_evidence(self):
        self.register['behaviors'][0]['status']['replacement']='scoped_live'
        self.assert_invalid('replacement requires')

    def test_live_replacement_requires_recorded_bypass_and_outcome(self):
        self.record.update(equivalent=True,bypassed=False)
        self.refresh_record()
        ev=self.register['evidence'][0]
        ev.update(kind='live_replacement',equivalence_pointer='/equivalent',bypass_pointer='/bypassed')
        behavior=self.register['behaviors'][0]
        behavior['status'].update(integration='live_equivalence',replacement='scoped_live')
        self.register['scenarios'].append({'id':'live','level':'live_equivalence','scope':'bounded actual replacement',
            'behaviors':['MV.test'],'evidence':['original'],'tests':['test.py']})
        # Replacement evidence carries equivalence, and can support that scenario.
        self.assert_invalid('original work bypass not established')
        self.record['bypassed']=True
        self.refresh_record()
        self.assertEqual(self.report()['errors'],[])

    def test_live_claim_needs_live_scenario_and_evidence(self):
        self.register['behaviors'][0]['status']['integration']='live_equivalence'
        self.assert_invalid('live integration without')
        self.assert_invalid('lacks matching scenario')

    def test_scenario_cannot_promote_headless_evidence(self):
        self.register['scenarios'][0]['level']='live_equivalence'
        self.assert_invalid('scenario evidence level mismatch')

    def test_justified_classification(self):
        self.register['classifications']=[{'build':'fixture','entry':4128,'category':'runtime_support',
            'reason':'Fixture-only support','documents':['finding.md']}]
        report=self.report()
        self.assertEqual(report['errors'],[])
        self.assertEqual(report['counts']['unclassified_functions'],0)
        self.assertEqual(report['counts']['unmapped_callback_candidates'],0)

    def test_unjustified_exclusion(self):
        self.register['classifications']=[{'build':'fixture','entry':4128,'category':'runtime_support',
            'reason':'','documents':[]}]
        self.assert_invalid('Classification requires')

    def test_import_binding(self):
        self.register['api_bindings']=[{'build':'fixture','iat_va':8192,'behaviors':['MV.test'],'scope':'fixture API'}]
        self.assertEqual(self.report()['counts']['unmapped_imports'],0)
        self.register['api_bindings'][0]['iat_va']=8193
        self.assert_invalid('unknown import slot')

    def test_duplicate_dispatch_value(self):
        self.register['dispatch_tables'][0]['entries'].append({'value':0,'behaviors':[]})
        self.assert_invalid('duplicate dispatch value')

    def test_json_pointer_escaping(self):
        self.assertEqual(pointer({'a/b':{'~':[5]}},'/a~1b/~0/0'),5)


class InventoryTests(unittest.TestCase):
    def test_unknown_image_is_rejected_with_before_after_manifest_checks(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary)
            (root/'tools').mkdir()
            verifier=root/'tools/original-manifest.sh'
            verifier.write_text('#!/bin/sh\necho verified >> "${0%/*}/checks"\n')
            verifier.chmod(0o755)
            executable=root/'unknown.exe'
            executable.write_bytes(b'unsupported fixture')
            output=root/'inventory.json'
            with patch.object(inventory_tool,'ROOT',root), patch.object(sys,'argv',[
                    'inventory-binary.py','--executable',str(executable),'--output',str(output)]):
                with self.assertRaisesRegex(ValueError,'Unsupported executable hash'):
                    inventory_tool.main()
            self.assertEqual((root/'tools/checks').read_text().splitlines(),['verified','verified'])
            self.assertFalse(output.exists())

    def test_range_union_preserves_padding_and_undiscovered_code(self):
        sections=[{'start':100,'end':130,'executable':True}, {'start':200,'end':220,'executable':False}]
        functions=[{'ranges':[[104,110],[108,114]]}, {'ranges':[[120,124]]}]
        self.assertEqual(inventory_tool.uncovered(sections,functions),[[100,104],[114,120],[124,130]])

    def test_pe_normalization_retains_external_and_unknown_flows(self):
        data=bytearray(0x400)
        data[:2]=b'MZ';struct.pack_into('<I',data,0x3c,0x80);data[0x80:0x84]=b'PE\0\0'
        struct.pack_into('<HH',data,0x84,0x14c,1);struct.pack_into('<H',data,0x94,224)
        opt=0x98;struct.pack_into('<H',data,opt,0x10b)
        struct.pack_into('<I',data,opt+28,0x400000);struct.pack_into('<I',data,opt+60,0x200)
        data[opt+224:opt+232]=b'.text\0\0\0';struct.pack_into('<4I',data,opt+232,0x200,0x1000,0x200,0x200)
        struct.pack_into('<I',data,opt+224+36,0x60000020)
        discovery={'functions':[{'entry':'0x401000','name':'fixture','thunk':False,
                   'ranges':[['0x401000','0x401004']],'data_references':['0xEntry Point','0x401010']}],
            'flows':[{'site':'0x401001','owner':'0x401000','kind':'call','computed':True,'targets':[],
                      'references':[{'target':'0xEXTERNAL:00000001','type':'DATA'}],'instruction':'CALL EAX'}]}
        result=inventory_tool.normalize(bytes(data),discovery)
        self.assertEqual(result['functions'][0]['data_references'],['0xEntry Point',0x401010])
        self.assertEqual(result['flows'][0]['references'][0]['target'],'0xEXTERNAL:00000001')
        self.assertEqual(result['unassigned_executable_ranges'],[[0x401004,0x401200]])
        discovery['functions'][0]['ranges']=[['0x401000','0x402000']]
        with self.assertRaisesRegex(ValueError,'not file-backed'):
            inventory_tool.normalize(bytes(data),discovery)


if __name__ == '__main__':
    unittest.main()
