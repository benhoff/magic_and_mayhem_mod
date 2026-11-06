#!/usr/bin/env python3
"""Scoped compatibility must preserve existing proof without waiving changed work."""
import copy
import ast
import hashlib
import json
import importlib.util
import subprocess
import tempfile
import io
from contextlib import redirect_stdout
from unittest.mock import patch
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_applicability import python_projection, evaluate, plan, PREFIX, source_dump
from coverage_claims import behavior_contract, fingerprint, validation_support


class ProjectionTests(unittest.TestCase):
    baseline=b'import math\nLIMIT=3\nUNUSED=8\ndef helper(x):\n return x+LIMIT\ndef checked(x):\n return helper(x)\ndef other():\n return 5\nif __name__ == "__main__":\n print(other())\n'

    def digest(self,raw):return python_projection(raw,['checked'])

    def test_unrelated_additions_comments_and_wrapper_do_not_reopen_callable(self):
        expected=self.digest(self.baseline)
        for raw in (self.baseline+b'\ndef extra():\n return 42\n',self.baseline.replace(b'return 5',b'return 19'),self.baseline.replace(b'UNUSED=8',b'UNUSED=9'),self.baseline.replace(b'print(other())',b'print("new wrapper")'),self.baseline.replace(b'return x+LIMIT',b'return x + LIMIT # formatting')):
            self.assertEqual(self.digest(raw),expected)

    def test_reachable_helpers_globals_imports_and_side_effects_invalidate(self):
        expected=self.digest(self.baseline)
        for raw in (self.baseline.replace(b'x+LIMIT',b'x-LIMIT'),self.baseline.replace(b'LIMIT=3',b'LIMIT=4'),self.baseline+b'\nimport random\n',self.baseline+b'\nchecked.cache=True\n',self.baseline+b'\ndef extra(value=checked(7)):\n return value\n'):
            self.assertNotEqual(self.digest(raw),expected)

    def test_dynamic_namespace_and_rebinding_refuse_scope(self):
        for raw in (b'def checked():\n return globals()["unknown"]()\n',b'from os import *\ndef checked():\n return 1\n',self.baseline+b'\nLIMIT=9\n',b'import sys\ndef checked():\n return sys.modules[__name__]\n',b'def checked():\n return getattr(object(), "dynamic")\n'):
            with self.assertRaises(ValueError):self.digest(raw)

    def test_unrelated_test_class_is_not_new_execution_credit(self):
        old=b'import unittest\nclass Existing(unittest.TestCase):\n @classmethod\n def setUpClass(cls): pass\n def test_old(self): self.assertTrue(True)\n'
        new=old+b'class Added(unittest.TestCase):\n def test_added(self): self.assertTrue(False)\n'
        self.assertEqual(python_projection(old,['Existing']),python_projection(new,['Existing']))
        self.assertNotEqual(python_projection(old,['Existing']),python_projection(old.replace(b'True',b'False'),['Existing']))

    def test_class_initializers_and_shadowed_descriptor_are_not_ignored(self):
        old=b'def checked(): return 1\n'
        for extra in (b'class Added:\n value=checked()\n',b'def classmethod(f):\n checked.cache=True\n return f\nclass Added:\n @classmethod\n def f(cls): pass\n',b'class Base:\n def __init_subclass__(cls): checked.cache=True\nclass Added(Base): pass\n'):
            self.assertNotEqual(self.digest(old),self.digest(old+extra))

    def test_ast_fingerprint_is_independent_of_empty_optional_version_fields(self):
        node=ast.parse('def checked(x): return x+1').body[0];expected=source_dump(node)
        changed=copy.deepcopy(node);changed._fields=(*changed._fields,'future_optional','future_list');changed.future_optional=None;changed.future_list=[]
        self.assertEqual(source_dump(changed),expected)
        self.assertIn('value=None',source_dump(ast.parse('value=None')))
        self.assertIn('value=None',source_dump(ast.parse('match value:\n case None: pass')))
        self.assertEqual(source_dump(ast.parse('def checked(x): return x+1')),
            "Module(body=[FunctionDef(name='checked', args=arguments(args=[arg(arg='x')]), body=[Return(value=BinOp(left=Name(id='x', ctx=Load()), op=Add(), right=Constant(value=1)))])])")

    def test_missing_duplicate_or_invalid_roots_refuse(self):
        for roots in ([],['missing'],['checked','checked'],['checked.member']):
            with self.assertRaises(ValueError):python_projection(self.baseline,roots)
        with self.assertRaises(ValueError):self.digest(b'not python :(')


