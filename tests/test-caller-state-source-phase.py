#!/usr/bin/env python3
"""A host-only review must never launder changed native or analysis sources."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from caller_state_source_phase import HOST,PREFIX,review,native_identity
class Reader:
 def __init__(self):
  self.old=dict(compiler_sources={'player.cpp':'p'},runtime_sources={'engine.c':'e',**{n:'old'for n in HOST}},player_binary={'sha256':'binary'},oracle={'native_sources':{'oracle.cpp':'o'},'binaries':[{'sha256':'oracle'}]},source_preimages={n:{'record':'old:'+n,'sha256':'old'}for n in HOST})
  self.new=copy.deepcopy(self.old)
  for n in HOST:self.new['runtime_sources'][n]='new';self.new['source_preimages'][n]={'record':'new:'+n,'sha256':'new'}
  self.ref={'record':'oldplan','sha256':'oldplan-sha'};self.proof=dict(prospective_plan=self.ref,sources={'engine.c':'e','analysis.py':'a',**{n:'old'for n in HOST}})
  self.repair=dict(kind='caller_state_capacity_failure_and_bounded_repair_plan',success=True,parent_plan=self.ref,allowed_post_retirement_controls=sorted(HOST),retained_parent_proofs=[{'record':'proof','sha256':'proof-sha'}]);self.new['capacity_repair']={'record':'repair','sha256':'repair-sha'}
  self.data={'oldplan':self.old,PREFIX+'input-plan-05.json':self.new,'repair':self.repair};self.hashes={'oldplan':'oldplan-sha','proof':'proof-sha','repair':'repair-sha',PREFIX+'input-plan-05.json':'newplan-sha','engine.c':'e','analysis.py':'a',**{n:'new'for n in HOST},**{'old:'+n:'old'for n in HOST},**{'new:'+n:'new'for n in HOST}}
 def digest(self,n):return self.hashes[n]
 def json(self,n):return self.data[n]
class Phase(unittest.TestCase):
 def setUp(self):self.r=Reader()
 def test_exact_retired_controls(self):self.assertTrue(review(self.r,'proof',self.r.proof)['verified'])
 def test_native_graph(self):
  self.r.new['runtime_sources']['engine.c']='changed'
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
 def test_changed_analysis(self):
  self.r.hashes['analysis.py']='changed'
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
 def test_changed_native_runtime(self):
  self.r.hashes['engine.c']='changed'
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
 def test_changed_player(self):
  self.r.new['player_binary']['sha256']='other'
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
 def test_changed_oracle(self):
  self.r.new['oracle']['binaries'][0]['sha256']='other'
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
 def test_unknown_prior_proof(self):
  self.r.repair['retained_parent_proofs']=[]
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
 def test_lost_controller_bytes(self):
  self.r.hashes['old:'+sorted(HOST)[0]]='wrong'
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
 def test_wrong_parent(self):
  self.r.repair['parent_plan']={}
  with self.assertRaises(ValueError):review(self.r,'proof',self.r.proof)
if __name__=='__main__':unittest.main()
