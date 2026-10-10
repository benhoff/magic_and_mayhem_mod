#!/usr/bin/env python3
"""Fail-closed evidence checks for the public native campaign smoke runner."""
import copy
import importlib.util
from pathlib import Path
import struct
import tempfile
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('campaign_smoke', ROOT / 'tools/test-native-campaign.py')
smoke = importlib.util.module_from_spec(spec)
spec.loader.exec_module(smoke)


class EvidenceTests(unittest.TestCase):
    def flow(self):
        return {'success': True, 'native_command_fallback': False, 'native_command_frames': 6,
                'steps': [{'step': s, 'screenshot_saved': True} for s in smoke.STEPS]}

    def test_complete_flow(self):
        smoke.validate_flow(self.flow())

    def test_portrait_requires_repeated_input_pixels_and_strict_throughput(self):
        import hashlib
        from PIL import Image
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);name='combat-05-portrait-stress'
            capture=dict(name=name,metadata=dict(native_saved=True,original_saved=True,fallback=False),image_sha256={})
            for route in ('native','original'):
                path=root/(name+'-'+route+'.png');Image.new('RGB',(800,600),(12,34,56)).save(path)
                capture['image_sha256'][route]=hashlib.sha256(path.read_bytes()).hexdigest()
            inputs=dict(portrait_stress=dict(seconds=15,cycles=5),captures=[capture],actions=[dict(action='portrait-stress-start',seconds=0)]+[dict(action='portrait-recenter',seconds=i*3) for i in range(5)]+[dict(action='portrait-stress-complete',seconds=15)])
            flow=dict(gameplay_rates=[dict(cycle=0,seconds=1,native_frames=40,painted_frames=30,native_fps=40,paint_fps=30) for i in range(15)])
            self.assertEqual(smoke.validate_portrait_stress(inputs,flow,root,15)['maximum_channel_error'],0)
            for mutate in [lambda i,f:i['actions'].pop(),lambda i,f:i['portrait_stress'].update(cycles=6),
                             lambda i,f:i['portrait_stress'].update(seconds=9),lambda i,f:i['captures'][0]['metadata'].update(fallback=True),
                             lambda i,f:f['gameplay_rates'][5].update(native_fps=9.99),
                             lambda i,f:f['gameplay_rates'][5].update(seconds=2.1)]:
                bad,slow=copy.deepcopy(inputs),copy.deepcopy(flow);mutate(bad,slow)
                with self.assertRaises(ValueError):smoke.validate_portrait_stress(bad,slow,root,15)
            path=root/(name+'-native.png');image=Image.open(path);image.putpixel((730,520),(14,34,56));image.save(path)
            with self.assertRaises(ValueError):smoke.validate_portrait_stress(inputs,flow,root,15)
            capture['image_sha256']['native']=hashlib.sha256(path.read_bytes()).hexdigest()
            with self.assertRaises(ValueError):smoke.validate_portrait_stress(inputs,flow,root,15)

    def test_original_window_or_missing_native_frames_cannot_pass(self):
        for updates in [{'success': False}, {'native_command_fallback': True},
                        {'native_command_fallback': None}, {'native_command_frames': 0},
                        {'native_command_frames': True}, {'native_command_frames': 5}]:
            with self.subTest(updates=updates):
                flow = self.flow(); flow.update(updates)
                with self.assertRaises(ValueError):
                    smoke.validate_flow(flow)

    def test_partial_flow_or_missing_screenshot_cannot_pass(self):
        for mutate in [lambda f: f['steps'].pop(), lambda f: f['steps'].reverse(),
                       lambda f: f['steps'][0].update(screenshot_saved=False)]:
            flow = self.flow(); mutate(flow)
            with self.assertRaises(ValueError):
                smoke.validate_flow(flow)

    def events(self, root):
        rows = []
        def event(kind, screen, arg=0, initialized=1, active=0x6cbb78):
            row = [0]*16
            row[0], row[1], row[2], row[3] = len(rows)+1, kind, 400, screen
            row[6], row[9], row[11] = initialized, arg, active
            rows.append(row)
        event(3, 3); event(3, 18)
        for _ in range(3):
            event(12, 2)
        event(3, 17, 4); event(20, 2)
        channel = bytearray(106496)
        channel[:16] = b'MNMMCM12' + struct.pack('<II', 12, len(channel))
        struct.pack_into('<I', channel, 36*4, 3)
        (root / 'channel.bin').write_bytes(channel)
        return rows

    def write_events(self, root, rows):
        (root / 'events.bin').write_bytes(b'MNMMENU1' + struct.pack('<II', 1, 64) +
                                         b''.join(struct.pack('<16I', *r) for r in rows))

    def test_independent_gameplay_and_resume_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); rows = self.events(root); self.write_events(root, rows)
            self.assertEqual(smoke.validate_events(root)['world_ticks'], 3)
            for kind in ['no_resume', 'wrong_thread', 'missing_action', 'before_cancel', 'bad_sequence']:
                changed = copy.deepcopy(rows)
                if kind == 'no_resume': changed.pop()
                elif kind == 'wrong_thread': changed[-1][2] = 401
                elif kind == 'missing_action': changed[0][1] = 2
                elif kind == 'before_cancel': changed[-1][0] = changed[-2][0]-1
                else: changed[0][0] = 2
                self.write_events(root, changed)
                with self.subTest(kind=kind), self.assertRaises(ValueError):
                    smoke.validate_events(root)

    def test_normal_quit_requires_no_resume_yes_and_final_ack(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);rows=self.events(root)
            def event(kind,screen,arg=0):
                r=[0]*16;r[0]=len(rows)+1;r[1]=kind;r[2]=400;r[3]=screen;r[4]=0x6a5088;r[6]=1;r[9]=arg;r[11]=0x6cbb78;rows.append(r)
            event(3,17,3);event(22,17,1);event(20,2);event(3,17,3);event(22,17,0);event(3,6,21);event(3,3,4)
            ch=bytearray((root/'channel.bin').read_bytes());struct.pack_into('<I',ch,36*4,7);(root/'channel.bin').write_bytes(ch)
            self.write_events(root,rows);smoke.validate_events(root,normal_quit=True)
            for index,field,value in [(8,9,0),(9,1,4),(11,2,401),(11,4,99)]:
                bad=copy.deepcopy(rows);bad[index-1][field]=value;self.write_events(root,bad)
                with self.assertRaises(ValueError):smoke.validate_events(root,normal_quit=True)
            self.write_events(root,rows);struct.pack_into('<I',ch,36*4,6);(root/'channel.bin').write_bytes(ch)
            with self.assertRaises(ValueError):smoke.validate_events(root,normal_quit=True)

    def test_normal_quit_flow_cannot_pass_bounded_termination(self):
        flow=self.flow()
        with self.assertRaises(ValueError):smoke.validate_flow(flow,normal_quit=True)
        flow['steps'] += [{'step':s,'screenshot_saved':True} for s in ['campaign-quit-menu','quit-no-resumed','campaign-quit-menu','campaign-defeat','quit-main']]
        with self.assertRaises(ValueError):smoke.validate_flow(flow,normal_quit=True)
        flow['normal_quit']=True;smoke.validate_flow(flow,normal_quit=True)

    def test_healing_requires_injury_caster_debit_captures_and_strict_rates(self):
        import hashlib
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)
            def actor(seq,tick,hp):return [seq,1,400,tick,0,0,0,1,hp,10,70,0,0,0,0xffffffff,12800]
            gp=[actor(1,1000,130),actor(2,1500,130),actor(3,2500,180)]
            cast=[1,1,400,2001,41,0,0,0,12800,12032,0,1,2,4,6,1]
            heal=[2,4,400,2000,0,0,0,130,180,1,50,0,41,0x48b5d8,0,0]
            from campaign_spell import HEADER_V3
            (root/'spell-events.bin').write_bytes(HEADER_V3+b''.join(struct.pack('<16I',*r) for r in [cast,heal]))
            (root/'gameplay-events.bin').write_bytes(b'MNMGP001'+struct.pack('<II',1,64)+b''.join(struct.pack('<16I',*r) for r in gp))
            callbacks=[(3,2),(22,2),(14,2),(25,0),(14,1),(7,2003),(7,2046),(7,12)]
            rows=[]
            for seq,(menu,arg) in enumerate(callbacks,1):
                row=[0]*16;row[0]=seq;row[1]=3;row[2]=400;row[3]=menu;row[9]=arg;rows.append(row)
            self.write_events(root,rows)
            ch=bytearray(106496);ch[:16]=b'MNMMCM12'+struct.pack('<II',12,len(ch));struct.pack_into('<I',ch,36*4,6);(root/'channel.bin').write_bytes(ch)
            inp=dict(success=True,seconds=20,owner=0,wizard_slot=0,initial_wizard=dict(health=200),healing=dict(before_sequence=1,after_sequence=3,first_health_change=heal),captures=[])
            for label in ('before-cure','after-cure'):
                name='healing-00-'+label;data=b'owned image fixture'
                for route in ('native','original'):(root/(name+'-'+route+'.png')).write_bytes(data)
                inp['captures'].append(dict(name=name,metadata=dict(native_saved=True,original_saved=True,fallback=False),image_sha256={r:hashlib.sha256(data).hexdigest() for r in ('native','original')}))
            flow=dict(success=True,healing_mode=True,native_command_fallback=False,native_command_frames=600,native_recoveries=0,native_error='',owner=0,
                      steps=[dict(step=n,screenshot_saved=True) for n in ['main','quick','setup','map','setup-ready','spells']]+[dict(step='healing-loadout',owner=0,assignments=[dict(spell='14'),dict(spell='41'),dict(spell='')])],
                      gameplay_rates=[dict(cycle=0,seconds=1,native_frames=40,painted_frames=30,native_fps=40,paint_fps=30) for i in range(20)])
            self.assertTrue(smoke.validate_healing(root,inp,flow,root,20)['verified'])
            for change in ('uninjured','wrong-proof','capture','fallback','slow','stall','loadout'):
                bad,f=copy.deepcopy(inp),copy.deepcopy(flow)
                if change=='uninjured':bad['initial_wizard']['health']=130
                elif change=='wrong-proof':bad['healing']['first_health_change'][8]=179
                elif change=='capture':bad['captures'].pop()
                elif change=='fallback':f['native_command_fallback']=True
                elif change=='slow':f['gameplay_rates'][0]['paint_fps']=9.99
                elif change=='stall':f['gameplay_rates'][0]['seconds']=2.1
                else:f['steps'][-1]['assignments'][0]['spell']='71'
                with self.subTest(change=change),self.assertRaises(ValueError):smoke.validate_healing(root,bad,f,root,20)

    def test_dialogue_readiness_checks_both_views_and_capture_hashes(self):
        import hashlib
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);name='combat-00-dialogue-check-0';data=b'owned fixture image'
            for r in ('native','original'):(root/(name+'-'+r+'.png')).write_bytes(data)
            inp=dict(dialogue_checks=[dict(capture=name,texts=dict(native='',original=''),visible=dict(native=False,original=False))],
                     captures=[dict(name=name,metadata=dict(native_saved=True,original_saved=True,fallback=False),image_sha256={r:hashlib.sha256(data).hexdigest() for r in ('native','original')})])
            self.assertTrue(smoke.validate_dialogue_readiness(inp,root)['verified'])
            for change in ('missing','visible','classification','image'):
                bad=copy.deepcopy(inp)
                if change=='missing':bad['dialogue_checks']=[]
                elif change=='visible':bad['dialogue_checks'][0]['visible']['original']=True
                elif change=='classification':bad['dialogue_checks'][0]['texts']['native']='Hermes'
                else:bad['captures'][0]['image_sha256']['native']='0'*64
                with self.assertRaises(ValueError):smoke.validate_dialogue_readiness(bad,root)

    def test_startup_frames_do_not_authorize_gameplay(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);rows=self.events(root)
            self.write_events(root,[r for r in rows if r[1]!=12])
            self.assertFalse(smoke.world_ready(root))
            self.write_events(root,rows)
            self.assertTrue(smoke.world_ready(root))
            rows[3][2]=401;self.write_events(root,rows)
            self.assertFalse(smoke.world_ready(root))

    def test_slow_or_missing_paints_fail_gameplay(self):
        flow=self.flow()
        flow['gameplay_rates']=[dict(cycle=c,seconds=1,native_frames=30,painted_frames=30,native_fps=30,paint_fps=30) for c in [0,1] for _ in range(10)]
        smoke.validate_flow(flow,stress_seconds=10)
        for field,value in [('painted_frames',1),('native_frames',1),('seconds',3)]:
            slow=copy.deepcopy(flow)
            for row in slow['gameplay_rates']:row[field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):smoke.validate_flow(slow,stress_seconds=10)
        flow['gameplay_rates']=[]
        with self.assertRaises(ValueError):smoke.validate_flow(flow,stress_seconds=10)

    def test_protocol_downgrade_cannot_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); rows = self.events(root); self.write_events(root, rows)
            channel = bytearray((root / 'channel.bin').read_bytes())
            channel[:8] = b'MNMMCMD8'; (root / 'channel.bin').write_bytes(channel)
            with self.assertRaises(ValueError): smoke.validate_events(root)


