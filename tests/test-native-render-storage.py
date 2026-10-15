#!/usr/bin/env python3
"""Save/Load evidence rejects cached paints and missing original resumptions."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('storage', ROOT/'tools/campaign_storage.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class StorageTests(unittest.TestCase):
    def flow(self):
        return dict(success=True, save_load=True, stage=12, normal_quit=True,
                    resident_world_requested=True, native_command_fallback=False,
                    native_recoveries=0, native_error='', native_command_frames=120,
                    checkpoints=[dict(phase=p, source_generation=str(100+i*100),
                                      source_base=str(i*100), frames=100+i*100,
                                      frame_base=i*100, native_active=True,
                                      fallback=False, error='') for i, p in enumerate(
                                          ['before-save', 'after-save', 'after-load'])])

    def test_fresh_worlds(self):
        self.assertEqual(len(m.validate_storage_flow(self.flow())), 3)

    def test_cached_frame_and_mode_rejected(self):
        for key, value in [('source_generation', '100'), ('native_active', False), ('fallback', True)]:
            flow = self.flow()
            flow['checkpoints'][2][key] = value
            with self.assertRaises(ValueError): m.validate_storage_flow(flow)
        flow = self.flow(); flow['resident_world_requested'] = False
        with self.assertRaises(ValueError): m.validate_storage_flow(flow)

    def fixture(self, root):
        flow = self.flow(); flow['saved_name'] = 'native-render-123456abcdef'
        saved = root/'game/Save'/ (flow['saved_name']+'.sav')
        saved.parent.mkdir(parents=True); saved.write_bytes(b'original serialized fixture')
        flow['saved_sha256'] = m.hashlib.sha256(saved.read_bytes()).hexdigest()
        owned = root/'owned'; owned.mkdir(); (owned/saved.name).write_bytes(saved.read_bytes())
        (root/'manifest.json').write_text(json.dumps(dict(save_store=dict(store=str(owned)))))
        (root/'save-store-result.json').write_text('{"merged":true}')
        (root/'lock-capture').mkdir();(root/'lock-capture/lifecycle.log').write_text('resident_storage_materialized 0 17 0 1 1\nresident_storage_materialized 0 17 0 0 1\n')
        rows=[]
        def add(event, screen, argument=0):
            r=[0]*16;r[0]=len(rows)+1;r[1]=event;r[2]=23;r[3]=screen;r[6]=1;r[8]=1;r[9]=argument;rows.append(r)
        for screen, arg in [(3,0),(18,65537),(18,0),(17,1),(13,0),(17,0),(12,0),(17,3),(6,21),(3,4)]:
            add(3,screen,arg)
            if screen in (12,13):add(4,screen);add(20,2)
            if (screen,arg)==(17,3):add(22,screen,0)
        def write(): (root/'events.bin').write_bytes(b'MNMMENU1'+struct.pack('<II',1,64)+b''.join(struct.pack('<16I',*r) for r in rows))
        write()
        return flow,rows,write

    def test_original_callbacks_and_owned_save(self):
        with tempfile.TemporaryDirectory() as d:
            flow,_,_=self.fixture(Path(d))
            self.assertTrue(m.validate_storage(Path(d),flow)['verified'])

    def test_original_resume_missing_or_outside_interval(self):
        for outside in (False,True):
            with tempfile.TemporaryDirectory() as d:
                flow,rows,write=self.fixture(Path(d))
                for r in rows:
                    if r[1]==20:r[1]=0
                if outside:rows[0][1]=20;rows[0][3]=2
                write()
                with self.assertRaises(ValueError):m.validate_storage(Path(d),flow)

    def test_save_mutation_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            flow,_,_=self.fixture(Path(d));flow['saved_sha256']='0'*64
            with self.assertRaises(ValueError):m.validate_storage(Path(d),flow)


if __name__ == '__main__': unittest.main()
