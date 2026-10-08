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
from coverage_ui import binary_view, enrich_behaviors, measure, safe_file, summarize, union_size, track_view, request_authority_allowed, CoverageServer, ROOT, TRACKS


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
