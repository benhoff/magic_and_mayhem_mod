#!/usr/bin/env python3
"""Coverage dashboard denominator, association, freshness and HTTP regressions."""
import copy
import http.client
import json
from pathlib import Path
import sys
import tempfile
import threading
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from coverage_ui import binary_view, enrich_behaviors, measure, safe_file, summarize, union_size, track_view, spell_view, request_authority_allowed, CoverageServer, ROOT, TRACKS


def behavior(bid='one', kind='recovered', implementation='partial'):
    return {'id': bid, 'kind': kind, 'subsystem': 'fixture', 'title': bid, 'scope': 'Selected arithmetic only',
            'confidence': 'provisional', 'original': [], 'documents': [], 'implementation': [], 'tests': [], 'evidence': [],
            'current_validation': {stage: {'state': 'current', 'evidence': ['proof', 'original', 'native', 'fresh-original']}
                                   for stage in ('implementation', 'comparison', 'integration', 'replacement')},
            'status': {'understanding': 'partial', 'implementation': implementation,
                       'comparison': 'none', 'integration': 'none', 'replacement': 'none'}}


class MetricsTest(unittest.TestCase):
    def test_union_and_empty_denominators(self):
        self.assertEqual(union_size([[10,20],[15,25],[10,20],[25,30],[40,45]]),25)
        self.assertEqual(measure(0,0), {'count':0,'total':0,'percent':None})

    def test_unique_interior_links_data_ranges_and_classifications(self):
        a=behavior();a['original']=[{'build':'b','address':13},{'build':'b','address':14},
                                  {'build':'b','address':33,'relation':'data'},
                                  {'build':'b','address':45,'recovered_range':'outside'}]
        b=behavior('two');b['original']=[{'build':'b','address':19}]
        inventory={'source_sha256':'fixture', 'functions':[
            {'entry':10,'name':'f1','ranges':[[10,20]]},
            {'entry':30,'name':'f2','ranges':[[30,40]]}],
            'sections':[{'start':10,'end':50,'executable':True}],
            'unassigned_executable_ranges':[[20,30],[40,50]],
            'flows':[{'kind':'call','owner':30,'targets':[10]},
                     {'kind':'call','owner':10,'targets':[30],'computed':True}]}
        register={'classifications':[{'build':'b','entry':30,'category':'runtime_support'}],
                  'recovered_ranges':[{'build':'b','id':'outside','start':40,'end':50}]}
        result=binary_view(register,{'b':inventory},[a,b])
        build=result['builds'][0]
        self.assertEqual(build['functions'],measure(1,2))
        self.assertEqual(build['bytes'],measure(10,40))
        self.assertEqual(build['classified'],1)
        self.assertEqual(result['functions'][0]['behaviors'],['one','two'])
        self.assertEqual(result['functions'][0]['callers'],[30])
        self.assertEqual(result['functions'][0]['callees'],[])
        self.assertEqual(result['functions'][1]['callers'],[])
        self.assertEqual(result['functions'][1]['behaviors'],[])

    def test_native_policies_and_partial_code_do_not_inflate_progress(self):
        a=behavior('baseline');a['status']['comparison']='recorded';a['comparison_freshness']='stale'
        b=behavior('policy','native_policy','scoped');b['comparison_freshness']='not_applicable'
        c=behavior('missing',implementation='none');c['comparison_freshness']='none'
        result=summarize([a,b,c])
        self.assertEqual(result['scoped'],measure(1,3))
        self.assertEqual(result['partial'],measure(1,3))
        self.assertEqual(result['missing'],measure(1,3))
        self.assertEqual(result['comparison'],measure(1,2))
        self.assertEqual(result['current_comparison'],measure(0,2))
        self.assertIsNone(summarize([b])['comparison']['percent'])

    def test_comparison_freshness_is_independent_of_other_evidence(self):
        a=behavior();a['evidence']=['old-static','fresh-original'];a['status']['comparison']='recorded'
        b=copy.deepcopy(a);b['id']='mixed';b['evidence'].append('old-original')
        native=behavior('native','native_policy');native['evidence']=[]
        evidence=[{'id':'old-static','kind':'static'},{'id':'fresh-original','kind':'isolated_original'},
                  {'id':'old-original','kind':'isolated_original'}]
        report={'findings':{'stale_evidence':[{'id':'old-static','changed_sources':['notes.md']},
                                             {'id':'old-original','changed_sources':['engine.cpp']}]},
                'behaviors': [a, b, native]}
        rows,_=enrich_behaviors({'behaviors':[a,b,native],'evidence':evidence,'scenarios':[]},report)
        self.assertEqual(rows[0]['comparison_freshness'],'current')
        self.assertIn('stale',rows[0]['gaps'])
        self.assertEqual(rows[1]['comparison_freshness'],'current')
        self.assertEqual(rows[1]['comparison_source_freshness'],'mixed')
        self.assertNotIn('comparison',rows[2]['gaps'])
        self.assertNotIn('replacement',rows[2]['gaps'])