class ApplicabilityTests(unittest.TestCase):
    def setUp(self):
        self.path='tools/analysis.py';self.old=b'def checked(x): return x+1\n';self.raw={self.path:self.old+b'def unrelated(): return 9\n'}
        self.b=dict(id='B',kind='native_policy',scope='Existing finite behavior',original=[],implementation=[self.path],tests=[],status=dict(understanding='scoped',implementation='scoped',comparison='none',integration='none',replacement='none'),evidence=['E'])
        self.ev=dict(id='E',kind='native_integration',record='old.json',sha256='a'*64)
        self.sources={'E':{self.path:hashlib.sha256(self.old).hexdigest()}}
        self.claim=dict(behavior='B',contract_sha256=behavior_contract(self.b,{}),scenarios={})
        self.claims={'E':{'B':self.claim}};self.base=PREFIX+'source.bin';self.raw[self.base]=self.old
        self.anchor=PREFIX+'anchor.json';self.data={self.anchor:dict(schema=1,behavior=copy.deepcopy(self.b),evidence=copy.deepcopy(self.ev))}
        self.report=PREFIX+'review.json'
        self.q=dict(schema=1,kind='scoped_source_applicability',id='R',preservation_only=True,reason='Reviewed checked() and its full module initialization/local dependency closure; unrelated definition added.',evidence=dict(id='E',record='old.json',sha256='a'*64,definition_sha256=fingerprint(self.ev)),behavior='B',contract_sha256=self.claim['contract_sha256'],scenarios={},preserve_status={'implementation':'scoped'},baseline_anchor=dict(record=self.anchor,sha256=self.digest(self.anchor)),source_contracts=[dict(path=self.path,phase='analysis',kind='python-callables-v1',recorded_sha256=self.sources['E'][self.path],baseline_record=self.base,symbols=['checked'],projection_sha256=python_projection(self.old,['checked']))])
        self.data[self.report]=self.q;self.reg=dict(behaviors=[self.b],evidence_applicability=[dict(id='R',record=self.report,sha256=self.digest(self.report))])

    def digest(self,path):return hashlib.sha256(self.raw[path] if path in self.raw else json.dumps(self.data[path],sort_keys=True).encode()).hexdigest()
    def refresh(self):self.reg['evidence_applicability'][0]['sha256']=self.digest(self.report)
    def run_review(self):return evaluate(ROOT,self.reg,{}, {'E':self.ev},self.sources,self.claims,{},lambda p:self.data[p],lambda p:self.raw[p],self.digest)
    def support(self,review):return validation_support(self.b,{}, {'E':self.ev},self.sources,self.claims,{}, {'E'},review,{'E':[self.path]})['implementation']

    def test_compatible_scope_retains_stage_without_freshening_raw_hash(self):
        reviews,diag=self.run_review();self.assertEqual(diag[0]['compatible_sources'],[self.path]);self.assertEqual(self.support(reviews['B'])['state'],'current');self.assertEqual(self.sources['E'][self.path],hashlib.sha256(self.old).hexdigest());self.assertNotEqual(self.digest(self.path),self.sources['E'][self.path]);self.assertEqual(plan([self.path],reviews['B']['E']),[])

    def test_changed_scoped_logic_requires_retained_analysis_not_game_run(self):
        self.raw[self.path]=b'def checked(x): return x-1\n';reviews,_=self.run_review();self.assertEqual(self.support(reviews['B'])['state'],'pending');self.assertEqual(plan([self.path],reviews['B']['E']),[dict(action='reanalyze-retained-evidence',sources=[self.path])])

    def test_uncovered_runtime_change_never_gets_waived(self):
        reviews,_=self.run_review();review=reviews['B']['E'];self.assertEqual(plan(['renderer/backend.cpp'],review),[dict(action='rerun-affected-scenario',sources=['renderer/backend.cpp'])]);result=validation_support(self.b,{}, {'E':self.ev},self.sources,self.claims,{}, {'E'},reviews['B'],{'E':[self.path,'renderer/backend.cpp']});self.assertEqual(result['implementation']['state'],'pending')

    def test_scope_and_scenario_changes_expire_review(self):
        self.b['scope']='Wider behavior';reviews,diag=self.run_review();self.assertEqual(reviews,{});self.assertIn('behavior_contract_changed',diag[0]['reasons'])
        self.b['scope']='Existing finite behavior';self.q['scenarios']={'new':'f'*64};self.refresh();reviews,diag=self.run_review();self.assertEqual(reviews,{});self.assertIn('scenario_contract_changed',diag[0]['reasons'])

    def test_review_cannot_support_a_new_stage_or_other_behavior(self):
        reviews,_=self.run_review();self.b['status']['implementation']='partial';self.assertEqual(self.support(reviews['B'])['state'],'pending');self.b['status']['implementation']='scoped';self.assertEqual(self.support({})['state'],'pending')

    def test_original_bytes_report_identity_and_projection_are_pinned(self):
        self.raw[self.base]+=b'# changed';
        with self.assertRaises(ValueError):self.run_review()
        self.raw[self.base]=self.old;self.q['source_contracts'][0]['projection_sha256']='f'*64;self.refresh()
        with self.assertRaises(ValueError):self.run_review()
        self.q['source_contracts'][0]['projection_sha256']=python_projection(self.old,['checked']);self.refresh();self.q['reason']='silently changed'
        with self.assertRaises(ValueError):self.run_review()

    def test_native_and_runtime_projection_or_status_anchor_forgery_refuse(self):
        entry=self.q['source_contracts'][0];entry['phase']='execution';self.refresh()
        with self.assertRaises(ValueError):self.run_review()
        entry['phase']='analysis';entry['path']='renderer/backend.cpp';self.sources['E'][entry['path']]=self.sources['E'][self.path];self.refresh()
        with self.assertRaises(ValueError):self.run_review()
        entry['path']=self.path;self.q['preserve_status']['integration']='live_equivalence';self.refresh()
        with self.assertRaises(ValueError):self.run_review()

    def test_applicability_revision_requires_explicit_append_only_supersession(self):
        newer=copy.deepcopy(self.q);newer.update(id='R2',supersedes='R',reason='Reviewed the next complete boundary; preserve the previous report.')
        path=PREFIX+'review2.json';self.data[path]=newer;self.reg['evidence_applicability'].append(dict(id='R2',record=path,sha256=self.digest(path)))
        reviews,diag=self.run_review();self.assertEqual(reviews['B']['E']['id'],'R2');self.assertEqual(diag[0]['state'],'superseded')
        newer['supersedes']='unknown';self.reg['evidence_applicability'][-1]['sha256']=self.digest(path)
        with self.assertRaises(ValueError):self.run_review()

    def test_source_boundaries_cannot_escape_or_relax_native_execution(self):
        entry=self.q['source_contracts'][0];entry['path']='tools/../runtime/code.py';self.sources['E'][entry['path']]=self.sources['E'][self.path];self.refresh()
        with self.assertRaises(ValueError):self.run_review()
        entry.update(path='renderer/code.cpp',kind='full-source-v1',phase='comparison',projection_sha256=self.sources['E'][self.path]);self.sources['E'][entry['path']]=self.sources['E'][self.path];self.refresh()
        with self.assertRaises(ValueError):self.run_review()

    def test_phase_plan_is_specific_and_unknown_sources_are_conservative(self):
        review=dict(compatible_sources=set(),phases={'checks.py':'controls','oracle.cpp':'comparison','analysis.py':'analysis'})
        steps=plan(['checks.py','oracle.cpp','analysis.py','runtime.cpp'],review);self.assertEqual({s['action'] for s in steps},{'rerun-controls','recompare-retained-captures','reanalyze-retained-evidence','rerun-affected-scenario'})


class CliTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        self.write('.evidence-store.json',json.dumps(dict(schema=1,archives=[])).encode())
        spec=importlib.util.spec_from_file_location('applicability_cli',ROOT/'tools/evidence-applicability.py');self.cli=importlib.util.module_from_spec(spec);spec.loader.exec_module(self.cli)
        self.source='tools/checker.py';self.write(self.source,b'def checked(x): return x+1\n');self.write('renderer/core.cpp',b'unchanged runtime\n')
        self.b=dict(id='B',kind='native_policy',scope='Existing bounded helper',original=[],implementation=[self.source],tests=[],status=dict(understanding='scoped',implementation='partial',comparison='none',integration='none',replacement='none'),evidence=['E'])
        record=dict(success=True,sources={p:self.sha(p) for p in (self.source,'renderer/core.cpp')},claims=[dict(behavior='B',contract_sha256=behavior_contract(self.b,{}),scenarios={})]);self.write('research/proof.json',json.dumps(record).encode())
        ev=dict(id='E',kind='native_integration',record='research/proof.json',sha256=self.sha('research/proof.json'),sources_pointer='/sources',claims_pointer='/claims',assertions=[dict(pointer='/success',equals=True)])
        self.register=dict(builds=[],behaviors=[self.b],evidence=[ev],scenarios=[]);self.save_register()
        for args in (['init','-q'],['config','user.name','Fixture'],['config','user.email','fixture@example.invalid'],['add','.'],['commit','-qm','accepted original fixture']):subprocess.run(['git','-C',str(self.root),*args],check=True,capture_output=True)
        self.write(self.source,b'def checked(x): return x+1\ndef unrelated(): return 9\n')

    def write(self,p,raw):
        path=self.root/p;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(raw)
    def sha(self,p):return hashlib.sha256((self.root/p).read_bytes()).hexdigest()
    def save_register(self):self.write('research/runtime/coverage/register.json',json.dumps(self.register).encode())
    def invoke(self,*extra):
        argv=['scope','create','--root',str(self.root),'--evidence','E','--behavior','B','--baseline','HEAD','--id','review-1','--reason','Reviewed named checked() closure; preserve the existing helper only.','--report',PREFIX+'fixture.json',*extra]
        with patch.object(sys,'argv',argv),redirect_stdout(io.StringIO()) as out:self.cli.main()
        return json.loads(out.getvalue())

    def test_cli_pins_existing_claim_and_source_without_new_execution(self):
        before=copy.deepcopy(self.register['evidence']);out=self.invoke('--callables','analysis',self.source,'checked')
        self.assertEqual(out['source_actions'],[]);self.assertFalse(out['original_execution_rerun'])
        current=json.loads((self.root/'research/runtime/coverage/register.json').read_text());self.assertEqual(current['evidence'],before)
        from coverage_evidence import Reader,identity
        with Reader(self.root) as reader:
            ref=current['evidence_applicability'][0];q=reader.json(ref['record']);self.assertEqual(reader.digest(ref['record']),ref['sha256']);self.assertTrue(identity((self.root/ref['record']).read_bytes()));self.assertEqual(q['preserve_status'],{'implementation':'partial'});self.assertEqual(reader.read_bytes(q['source_contracts'][0]['baseline_record']),b'def checked(x): return x+1\n')

    def test_invalid_late_boundary_has_no_partial_register_or_report_writes(self):
        before=(self.root/'research/runtime/coverage/register.json').read_bytes()
        with self.assertRaises(ValueError):self.invoke('--callables','analysis',self.source,'checked','--callables','analysis','renderer/core.cpp','checked')
        self.assertEqual((self.root/'research/runtime/coverage/register.json').read_bytes(),before);self.assertFalse((self.root/PREFIX).exists())

    def test_new_status_or_widened_contract_cannot_retrofit_old_execution(self):
        for key,value in [('scope','Wider helper'),('status',dict(self.b['status'],implementation='scoped'))]:
            old=copy.deepcopy(self.b[key]);self.b[key]=value;self.save_register()
            with self.assertRaises(ValueError):self.invoke('--callables','analysis',self.source,'checked')
            self.b[key]=old;self.save_register()
        self.assertFalse((self.root/PREFIX).exists())

    def test_changed_analysis_is_classified_without_becoming_current(self):
        self.write(self.source,b'def checked(x): return x-1\n');out=self.invoke('--callables','analysis',self.source,'checked');self.assertEqual(out['source_actions'],[dict(action='reanalyze-retained-evidence',sources=[self.source])])


if __name__=='__main__':unittest.main()
