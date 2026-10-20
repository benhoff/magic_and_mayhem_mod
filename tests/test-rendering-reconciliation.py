#!/usr/bin/env python3
"""Source/scope review guard tests; no game or execution-equivalence claims."""
import copy
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_claims import behavior_contract
from rendering_reconciliation import load_reviews,safe

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

class ReviewTest(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        (self.root/'code.cpp').write_text('int render_owned(){return 1;}\n')
        (self.root/'scope.md').write_text('Selected owned inputs; full original ownership remains open.\n')
        self.b={'id':'one','kind':'native_policy','scope':'Selected owned renderer.', 'original':[],
                'implementation':['code.cpp'],'tests':[],'documents':['scope.md'],
                'status':{'implementation':'partial'},'implementation_review':{'review':'review-one','entry':'one'}}
        self.doc={'schema':1,'id':'review-one','track':'rendering','document':'scope.md','static_review_only':True,
                  'reviewed_count':1,'reviewed_behaviors':['one'],'counts':{'bounded_code':1},
                  'source_sha256':{p:sha(self.root/p) for p in ('code.cpp','scope.md')},'entries':[{
                      'behavior':'one','role':'native_capability','disposition':'bounded_code','finding':'Owned renderer entry exists.',
                      'remaining':'Original and full-session validation remain open.','recorded_implementation':'partial',
                      'contract_sha256':behavior_contract(self.b,{}),'reviewed_paths':['code.cpp','scope.md'],
                      'code_witness':{'path':'code.cpp','line':1,'text':'int render_owned(){return 1;}'},
                      'boundary_witness':{'path':'scope.md','line':1,'text':(self.root/'scope.md').read_text().strip()},'related_behaviors':[]} ]}
        self.r={'builds':[],'behaviors':[self.b],'implementation_reviews':[]};self.write()
    def write(self):
        p=self.root/'review.json';p.write_text(json.dumps(self.doc));self.r['implementation_reviews']=[{'id':'review-one','path':'review.json','sha256':sha(p)}]
    def test_static_review_never_promotes_code_or_validation(self):
        before=copy.deepcopy(self.r);out=load_reviews(self.root,self.r)[0]
        self.assertEqual(out['counts'],{'bounded_code':1});self.assertEqual(out['freshness'],'current')
        self.assertEqual(self.r,before);self.assertEqual(self.b['status'],{'implementation':'partial'})
    def test_changed_code_scope_and_status_remain_historical(self):
        (self.root/'code.cpp').write_text('int render_owned(){return 2;}\n')
        row=load_reviews(self.root,self.r)[0];self.assertEqual(row['freshness'],'historical');self.assertEqual(row['entries'][0]['changed_sources'],['code.cpp'])
        self.b['scope']='Broader scope';self.assertFalse(load_reviews(self.root,self.r)[0]['entries'][0]['contract_current'])
        self.b['scope']='Selected owned renderer.';self.b['status']['implementation']='scoped'
        self.assertEqual(load_reviews(self.root,self.r)[0]['freshness'],'historical')
    def test_pending_source_review_is_historical(self):
        self.doc['entries'][0]['pending_source_review']=['code.cpp'];self.write()
        self.assertEqual(load_reviews(self.root,self.r)[0]['freshness'],'historical')

    def test_report_hash_witness_and_count_corruption_refuse(self):
        (self.root/'review.json').write_text('{}')
        with self.assertRaisesRegex(ValueError,'hash mismatch'):load_reviews(self.root,self.r)
        self.doc['entries'][0]['code_witness']['text']='invented body';self.write()
        with self.assertRaisesRegex(ValueError,'witness differs'):load_reviews(self.root,self.r)
        self.doc['entries'][0]['code_witness']['text']='int render_owned(){return 1;}'
        self.doc['counts']={'bounded_code':2};self.write()
        with self.assertRaisesRegex(ValueError,'counts disagree'):load_reviews(self.root,self.r)
    def test_missing_duplicate_and_orphan_links_refuse(self):
        self.doc['entries'].append(copy.deepcopy(self.doc['entries'][0]));self.write()
        with self.assertRaisesRegex(ValueError,'Duplicate or incomplete'):load_reviews(self.root,self.r)
        self.doc['entries'].pop();self.b['implementation_review']={'review':'other','entry':'one'};self.write()
        with self.assertRaisesRegex(ValueError,'register review link'):load_reviews(self.root,self.r)
        self.r['implementation_reviews']=[]
        with self.assertRaisesRegex(ValueError,'Orphan'):load_reviews(self.root,self.r)
    def test_missing_source_and_fake_relations_refuse(self):
        self.doc['entries'][0]['reviewed_paths']=['scope.md'];self.write()
        with self.assertRaisesRegex(ValueError,'Incomplete reviewed'):load_reviews(self.root,self.r)
        self.doc['entries'][0]['reviewed_paths']=['code.cpp','scope.md'];self.doc['entries'][0]['related_behaviors']=['unknown'];self.write()
        with self.assertRaisesRegex(ValueError,'Invalid narrower'):load_reviews(self.root,self.r)
    def test_paths_cannot_open_original_working_or_escape(self):
        for p in ['../escape','/etc/passwd','original/game.exe','working/a','x/../original/game.exe']:
            with self.subTest(path=p),self.assertRaises(ValueError):safe(self.root,p)
        (self.root/'alias').symlink_to('/etc')
        with self.assertRaises(ValueError):safe(self.root,'alias/passwd')
        self.assertEqual(safe(self.root,'sub/../code.cpp'),self.root/'code.cpp')

class ActualReviewTest(unittest.TestCase):
    def test_all_reviewed_contracts_have_explicit_source_scope_decisions(self):
        root=Path(__file__).resolve().parents[1];r=json.loads((root/'research/runtime/coverage/register.json').read_text())
        reviews=[row for row in load_reviews(root,r) if row['track']=='rendering']
        entries=[entry for review in reviews for entry in review['entries']]
        from collections import Counter
        self.assertEqual(sum(review['reviewed_count'] for review in reviews),207)
        self.assertEqual(Counter(entry['disposition'] for entry in entries),{'bounded_code': 75, 'acceptance_scope': 7, 'diagnostic_tooling': 111, 'umbrella_scope': 4, 'original_scope': 10})
        rows={row['behavior']:row for row in entries}
        self.assertEqual(rows['AN.world-facing']['recorded_implementation'],'partial')
        self.assertIn('native_providers.cpp',rows['AN.world-facing']['code_witness']['path'])
        self.assertEqual(rows['RS.displacement-final-word-read']['disposition'],'bounded_code')
        self.assertIn('distortion_word_read.hpp',rows['RS.displacement-final-word-read']['code_witness']['path'])
        self.assertEqual(rows['NR.surfaces']['disposition'],'bounded_code')
        self.assertIn('NR.surface-backend.formats-pitch',rows['NR.surfaces']['related_behaviors'])
        self.assertIn('RS.world-kind8-methods',rows['RS.world-kind8-virtual']['related_behaviors'])
        self.assertEqual(rows['NR.live-drawing-replacement']['disposition'],'acceptance_scope')
        self.assertTrue(all(row['finding'] and row['remaining'] and row['reviewed_paths'] for row in rows.values()))
if __name__=='__main__':unittest.main()