class FileBoundaryTest(unittest.TestCase):
    def test_only_registered_files_no_traversal_original_or_symlink_escape(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'finding.md').write_text('scope')
            (root/'original').mkdir();(root/'original/game.exe').write_bytes(b'immutable')
            (root/'escape').symlink_to('/etc/passwd')
            allowed={'finding.md','../finding.md','original/game.exe','escape'}
            self.assertEqual(safe_file(root,'finding.md',allowed),root/'finding.md')
            for p in ['../finding.md','original/game.exe','escape','unregistered.txt','/etc/passwd']:
                with self.subTest(path=p),self.assertRaises(ValueError):safe_file(root,p,allowed)


class TracksTest(unittest.TestCase):
    def definition(self, requirements):
        return {'tracks':[{'id':'track','subsystems':['fixture'],'planned':False,
                           'milestones':[{'id':'goal','requirements':requirements}]}]}

    def test_partial_or_missing_contract_never_earns_a_demonstrated_milestone(self):
        a=behavior('a',implementation='scoped');a['evidence']=['proof']
        b=behavior('b');b['evidence']=['proof']
        definition=self.definition([{'behaviors':['a','b'],'status':{'implementation':'scoped'}}])
        evidence=[{'id':'proof','kind':'native_integration','freshness':'current'}]
        t=track_view(definition,[a,b],evidence)[0]
        self.assertEqual(t['progress'],measure(0,1))
        self.assertEqual(t['milestones'][0]['state'],'in_progress')
        t=track_view(definition,[a],evidence)[0]
        self.assertEqual(t['milestones'][0]['state'],'unknown')
        self.assertEqual(t['milestones'][0]['missing'],['b'])
        self.assertEqual(t['progress']['count'],0)

    def test_validation_requirements_are_independent_of_implementation(self):
        a=behavior(implementation='partial');a['status']['comparison']='recorded';a['evidence']=['original']
        d=self.definition([{'behaviors':['one'],'status':{'implementation':'scoped','comparison':'recorded'}}])
        e=[{'id':'original','kind':'isolated_original','freshness':'current'}]
        t=track_view(d,[a],e)[0];c=t['milestones'][0]['checks'][0]
        self.assertEqual(t['progress']['count'],0)  # combined milestone remains incomplete
        self.assertFalse(c['met']);self.assertTrue(c['validation_met']);self.assertTrue(c['validation_current'])
        e[0]['freshness']='stale'
        c=track_view(d,[a],e)[0]['milestones'][0]['checks'][0]
        self.assertTrue(c['validation_met']);self.assertFalse(c['validation_current'])
        a['status']['implementation']='scoped';a['evidence']=[]
        c=track_view(d,[a],e)[0]['milestones'][0]['checks'][0]
        self.assertFalse(c['validation_met']);self.assertFalse(c['validation_current'])
        a['evidence']=['original'];a['current_validation']={}
        c=track_view(d,[a],[{**e[0],'freshness':'current'}])[0]['milestones'][0]['checks'][0]
        self.assertTrue(c['validation_met']);self.assertFalse(c['validation_current'])
        a['status']['comparison']='none'
        c=track_view(d,[a],e)[0]['milestones'][0]['checks'][0]
        self.assertFalse(c['validation_met'])
        a['kind']='native_policy';a['status']['comparison']='recorded'
        c=track_view(d,[a],e)[0]['milestones'][0]['checks'][0]
        self.assertFalse(c['validation_met']);self.assertFalse(c['validation_current'])

    def test_invalid_status_targets_fail_before_progress_is_generated(self):
        definition = self.definition([{'behaviors': ['one'], 'status': {'integration': ['scoped_live']}}])
        with self.assertRaisesRegex(ValueError, 'invalid integration target'):
            track_view(definition, [], [])

    def test_live_equivalence_retains_an_accepted_integration_milestone(self):
        a = behavior(implementation='scoped')
        a['status']['integration'] = 'live_equivalence'
        a['evidence'] = ['proof']
        d = self.definition([{'behaviors': ['one'], 'status': {
            'integration': ['headless', 'preview', 'live_observation', 'live_equivalence']}}])
        e = [{'id': 'proof', 'kind': 'live_equivalence', 'freshness': 'current'}]
        self.assertEqual(track_view(d, [a], e)[0]['current'], 1)

    def test_unbound_fresh_sources_do_not_count_as_current_execution(self):
        a = behavior(implementation='scoped')
        a['evidence'] = ['proof']
        a['current_validation'] = {}
        d = self.definition([{'behaviors': ['one'], 'status': {'implementation': 'scoped'}}])
        e = [{'id': 'proof', 'kind': 'native_integration', 'freshness': 'current'}]
        result = track_view(d, [a], e)[0]
        self.assertEqual(result['progress']['count'], 1)  # historical stage remains visible
        self.assertEqual(result['current'], 0)

    def test_recorded_achievement_and_current_proof_are_independent(self):
        a=behavior(implementation='scoped');a['status'].update(comparison='recorded',integration='headless')
        a['evidence']=['original','native']
        d=self.definition([{'behaviors':['one'],'status':{'implementation':'scoped','comparison':'recorded','integration':['headless']}}])
        e=[{'id':'original','kind':'isolated_original','freshness':'stale'},
           {'id':'native','kind':'native_integration','freshness':'current'}]
        t=track_view(d,[a],e)[0]
        self.assertEqual(t['progress'],measure(1,1));self.assertEqual(t['current'],0)
        e[0]['freshness']='current'
        self.assertEqual(track_view(d,[a],e)[0]['current'],1)
        self.assertEqual(track_view(d,[a],e[:1])[0]['progress']['count'],0)

    def test_observation_and_native_policy_cannot_satisfy_replacement(self):
        d=self.definition([{'behaviors':['one'],'status':{'implementation':'scoped','replacement':'scoped_live'}}])
        a=behavior(implementation='scoped');a['status']['replacement']='scoped_live';a['evidence']=['proof']
        e=[{'id':'proof','kind':'live_observation','freshness':'current'}]
        self.assertEqual(track_view(d,[a],e)[0]['progress']['count'],0)
        e[0]['kind']='live_replacement'
        self.assertEqual(track_view(d,[a],e)[0]['progress']['count'],1)
        a['kind']='native_policy'
        self.assertEqual(track_view(d,[a],e)[0]['progress']['count'],0)

    def test_large_dispatch_checklist_counts_one_goal_and_empty_group_is_unknown(self):
        d=self.definition([{'subsystem':'fixture','status':{'implementation':'scoped'}}])
        e=[{'id':'proof','kind':'native_integration','freshness':'current'}]
        rows=[behavior(str(i),implementation='scoped') for i in range(104)]
        for b in rows:b['evidence']=['proof']
        self.assertEqual(track_view(d,rows,e)[0]['progress'],measure(1,1))
        rows[-1]['status']['implementation']='none'
        self.assertEqual(track_view(d,rows,e)[0]['progress'],measure(0,1))
        self.assertEqual(track_view(d,[],e)[0]['milestones'][0]['state'],'unknown')

    def test_curated_goal_ids_are_registered_and_future_changes_stay_separate(self):
        definition=json.loads((ROOT/TRACKS).read_text())
        register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        ids={b['id'] for b in register['behaviors']}
        for t in definition['tracks']:
            self.assertTrue(t['milestones'])
            for m in t['milestones']:
                for r in m['requirements']:self.assertTrue(set(r.get('behaviors',[]))<=ids,(t['id'],m['id']))
        planned=[t for t in definition['tracks'] if t['planned']]
        self.assertEqual([t['id'] for t in planned],['future-gameplay'])
        self.assertEqual({bid for m in planned[0]['milestones'] for r in m['requirements'] for bid in r['behaviors']},
                         {'GP.commander','GP.veterancy','GP.mana'})

    def test_native_rendering_focus_links_and_replacement_requirements(self):
        definition=json.loads((ROOT/TRACKS).read_text())
        register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        track=next(t for t in definition['tracks'] if t['id']=='rendering')
        focus=track['focus']
        by_id={b['id']:b for b in register['behaviors']}
        self.assertIn('RS.world-initial-state',focus['next_behaviors'])
        self.assertTrue(set(focus['next_behaviors'])<=by_id.keys())
        documents={p for b in register['behaviors'] for p in b['documents']}
        self.assertTrue(set(focus['documents'])<=documents)
        goals={m['id']:m for m in track['milestones']}
        self.assertEqual(goals['word-takeover']['requirements'][0]['status']['replacement'],'scoped_live')
        self.assertEqual(goals['complete-replacement']['requirements'][0]['behaviors'],['NR.live-drawing-replacement'])
        self.assertEqual(goals['world-initialization']['requirements'][0]['status']['understanding'],'scoped')
        # Native preview must not earn an original comparison or replacement gate.
        self.assertNotIn('replacement',goals['world-shadow']['requirements'][0]['status'])
        self.assertNotIn('comparison',goals['canvas-history']['requirements'][0]['status'])

    def test_simulation_roadmap_keeps_native_results_and_original_gates_separate(self):
        definition=json.loads((ROOT/TRACKS).read_text())
        register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        track=next(t for t in definition['tracks'] if t['id']=='simulation')
        documents={p for b in register['behaviors'] for p in b['documents']}
        self.assertTrue(set(track['focus']['documents']) <= documents)
        self.assertTrue({'creature-ai','creature-execution','casting','combat','native-simulation'} <= set(track['subsystems']))
        goals={m['id']:m for m in track['milestones']}
        self.assertEqual(goals['decision-comparison']['requirements'][0]['status']['comparison'],'recorded')
        self.assertNotIn('comparison',goals['owner-skirmish']['requirements'][0]['status'])
        self.assertNotIn('replacement',goals['playable-campaign']['requirements'][0]['status'])
        self.assertEqual(goals['live-simulation']['requirements'][0]['status']['replacement'],'scoped_live')
        # Even perfect native execution cannot complete the original tick/lifetime gates.
        evidence=[{**e,'freshness':'current'} for e in register['evidence']]
        result=track_view({'tracks':[track]},register['behaviors'],evidence)[0]
        states={m['id']:m['state'] for m in result['milestones']}
        self.assertEqual(states['owner-skirmish'],'demonstrated')
        self.assertEqual(states['decision-comparison'],'demonstrated')
        self.assertEqual(states['wizard-strategy'],'not_started')
        self.assertEqual(states['original-tick-lifetime'],'not_started')
        self.assertEqual(states['live-simulation'],'not_started')
        self.assertFalse(any(m['current'] for m in result['milestones']))

    def test_rendering_followup_keeps_delivered_code_and_acceptance_separate(self):
        definition=json.loads((ROOT/TRACKS).read_text())
        register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        track=next(t for t in definition['tracks'] if t['id']=='rendering')
        self.assertIn('Validate bounded',track['focus']['next_title'])
        evidence=[{**e,'freshness':'current'} for e in register['evidence']]
        result=track_view({'tracks':[track]},register['behaviors'],evidence)[0]
        goals={m['id']:m for m in result['milestones']}
        self.assertEqual(goals['extended-2d']['state'],'demonstrated')
        self.assertEqual(goals['isolated-recovery']['state'],'demonstrated')
        # Bounded native launch has shipped; whole-renderer acceptance stays separate.
        self.assertEqual(goals['native-only-launch']['state'],'demonstrated')
        self.assertEqual(goals['owned-copy-readers']['state'],'in_progress')
        self.assertEqual(goals['generation-restart']['state'],'in_progress')
        self.assertEqual(goals['complete-replacement']['state'],'not_started')
        self.assertIn('NP.native-isolated-render-recovery',result['behaviors'])
        self.assertTrue(set(track['focus']['implementation_behaviors']) <= set(result['behaviors']))
        self.assertIn('partially implemented and unimplemented',track['focus']['boundary'])
        # Native policy controls cannot require or substitute for original equivalence.
        self.assertNotIn('comparison',goals['extended-2d']['requirements'][0]['status'])
        self.assertNotIn('replacement',goals['isolated-recovery']['requirements'][0]['status'])


