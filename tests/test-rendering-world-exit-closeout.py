#!/usr/bin/env python3
"""World-exit negative controls over the actual retained normal executions."""
import copy,importlib.util,os,struct,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('exit_closeout',ROOT/'tools/close-rendering-world-exit.py');exit=importlib.util.module_from_spec(spec);spec.loader.exec_module(exit)


@unittest.skipUnless(os.environ.get('MNM_WORLD_EXIT_RESULT'),'Supply the retained eight-domain result for actual-case controls')
class RetainedWorldExitTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        from coverage_evidence import Reader
        cls.reader=Reader(ROOT);cls.report=cls.reader.json(os.environ['MNM_WORLD_EXIT_RESULT']);cls.normal=exit.normal_module()
        cls.execution=next(v for v in cls.report['executions'] if v['scenario']=='normal-quick' and v['domain']==['player-software',[800,600]])
        cls.a=cls.reader.json(cls.execution['pair']['acquisition']['record']);cls.c=cls.reader.json(cls.execution['pair']['comparison']['record'])
        streams=[cls.reader.read_bytes(v['record']) for v in cls.a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        cls.observed=exit.acquisition(streams,cls.a['session_identity']);cls.lifetimes=exit.imported_lifetimes(cls.observed['commands'])
        cls.channel=cls.normal.retained_artifact(cls.reader,cls.a,'world-continuous.bin');cls.terminal=cls.normal.retained_artifact(cls.reader,cls.a,'world-terminal-0001.bin')
        cls.cleanup=cls.normal.cleanup_proof(cls.reader,cls.a)
    @classmethod
    def tearDownClass(cls):cls.reader.close()
    def args(self):return copy.deepcopy([self.a,self.c,self.observed,self.lifetimes,self.cleanup,self.execution['proof']['terminal_source']])
    def refuses(self,args):
        with self.assertRaises(ValueError):exit.exit_state(*args)
    def rewritten(self,change):
        raw=self.observed['commands'];out=[raw[:16]]
        for op,seq,payload in exit.commands(raw):
            op,payload=change(op,seq,payload);out.append(struct.pack('<3I',op,seq,len(payload))+payload)
        return b''.join(out)
    def test_eight_actual_domains_and_current_raw_terminal_case_pass(self):
        self.assertEqual(len(self.report['executions']),8)
        for v in self.report['executions']:
            self.assertTrue(v['proof']['verified']);self.assertEqual(v['proof']['source_imports'],len(v['proof']['import_incarnations']))
        self.assertEqual(exit.exit_state(*self.args())['last_import'],self.execution['proof']['last_import'])
        self.assertEqual(exit.terminal_source(self.channel,self.terminal,self.lifetimes,self.a['flow']['states'][0]['thread']),self.execution['proof']['terminal_source'])
    def test_import_needs_actual_live_surface_and_constant_nonce(self):
        first=self.lifetimes['imports'][0]['import_sequence']
        for offset,value in [(0,0xffffffff),(12,0)]:
            def change(op,seq,payload):
                if seq==first:
                    body=bytearray(payload);struct.pack_into('<I',body,offset,value);payload=bytes(body)
                return op,payload
            with self.subTest(offset=offset),self.assertRaises(ValueError):exit.imported_lifetimes(self.rewritten(change))
    def test_deleted_source_cannot_be_replaced_by_unknown_owner(self):
        deletion=self.lifetimes['imports'][-1]['deleted_sequence']
        with self.assertRaises(ValueError):exit.imported_lifetimes(self.rewritten(lambda op,seq,p:(op,struct.pack('<I',0xffffffff) if seq==deletion else p)))
    def test_final_source_generation_must_equal_all_returns_and_original_history(self):
        for field in ['imports','completed','verified_returns']:
            args=self.args();args[0]['resident'][field]=str(int(args[0]['resident'][field])+1)
            with self.subTest(field=field):self.refuses(args)
        args=self.args();args[1]['original_reference']['worlds']-=1;self.refuses(args)
    def test_normal_stop_requires_the_exact_cancelled_source_policy(self):
        thread=self.a['flow']['states'][0]['thread']
        for word,value in [(6,0),(8,0),(9,0),(10,0),(11,1),(12,1),(14,1)]:
            raw=bytearray(self.channel);struct.pack_into('<I',raw,word*4,value)
            with self.subTest(word=word),self.assertRaises(ValueError):exit.terminal_source(bytes(raw),self.terminal,self.lifetimes,thread)
        step=(len(self.channel)-128)//2
        for i in range(2):
            for word,value in [(0,2),(2,0)]:
                raw=bytearray(self.channel);struct.pack_into('<I',raw,128+i*step+word*4,value)
                with self.subTest(slot=i,word=word),self.assertRaises(ValueError):exit.terminal_source(bytes(raw),self.terminal,self.lifetimes,thread)
    def test_source_failure_wrong_next_writer_or_wrong_thread_is_not_normal_exit(self):
        thread=self.a['flow']['states'][0]['thread']
        for word,value in [(2,3),(3,0),(4,1),(6,2),(7,0),(8,0),(10,thread+1),(11,1)]:
            raw=bytearray(self.terminal);struct.pack_into('<I',raw,word*4,value)
            with self.subTest(word=word),self.assertRaises(ValueError):exit.terminal_source(self.channel,bytes(raw),self.lifetimes,thread)
    def test_quit_must_follow_the_actual_results_quick_main_chain(self):
        args=self.args();args[1]['original_menu_actions']['original_callbacks'][-3]=[26,1];self.refuses(args)
        args=self.args();args[0]['flow']['states'][-1]['screen']=22;self.refuses(args)
        args=self.args();args[0]['flow']['world_visits']=1;self.refuses(args)
    def test_exit_does_not_accept_held_jobs_or_process_exit_alone(self):
        for key,value in [('verified',False),('producer_stopped',False),('normal_quit',False),('pending_jobs',1),('graphics_owners',1)]:
            args=self.args();args[4][key]=value
            with self.subTest(key=key):self.refuses(args)
    def test_no_fallback_or_abnormal_original_completion(self):
        for target,path,value in [(0,('private_wine_stopped',),False),(0,('retirement','forwarded'),1),(0,('flow','native_fallback'),True),(0,('flow','native_error'),'failure'),(1,('success',),False)]:
            args=self.args();where=args[target]
            for key in path[:-1]:where=where[key]
            where[path[-1]]=value
            with self.subTest(path=path):self.refuses(args)
    def test_uncompared_or_mismatching_last_world_never_passes(self):
        for field,value in [('world',0),('mismatching_pixels',1),('actual_sha256','0'*64)]:
            args=self.args();paint=int(self.execution['proof']['last_swapped_world']['paint'])
            for row in args[1]['comparison']['comparisons']:
                if row['paint']==paint:row[field]=value
            with self.subTest(field=field):self.refuses(args)
    def test_real_swap_multiplicity_and_command_end_are_required(self):
        args=self.args();args[2]['swaps'].append(args[2]['swaps'][-1]);self.refuses(args)
        args=self.args();args[2]['command_rows'][-2][0]['commands']+=1;self.refuses(args)
    def test_source_and_presentation_cannot_retire_before_the_last_paint(self):
        args=self.args();gen=int(self.execution['proof']['last_swapped_world']['world']);args[3]['imports'][gen-1]['deleted_sequence']=self.execution['proof']['last_swapped_world']['commands'];self.refuses(args)
        args=self.args();args[2]['paints'][int(self.execution['proof']['last_swapped_world']['paint'])][0]['native_gpu_frame']=False;self.refuses(args)
    def test_last_import_may_be_coalesced_but_every_import_must_retire(self):
        for v in self.report['executions']:
            p=v['proof'];self.assertLessEqual(int(p['last_swapped_world']['world']),p['source_imports'])
            for row in p['import_incarnations']:self.assertLess(row['import_sequence'],row['deleted_sequence']);self.assertLess(row['deleted_sequence'],p['command_end'])
    def test_only_world_exit_rows_change_and_all_eight_domains_are_required(self):
        binding=self.reader.json(self.report['accepted_parent']['binding']['record']);parent=self.reader.json(binding['inputs']['progress']['record']);before=copy.deepcopy(parent)
        plan=self.reader.json('docs/native-rendering-closure.json');runs=self.report['executions'];rows=exit.close_rows(plan,parent,runs)
        self.assertEqual(parent,before);self.assertEqual(sum(v['state']=='passed' for v in rows),36)
        for a,b in zip(parent['requirements'],rows):
            if a['id']!=exit.LEAF:self.assertEqual(a,b)
        for bad in [runs[:-1],runs[:-1]+[runs[0]],runs+[runs[0]]]:
            with self.assertRaises(ValueError):exit.close_rows(plan,parent,bad)


if __name__=='__main__':unittest.main()
