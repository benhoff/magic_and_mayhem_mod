#!/usr/bin/env python3
"""Mutation controls for EP05 semantic admission; fixtures are not live evidence."""
import copy
import importlib.util
import struct
import sys
import unittest
import tempfile
import hashlib
import os
import signal
import subprocess
import shutil
import time
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from rendering_progression import validate_flow, validate_callbacks, prepare_launch_manifest
from rendering_build_binding import native_runtime_source_paths


def flow(case='progression'):
    def row(event, screen, seq, ack, **extra):
        return dict(event=event, screen=screen, sequence=seq, ack=ack, thread=40, **extra)
    states = [row('main',3,2,0), row('region-entry',18,4,1,region=1),
              row('natural-report',6,8,3,outcome=int(case!='defeat'),title='Original report',texts=['Score']*21),
              row('result-return',4,10,4), row('region-entry',18,12,5,region=2 if case!='defeat' else 1)]
    return dict(schema=2,world_observation_policy='fresh-generation-after-handoff-v1',case=case, size='800x600', success=True, normal_quit=True, native_command_fallback=False, native_error='',
                states=states, worlds=[dict(realm='Celtic',region=n,world=i+1,after_world=i,paint=i+10,frames=i+20,width=800,height=600)
                                      for i,n in enumerate([1,2] if case!='defeat' else [1,1])])


def trace(quit_first=False):
    values=[]
    def ticks():
        for counter in range(3):
            for event in (11,12):
                r=[0]*16;r[0]=len(values)+1;r[1]=event;r[2]=40;r[3]=2;r[4]=r[11]=0x6cbb78;r[5]=0x5c5dd8;r[6]=1;r[9]=counter;r[12]=4;values.append(r)
    ticks()
    for event,screen,arg,active in [(3,3,0,0),(2,6,21,0x6db967),(3,6,21,0x6db967),(28,2,0,0x659408),(3,3,4,0)]:
        if quit_first and event==3 and screen==3 and arg==0:
            values.append([len(values)+1,3,40,17,0,0,0,0,0,3,0,0,0,0,0,0])
        row=[0]*16;row[0]=len(values)+1;row[1]=event;row[2]=40;row[3]=screen;row[9]=arg;row[11]=active
        if screen==6:row[4]=0x6db967;row[5]=0x5c5ebc;row[6]=1;row[12]=5
        if event==28:row[4]=0x6cbb78;row[5]=0x5c5dd8;row[12]=3
        values.append(row)
        if event==28:ticks()
    return b'MNMMENU1'+struct.pack('<II',1,64)+b''.join(struct.pack('<16I',*r) for r in values)


class NativeRuntimeBindingTests(unittest.TestCase):
    def roots(self,root):
        for name in ('runtime','reconstruction','protocols','tools'):(root/name).mkdir()

    def test_native_inputs_are_frozen_without_captures_or_original_media(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);self.roots(root)
            for name in ('runtime/reader.h','protocols/wire.json','tools/build-draw.py',
                         'runtime/capture.bin','tools/unrelated.py','working/private.h','original/game.h'):
                path=root/name;path.parent.mkdir(exist_ok=True);path.write_text('input')
            self.assertEqual(native_runtime_source_paths(root),{'runtime/reader.h','protocols/wire.json','tools/build-draw.py'})

    def test_missing_root_and_escaping_code_refuse_before_execution(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)/'repo';root.mkdir()
            with self.assertRaises(ValueError):native_runtime_source_paths(root)
            self.roots(root);outside=Path(directory)/'outside.h';outside.write_text('code')
            (root/'runtime/escape.h').symlink_to(outside)
            with self.assertRaises(ValueError):native_runtime_source_paths(root)


