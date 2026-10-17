#!/usr/bin/env python3
"""DRAW-01 census guards: unknowns, false closure and malformed journals."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_inventory import (ROOT, build_report, caller_returns, hook_entries,
                               producer_trace, reconcile, route_digest, selector_key)

spec = importlib.util.spec_from_file_location('drawing_report', ROOT / 'tools/report-drawing-inventory.py')
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)


class DrawingInventoryTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.original = '40209ca7' + '0' * 56
        (self.root / 'research.md').write_text('Selected route; other callers remain unknown.\n')
        (self.root / 'hooks.c').write_text('#define HOOKS 1\n{0x401000,5,0,{0x55},producer_enter_0,-1}\n')
        self.static = {'source_sha256': self.original, 'image_base': '0x400000',
                       'ranges': {'selected': [0x401000, 0x401020]},
                       'callers': {'selected': [{'call_va': '0x402000', 'discovered_owners': [],
                                                'context': [' 402000:\te8 fb ef ff ff \tcall 0x401000']}]},
                       'indirect_calls': {'selected': [{'site': '0x401008', 'instruction': 'call eax', 'discovered_owners': []}]},
                       'computed_jumps': {'selected': [{'site': '0x401010', 'instruction': 'jmp eax'}]}, 'success': True}
        self.record = {'success': True, 'original': self.original, 'counts': {'copy': 4},
                       'entries': {'0x401000': 4}, 'rows': [{'queue': 1, 'status': 0, 'capacity': 16, 'draw_count': 4, 'rows_captured': 4, 'kind_counts': {'-2': 1, '33': 3}}]}
        self.register = {'builds': [{'id': 'build', 'source_sha256': self.original}],
                         'behaviors': [{'id': 'route', 'original': [{'build': 'build', 'address': 0x401000}],
                                        'status': {'replacement': 'none'}}], 'evidence': [],
                         'dispatch_tables': [{'id': 'table', 'scope': 'Unresolved default', 'complete': False, 'entries': []}]}
        for eid, value, kind in [('static', self.static, 'static'), ('live', self.record, 'live_observation')]:
            self.register['evidence'].append({'id': eid, 'record': eid + '.json', 'kind': kind,
                                             'assertions': [{'pointer': '/success', 'equals': True}]})
            self.write_record(eid, value)
        entry = {'id': 'entry', 'entry': 0x401000, 'behaviors': ['route'], 'owner': 'runtime/scene',
                 'classification': 'producer', 'scope': 'Selected entry', 'confidence': 'high_within_scope',
                 'support': {'decision': 'scoped_native', 'reason': 'Selected corpus only'},
                 'side_effects': ['Pixel and caller state'], 'remaining': 'Other callers remain unknown',
                 'documents': ['research.md'], 'static_windows': ['selected'],
                 'selectors': [{'kind': 'entry', 'identity': '0x401000'}]}
        operation = copy.deepcopy(entry)
        operation.update(id='operation', static_windows=[], selectors=[{'kind': 'operation', 'identity': 'copy'}])
        operation.pop('entry')
        hidden = copy.deepcopy(operation)
        hidden.update(id='hidden', selectors=[{'kind': 'draw_kind', 'identity': -2}])
        visible = copy.deepcopy(hidden)
        visible.update(id='visible', selectors=[{'kind': 'draw_kind', 'identity': 33}])
        self.catalog = {'schema': 1, 'build': 'build', 'source_sha256': self.original, 'scope': 'Bounded fixture',
                        'static_source': {'evidence': 'static'}, 'hook_source': 'hooks.c', 'dispatch_tables': ['table'],
                        'producer_operations': {'6': 'copy', '10': 'checkpoint', '11': 'queue_entry', '12': 'queue_return'},
                        'routes': [entry, operation, hidden, visible],
                        'scenarios': [{'id': 'startup', 'required': True, 'scope': 'Startup only', 'remaining': 'Complete caller census pending', 'census_review': None},
                                      {'id': 'save-load', 'required': True, 'scope': 'Save/load', 'remaining': 'No drawing census', 'census_review': None}],
                        'observations': [{'id': 'cohort', 'evidence': 'live', 'scenario': 'startup', 'scope': 'One bounded cohort',
                                          'build_pointer': '/original', 'projections': [
                                              {'kind': 'counts', 'pointer': '/counts', 'selector': 'operation', 'activity': 'live_original_active'},
                                              {'kind': 'counts', 'pointer': '/entries', 'selector': 'entry', 'activity': 'live_original_active'},
                                              {'kind': 'kind_rows', 'pointer': '/rows', 'selector': 'draw_kind', 'activity': 'live_original_active'}]}]}

    def write_record(self, eid, value):
        data = json.dumps(value).encode()
        (self.root / (eid + '.json')).write_bytes(data)
        e = next(e for e in self.register['evidence'] if e['id'] == eid)
        e['sha256'] = hashlib.sha256(data).hexdigest()

    def report(self):
        return reconcile(self.root, self.catalog, self.register)

    def test_inventory_support_does_not_promote_replacement_or_completion(self):
        before = copy.deepcopy(self.register)
        r = self.report()
        self.assertFalse(r['draw01_complete'])
        self.assertFalse(r['scenarios'][1]['closed_route_census'])
        self.assertEqual(r['unaccounted'], [])
        self.assertEqual(self.register, before)

    def test_new_entry_operation_and_draw_kind_remain_unaccounted(self):
        self.record['counts']['new_operation'] = 2
        self.record['entries']['0x401100'] = 3
        self.record['rows'][0].update(draw_count=5, rows_captured=5, kind_counts={'-2': 1, '33': 3, '77': 1})
        self.write_record('live', self.record)
        r = self.report()
        self.assertEqual({x['identity'] for x in r['unaccounted']}, {'new_operation', '0x401100', '77'})
        self.assertFalse(r['draw01_complete'])

    def test_new_hook_entry_has_no_implicit_owner(self):
        (self.root / 'hooks.c').write_text('#define HOOKS 2\n{0x401000,5,0,{0x55},producer_enter_0,-1}\n{0x401100,5,0,{0x55},producer_enter_1,-1}\n')
        r = self.report()
        self.assertEqual(r['unaccounted'][0]['identity'], '0x401100')
        self.assertEqual(r['unaccounted'][0]['activity'], 'instrumented_only')

    def metadata(self):
        self.record['metadata'] = [{'phase': 3, 'entry': 0x401000, 'count': 4},
                                   {'phase': 3, 'entry': 0x401000, 'count': 2},
                                   {'phase': 5, 'entry': 0x401000, 'count': 8}]
        self.catalog['observations'][0]['projections'] = [
            {'kind': 'census_entry_rows', 'pointer': '/metadata', 'phase': 3,
             'selector': 'entry', 'activity': 'live_original_active'}]
        self.write_record('live', self.record)

    def cpu(self):
        self.api()
        site = self.api_record['calls'][0]['site']
        row = self.api_record['calls'][0]
        row.update(bytes='ff15dc036900', operand='DWORD PTR ds:0x6903dc',
                   target_mode={'kind': 'absolute_memory', 'address': 0x6903dc},
                   semantic_contract='unreviewed')
        self.write_record('api-static', self.api_record)
        self.catalog['cpu_site_sources'] = self.catalog.pop('api_site_sources')
        route = self.catalog['routes'][-1]
        route.pop('api_site'); route.update(cpu_site=site, classification='unresolved',
            support={'decision':'original_only','reason':'Diagnostic original CALL only'},
            selectors=[{'kind':'cpu_site','identity':site}])
        self.catalog['observations'][0]['projections'] = [{
            'kind':'cpu_site_rows','pointer':'/api/targets','phase':3,
            'selector':'cpu_site','activity':'live_original_active'}]
        self.record['call_boundary_role'] = 'cpu_backend_dispatch'
        self.record['api'] = {'journal_integrity':True,'original_sha256':self.original,
            'targets':[{'phase':3,'site':site,'count':4,'target_state':2,
                        'target_slot':0x6903dc,'target_value':0x596cb8}]}
        self.write_record('live', self.record)

    def test_cpu_selector_is_separate_and_does_not_close_scenarios(self):
        self.cpu(); r = self.report()
        self.assertEqual(r['counts']['cpu_registered_sites'], 1)
        self.assertEqual(r['counts']['cpu_observed_sites'], 1)
        self.assertEqual(r['counts']['api_registered_sites'], 0)
        self.assertEqual(r['activities'][0]['selector'], 'cpu_site')
        self.assertFalse(r['draw01_complete'])

    def test_cpu_site_namespace_and_unknown_contract_are_required(self):
        self.cpu()
        for changes in [{'bytes':'ff15e0036900','operand':'DWORD PTR ds:0x6903e0','target_mode':{'kind':'absolute_memory','address':0x6903e0}}, {'semantic_contract':'unlock'}]:
            old = copy.deepcopy(self.api_record['calls'][0]); self.api_record['calls'][0].update(changes)
            self.write_record('api-static', self.api_record)
            with self.assertRaises(ValueError): self.report()
            self.api_record['calls'][0] = old

    def test_cpu_site_cannot_claim_typed_client_or_native_admission(self):
        self.cpu(); route = self.catalog['routes'][-1]
        for key,value in [('client_contract',{}),('classification','surface_access'),('support',{'decision':'scoped_native','reason':'Not earned'})]:
            before=copy.deepcopy(route); route[key]=value
            with self.assertRaises(ValueError): self.report()
            route.clear(); route.update(before)

    def test_cpu_activity_requires_role_and_intact_journal(self):
        self.cpu()
        for key,value in [('call_boundary_role','graphics_api'),('api',{'journal_integrity':False,'original_sha256':self.original})]:
            before=copy.deepcopy(self.record[key]);self.record[key]=value;self.write_record('live',self.record)
            with self.assertRaises(ValueError):self.report()
            self.record[key]=before

    def test_cpu_unknown_live_site_remains_unaccounted(self):
        self.cpu(); self.record['api']['targets'][0]['site']+=16;self.write_record('live',self.record)
        self.assertEqual(self.report()['unaccounted'][0]['selector'],'cpu_site')

    def test_cpu_excluded_phase_is_still_validated(self):
        self.cpu();self.record['api']['targets'][0].update(phase=14,target_state=0);self.write_record('live',self.record)
        with self.assertRaises(ValueError):self.report()

    def test_cpu_static_calls_require_every_route(self):
        self.cpu();self.catalog['routes'].pop()
        with self.assertRaises(ValueError):self.report()

    def test_terminal_profile_requires_original_wrapper_contract(self):
        route=self.catalog['routes'][0];route['observation_profile']='terminal_flow'
        with self.assertRaises(ValueError):self.report()
        route['observation_profile']='unknown'
        with self.assertRaises(ValueError):self.report()

    def test_metadata_phase_projection_keeps_cohorts_separate(self):
        self.metadata(); r = self.report()
        self.assertEqual([(a['identity'], a['count']) for a in r['activities']], [(0x401000, 6)])
        self.assertFalse(r['scenarios'][0]['closed_route_census'])

    def test_metadata_unknown_entry_is_unaccounted(self):
        self.metadata(); self.record['metadata'][0]['entry'] = 0x401100
        self.write_record('live', self.record)
        self.assertEqual(self.report()['unaccounted'][0]['identity'], 0x401100)

    def test_metadata_projection_rejects_invalid_even_excluded_rows(self):
        self.metadata()
        for field, value in [('phase', True), ('phase', 15), ('entry', False), ('count', -1)]:
            with self.subTest(field=field, value=value):
                row = self.record['metadata'][2]; before = row[field]; row[field] = value
                self.write_record('live', self.record)
                with self.assertRaises(ValueError): self.report()
                row[field] = before

    def metadata_hooks(self):
        self.record['hooks'] = [{'tag': 0, 'entry': 0x401000, 'route': 'entry'}]
        self.record['refusals'] = []
        self.catalog['metadata_hook_cohorts'] = [{'observation': 'cohort',
            'pointer': '/hooks', 'refused_pointer': '/refusals'}]
        self.write_record('live', self.record)

    def api(self):
        site = 0x401008
        self.api_record = {'schema': 1, 'source_sha256': self.original, 'input_unchanged': True,
            'calls': [{'site': site, 'return_pc': site + 6, 'bytes': 'ff9180000000',
                       'operand': 'DWORD PTR [ecx+0x80]',
                       'target_mode': {'kind': 'register_memory', 'register': 'ecx', 'displacement': 128},
                       'discovered_owners': [], 'exact_import': None}]}
        self.register['evidence'].append({'id': 'api-static', 'record': 'api-static.json', 'kind': 'static',
            'assertions': [{'pointer': '/input_unchanged', 'equals': True}]})
        self.write_record('api-static', self.api_record)
        self.catalog['api_site_sources'] = [{'evidence': 'api-static'}]
        route = copy.deepcopy(self.catalog['routes'][0]); route.pop('entry')
        route.update(id='api', api_site=site, classification='unresolved', static_windows=[],
                     selectors=[{'kind': 'api_site', 'identity': site}])
        self.catalog['routes'].append(route)
        self.record['api'] = {'journal_integrity': True, 'original_sha256': self.original,
            'targets': [{'phase': 3, 'site': site, 'target_state': 2, 'target_slot': 0x801080,
                         'target_value': 0x900000, 'count': 4},
                        {'phase': 3, 'site': site, 'target_state': 2, 'target_slot': 0x802080,
                         'target_value': 0x900000, 'count': 2},
                        {'phase': 5, 'site': site, 'target_state': 2, 'target_slot': 0x803080,
                         'target_value': 0x900000, 'count': 8}]}
        self.catalog['observations'][0]['projections'] = [
            {'kind': 'api_site_rows', 'pointer': '/api/targets', 'phase': 3,
             'selector': 'api_site', 'activity': 'live_original_active'}]
        self.write_record('live', self.record)

    def test_api_sites_are_exact_CALLs_separate_from_entries_and_owners(self):
        self.api(); r = self.report()
        self.assertEqual([(a['identity'], a['count']) for a in r['activities']], [(0x401008, 6)])
        self.assertEqual(r['static_api_sites'][0]['discovered_owners'], [])
        self.assertEqual(r['static_api_sites'][0]['target_mode']['displacement'], 128)
        self.assertEqual(r['counts']['api_registered_sites'], 1)
        self.assertEqual(r['counts']['api_observed_sites'], 1)
        self.assertFalse(r['draw01_complete'])
        self.assertEqual(r['counts']['instrumented_entries'], 1)

    def client(self):
        self.api()
        self.layouts = {'schema': 1, 'original_sha256': self.original,
            'contracts': [{'site': 0x401008, 'call_bytes': 'ff9180000000',
                'interface': 'IDirectDrawSurface2', 'method': 'Unlock', 'formal_words': 2,
                'receiver_evidence': 'Explicit test receiver reviewed at this exact CALL; no offset-only inference.'}],
            'excluded_reload_callbacks': []}
        self.client_review = {'schema': 1, 'original_sha256': self.original,
            'layouts': 'layouts.json', 'sources': {}, 'reviewed_layouts': 1, 'excluded_callbacks': 0}
        self.register['evidence'].append({'id': 'client-static', 'record': 'client-static.json',
            'kind': 'static', 'assertions': [{'pointer': '/reviewed_layouts', 'equals': 1}]})
        self.catalog['api_layout_sources'] = [{'evidence': 'client-static'}]
        self.write_layouts()
        self.catalog['routes'][-1]['classification'] = 'surface_access'
        self.catalog['routes'][-1]['client_contract'] = {
            'source_evidence': 'client-static', 'layout_sha256': self.client_review['sources']['layouts.json'],
            'interface': 'IDirectDrawSurface2', 'method': 'Unlock', 'formal_words': 2,
            'receiver_evidence': self.layouts['contracts'][0]['receiver_evidence']}

    def write_layouts(self):
        raw = json.dumps(self.layouts).encode(); (self.root / 'layouts.json').write_bytes(raw)
        self.client_review['sources']['layouts.json'] = hashlib.sha256(raw).hexdigest()
        self.write_record('client-static', self.client_review)

    def test_reviewed_client_preserves_exact_site_and_independent_closure(self):
        self.client(); r = self.report()
        self.assertEqual(r['counts']['api_reviewed_clients'], 1)
        self.assertEqual(r['counts']['api_unreviewed_clients'], 0)
        self.assertEqual(r['reviewed_api_clients'][0]['contract']['method'], 'Unlock')
        self.assertEqual(r['static_api_sites'][0]['discovered_owners'], [])
        self.assertFalse(r['draw01_complete'])

    def test_reviewed_client_route_cannot_change_type_formals_provenance_or_class(self):
        self.client(); route = self.catalog['routes'][-1]
        for field, value in [('interface', 'IUnknown'), ('method', 'Release'),
                             ('formal_words', 1), ('receiver_evidence', 'Offset alone'),
                             ('layout_sha256', '0' * 64), ('source_evidence', 'static')]:
            original = route['client_contract'][field]; route['client_contract'][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'client contract'): self.report()
            route['client_contract'][field] = original
        route['classification'] = 'lifecycle'
        with self.assertRaisesRegex(ValueError, 'classification'): self.report()

    def test_client_layout_build_bytes_prototype_and_receiver_reject_even_new_hash(self):
        self.client(); original = copy.deepcopy(self.layouts)
        for field, value in [('site', True), ('site', 0x401009), ('call_bytes', '909090909090'),
                             ('interface', 'IUnknown'), ('method', 'Unknown'),
                             ('formal_words', True), ('formal_words', 3), ('receiver_evidence', '')]:
            self.layouts = copy.deepcopy(original); self.layouts['contracts'][0][field] = value; self.write_layouts()
            with self.subTest(field=field), self.assertRaises(ValueError): self.report()
        for field, value in [('schema', True), ('schema', 2), ('original_sha256', 'wrong')]:
            self.layouts = copy.deepcopy(original); self.layouts[field] = value; self.write_layouts()
            with self.subTest(field=field), self.assertRaises(ValueError): self.report()

    def test_client_review_requires_static_build_bound_hash_and_safe_path(self):
        self.client(); original = copy.deepcopy(self.client_review)
        for field, value in [('schema', True), ('original_sha256', 'wrong'),
                             ('layouts', 'working/layouts.json'), ('layouts', '../layouts.json'),
                             ('layouts', '/tmp/layouts.json'), ('reviewed_layouts', True), ('reviewed_layouts', 2)]:
            self.client_review = copy.deepcopy(original); self.client_review[field] = value
            self.write_record('client-static', self.client_review)
            with self.subTest(field=field), self.assertRaises(ValueError): self.report()
        self.client_review = copy.deepcopy(original); self.client_review['sources']['layouts.json'] = '0' * 64
        self.write_record('client-static', self.client_review)
        with self.assertRaisesRegex(ValueError, 'source hash'): self.report()
        self.client_review = copy.deepcopy(original); self.write_record('client-static', self.client_review)
        next(e for e in self.register['evidence'] if e['id'] == 'client-static')['kind'] = 'live_observation'
        with self.assertRaisesRegex(ValueError, 'static review'): self.report()

    def test_client_duplicate_and_excluded_callback_declarations_reject(self):
        self.client(); original = copy.deepcopy(self.layouts)
        next(e for e in self.register['evidence'] if e['id'] == 'client-static')['assertions'][0]['equals'] = 2
        self.layouts['contracts'] *= 2; self.client_review['reviewed_layouts'] = 2; self.write_layouts()
        with self.assertRaisesRegex(ValueError, 'Duplicate'): self.report()
        next(e for e in self.register['evidence'] if e['id'] == 'client-static')['assertions'][0]['equals'] = 1
        self.layouts = copy.deepcopy(original); self.client_review['reviewed_layouts'] = 1
        self.layouts['excluded_reload_callbacks'] = [0x401008]; self.client_review['excluded_callbacks'] = 1; self.write_layouts()
        with self.assertRaisesRegex(ValueError, 'excluded'): self.report()
        self.layouts['excluded_reload_callbacks'] *= 2; self.client_review['excluded_callbacks'] = 2; self.write_layouts()
        with self.assertRaisesRegex(ValueError, 'exclusions'): self.report()

    def test_unreviewed_virtual_offset_cannot_copy_a_client_contract(self):
        self.client(); self.catalog.pop('api_layout_sources')
        with self.assertRaisesRegex(ValueError, 'Unreviewed'): self.report()

    def test_api_unknown_site_is_not_nearest_entry_or_dropped(self):
        self.api(); self.record['api']['targets'][0]['site'] += 1; self.write_record('live', self.record)
        self.assertEqual(self.report()['unaccounted'][0]['identity'], 0x401009)

    def test_api_static_site_build_bytes_return_mode_and_duplicates_refuse(self):
        self.api(); original = copy.deepcopy(self.api_record)
        for field, value in [('source_sha256', 'wrong'), ('schema', True), ('schema', 2)]:
            self.api_record = copy.deepcopy(original); self.api_record[field] = value
            self.write_record('api-static', self.api_record)
            with self.subTest(field=field, value=value), self.assertRaises(ValueError): self.report()
        for field, value in [('return_pc', 0x40100f), ('bytes', '909090909090'),
                             ('target_mode', {'kind': 'register_memory', 'register': 'edx', 'displacement': 128}),
                             ('discovered_owners', [True])]:
            self.api_record = copy.deepcopy(original); self.api_record['calls'][0][field] = value
            self.write_record('api-static', self.api_record)
            with self.subTest(field=field), self.assertRaises(ValueError): self.report()
        self.api_record = copy.deepcopy(original); self.api_record['calls'] *= 2
        self.write_record('api-static', self.api_record)
        with self.assertRaisesRegex(ValueError, 'Duplicate'): self.report()

    def test_api_import_slot_and_static_evidence_kind_refuse(self):
        self.api(); self.api_record['calls'][0]['exact_import'] = {'iat_va': 0x801080}
        self.write_record('api-static', self.api_record)
        with self.assertRaisesRegex(ValueError, 'import'): self.report()
        self.api_record['calls'][0]['exact_import'] = None; self.write_record('api-static', self.api_record)
        next(e for e in self.register['evidence'] if e['id'] == 'api-static')['kind'] = 'live_observation'
        with self.assertRaisesRegex(ValueError, 'static evidence'): self.report()

    def test_api_static_census_needs_every_exact_route(self):
        self.api(); self.catalog['routes'].pop()
        with self.assertRaisesRegex(ValueError, 'lacks reviewed route'): self.report()

    def test_api_routes_cannot_be_function_entries_or_unbound_points(self):
        self.api(); route = self.catalog['routes'][-1]
        route['entry'] = route['api_site']
        with self.assertRaisesRegex(ValueError, 'separate from function'): self.report()
        route.pop('entry'); route['api_site'] += 1
        with self.assertRaisesRegex(ValueError, 'exact static CALL route'): self.report()

    def test_api_projection_validates_malformed_excluded_rows(self):
        self.api(); row = self.record['api']['targets'][2]
        for field, value in [('phase', True), ('phase', 15), ('site', False), ('count', -1),
                             ('target_state', True), ('target_state', 0), ('target_value', -1), ('target_slot', False)]:
            before = row[field]; row[field] = value; self.write_record('live', self.record)
            with self.subTest(field=field), self.assertRaises(ValueError): self.report()
            row[field] = before

    def test_api_projection_requires_build_bound_integrity_even_empty(self):
        self.api(); self.record['api']['targets'] = []
        for field, value in [('journal_integrity', False), ('original_sha256', 'wrong')]:
            before = self.record['api'][field]; self.record['api'][field] = value; self.write_record('live', self.record)
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'intact build-bound'): self.report()
            self.record['api'][field] = before

    def test_metadata_hooks_are_recorded_instrumentation_not_activity(self):
        self.metadata_hooks(); r = self.report()
        self.assertEqual(r['counts']['metadata_instrumented_entries'], 1)
        self.assertEqual(r['metadata_hook_entries'][0]['activity'], 'recorded_instrumentation_only')
        self.assertFalse(r['draw01_complete'])

    def test_metadata_new_hook_is_unaccounted_and_bad_route_refuses(self):
        self.metadata_hooks(); self.record['hooks'][0]['route'] = 'inferred'
        self.write_record('live', self.record)
        with self.assertRaisesRegex(ValueError, 'route identity'): self.report()
        self.record['hooks'][0]['entry'] = 0x401100; self.write_record('live', self.record)
        self.assertEqual(self.report()['unaccounted'][0]['activity'], 'metadata_instrumented_only')

    def test_metadata_hook_duplicate_tag_or_entry_refuses(self):
        self.metadata_hooks(); self.record['hooks'].append(dict(self.record['hooks'][0]))
        self.write_record('live', self.record)
        with self.assertRaisesRegex(ValueError, 'tags'): self.report()
        self.record['hooks'][1]['tag'] = 1; self.write_record('live', self.record)
        with self.assertRaisesRegex(ValueError, 'Duplicate'): self.report()

    def supplemental(self):
        self.catalog['caller_sources'] = [{'evidence': 'extra'}]
        self.register['evidence'].append({'id': 'extra', 'record': 'extra.json', 'kind': 'static',
                                         'assertions': [{'pointer': '/input_unchanged', 'equals': True}]})
        record = {'schema': 1, 'source_sha256': self.original, 'image_base': 0x400000, 'input_unchanged': True,
                  'callers': [{'call_va': 0x402000, 'return_pc': 0x402005, 'entry': 0x401000,
                               'bytes': 'e8fbefffff', 'discovered_owners': []}]}
        self.write_record('extra', record)
        return record

    def test_supplemental_exact_caller_preserves_unassigned_owner(self):
        self.supplemental()
        self.assertEqual(self.report()['static_caller_returns']['0x402005']['entry'], 0x401000)
        self.assertEqual(self.report()['static_caller_returns']['0x402005']['discovered_owners'], [])
        self.assertFalse(self.report()['draw01_complete'])

    def test_changed_supplemental_build_call_and_evidence_kind_refuse(self):
        record = self.supplemental()
        record['source_sha256'] = 'other'
        self.write_record('extra', record)
        with self.assertRaisesRegex(ValueError, 'build/schema'): self.report()
        record['source_sha256'] = self.original
        record['callers'][0]['bytes'] = 'e8fcefffff'
        self.write_record('extra', record)
        with self.assertRaisesRegex(ValueError, 'disagrees'): self.report()
        self.register['evidence'][-1]['kind'] = 'live_observation'
        with self.assertRaisesRegex(ValueError, 'requires static'): self.report()

    def test_hook_parser_refuses_changed_shape_duplicate_and_missing_tag(self):
        for text in ('#define HOOKS 1\nunknown_initializer', '#define HOOKS 2\n{0x401000,5,0,{0x55},producer_enter_0,-1}\n{0x401000,5,0,{0x55},producer_enter_1,-1}', '#define HOOKS 1\n{0x401000,5,0,{0x55},producer_enter_9,-1}'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                hook_entries(text)

    def interior(self, entry=0x401010):
        data = json.dumps({'source_sha256': self.original, 'functions': [{'entry': 0x401000, 'ranges': [[0x401000, 0x401020]]}]}).encode()
        (self.root / 'binary.json').write_bytes(data)
        self.register['builds'][0].update(inventory='binary.json', inventory_sha256=hashlib.sha256(data).hexdigest())
        row = copy.deepcopy(self.catalog['routes'][0]);row.update(id='interior', entry=entry, original_anchor=0x401000,
                                                                static_windows=[], selectors=[{'kind': 'entry', 'identity': hex(entry)}])
        self.catalog['routes'].append(row)

    def test_interior_hook_uses_explicit_function_anchor_not_nearest_function(self):
        self.interior()
        self.assertTrue(self.report()['inventory_valid'])
        self.catalog['routes'][-1]['original_anchor'] = 0x401001
        with self.assertRaisesRegex(ValueError, 'original behavior anchor'):
            self.report()

    def test_out_of_body_interior_hook_and_changed_binary_inventory_refuse(self):
        self.interior(0x401030)
        with self.assertRaisesRegex(ValueError, 'outside explicitly'):
            self.report()
        self.catalog['routes'][-1].update(entry=0x401010, selectors=[{'kind': 'entry', 'identity': '0x401010'}])
        (self.root / 'binary.json').write_text('{}')
        with self.assertRaisesRegex(ValueError, 'inventory changed'):
            self.report()

    def test_additional_patch_census_preserves_new_entry_and_rejects_parser_drift(self):
        self.catalog['additional_hook_sources'] = [{'path': 'patch.c', 'kind': 'interior_jump'}]
        (self.root / 'patch.c').write_text('jump((u8*)0x401111, entry,6);')
        self.assertEqual(self.report()['unaccounted'][0]['identity'], '0x401111')
        (self.root / 'patch.c').write_text('different_patch_mechanism();')
        with self.assertRaisesRegex(ValueError, 'declaration changed'):
            self.report()

    def test_static_window_census_cannot_disappear(self):
        self.catalog['routes'][0]['static_windows'] = []
        with self.assertRaisesRegex(ValueError, 'census changed'):
            self.report()

    def test_return_pc_uses_exact_call_length_and_preserves_missing_owner(self):
        r = caller_returns(self.static)
        self.assertEqual(r[0x402005], {'entry': 0x401000, 'call_va': 0x402000, 'discovered_owners': []})
        self.static['callers']['selected'][0]['context'] = ['402000: e8 00 call 0x401111']
        with self.assertRaisesRegex(ValueError, 'exact CALL bytes'):
            caller_returns(self.static)

    def test_invalid_route_links_ownership_effects_and_decisions_refuse(self):
        for change in ({'owner': ''}, {'behaviors': ['missing']}, {'side_effects': []}, {'classification': 'guessed'}, {'support': {'decision': 'complete', 'reason': 'guess'}}, {'entry': 0x401001}):
            original = copy.deepcopy(self.catalog)
            self.catalog['routes'][0].update(change)
            with self.subTest(change=change), self.assertRaises((ValueError, KeyError)):
                self.report()
            self.catalog = original

    def test_ambiguous_selectors_refuse(self):
        row = copy.deepcopy(self.catalog['routes'][1]); row['id'] = 'duplicate'
        self.catalog['routes'].append(row)
        with self.assertRaisesRegex(ValueError, 'Ambiguous'):
            self.report()

    def test_record_hash_and_original_build_must_match(self):
        (self.root / 'live.json').write_text('{}')
        with self.assertRaisesRegex(ValueError, 'changed evidence'):
            self.report()
        self.record['original'] = 'wrong'; self.write_record('live', self.record)
        with self.assertRaisesRegex(ValueError, 'build mismatch'):
            self.report()

    def test_offline_or_static_activity_cannot_count_as_live(self):
        for kind in ('static', 'isolated_original', 'native_integration'):
            self.register['evidence'][1]['kind'] = kind
            with self.subTest(kind=kind), self.assertRaisesRegex(ValueError, 'Offline fixture'):
                self.report()

    def test_incomplete_draw_rows_and_boolean_counts_refuse(self):
        self.record['rows'][0]['rows_captured'] = 3; self.write_record('live', self.record)
        with self.assertRaisesRegex(ValueError, 'Incomplete original'):
            self.report()
        self.record['rows'][0]['rows_captured'] = 4; self.record['counts']['copy'] = True; self.write_record('live', self.record)
        with self.assertRaisesRegex(ValueError, 'nonnegative integers'):
            self.report()

    def test_hidden_signed_kind_is_preserved(self):
        self.assertEqual(selector_key('draw_kind', '-2'), selector_key('draw_kind', '0xfffffffe'))
        self.assertEqual(next(x for x in self.report()['activities'] if x['identity'] == '-2')['route'], 'hidden')

    def close_startup(self):
        self.catalog['scenarios'][0]['census_review'] = {'observation': 'cohort', 'pointer': '/census', 'reason': 'Exact scoped trace reviewed'}
        self.record['census'] = {'complete': True, 'caller_attribution_complete': True, 'side_effect_inventory_complete': True,
                                 'owner_classification_complete': True, 'lost_events': 0, 'unknown_routes': 0,
                                 'route_census_sha256': route_digest(self.catalog)}
        self.write_record('live', self.record)

    def test_pixel_success_and_closed_journal_do_not_close_scenario(self):
        self.assertTrue(self.record['success'])
        self.assertFalse(self.report()['scenarios'][0]['closed_route_census'])
        self.close_startup()
        self.assertTrue(self.report()['scenarios'][0]['closed_route_census'])
        self.assertFalse(self.report()['draw01_complete'])  # save/load and dispatch still open

    def test_false_stale_or_lost_event_closure_refuses(self):
        for field, value in [('lost_events', 1), ('unknown_routes', 1), ('complete', False), ('route_census_sha256', 'old')]:
            self.close_startup(); self.record['census'][field] = value; self.write_record('live', self.record)
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.report()

    def test_route_edit_invalidates_exact_census_closure(self):
        self.close_startup(); self.catalog['routes'][0]['remaining'] = 'New failure branch'
        with self.assertRaisesRegex(ValueError, 'exact reviewed'):
            self.report()

    def test_all_required_closures_and_dispatch_are_needed_for_completion(self):
        self.close_startup()
        self.catalog['scenarios'] = self.catalog['scenarios'][:1]
        self.register['dispatch_tables'][0]['complete'] = True
        self.assertTrue(self.report()['draw01_complete'])
        self.catalog['routes'][0]['support']['decision'] = 'unresolved'
        self.record['census']['route_census_sha256'] = route_digest(self.catalog)
        self.write_record('live', self.record)
        self.assertFalse(self.report()['draw01_complete'])

    def test_forged_activity_classification_refuses(self):
        self.catalog['observations'][0]['projections'][0]['activity'] = 'complete_replacement'
        with self.assertRaisesRegex(ValueError, 'activity classification'):
            self.report()

    def test_incomplete_dispatch_and_indirect_flows_stay_visible(self):
        r = self.report()
        self.assertEqual(r['dispatch_tables'][0]['entries'], [])
        self.assertEqual(r['static_indirect_calls'][0]['discovered_owners'], [])
        self.assertEqual(len(r['static_computed_jumps']), 1)

    def test_missing_scenario_research_and_protected_paths_refuse(self):
        self.catalog['observations'][0]['scenario'] = 'missing'
        with self.assertRaisesRegex(ValueError, 'scenario'):
            self.report()
        self.catalog['observations'][0]['scenario'] = 'startup'
        self.catalog['routes'][0]['documents'] = ['../research.md']
        with self.assertRaisesRegex(ValueError, 'research'):
            self.report()
        for path in (ROOT / 'original/unsafe.json', ROOT / '.git/unsafe.json'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                cli.output_paths([path])

    def journal(self, version=1, caller=0x402005, op=6):
        header = bytearray(64);header[:8] = ('MNMPRO0' + str(version)).encode()
        struct.pack_into('<5I', header, 8, version, 64, int(self.original[:8], 16), 65536, 1)
        entry = [0] * 24;entry[0] = 96;entry[1] = 1;entry[2] = 11;entry[14] = 1
        first = [0] * 24;first[0] = 96;first[1] = 2;first[2] = op;first[22] = caller
        final = [0] * 24;final[0] = 96;final[1] = 3;final[2] = 12;final[14] = 1
        data = bytes(header) + struct.pack('<24I', *entry) + struct.pack('<24I', *first) + struct.pack('<24I', *final)
        stream = self.root / 'producer.bin';stream.write_bytes(data)
        done = self.root / 'producer.done';done.write_bytes(b'MNMPDONE' + struct.pack('<6I', 1, 1, 3, 0, 0, len(data)))
        return stream, done

    def parse(self, stream, done, runtime='0x400000'):
        return producer_trace(stream, done, hashlib.sha256(stream.read_bytes()).hexdigest(), 'startup', runtime, self.report())

    def test_closed_journals_and_relocated_exact_caller(self):
        for version in (1, 2, 3):
            stream, done = self.journal(version, caller=0x702005)
            r = self.parse(stream, done, '0x700000')
            self.assertTrue(r['closed_journal'])
            self.assertEqual(next(x for x in r['rows'] if x['operation'] == 'copy')['entry_route'], 'entry')
            self.assertEqual(r['unattributed_callers'], [])

    def test_nearby_and_external_callers_never_get_nearest_entry(self):
        for caller in (0x402006, 0x1000, 0xffffffff):
            stream, done = self.journal(caller=caller)
            r = self.parse(stream, done)
            self.assertEqual(r['unattributed_callers'][0]['caller_va'], hex(caller))
            self.assertIsNone(r['unattributed_callers'][0]['entry_route'])

    def test_unknown_opcode_is_retained_not_silently_dropped(self):
        stream, done = self.journal(op=77)
        r = self.parse(stream, done)
        self.assertIn('unknown-op-77', {row['operation'] for row in r['unaccounted_operations']})

    def test_truncated_out_of_order_wrong_build_or_payload_journals_refuse(self):
        for offset, value in [(4, 1), (16, 0), (68, 99), (64, 100), (64 + 23 * 4, 1), (64 + 2 * 96 + 14 * 4, 2), (64 + 2 * 4, 6)]:
            stream, done = self.journal();data = bytearray(stream.read_bytes());struct.pack_into('<I', data, offset, value);stream.write_bytes(data)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                self.parse(stream, done)
        stream, done = self.journal();stream.write_bytes(stream.read_bytes()[:-1])
        with self.assertRaises(ValueError):
            self.parse(stream, done)

    def test_missing_failed_or_mismatched_completion_and_hash_refuse(self):
        for offset, value in [(12, 2), (16, 1), (20, 1), (28, 0)]:
            stream, done = self.journal();data = bytearray(done.read_bytes());struct.pack_into('<I', data, offset, value);done.write_bytes(data)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                self.parse(stream, done)
        stream, done = self.journal()
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            producer_trace(stream, done, 'wrong', 'startup', '0x400000', self.report())

    def test_output_paths_never_overwrite(self):
        target = self.root / 'existing.json';target.write_text('{}')
        for paths in ([target], [self.root / 'new', self.root / 'new']):
            with self.subTest(paths=paths), self.assertRaises(ValueError):
                cli.output_paths(paths)

    def test_repository_census_and_required_journeys(self):
        r = build_report()
        self.assertEqual(r['counts']['static_windows'], 24)
        self.assertEqual(r['counts']['instrumented_entries'], 57)  # Current master producer hooks, including optional observations.
        self.assertEqual(r['counts']['required_scenarios'], 14)
        self.assertFalse(r['draw01_complete'])
        # Retain pre-existing master inventory debt without inventing semantics.
        self.assertEqual(r['unaccounted'], [
            {'selector': 'entry', 'identity': '0x547d80', 'activity': 'instrumented_only', 'tag': 52},
            {'selector': 'entry', 'identity': '0x58f890', 'activity': 'instrumented_only', 'tag': 53},
            {'selector': 'entry', 'identity': '0x58f9c0', 'activity': 'instrumented_only', 'tag': 54},
            {'selector': 'entry', 'identity': '0x58f750', 'activity': 'instrumented_only', 'tag': 55},
        ])
        self.assertTrue({'save-load', 'map-transitions', 'battle-results', 'shutdown'} <= {s['id'] for s in r['scenarios']})


if __name__ == '__main__':
    unittest.main()
