#!/usr/bin/env python3
"""Guards for external native menu observations and finite acceptance joins."""
import copy,importlib.util,json,os,struct,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('public_menu_driver',ROOT/'tools/run-rendering-menu-acceptance.py');driver=importlib.util.module_from_spec(spec);spec.loader.exec_module(driver)

class OriginalMenuReadTests(unittest.TestCase):
    def wire(self,version=22,size=524288,screen=14):
        raw=bytearray(size);raw[:8]=(('MNMMCMD' if version<10 else 'MNMMCM')+str(version)).encode()
        struct.pack_into('<II',raw,8,version,size);struct.pack_into('<8I',raw,128,2,1,screen,1,0,0,123,0)
        struct.pack_into('<II',raw,160,1,2);struct.pack_into('<13I',raw,168,*range(13))
        for i in range(4):struct.pack_into('<4I',raw,220+i*48,1,0,i,10)
        raw[412:416]=b'One\0';raw[540:544]=b'One\0';raw[668:672]=b'Two\0';return raw
    def read(self,raw):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'channel.bin';p.write_bytes(raw);return driver.state(p)
    def test_complete_large_public_and_v2_setup_snapshots(self):
        for version,size in [(2,32768),(22,524288)]:
            row,raw=self.read(self.wire(version,size));self.assertEqual(row['setup']['rules'],list(range(13)));self.assertEqual(row['maps'],['One','Two']);self.assertEqual(len(raw),size)
    def test_odd_engine_lane_is_not_an_observation(self):
        raw=self.wire();struct.pack_into('<I',raw,128,3);self.assertIsNone(self.read(raw))
    def test_bad_extent_version_and_ready_refuse(self):
        for offset,value in [(8,23),(12,32768),(140,2)]:
            raw=self.wire();struct.pack_into('<I',raw,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):self.read(raw)
    def test_map_inventory_and_unterminated_names_refuse(self):
        raw=self.wire();struct.pack_into('<I',raw,164,129)
        with self.assertRaises(ValueError):self.read(raw)
        raw=self.wire();raw[412:540]=b'x'*128
        with self.assertRaises(ValueError):self.read(raw)
    def test_preferences_original_resolution_context(self):
        raw=self.wire(screen=10);struct.pack_into('<7i',raw,44724,10,-2000,1,0,1,1,1)
        row,_=self.read(raw);self.assertEqual(row['preferences'][2],1)
    def test_missing_uninitialized_or_aliased_channel(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'channel.bin';self.assertIsNone(driver.state(p));p.write_bytes(bytes(524288));self.assertIsNone(driver.state(p))
            link=Path(d)/'alias';link.symlink_to(p)
            with self.assertRaises(ValueError):driver.state(link)




@unittest.skipUnless(os.environ.get('MNM_MENU_ACCEPTANCE_CASES'),'Live case references are supplied for evidence-backed controls')
class RecordedMenuProofTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        from coverage_evidence import Reader
        import rendering_menu_acceptance as menu
        cls.reader=Reader(ROOT);cls.menu=menu
        selected=cls.reader.json(os.environ['MNM_MENU_ACCEPTANCE_CASES'])
        names=[v['record'] for v in selected['cases']] if selected['kind']=='native_public_menu_case_selection' else [os.environ['MNM_MENU_ACCEPTANCE_CASES']]
        cls.cases=[cls.reader.json(n) for n in names];cls.names=names
        spec=importlib.util.spec_from_file_location('case_callbacks',ROOT/'tools/test-menu-observer.py');decoder=importlib.util.module_from_spec(spec);spec.loader.exec_module(decoder)
        cls.calls=[[[v['menu_id'],v['argument']] for v in decoder.decode(menu.artifact(cls.reader,a,'events.bin')) if v['event']==3] for a in cls.cases]
    @classmethod
    def tearDownClass(cls):cls.reader.close()
    def case(self):return copy.deepcopy(self.cases[0])
    def refuses(self,a,calls=None):
        with self.assertRaises(ValueError):self.menu.assess(a,self.calls[0] if calls is None else calls)
    def test_actual_positive_cases_bind_raw_channels_images_and_original_callbacks(self):
        for a,c in zip(self.cases,self.calls):self.assertTrue(self.menu.validate_artifacts(self.reader,a,c)['verified'])
    def test_failed_forced_or_non_normal_completion_never_passes(self):
        for key,value in [('success',False),('sources_stable',False),('private_wine_stopped',False),('original_game_runs',0),('exit_code',1),('forced_private_prefix_stop',True)]:
            a=self.case();a[key]=value
            with self.subTest(key=key):self.refuses(a)
    def test_original_owner_ack_generation_and_actual_callbacks_are_required(self):
        for key,value in [('thread',0),('ack',0),('generation',0),('ready',False),('handoff',1)]:
            a=self.case();a['states'][3][key]=value
            with self.subTest(key=key):self.refuses(a)
        calls=copy.deepcopy(self.calls[0]);calls[4]=[14,1];self.refuses(self.case(),calls)
    def test_original_high_low_context_and_typed_flags_are_required(self):
        a=self.case();a['states'][1]['preferences'][2]=1-a['states'][1]['preferences'][2];self.refuses(a)
        a=self.case();a['configuration']['software_flag']=0;self.refuses(a)
    def test_ready_main_focus_and_actual_host_identity_are_required(self):
        for field,value in [('focused',False),('enabled',False),('showing',False)]:
            a=self.case();shot=next(v for v in a['screenshots'] if v['label']=='main-ready-returned')
            next(v for v in shot['nodes'] if v.get('accessible_id','').endswith('.mainMenuAction2'))[field]=value
            with self.subTest(field=field):self.refuses(a)
        a=self.case();a['screenshots'][0]['host_focus']['window']+=1;self.refuses(a)
    def test_cancel_return_control_focus_is_not_inferred_from_main_focus(self):
        a=self.case();shot=next(v for v in a['screenshots'] if v['label']=='map-cancel-preserved')
        next(v for v in shot['nodes'] if v.get('accessible_id','').endswith('.singlePlayerPortrait0'))['focused']=False;self.refuses(a)
    def test_changed_draft_and_original_applied_rule_effects_are_required(self):
        a=self.case();a['proof']['cancelled_draft'][0]['after']=a['proof']['cancelled_draft'][0]['before'];self.refuses(a)
        a=self.case();a['states'][7]['setup']['rules'][0]+=1;self.refuses(a)
    def test_map_cancel_keeps_complete_current_setup(self):
        for field in ('map','portrait'):
            a=self.case()
            if field=='map':a['states'][8]['setup']['map']+=1
            else:a['states'][8]['setup']['players'][1]['portrait']+=1
            with self.subTest(field=field):self.refuses(a)
    def test_outer_cancel_restores_entry_defaults_not_nested_confirmation(self):
        a=self.case();a['states'][12]['setup']=copy.deepcopy(a['states'][10]['setup']);self.refuses(a)
    def test_original_fresh_ai_cosmetics_are_allowed_but_settings_are_not(self):
        a=self.case();a['states'][6]['setup']['players'][1]['name']='different original fresh AI';a['states'][6]['setup']['players'][1]['portrait']^=1
        self.assertTrue(self.menu.assess(a,self.calls[0])['verified'])
        for field in ('active','colour','handicap'):
            a=self.case();a['states'][6]['setup']['players'][1][field]+=1
            with self.subTest(field=field):self.refuses(a)
    def test_unintended_worlds_and_surviving_resources_refuse(self):
        a=self.case();a['resident']['imports']='1';self.refuses(a)
        a=self.case();a['retirement']['graphics_retirement']['counts']['surfaces']=1;self.refuses(a)
    def test_raw_snapshot_tampering_is_not_masked_by_success_flags(self):
        a=self.case();target=next(v for v in a['artifacts'] if v['path']==a['states'][0]['wire_path'])['record'];reader=self.reader
        class Changed:
            def read_bytes(self,name):
                raw=reader.read_bytes(name)
                return bytes([raw[0]^1])+raw[1:] if name==target else raw
        with self.assertRaises(ValueError):self.menu.validate_artifacts(Changed(),a,self.calls[0])
        # Actual retained cases supply the metadata; erase only their Main images
        # in memory to prove accessible SHOWING alone cannot satisfy visual output.
        import io,hashlib
        from PIL import Image
        a=self.case();buffer=io.BytesIO();Image.new('RGB',(1280,1024)).save(buffer,format='PNG');blank=buffer.getvalue();changed={}
        for shot in a['screenshots']:
            if not shot['label'].startswith('main-ready-'):continue
            entry=next(v for v in a['artifacts'] if v['path']==shot['path']);changed[entry['record']]=blank
            shot['sha256']=entry['sha256']=hashlib.sha256(blank).hexdigest();entry['bytes']=len(blank)
        class BlankMain:
            def read_bytes(self,name):return changed[name] if name in changed else reader.read_bytes(name)
        with self.assertRaisesRegex(ValueError,'No retained painted ready Main'):
            self.menu.validate_artifacts(BlankMain(),a,self.calls[0])
    def test_all_four_domains_and_exact_parent_rows_are_required(self):
        if len(self.cases)!=4:self.skipTest('Four-case join is checked after the missing domains finish')
        binding=self.reader.json('research/runtime/rendering-loading-handoff-results-20261105/current-closeout-binding-01.json');parent=self.reader.json(binding['inputs']['progress']['record'])
        plan=self.reader.json('docs/native-rendering-closure.json');before=copy.deepcopy(parent)
        runs=[dict(identity=a['session'],configuration=a['configuration'],acquisition=dict(record=n,sha256=self.reader.digest(n)),proof=self.menu.assess(a,c)) for a,c,n in zip(self.cases,self.calls,self.names)]
        rows=self.menu.close_rows(plan,parent,runs);self.assertEqual(parent,before);self.assertEqual(sum(v['state']=='passed' for v in rows),34)
        for a,b in zip(parent['requirements'],rows):
            if a['id'] not in self.menu.LEAVES:self.assertEqual(a,b)
        for bad in [runs[:-1],runs[:-1]+[runs[0]],runs+[runs[0]]]:
            with self.assertRaises(ValueError):self.menu.close_rows(plan,parent,bad)
        passed=next(v for v in parent['requirements'] if v['state']=='passed');pending=next(v for v in parent['requirements'] if v['state']=='pending');passed['state']='pending';pending['state']='passed'
        with self.assertRaises(ValueError):self.menu.close_rows(plan,parent,runs)

if __name__=='__main__':unittest.main()
