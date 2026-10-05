#!/usr/bin/env python3
"""Synthetic import checks: no original artifacts, game run or user configuration."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('store',ROOT/'tools/menu_preferences_store.py');store=importlib.util.module_from_spec(spec);spec.loader.exec_module(store)
def module(name,path):
    s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
BASE=b'[VIDEO]\r\nIsHighRes=TRUE ; keep\r\nCutDownAnims=FALSE\r\nDialogSpeed=1\r\nMaxFramesPerSec=20\r\nWindowSize=0\r\nTerrainLightLevels=64\r\nPlayFMV=FALSE\r\n[SOUND]\r\nMusicVolume=9\r\nSFXVolume=-250\r\nCDMusicEnabled=FALSE\r\n'
class StoreChecks(unittest.TestCase):
    def test_schema_and_bounds(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'store.json';self.assertEqual(store.load(p)['revision'],'missing')
            for values in ([9,-500,0,0,2,1,1],[15,-2500,1,1,0,2,0]):
                p.write_text(json.dumps(dict(schema=1,source_sha256=store.BUILD,values=values)));self.assertEqual(store.load(p)['values'],values)
            for values in ([9,-2501,0,0,0,0,0],[True,-500,0,0,0,0,0],[9,-500,0,0,0,0,2],[9.0,-500,0,0,0,0,0]):
                p.write_text(json.dumps(dict(schema=1,source_sha256=store.BUILD,values=values)));self.assertIsNone(store.load(p)['values'])
            for raw in ('{','{"schema":1,"schema":1}',json.dumps(dict(schema=1,source_sha256='wrong',values=[9,-500,0,0,0,0,0]))):
                p.write_text(raw);before=p.read_bytes();self.assertIsNone(store.load(p)['values']);self.assertEqual(p.read_bytes(),before)
            p.write_bytes(b'x'*65537);self.assertEqual(store.load(p)['revision'],'oversized')
    def test_overlay_preserves_unknowns_and_comments(self):
        out=store.overlay(BASE,[15,-2500,1,1,2,2,1]);self.assertIn(b'IsHighRes=FALSE ; keep\r\n',out);self.assertIn(b'MaxFramesPerSec=14\r\n',out)
        for unchanged in (b'TerrainLightLevels=64',b'PlayFMV=FALSE',b'CDMusicEnabled=FALSE'):self.assertIn(unchanged,out)
        for text in (BASE+b'SFXVolume=-10\r\n',BASE.replace(b'MusicVolume=9\r\n',b'')):
            with self.assertRaises(ValueError):store.overlay(text,[0,-500,0,0,0,0,0])
    def test_both_cfg_copies_and_preflight(self):
        encoder=module('encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(ROOT)
        with tempfile.TemporaryDirectory() as d:
            game=Path(d)/'game';(game/'CFG/Encrypted').mkdir(parents=True);plain=game/'CFG/prefs.cfg';packed=game/'CFG/Encrypted/prefs.cfg';p=Path(d)/'store.json'
            plain.write_bytes(BASE);packed.write_bytes(encoder.encode(BASE,decoder)[0]);values=[9,-500,0,0,2,1,1];p.write_text(json.dumps(dict(schema=1,source_sha256=store.BUILD,values=values)))
            snapshot=store.stage(game,p,encoder,decoder);self.assertEqual(snapshot['values'],values);self.assertEqual(plain.read_bytes(),decoder.decode(packed.read_bytes())[1]);self.assertEqual(plain.read_bytes(),store.overlay(BASE,values))
            plain.write_bytes(BASE);packed.write_bytes(encoder.encode(BASE+b'SFXVolume=-10\r\n',decoder)[0]);before=(plain.read_bytes(),packed.read_bytes())
            with self.assertRaises(ValueError):store.stage(game,p,encoder,decoder)
            self.assertEqual((plain.read_bytes(),packed.read_bytes()),before)
if __name__=='__main__':unittest.main()