class ProgressionTests(unittest.TestCase):
    def test_distinct_legitimate_routes_remain_preparation_not_pixel_acceptance(self):
        for case in ('progression','victory','defeat'):
            result=validate_flow(flow(case),case)
            self.assertTrue(result['semantic_route_verified'])
            self.assertFalse(result['pixel_history_accepted'])
            self.assertFalse(result['ownership_retirement_accepted'])
            self.assertFalse(result['supported_player_accepted'])

    def test_reload_repeated_map_wrong_outcome_quit_and_owner_drift_refuse(self):
        mutations=[lambda f:f['worlds'][1].update(width=640),
                   lambda f:f['worlds'][1].update(region=1),
                   lambda f:f['worlds'][1].update(world=1),
                   lambda f:f['states'][2].update(outcome=0),
                   lambda f:f['states'][2]['texts'].__setitem__(19,'You quit the battle.'),
                   lambda f:f['states'][3].update(ack=3),
                   lambda f:f['states'][3].update(sequence=6),
                   lambda f:f['states'][3].update(thread=44),
                   lambda f:f['states'].append(dict(event='load',thread=40)),
                   lambda f:f.update(success=False),
                   lambda f:f.update(normal_quit=False),
                   lambda f:f.update(native_command_fallback=True),
                   lambda f:f['worlds'][1].update(paint=0),
                   lambda f:f['worlds'][1].pop('after_world'),
                   lambda f:f['worlds'][1].update(after_world=2),
                   lambda f:f.update(schema=1)]
        for mutation in mutations:
            f=flow();mutation(f)
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):validate_flow(f,'progression')

    def test_defeat_cannot_reuse_victory(self):
        f=flow('defeat');f['states'][2]['outcome']=1
        with self.assertRaises(ValueError):validate_flow(f,'defeat')

    def test_original_callback_proof_required_and_quit_cannot_supply_defeat(self):
        self.assertTrue(validate_callbacks(trace(),flow())['verified'])
        for data in (trace(True), trace()[:-1], trace().replace(struct.pack('<I',0x6db967),struct.pack('<I',0x6cbb78))):
            with self.assertRaises(ValueError):validate_callbacks(data,flow())

    def test_each_world_needs_a_new_original_tick_epoch_after_result_return(self):
        rows=[list(r) for r in struct.iter_unpack('<16I',trace()[16:])]
        later=[i for i,r in enumerate(rows) if r[1] in (11,12)][6:]
        variants=[]
        variants.append([r for i,r in enumerate(rows) if i not in later])
        for field,value in [(2,41),(9,9),(11,0x659408)]:
            changed=[list(r) for r in rows];changed[later[0]][field]=value;variants.append(changed)
        for changed in variants:
            for i,r in enumerate(changed,1):r[0]=i
            data=trace()[:16]+b''.join(struct.pack('<16I',*r) for r in changed)
            with self.assertRaises(ValueError):validate_callbacks(data,flow())

    def test_realm_cleanup_preserves_parent_ack_and_original_identity(self):
        f=flow();f['states'] += [dict(event='cleanup-report',screen=6,sequence=14,ack=6,thread=40),
            dict(event='cleanup-realm',screen=4,sequence=16,ack=7,thread=40,kind=1,location=2),
            dict(event='cleanup-main',screen=3,sequence=18,ack=7,thread=40)]
        self.assertTrue(validate_flow(f,'progression')['semantic_route_verified'])
        for index,field,value in ((-2,'location',1),(-2,'kind',2),(-2,'ack',6),(-1,'ack',8),(-1,'sequence',16)):
            bad=copy.deepcopy(f);bad['states'][index][field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):validate_flow(bad,'progression')

    def test_realm_cleanup_needs_second_report_pop_mode4_modal_and_main_before_quit(self):
        f=flow();f['states'].append(dict(event='cleanup-realm',thread=40))
        rows=[list(r) for r in struct.iter_unpack('<16I',trace()[16:])];quit_row=rows.pop()
        begin=next(r for r in rows if r[1]==2 and r[3]==6);returned=next(r for r in rows if r[1]==3 and r[3]==6)
        pop=next(r for r in rows if r[1]==28)
        cleanup_start=len(rows)
        rows += [list(begin),list(returned),list(pop)]
        for modal in (0,123):
            for event,arg,result in ((13,4,5),(14,0,5),(15,modal,0)):
                rows.append([0,event,40,17,0x6a5088,0x5c6644,1,0,0,arg,result,0x6a5088,4,0,0,0])
        rows += [[0,4,40,3,0x657ce0,0x5c63e4,1,0,0,0,0,0x657ce0,2,0,0,0],quit_row]
        def encode(values):
            return trace()[:16]+b''.join(struct.pack('<16I',i,*r[1:]) for i,r in enumerate(values,1))
        self.assertIsNotNone(validate_callbacks(encode(rows),f)['original_realm_exit'])
        for mutation in ('mode','modal','main','cleanup-report'):
            bad=copy.deepcopy(rows)
            if mutation=='mode':
                for r in bad:
                    if r[1]==13:r[9]=2
            if mutation=='modal':bad=[r for r in bad if not (r[1]==15 and r[9])]
            if mutation=='main':
                for r in bad:
                    if r[1]==4 and r[3]==3:r[12]=3
            if mutation=='cleanup-report':bad.pop(cleanup_start)
            with self.subTest(mutation=mutation),self.assertRaises(ValueError):validate_callbacks(encode(bad),f)

    def test_launcher_manifest_is_materialized_for_the_real_consumer(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);game=root/'game';game.mkdir();manifest=root/'manifests/game-nocd.tsv'
            for name in ('Chaos.exe','jpeg.dll','CDROMDrive.cfg'):(game/name).write_bytes(name.encode())
            raw=prepare_launch_manifest(game,manifest)
            self.assertEqual(raw,manifest.read_bytes())
            records=[row.split('\t') for row in raw.decode().splitlines()[1:]]
            self.assertEqual({r[2]:r[0] for r in records},{n:hashlib.sha256(n.encode()).hexdigest() for n in ('Chaos.exe','jpeg.dll','CDROMDrive.cfg')})
            self.assertEqual(prepare_launch_manifest(game,manifest),raw)
            (game/'Chaos.exe').write_bytes(b'changed input')
            with self.assertRaises(ValueError):prepare_launch_manifest(game,manifest)
            self.assertEqual(manifest.read_bytes(),raw)

    def test_malformed_or_duplicate_manifest_cannot_replace_existing_pins(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);game=root/'game';game.mkdir();manifest=root/'manifest.tsv'
            for name in ('Chaos.exe','jpeg.dll','CDROMDrive.cfg'):(game/name).write_bytes(b'input')
            raw=prepare_launch_manifest(game,manifest)
            for bad in (raw.replace(b'\t',b'|'),raw+raw.splitlines(keepends=True)[1]):
                manifest.write_bytes(bad)
                with self.assertRaises(ValueError):prepare_launch_manifest(game,manifest)
                self.assertEqual(manifest.read_bytes(),bad)

    def test_projection_bounded_for_remote_target_and_portal(self):
        spec=importlib.util.spec_from_file_location('outcome_input',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(x=0,y=0,z=0)
        for target in [dict(x=100,y=-100,z=30),dict(x=1,y=2,z=0),hero]:
            x,y=module.target_point(hero,target)
            self.assertTrue(0<=x<800 and 0<=y<600)

    def test_victory_preserves_combat_mana_while_defeat_never_casts(self):
        spec=importlib.util.spec_from_file_location('outcome_policy',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(x=0,y=0,z=0,health=200,mana=50*256)
        enemy=dict(x=3,y=2,z=0,health=100)
        choose=module.combat_choice
        self.assertEqual(choose('progression',hero,enemy,0,0,10),'fireball')
        self.assertEqual(choose('victory',hero,enemy,1,1,10),'fireball')
        self.assertEqual(choose('progression',hero,enemy,2,2,10),'fireball')
        self.assertEqual(choose('defeat',hero,enemy,0,0,10),'move')
        enemy.update(x=1,y=0)
        self.assertEqual(choose('defeat',hero,enemy,0,0,10),'idle')
        hero.update(mana=0,health=50)
        self.assertEqual(choose('victory',hero,enemy,0,6,10),'retreat')
        enemy.update(x=5,y=0)
        self.assertEqual(choose('victory',hero,enemy,0,6,10),'retreat')
        enemy.update(x=20,y=0)
        hero.update(mana=50*256,health=200)
        self.assertEqual(choose('victory',hero,enemy,0,0,10),'move')
        enemy['health']=0
        self.assertEqual(choose('victory',hero,enemy,0,6,10),'portal')

    def test_firing_line_refuses_living_allies_between_caster_and_target(self):
        spec=importlib.util.spec_from_file_location('line_policy',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(slot=0,owner=0,x=0,y=0,z=0,active=1,health=200)
        target=dict(slot=1,owner=1,x=6,y=0,z=0,active=1,health=100)
        ally=dict(slot=2,owner=0,x=3,y=0,z=0,active=1,health=90)
        def check(**changes):
            candidate=dict(ally,**changes)
            return module.firing_line_allies(hero,target,{0:hero,1:target,2:candidate})
        self.assertEqual(check(),[2])
        for changes in (dict(x=-1),dict(x=7),dict(y=2),dict(active=0),dict(health=0),dict(owner=1)):
            self.assertEqual(check(**changes),[])
        target.update(x=0)
        self.assertEqual(module.firing_line_allies(hero,target,{2:ally}),[])

    @unittest.skipUnless(shutil.which('xvfb-run'),'Xvfb required for helper regression')
    def test_report_callback_retires_helper_before_world_pop_or_return(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);out=root/'input.json'
            # Authored protocol prefix: Main callback, then original report OK
            # dispatch. No World-pop, callback return or gameplay file exists.
            prefix=[list(r) for r in struct.iter_unpack('<16I',trace()[16:])
                    if (r[1],r[3],r[9]) in ((3,3,0),(2,6,21))]
            self.assertEqual(len(prefix),2)
            for i,r in enumerate(prefix,1):r[0]=i
            (root/'events.bin').write_bytes(trace()[:16]+b''.join(struct.pack('<16I',*r) for r in prefix))
            spec=importlib.util.spec_from_file_location('helper_display',ROOT/'tools/run-native-owned.py')
            launcher=importlib.util.module_from_spec(spec);spec.loader.exec_module(launcher)
            host=root/'host';host.mkdir();server,owner,_=launcher.start_display(host)
            try:
                result=subprocess.run([sys.executable,ROOT/'tools/campaign-outcome-input.py',
                    '--rect','0','0','800','600','--case','defeat','--experiment',root,
                    '--seconds','30','--output',out],capture_output=True,timeout=8,
                    env=dict(os.environ,DISPLAY=owner['display'],XAUTHORITY=owner['authority']))
            finally:
                server.terminate();server.wait(timeout=5)
            self.assertEqual(result.returncode,0,result.stderr.decode()+(out.read_text() if out.exists() else ''))
            item=json.loads(out.read_text())
            self.assertTrue(item['success'])
            self.assertIsNone(item['original_report_dispatch_observed']['return_sequence'])
            self.assertFalse(item['semantic_outcome_accepted'])
            self.assertFalse(item['simulation_writes'])
            self.assertEqual(item['actions'],[])

    @unittest.skipUnless(shutil.which('xvfb-run'),'Xvfb required for killed-helper regression')
    def test_helper_snapshot_survives_owner_termination(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);out=root/'input.json'
            (root/'events.bin').write_bytes(b'MNMMENU1'+struct.pack('<II',1,64))
            (root/'gameplay-events.bin').write_bytes(b'MNMGP001'+struct.pack('<II',1,64))
            process=subprocess.Popen(['xvfb-run','-a',sys.executable,ROOT/'tools/campaign-outcome-input.py',
                '--rect','0','0','800','600','--case','progression','--experiment',root,
                '--seconds','30','--output',out],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,start_new_session=True)
            try:
                deadline=time.monotonic()+5
                while not out.exists() and process.poll() is None and time.monotonic()<deadline:time.sleep(.02)
                self.assertTrue(out.exists(),'No bounded snapshot while helper is running')
            finally:
                if process.poll() is None:os.killpg(process.pid,signal.SIGTERM)
                process.wait(timeout=5)
            record=json.loads(out.read_text())
            self.assertFalse(record['success'])
            self.assertFalse(record['simulation_writes'])
            self.assertEqual(record['actions'],[])


if __name__=='__main__':unittest.main()