class SpellProgressTest(unittest.TestCase):
    def setUp(self):
        self.definition = json.loads((ROOT/TRACKS).read_text())
        self.register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        self.by_id = {b['id']: b for b in self.register['behaviors']}

    def test_named_and_unused_catalogs_partition_all_ids_without_removing_gates(self):
        result = spell_view(self.definition, self.register['behaviors'])
        named, unused = set(result['named_behaviors']), set(result['unused_behaviors'])
        self.assertEqual((len(named), len(unused)), (75, 29))
        self.assertFalse(named & unused)
        self.assertEqual(named | unused, {b['id'] for b in self.register['behaviors']
                                         if b['subsystem'] == 'spell_dispatch'})
        self.assertTrue(all(self.by_id[bid]['title'].endswith(': Unused') for bid in unused))
        evidence = [{**e, 'freshness': 'current'} for e in self.register['evidence']]
        tracks = track_view(self.definition, self.register['behaviors'], evidence)
        gameplay = next(t for t in tracks if t['id'] == 'gameplay')
        self.assertFalse(unused & set(gameplay['behaviors']))
        self.assertTrue(named <= set(gameplay['behaviors']))
        full = next(m for m in gameplay['milestones'] if m['id'] == 'spells')
        self.assertTrue((named | unused) <= set(full['behaviors']))
        self.assertNotEqual(full['state'], 'demonstrated')

    def test_delivered_components_count_once_and_whole_spell_stages_stay_pending(self):
        result = spell_view(self.definition, self.register['behaviors'])
        ids = [bid for group in result['groups'] for bid in group['behaviors']]
        self.assertEqual(len(ids), len(set(ids)))
        self.assertEqual(len(ids), 19)
        for entry in result['spells']:
            b = self.by_id[entry['behavior']]
            with self.subTest(spell=b['id']):
                self.assertEqual(b['status']['implementation'], 'none' if b['id'] == 'SP.017' else 'partial')
                for stage in ('comparison', 'integration', 'replacement'):
                    self.assertEqual(b['status'][stage], 'none')
                for component in entry['tasks'] + entry['effects']:
                    self.assertTrue(set(self.by_id[component]['implementation']) <= set(b['implementation']))
                    if self.by_id[component]['implementation']:
                        self.assertTrue(set(self.by_id[component]['tests']) <= set(b['tests']))
        self.assertEqual(self.by_id['CAST.cure-delivery']['status']['implementation'], 'none')
        self.assertEqual(self.by_id['AI.wizard-controller']['status']['implementation'], 'none')

    def test_both_tracks_credit_existing_components_without_double_counting_overview(self):
        evidence = [{**e, 'freshness': 'current'} for e in self.register['evidence']]
        tracks = {t['id']: t for t in track_view(self.definition, self.register['behaviors'], evidence)}
        for tid in ('simulation', 'gameplay'):
            track = tracks[tid]
            for bid in ('CAST.admission', 'AI.wizard-target-task', 'AI.wizard-projectile-task',
                        'NP.native-wizard-encounter'):
                self.assertIn(bid, track['behaviors'])
            milestones = {m['id']: m for m in track['milestones']}
            self.assertEqual(milestones['casting-foundations']['state'], 'demonstrated')
            self.assertEqual(milestones['wizard-spell-tasks']['state'], 'demonstrated')
            self.assertFalse(milestones['wizard-spell-tasks']['current'])
        overview = {bid for t in tracks.values() for bid in t['behaviors']}
        self.assertEqual(list(overview).count('CAST.admission'), 1)

    def test_invalid_crosswalks_and_exclusions_fail_instead_of_hiding_missing_work(self):
        for change in ('missing', 'duplicate', 'unused', 'spell', 'link'):
            d = copy.deepcopy(self.definition)
            plan = d['spell_progress']
            if change == 'missing': plan['groups'][0]['behaviors'].append('missing-contract')
            elif change == 'duplicate': plan['groups'][1]['behaviors'].append(plan['groups'][0]['behaviors'][0])
            elif change == 'unused': plan['unused_behaviors'].append('missing-slot')
            elif change == 'spell': plan['spells'].append(plan['spells'][0])
            else: plan['spells'][0]['tasks'].append('missing-component')
            with self.subTest(change=change), self.assertRaises(ValueError):
                spell_view(d, self.register['behaviors'])
        d = self.definition
        d['tracks'][0]['summary_exclusions'] = ['missing-contract']
        with self.assertRaisesRegex(ValueError, 'Unknown track summary exclusion'):
            track_view(d, self.register['behaviors'], [])
        self.assertIsNone(spell_view({'tracks': []}, []))


