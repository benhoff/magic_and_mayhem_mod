"""Native-only launch conflicts, staged closure and positive shutdown accounting."""
import copy
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import native_only_contract as contract
from native_output_owner import REQUIRED
from native_pixel_counters import ROUTES


class Contract(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.repo=Path(self.temp.name);self.root=self.repo/'session';self.root.mkdir()
        (self.root/'game').mkdir()
        self.env=contract.configure({},[])
        self.metadata=dict(origin='menu_action_bridge',native_only_policy=contract.POLICY,
            glyph_backend_mode='takeover',glyph_capture=False,native_media=True,native_draw_startup_retirement=True,
            **{key:True for key in REQUIRED})
        self.source=self.repo/'runtime/native-draw/native_only.h';self.source.parent.mkdir(parents=True)
        self.source.write_text('source')
        sources={'runtime/native-draw/native_only.h':hashlib.sha256(b'source').hexdigest()}
        startup=self.source.with_name('startup_retirement.h');startup.write_text('startup')
        sources['runtime/native-draw/startup_retirement.h']=hashlib.sha256(b'startup').hexdigest()
        for name,(dll,key) in contract.BUILDS.items():
            (self.root/'game'/dll).write_bytes(dll.encode());digest=hashlib.sha256(dll.encode()).hexdigest()
            self.metadata[key]=digest
            self.write(name+'-build.json',dict(sha256=digest,sources=sources,directdraw_imports=False))

    def write(self,name,value):
        (self.root/name).write_text(json.dumps(value))

    def shutdown(self):
        self.write('native-output-admission.json',dict(session=str(self.root),native_only=dict(policy=contract.POLICY)))
        self.write('native-only-session.json',dict(schema=1,policy=contract.POLICY,session=str(self.root),state='completed',normal_quit=True,fallback=False,error='',launcher_exit=0,renderer_retired=True,restart_requested=False,force_stop_verified=False))
        self.write('original-process-result.json',dict(original_exit_code=0))
        self.write('resident-world-report.json',dict(completed=3,imports=3,verified_returns=3,pending=0,error='',normal_quit_retired=True))
        (self.root/'native-draw-retirement.bin').write_bytes(struct.pack('<16I',
            0x444d4e4d,0x31544552,1,64,1,0,0,0,0,0,0,2,69,0,0,0))
        (self.root/'game/native-draw-startup-retirement.bin').write_bytes(struct.pack('<16I',
            0x444d4e4d,0x31425250,1,64,0xbdb6f,2,0,1,1,1,0,1,0,1,0,0))
        values=[0x50504e4d,2,(4+6*32*3)*4,6]
        for routes in ROUTES.values():
            for i in range(32):values.extend([1,int(i==0),0] if i<len(routes) else [0,0,0])
        (self.root/'game/native-pixel-producers.bin').write_bytes(struct.pack('<'+'I'*len(values),*values))

    def test_startup_selects_all_required_capabilities(self):
        self.assertEqual(contract.configure({},['--skip-movies','--live-menu-test','file']),self.env)
        for key,value in contract.REQUIRED_ENV.items():
            for wrong in ('0','', 'wrong'):
                with self.subTest(key=key,wrong=wrong),self.assertRaises(ValueError):contract.configure({key:wrong},[])
        for key in contract.FORBIDDEN_ENV:
            with self.assertRaises(ValueError):contract.configure({key:'1'},[])

    def test_cli_rejects_early_return_and_override_modes(self):
        for arg in (*contract.FORBIDDEN_ARGS,'--media','--main-menu','--skip-movies=0','--unknown'):
            for form in (arg,arg+'=value'):
                with self.subTest(form=form),self.assertRaises(ValueError):contract.configure({},[form])
        for args in (['--live-menu-test'],['--live-menu-test','--skip-movies'],['--music-assets=']):
            with self.assertRaises(ValueError):contract.configure({},args)
        contract.configure({},['--live-menu-test=file','--music-assets','music'])

    def test_staged_closure_admits_and_changed_binary_or_source_refuses(self):
        result=contract.admit(self.root,self.repo,self.metadata,self.env)
        self.assertEqual(len(result['binaries']),10)
        self.assertFalse(result['complete_replacement'])
        self.source.write_text('changed')
        with self.assertRaises(ValueError):contract.admit(self.root,self.repo,self.metadata,self.env)
        self.source.write_text('source');(self.root/'game/MnmRender.dll').write_bytes(b'changed')
        with self.assertRaises(ValueError):contract.admit(self.root,self.repo,self.metadata,self.env)

    def test_legacy_basename_and_full_compiler_receipts_are_both_checked(self):
        for name,prefix in [('bridge','runtime/menu'),('render-bridge','runtime/render')]:
            p=self.repo/prefix/'entry.h';p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b'entry')
            manifest=json.loads((self.root/(name+'-build.json')).read_text())
            digest=hashlib.sha256(b'entry').hexdigest();manifest['sources']={'entry.h':digest}
            manifest['compiled_sources']={prefix+'/entry.h':digest}
            self.write(name+'-build.json',manifest)
        contract.admit(self.root,self.repo,self.metadata,self.env)
        (self.repo/'runtime/menu/entry.h').write_bytes(b'changed')
        with self.assertRaises(ValueError):contract.admit(self.root,self.repo,self.metadata,self.env)

    def test_missing_admission_environment_refuses_instead_of_defaulting(self):
        for key in contract.REQUIRED_ENV:
            environment=dict(self.env);environment.pop(key)
            with self.subTest(key=key),self.assertRaises(ValueError):contract.admit(self.root,self.repo,self.metadata,environment)

    def test_each_missing_policy_and_glyph_movie_capability_refuses(self):
        for key in (*REQUIRED,'native_only_policy','glyph_backend_mode','glyph_capture','native_media'):
            meta=dict(self.metadata);meta.pop(key)
            with self.subTest(key=key),self.assertRaises(ValueError):contract.admit(self.root,self.repo,meta,self.env)
        meta=dict(self.metadata,native_media=False,skip_movies=True)
        contract.admit(self.root,self.repo,meta,self.env)

    def test_matching_shutdown_and_menu_only_zero_traffic_are_admitted(self):
        self.shutdown();self.assertEqual(contract.outcome(self.root)['worlds'],3)
        self.write('resident-world-report.json',dict(completed=0,imports=0,verified_returns=0,pending=0,error='',normal_quit_retired=True))
        self.assertEqual(contract.outcome(self.root)['worlds'],0)

    def test_real_qt_uint64_retirement_form_is_exact_not_float_coercion(self):
        self.shutdown()
        world=dict(completed='9007199254740993',imports='9007199254740993',verified_returns='9007199254740993',pending=False,error='',normal_quit_retired=True)
        self.write('resident-world-report.json',world)
        self.assertEqual(contract.outcome(self.root)['worlds'],9007199254740993)
        for value in (True,False,1.0,-1,'01','-1','1.0','',str(1<<64)):
            self.write('resident-world-report.json',dict(world,completed=value))
            with self.subTest(value=value),self.assertRaises(ValueError):contract.outcome(self.root)
        self.write('resident-world-report.json',dict(world,imports='9007199254740992'))
        with self.assertRaises(ValueError):contract.outcome(self.root)

    def test_normal_exit_and_balanced_world_cannot_hide_graphics_owners(self):
        self.shutdown()
        header=[0x444d4e4d,0x31544552,1,64,1,1,1,0,0,0,960000,2,69,0,0,0]
        rows=[1,0x1234]+[0]*14+[2,0x5678]+[0]*14
        receipt=self.root/'native-draw-retirement.bin'
        receipt.write_bytes(struct.pack('<48I',*(header+rows)))
        with self.assertRaisesRegex(ValueError,'survived normal Quit'):contract.outcome(self.root)
        receipt.unlink()
        with self.assertRaises(ValueError):contract.outcome(self.root)

    def test_abnormal_shutdown_or_cross_session_receipt_never_succeeds(self):
        self.shutdown()
        cases={'native-only-session.json':{'state':'failed','normal_quit':False,'fallback':True,'launcher_exit':1,'session':'/foreign'},
               'resident-world-report.json':{'imports':2,'pending':1,'error':'failure','normal_quit_retired':False},
               'original-process-result.json':{'original_exit_code':124},
               'native-output-admission.json':{'session':'/foreign','native_only':{}}}
        for name,mutations in cases.items():
            original=json.loads((self.root/name).read_text())
            for key,value in mutations.items():
                self.write(name,dict(original,**{key:value}))
                with self.subTest(name=name,key=key),self.assertRaises(ValueError):contract.outcome(self.root)
            self.write(name,original)
        for name in contract.REFUSALS:
            p=self.root/'game'/name;p.write_bytes(b'refusal')
            with self.subTest(name=name),self.assertRaises(ValueError):contract.outcome(self.root)
            p.unlink()
        p=self.root/'game/native-pixel-producers.bin';raw=bytearray(p.read_bytes());struct.pack_into('<I',raw,24,1);p.write_bytes(raw)
        with self.assertRaises(ValueError):contract.outcome(self.root)

    def test_completion_requires_typed_current_schema_and_renderer_retirement(self):
        self.shutdown()
        path=self.root/'native-only-session.json';original=json.loads(path.read_text())
        for key,values in {'schema':[None,True,1.0,2],
                           'launcher_exit':[False,0.0,'0'],
                           'renderer_retired':[None,False,1],
                           'restart_requested':[None,True,0],
                           'force_stop_verified':[None,True,0]}.items():
            for value in values:
                self.write(path.name,dict(original,**{key:value}))
                with self.subTest(key=key,value=value),self.assertRaises(ValueError):contract.outcome(self.root)
        self.write(path.name,original)
        for value in [False,0.0,'0']:
            self.write('original-process-result.json',dict(original_exit_code=value))
            with self.subTest(original_exit=value),self.assertRaises(ValueError):contract.outcome(self.root)

    def test_restart_requires_dead_matching_runner_and_positive_retirement(self):
        import subprocess
        self.shutdown()
        child=subprocess.Popen(['sleep','10'])
        identity=contract.process_identity(child.pid)
        self.write('native-output-admission.json',dict(session=str(self.root),native_only=dict(policy=contract.POLICY),runner_identity=identity))
        shell=dict(schema=1,policy=contract.POLICY,session=str(self.root),state='restarted',normal_quit=False,
                   fallback=False,error='old failure',restart_requested=True,force_stop_verified=True,renderer_retired=True)
        self.write('native-only-session.json',shell)
        try:
            with self.assertRaises(ValueError):contract.outcome(self.root)
        finally:
            child.terminate();child.wait()
        result=contract.outcome(self.root);self.assertEqual(result['state'],'restarted')
        self.assertFalse(result['rendering_accepted']);self.assertEqual(result['previous_error'],'old failure')
        for key in ('restart_requested','force_stop_verified','renderer_retired'):
            self.write('native-only-session.json',dict(shell,**{key:False}))
            with self.subTest(key=key),self.assertRaises(ValueError):contract.outcome(self.root)


if __name__=='__main__':unittest.main()
