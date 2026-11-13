#!/usr/bin/env python3
import ast,copy,importlib.util,inspect,sys,tempfile,unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock,patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_interaction_pause as m
import rendering_interaction_assessment as original
from broader_interaction_pause_launch import pause_binding

def row(i,mode=1,counter=11):
 return dict(ordinal=i,kind=1,thread=7,tick=i*1000,frame=dict(generation=i,nonce='42',owner=7),mode=mode,input_mode=0,counter=counter,wizard=dict(slot=0,type=0,owner=0,active=1,health=400))

class State(unittest.TestCase):
 def test_actual_pause_byte_is_distinct_from_input_mode(self):
  r=row(1,0);r['input_mode']=1;self.assertEqual(m.checked_pause_state([r])['mode'],0)
 def test_missing_observation_refuses(self):
  with self.assertRaises(ValueError):m.checked_pause_state([])
 def test_dead_wizard_refuses(self):
  r=row(1);r['wizard']['health']=0
  with self.assertRaises(ValueError):m.checked_pause_state([r])
 def test_wrong_owner_refuses(self):
  r=row(1);r['wizard']['owner']=1
  with self.assertRaises(ValueError):m.checked_pause_state([r])
 def test_already_paused_sends_no_Space(self):
  j=SimpleNamespace(session='session',held_key=Mock(),pause_transitions=[])
  with patch.object(m,'live_ui',return_value=[row(1)]):m.set_pause(j,1,'interaction-pause')
  j.held_key.assert_not_called();self.assertFalse(j.pause_transitions[0]['physical_Space_sent'])
 def test_public_Space_observes_actual_pause_byte(self):
  j=SimpleNamespace(session='session',held_key=Mock(),pause_transitions=[])
  with patch.object(m,'live_ui',side_effect=[[row(1,0)],[row(2,1)]]):m.set_pause(j,1,'interaction-pause')
  j.held_key.assert_called_once_with('interaction-pause',32,.22,.6);self.assertTrue(j.pause_transitions[0]['physical_Space_sent'])
 def test_input_mode_alone_cannot_confirm_pause(self):
  a=row(1,0);a['input_mode']=1;j=SimpleNamespace(session='session',held_key=Mock(),pause_transitions=[])
  with patch.object(m,'live_ui',return_value=[a]),patch.object(m.time,'monotonic',side_effect=[0,0,6]),patch.object(m.time,'sleep'):
   with self.assertRaises(ValueError):m.set_pause(j,1,'interaction-pause')
  self.assertEqual(j.held_key.call_count,1);self.assertEqual(j.pause_transitions,[])
 def test_nonce_change_refuses(self):
  a=row(1,0);b=row(2);b['frame']['nonce']='43';j=SimpleNamespace(session='session',held_key=Mock(),pause_transitions=[])
  with patch.object(m,'live_ui',side_effect=[[a],[b]]):
   with self.assertRaises(ValueError):m.set_pause(j,1,'interaction-pause')
 def test_stream_requires_version3(self):
  with tempfile.TemporaryDirectory()as tmp:
   (Path(tmp)/'ui-acceptance.bin').write_bytes(b'x'*16)
   with patch.object(m,'decode',return_value=dict(wire_version=2,rows=[])):
    with self.assertRaises(ValueError):m.live_ui(tmp)

class Proof(unittest.TestCase):
 def setUp(self):
  self.rows=[row(1,0,10)]+[row(i)for i in range(2,15)]+[row(i,0,i-3)for i in range(15,23)]
  self.transitions=[dict(policy=m.POLICY,action='interaction-pause',wanted=1,physical_Space_sent=True,before_ui_row=1,after_ui_row=2),dict(policy=m.POLICY,action='interaction-resume',wanted=0,physical_Space_sent=True,before_ui_row=14,after_ui_row=15)]
  self.actions=[dict(action=label,symbol=32,keycode=65,held_ms=220,down_up=True,host_focus=dict(host_focused=True))for label in('interaction-pause','interaction-resume')]
  self.actions += [dict(action='source-sprite-focus',source_generation=1),dict(action='source-sprite-focus',source_generation=7)]+[dict(action='source-sprite-target',source_generation=i)for i in range(3,13)]
  self.a=dict(success=True,sources_stable=True,resident=dict(nonce='42'),proof=dict(public_pause_collection=self.transitions),actions=self.actions)
  self.keys=dict(owner=7,rows=[dict(ordinal=i,code=32,tick=t,thread=7,owned=True,input_sequence=100+i*2,down=d)for i,(t,d)in enumerate([(1200,True),(1450,False),(14200,True),(14450,False)],1)])
 def assess(self):
  with patch.object(m,'artifact',return_value=b'fixture'),patch.object(m,'decode',return_value=dict(wire_version=3,rows=self.rows)),patch.object(m,'decode_keys',return_value=self.keys):return m.verify_pause(None,self.a)
 def refuses(self):
  with self.assertRaises((ValueError,KeyError)):self.assess()
 def test_complete_pause_and_resume(self):
  q=self.assess();self.assertTrue(q['verified']);self.assertEqual(len(q['original_Space_key_proofs']),2);self.assertEqual(len(q['resumed_simulation_counters']),8)
 def test_failed_case_cannot_receive_pause_credit(self):self.a['success']=False;self.refuses()
 def test_counter_advancing_while_pause_byte_set_refuses(self):self.rows[6]['counter']=12;self.refuses()
 def test_pause_byte_unset_despite_input_mode1_refuses(self):
  for r in self.rows[1:14]:r['mode']=0;r['input_mode']=1
  self.refuses()
 def test_dead_wizard_in_final_away_interval_refuses(self):self.rows[13]['wizard']['active']=0;self.rows[13]['wizard']['health']=0;self.refuses()
 def test_seven_resumed_counters_refuse(self):self.rows[-1]['counter']=self.rows[-2]['counter'];self.refuses()
 def test_source_nonce_change_refuses(self):self.rows[7]['frame']['nonce']='43';self.refuses()
 def test_foreign_resident_nonce_refuses(self):self.a['resident']['nonce']='43';self.refuses()
 def test_target_outside_pause_interval_refuses(self):self.actions[-1]['source_generation']=16;self.refuses()
 def test_missing_physical_Space_refuses(self):del self.actions[0];self.refuses()
 def test_original_key_release_missing_refuses(self):del self.keys['rows'][1];self.refuses()
 def test_unowned_original_Space_refuses(self):self.keys['rows'][0]['owned']=False;self.refuses()
 def test_foreign_original_key_thread_refuses(self):self.keys['rows'][0]['thread']=8;self.refuses()
 def test_claimed_already_paused_with_action_refuses(self):self.transitions[0]['physical_Space_sent']=False;self.rows[0]['mode']=1;self.refuses()
 def test_negative_row_index_refuses(self):self.transitions[0]['before_ui_row']=0;self.refuses()
 def test_missing_target_refuses(self):self.actions.pop();self.refuses()

