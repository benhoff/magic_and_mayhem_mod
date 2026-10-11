#!/usr/bin/env python3
"""Independent session admission, temporal results and backend oracle isolation."""
import copy
import gzip
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import drawing_session as session
from drawing_corpus import sha,BUILD

class SessionTest(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        (self.root/'fixtures').mkdir();(self.root/'research/runtime/coverage').mkdir(parents=True)
        self.planes=[b'\x34\x12'*4,b'\x67\x45'*4]
        def pack(name,data):
            packed=gzip.compress(data,mtime=0);path='fixtures/'+name+'.gz';(self.root/path).write_bytes(packed)
            return {'path':path,'encoding':'gzip','sha256':sha(packed),'raw_sha256':sha(data),'historical_path':'capture/'+name+'.bin'}
        owned=pack('state',b'owned drawing state, independent of presented pixels')
        oracles=[pack('frame-'+str(i),b) for i,b in enumerate(self.planes)]
        self.catalog={'schema':1,'id':'camera-session','build':'nocd','source_sha256':BUILD,'evidence':'original',
            'historical_report_sha256':'','session_identity':'session-1','scenario':'camera-motion','requirements':['pan'],
            'operations':['flags-and-return-values'],'scope':'Synthetic collector contract for guard tests only.','remaining':[],
            'presentation':{'boundary':'presented_frame','first_index':10,'last_index':11,'dropped_frames':0,'sampling_interval':1},
            'actions':[{'index':0,'action':'start','before_presentation':10},{'index':1,'action':'pan-right','before_presentation':11}],
            'inputs':[{'id':'state','role':'owned-state','artifact':owned}],
            'frames':[{'identity':{'session':'session-1','presentation_index':10+i,'width':2,'height':2,'pixel_format':'rgb565','actions_completed':1+i},'input_ids':['state'],'oracle':o} for i,o in enumerate(oracles)],
            'effects':[{'id':'result-1','operation':'flags-and-return-values','presentation_index':11,'result':{'return':0,'last_error':0}}]}
        self.original={'success':True,'source_executable_sha256':BUILD,'original_manifest_verified_before_after':True,
            'original_pixels_used_as_native_inputs':False,'drawing_session':{},'inputs':{e['historical_path']:e['raw_sha256'] for e in [owned,*oracles]}}
        self.register={'builds':[{'id':'nocd','source_sha256':BUILD}],'evidence':[{'id':'original','kind':'isolated_original','build':'nocd',
            'record':'original-report.json','sha256':'','original_hash_pointer':'/source_executable_sha256','assertions':[{'pointer':'/success','equals':True}]}]}
        self.refresh()
        self.executable=self.root/'backend';self.executable.write_bytes(b'pinned test backend')

    def refresh(self):
        self.original['drawing_session']={k:copy.deepcopy(self.catalog[k]) for k in ('id','session_identity','scenario','requirements','operations','presentation','actions','frames','effects','inputs','scope','remaining')}
        data=(json.dumps(self.original,indent=2)+'\n').encode();(self.root/'original-report.json').write_bytes(data)
        self.catalog['historical_report_sha256']=sha(data);self.register['evidence'][0]['sha256']=sha(data)
        (self.root/'catalog.json').write_text(json.dumps(self.catalog))
        (self.root/'research/runtime/coverage/register.json').write_text(json.dumps(self.register))

    def test_independent_admission(self):
        inputs,frames,effects=session.admit(self.root,self.catalog,self.register)
        self.assertEqual(len(inputs),1);self.assertEqual(len(frames),2);self.assertEqual(len(effects),1)

    def test_original_kind_and_manifest_guards(self):
        self.register['evidence'][0]['kind']='native_integration'
        with self.assertRaisesRegex(ValueError,'original execution'):session.admit(self.root,self.catalog,self.register)
        self.register['evidence'][0]['kind']='isolated_original';self.original['original_manifest_verified_before_after']=False;self.refresh()
        with self.assertRaisesRegex(ValueError,'manifest'):session.admit(self.root,self.catalog,self.register)

    def test_scope_cannot_be_relabelled(self):
        c=copy.deepcopy(self.catalog);c['requirements'].append('four-orientations')
        with self.assertRaisesRegex(ValueError,'differs from original'):session.admit(self.root,c,self.register)

    def test_discontinuous_history_refuses(self):
        for key,value in (('sampling_interval',2),('dropped_frames',1),('last_index',12),('boundary','composed_canvas')):
            original=copy.deepcopy(self.catalog);self.catalog['presentation'][key]=value;self.refresh()
            with self.assertRaisesRegex(ValueError,'partial or discontinuous'):session.admit(self.root,self.catalog,self.register)
            self.catalog=original

    def test_frame_order_and_action_binding_refuse(self):
        self.catalog['frames'].reverse();self.refresh()
        with self.assertRaisesRegex(ValueError,'frame/extent'):session.admit(self.root,self.catalog,self.register)
        self.catalog['frames'].reverse();self.catalog['frames'][1]['identity']['actions_completed']=1;self.refresh()
        with self.assertRaisesRegex(ValueError,'action/input'):session.admit(self.root,self.catalog,self.register)

    def test_unknown_inputs_and_effect_omission_refuse(self):
        self.catalog['frames'][0]['input_ids']=['missing'];self.refresh()
        with self.assertRaisesRegex(ValueError,'action/input'):session.admit(self.root,self.catalog,self.register)
        self.catalog['frames'][0]['input_ids']=['state'];self.catalog['effects']=[];self.refresh()
        with self.assertRaisesRegex(ValueError,'caller-visible result'):session.admit(self.root,self.catalog,self.register)

    def test_partial_or_changed_oracle_refuses(self):
        self.catalog['frames'][0]['identity']['width']=1;self.refresh()
        with self.assertRaisesRegex(ValueError,'oracle provenance/extent'):session.admit(self.root,self.catalog,self.register)

    def test_extra_action_fields_cannot_leak_oracle_metadata(self):
        self.catalog['actions'][0]['oracle_path']='fixtures/frame-0.gz';self.refresh()
        with self.assertRaisesRegex(ValueError,'action fields'):session.admit(self.root,self.catalog,self.register)

    def test_extra_frame_fields_cannot_leak_expected_hashes(self):
        self.catalog['frames'][0]['identity']['oracle_sha256']=self.catalog['frames'][0]['oracle']['raw_sha256'];self.refresh()
        with self.assertRaisesRegex(ValueError,'identity fields'):session.admit(self.root,self.catalog,self.register)

    def execute(self,command,**kwargs):
        inputs,output=Path(command[-2]),Path(command[-1]);declaration=json.loads((inputs/'session.json').read_text())
        self.assertNotIn('oracle',json.dumps(declaration));self.assertNotIn('raw_sha256',json.dumps(declaration))
        self.assertNotIn('result',declaration['effects'][0])
        self.assertEqual(sorted(p.name for p in inputs.iterdir()),['session.json','state.bin'])
        output.mkdir()
        frames=[]
        for i,(entry,pixels) in enumerate(zip(declaration['frames'],self.planes)):
            (output/f'frame-{i:06}.bin').write_bytes(pixels);frames.append({'identity':entry['identity'],'sha256':sha(pixels)})
        native={'success':True,'session_identity':'session-1','remaining_surfaces':0,'original_pixels_used_as_native_inputs':False,
                'original_oracles_read':False,'frames':frames,'effects':self.catalog['effects']}
        if getattr(self,'wrong_result',False):native['effects']=copy.deepcopy(native['effects']);native['effects'][0]['result']['return']=-1
        if getattr(self,'bool_result',False):native['effects']=copy.deepcopy(native['effects']);native['effects'][0]['result']['return']=False
        if getattr(self,'bool_frame',False):native['frames'][0]['identity']=copy.deepcopy(native['frames'][0]['identity']);native['frames'][0]['identity']['actions_completed']=True
        if getattr(self,'reorder',False):native['frames'].reverse()
        (output/'session-report.json').write_text(json.dumps(native))
        return subprocess.CompletedProcess(command,0,b'',b'')

    def test_backend_only_receives_owned_inputs(self):
        with patch.object(session.subprocess,'run',side_effect=self.execute):
            result=session.check(root=self.root,catalog_path='catalog.json',executable=self.executable)
        self.assertTrue(result['success']);self.assertEqual(result['pixels_compared'],8)

    def test_pixel_equality_cannot_hide_wrong_return_value(self):
        self.wrong_result=True
        with patch.object(session.subprocess,'run',side_effect=self.execute):
            result=session.check(root=self.root,catalog_path='catalog.json',executable=self.executable)
        self.assertFalse(result['success']);self.assertEqual(result['mismatches'],0);self.assertFalse(result['effects_matching'])

    def test_equal_python_values_do_not_hide_wrong_result_types(self):
        self.bool_result=True
        with patch.object(session.subprocess,'run',side_effect=self.execute):
            result=session.check(root=self.root,catalog_path='catalog.json',executable=self.executable)
        self.assertFalse(result['effects_matching']);self.assertFalse(result['success'])

    def test_native_frame_identity_types_must_match(self):
        self.bool_frame=True
        with patch.object(session.subprocess,'run',side_effect=self.execute),self.assertRaisesRegex(ValueError,'reordered frame'):
            session.check(root=self.root,catalog_path='catalog.json',executable=self.executable)

    def test_backend_frame_reordering_refuses(self):
        self.reorder=True
        with patch.object(session.subprocess,'run',side_effect=self.execute),self.assertRaisesRegex(ValueError,'reordered frame'):
            session.check(root=self.root,catalog_path='catalog.json',executable=self.executable)

    def test_unsupported_retains_reference(self):
        with patch.object(session.subprocess,'run',return_value=subprocess.CompletedProcess([],64,b'',b'')):
            result=session.check(root=self.root,catalog_path='catalog.json',executable=self.executable)
        self.assertEqual(result['outcome'],'unsupported');self.assertTrue(result['original_reference_admitted']);self.assertFalse(result['success'])

    def test_portable_retention_and_no_overwrite(self):
        from drawing_corpus import load_tool,artifact
        import shutil
        retain=load_tool('retain-drawing-session')
        for e in [i['artifact'] for i in self.catalog['inputs']]+[f['oracle'] for f in self.catalog['frames']]:
            path=self.root/e['historical_path'];path.parent.mkdir(parents=True,exist_ok=True)
            path.write_bytes(artifact(self.root,e))
        destination=self.root/'tests/fixtures/new-session'
        with patch.object(retain,'ROOT',self.root):
            catalog=retain.retain(self.root,'original',destination)
            with self.assertRaisesRegex(ValueError,'new directory'):retain.retain(self.root,'original',destination)
        shutil.rmtree(self.root/'capture');shutil.rmtree(self.root/'fixtures')
        inputs,frames,_=session.admit(self.root,catalog,self.register)
        self.assertEqual(len(inputs),1);self.assertEqual(len(frames),2)

    def test_full_pixel_formats_and_border_diagnostics(self):
        for fmt,channels in session.FORMATS.items():
            expected=bytes(4*channels);actual=bytearray(expected);actual[-1]=1
            result=session.plane_compare(bytes(actual),expected,2,2,fmt)
            self.assertEqual(result['bounds'],[1,1,1,1]);self.assertEqual(result['mismatching_pixels'],1)
            with self.assertRaisesRegex(ValueError,'entire'):session.plane_compare(bytes(actual[:-1]),expected,2,2,fmt)

if __name__=='__main__':unittest.main()