class CombatEvidenceTests(unittest.TestCase):
    def fixture(self, root):
        def creature(slot, kind, owner, hp):
            return [0,1,400,100,slot,kind,owner,1,hp,10,71,4,1,1,0xffffffff,0]
        rows=[creature(0,0,0,200),creature(1,10,2,0),creature(3,14,0,90),creature(1,14,0,90),
              [0,2,400,200,1,14,0,90,2,10,2,110,0,1,1,0]]
        for index,row in enumerate(rows,1):row[0]=index
        inputs=dict(player_owner=0,wizard_slot=0,summon=dict(before_sequence=2,after_sequence=3,slot=3,
                    before_capture='combat-00-before-summon',after_capture='combat-01-after-summon'),
                    additional_summons=[dict(before_sequence=3,after_sequence=4,slot=1)],captures=[])
        for name,count in [('combat-00-before-summon','0/15'),('combat-01-after-summon','1/15')]:
            hashes={}
            for route in ('native','original'):
                image=root/(name+'-'+route+'.png');image.write_bytes(b'synthetic image fixture '+route.encode())
                hashes[route]=smoke.sha(image)
            inputs['captures'].append(dict(name=name,ocr=dict(native=count,original=count),image_sha256=hashes,
                metadata=dict(native_saved=True,original_saved=True,fallback=False)))
        return rows,inputs

    def write_trace(self, root, rows):
        spell=[1,1,400,100,14,0,0,0,12800,8448,0xffffffff,1,2,4,6,1]
        (root/'spell-events.bin').write_bytes(b'MNMCA001'+struct.pack('<II',1,64)+struct.pack('<16I',*spell))
        (root/'gameplay-events.bin').write_bytes(b'MNMGP001'+struct.pack('<II',1,64)+b''.join(struct.pack('<16I',*r) for r in rows))

    def test_actual_melee_and_summons_including_reused_original_slot(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);rows,inputs=self.fixture(root);self.write_trace(root,rows)
            result=smoke.validate_casting_combat(root,inputs,root)
            self.assertTrue(result['casting_verified']);self.assertTrue(result['combat_verified'])
            self.assertEqual(result['combat_source_slots'],[1])

    def test_wizard_can_finish_enemy_damaged_by_summoned_zombie(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);rows,inputs=self.fixture(root);rows[-1][12]=104
            rows.append([6,2,400,220,0,0,0,200,2,10,2,1,0,1,1,0]);self.write_trace(root,rows)
            result=smoke.validate_casting_combat(root,inputs,root)
            self.assertEqual(result['first_lethal_melee'][4],0)
            rows[-1][8]=99;self.write_trace(root,rows)
            with self.assertRaises(ValueError):smoke.validate_casting_combat(root,inputs,root)

    def test_orders_script_damage_and_nonplayer_damage_cannot_pass(self):
        for mutate in [lambda r:r[-1].__setitem__(1,1),lambda r:r[-1].__setitem__(12,110),
                       lambda r:r[-1].__setitem__(12,104),
                       lambda r:r[-1].__setitem__(6,2),lambda r:r[-1].__setitem__(10,0),
                       lambda r:r[-1].__setitem__(13,0),lambda r:r[-1].__setitem__(7,0),
                       lambda r:r[-1].__setitem__(11,0),lambda r:r[-1].__setitem__(4,99),
                       lambda r:r[-1].__setitem__(2,401)]:
            with tempfile.TemporaryDirectory() as directory:
                root=Path(directory);rows,inputs=self.fixture(root);mutate(rows);self.write_trace(root,rows)
                with self.assertRaises(ValueError):smoke.validate_casting_combat(root,inputs,root)

    def test_existing_wrong_owner_or_dead_zombie_cannot_prove_summon(self):
        for mutate in [lambda r:r[1].__setitem__(5,14),lambda r:r[2].__setitem__(6,2),
                       lambda r:r[2].__setitem__(8,0),lambda r:r[2].__setitem__(7,0)]:
            with tempfile.TemporaryDirectory() as directory:
                root=Path(directory);rows,inputs=self.fixture(root);mutate(rows)
                if rows[1][5]==14:rows[1][6]=0
                self.write_trace(root,rows)
                with self.assertRaises(ValueError):smoke.validate_casting_combat(root,inputs,root)

    def test_native_count_missing_image_or_modified_capture_cannot_pass(self):
        for case in ('wrong-native-count','missing-original-count','changed-image','fallback'):
            with tempfile.TemporaryDirectory() as directory:
                root=Path(directory);rows,inputs=self.fixture(root);self.write_trace(root,rows)
                if case=='wrong-native-count':inputs['captures'][1]['ocr']['native']='0/15'
                elif case=='missing-original-count':del inputs['captures'][1]['ocr']['original']
                elif case=='changed-image':(root/'combat-01-after-summon-native.png').write_bytes(b'changed')
                else:inputs['captures'][1]['metadata']['fallback']=True
                with self.assertRaises(ValueError):smoke.validate_casting_combat(root,inputs,root)

    def test_truncated_trace_cannot_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);rows,inputs=self.fixture(root);self.write_trace(root,rows)
            path=root/'gameplay-events.bin';path.write_bytes(path.read_bytes()[:-1])
            with self.assertRaises(ValueError):smoke.validate_casting_combat(root,inputs,root)


if __name__ == '__main__':
    unittest.main()