class Selection(unittest.TestCase):
 def test_every_other_proof_AST_node_is_preserved(self):
  source=inspect.getsource(original.interaction_proof);tree=ast.parse(m.selected_interaction_proof_source(source));original_if=ast.parse("if mode+'-resume' in windows:\n required.append(mode+'-resume');interval(mode+'-resume')").body[0];counts=dict(initial=0,terminal=0,old=0)
  class Restore(ast.NodeTransformer):
   def visit_If(self,node):
    if ast.unparse(node.test)=="'interaction-pause' in windows":counts['initial']+=1;return None
    return self.generic_visit(node)
   def visit_Expr(self,node):
    if ast.unparse(node)in["required.append('interaction-resume')","interval('interaction-resume')"]:counts['terminal']+=1;return None
    return self.generic_visit(node)
   def visit_For(self,node):
    node=self.generic_visit(node)
    if ast.unparse(node.target)=='(mode, scaling)':node.body.insert(0,copy.deepcopy(original_if));counts['old']+=1
    return node
  restored=Restore().visit(tree);self.assertEqual(counts,dict(initial=1,terminal=2,old=1));self.assertEqual(ast.dump(ast.parse(source),include_attributes=False),ast.dump(restored,include_attributes=False))
 def test_changed_original_schedule_refuses(self):
  source=inspect.getsource(original.interaction_proof).replace("mode+'-resume' in windows","mode+'-other' in windows")
  with self.assertRaises(ValueError):m.selected_interaction_proof_source(source)
 def test_full_outer_assessor_code_remains_identical(self):
  path=ROOT/'tools/compare-broader-paused-interaction.py';sp=importlib.util.spec_from_file_location('paused_comparison_control',path);module=importlib.util.module_from_spec(sp);sp.loader.exec_module(module);proof=Mock();history=Mock();bound=module.bind_assessor(original.assess,proof,history);self.assertIs(bound.__code__,original.assess.__code__);self.assertIs(bound.__globals__['interaction_proof'],proof);self.assertIs(bound.__globals__['source_contexts'],history)

class Launch(unittest.TestCase):
 def binding(self):
  calls=[]
  def factory(command,*args,**kwargs):calls.append((command,kwargs));return 'child'
  class Base:
   def __init__(self):self.root=ROOT;self.plan=dict(family='interaction',player_binary=dict(path='owned-player'),interaction_pause_policy=dict(policy=m.POLICY));self.popen=factory
   def patch(self,module):return module
   def install(self):
    import subprocess
    subprocess.Popen=self.popen
   def restore(self):pass
   def receipt(self):return {}
  return pause_binding(Base)(),calls,factory
 def test_UI_observer_only_on_exact_owned_Native_launch(self):
  import subprocess
  b,calls,factory=self.binding()
  with patch.object(subprocess,'Popen',factory):
   b.install();subprocess.Popen([str(ROOT/'owned-player')],env={'existing':'kept'});subprocess.Popen(['other'],env={'existing':'kept'});b.restore();self.assertIs(subprocess.Popen,factory)
  self.assertEqual(calls[0][1]['env'],{'existing':'kept','MNM_ACCEPTANCE_UI':'1'});self.assertEqual(calls[1][1]['env'],{'existing':'kept'});self.assertEqual(len(b.pause_launches),1)
 def test_conflicting_UI_flag_refuses_before_parent(self):
  import subprocess
  b,calls,factory=self.binding()
  with patch.object(subprocess,'Popen',factory):
   b.install()
   with self.assertRaises(ValueError):subprocess.Popen([str(ROOT/'owned-player')],env={'MNM_ACCEPTANCE_UI':'0'})
   b.restore()
  self.assertEqual(calls,[])
 def test_exact_controller_patched_once_with_schedule_only(self):
  b,calls,factory=self.binding();ready=lambda _:None;module=SimpleNamespace(__file__='run-rendering-interaction.py',InteractionJourney=type('Old',(),{'ready_target':ready}));b.patch(module);selected=module.InteractionJourney;b.patch(module);self.assertIs(selected,module.InteractionJourney);self.assertIs(selected.ready_target,ready);self.assertIs(selected.interaction_schedule,m.interaction_schedule)
 def test_missing_observer_execution_refuses_receipt(self):
  b,calls,factory=self.binding()
  with self.assertRaises(ValueError):b.receipt()

if __name__=='__main__':unittest.main()