class BindAuthorityTest(unittest.TestCase):
    def test_wildcard_and_specific_ipv4_hosts(self):
        for host in ('192.168.1.20:8786','10.0.0.2:8786','127.0.0.1:8786','localhost:8786'):
            self.assertTrue(request_authority_allowed('0.0.0.0',8786,host))
        self.assertTrue(request_authority_allowed('192.168.1.20',8786,'192.168.1.20:8786'))
        self.assertFalse(request_authority_allowed('192.168.1.20',8786,'10.0.0.2:8786'))
        self.assertFalse(request_authority_allowed('127.0.0.1',8786,'192.168.1.20:8786'))
        for host in ('outside.example:8786','192.168.1.20:80','user@192.168.1.20:8786',
                     '192.168.1.20:8786/path','192.168.1.20:8786#x','192.168.1.20',
                     '[::1]:8786','999.1.1.1:8786','192.168.1.20:bad',''):
            self.assertFalse(request_authority_allowed('0.0.0.0',8786,host),host)


class HTTPTest(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name)
        (self.root/'apps/coverage-ui').mkdir(parents=True)
        (self.root/'apps/coverage-ui/index.html').write_text('dashboard')
        (self.root/'finding.md').write_text('<script>quoted research</script>')
        self.mock=patch('coverage_ui.build_snapshot',return_value=({'schema':1,'version':1},{'finding.md'}))
        self.builder=self.mock.start()
        self.server=CoverageServer(('127.0.0.1',0),self.root)
        self.thread=threading.Thread(target=self.server.serve_forever,daemon=True);self.thread.start()

    def tearDown(self):
        self.server.shutdown();self.server.server_close();self.thread.join();self.mock.stop();self.tmp.cleanup()

    def request(self,path,method='GET',headers=None):
        conn=http.client.HTTPConnection('127.0.0.1',self.server.server_port,timeout=5)
        conn.request(method,path,headers=headers or {})
        response=conn.getresponse();result=(response.status,dict(response.getheaders()),response.read());conn.close();return result

    def test_api_static_plaintext_and_denied_boundaries(self):
        self.assertEqual(self.request('/')[0],200)
        self.assertEqual(json.loads(self.request('/api/coverage')[2])['version'],1)
        status,headers,body=self.request('/files?path=finding.md')
        self.assertEqual(status,200);self.assertTrue(headers['Content-Type'].startswith('text/plain'))
        self.assertEqual(headers['X-Content-Type-Options'],'nosniff')
        self.assertIn(b'<script>',body)
        for path in ['/files?path=original/game.exe','/files?path=../finding.md','/tools/coverage_ui.py']:
            self.assertEqual(self.request(path)[0],404)
        self.assertEqual(self.request('/api/coverage',headers={'Host':'outside.example'})[0],403)

    def test_registered_sqlite_report_is_served_without_expanding_its_reference(self):
        import io
        from coverage_evidence import Store, pointer
        name = 'research/runtime/endpoint-result.json'
        path = self.root/name
        path.parent.mkdir(parents=True)
        data = b'{"success":false,"scope":"retained negative evidence"}\n'
        store = Store(self.root/'working/coverage/evidence-pending.sqlite3')
        try:
            with store.db:
                sha = store.put(io.BytesIO(data), len(data))
        finally:
            store.close()
        reference = pointer(sha, len(data))
        path.write_bytes(reference)
        with self.server.refresh_lock:
            self.server.allowed_files.add(name)
        status, headers, body = self.request('/files?path='+name)
        self.assertEqual(status, 200)
        self.assertEqual(body, data)
        self.assertTrue(headers['Content-Type'].startswith('text/plain'))
        self.assertEqual(path.read_bytes(), reference)
        self.assertEqual(self.request('/api/refresh','POST',{'Origin':'https://outside.example'})[0],403)

    def test_wildcard_bind_accepts_lan_url_and_same_origin_refresh(self):
        # Rebind a real wildcard listener; connect over loopback with LAN headers.
        self.server.shutdown();self.server.server_close();self.thread.join()
        self.server=CoverageServer(('0.0.0.0',0),self.root)
        self.thread=threading.Thread(target=self.server.serve_forever,daemon=True);self.thread.start()
        host=f'192.168.1.20:{self.server.server_port}'
        self.assertEqual(self.request('/',headers={'Host':host})[0],200)
        self.assertEqual(self.request('/api/coverage',headers={'Host':host})[0],200)
        self.assertEqual(self.request('/api/refresh','POST',{'Host':host,'Origin':'http://'+host})[0],200)
        self.assertEqual(self.request('/api/refresh','POST',{'Host':host,'Origin':'http://outside.example'})[0],403)
        self.assertEqual(self.request('/api/refresh','POST',{'Host':host,'Origin':f'http://10.0.0.2:{self.server.server_port}'})[0],403)
        self.assertEqual(self.request('/api/coverage',headers={'Host':f'outside.example:{self.server.server_port}'})[0],403)

    def test_explicit_refresh_and_failed_refresh_preserve_snapshot(self):
        self.builder.return_value=({'schema':1,'version':2},{'finding.md'})
        self.assertEqual(json.loads(self.request('/api/refresh','POST')[2])['version'],2)
        self.builder.side_effect=ValueError('malformed register')
        self.assertEqual(self.request('/api/refresh','POST')[0],500)
        self.assertEqual(json.loads(self.request('/api/coverage')[2])['version'],2)


if __name__=='__main__':unittest.main()
