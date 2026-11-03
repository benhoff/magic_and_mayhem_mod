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
import threading
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
    def threat_fixture(self):
        spec=importlib.util.spec_from_file_location('outcome_threat_policy',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        # Exact v29 cleared-bridge snapshot: Enemy6 is already pursuing wizard0.
        hero=dict(sequence=4854,slot=0,type=0,owner=0,active=1,health=141,x=31,y=52,z=4,target=6,mana=9728)
        follower=dict(sequence=4855,slot=3,type=14,owner=0,active=1,health=90,x=29,y=53,z=4,target=6,mana=0)
        escort=dict(sequence=4853,slot=6,type=11,owner=1,active=1,health=30,x=32,y=52,z=4,target=0,mana=0)
        return module,hero,follower,escort

    def test_retained_owner_one_escort_is_prioritized_before_distant_camulos(self):
        module,hero,follower,escort=self.threat_fixture()
        boss=dict(escort,slot=4,type=24,health=100,x=44,y=27,target=0)
        neutral=dict(escort,slot=2,type=10,owner=2,x=31,y=52)
        actors={v['slot']:v for v in (hero,follower,escort,boss,neutral)}
        self.assertEqual([v['slot'] for v in module.engaged_threats(hero,actors,3)],[6,2])
        self.assertEqual(module.combat_choice('victory',hero,escort,1,1,4),'fireball')
        self.assertEqual(module.combat_choice('defeat',hero,escort,1,1,4),'idle')

    def test_threat_policy_rejects_inactive_friendly_unengaged_and_remote_actors(self):
        module,hero,follower,escort=self.threat_fixture()
        for change in (dict(active=0),dict(health=0),dict(owner=0),dict(owner=0xffffffff),
                       dict(target=0xffffffff),dict(target=9),dict(x=42,y=52),dict(type=24)):
            candidate=dict(escort,**change)
            with self.subTest(change=change):
                self.assertEqual(module.engaged_threats(hero,{0:hero,3:follower,6:candidate},3),[])

    def test_only_a_living_owned_zombie_is_a_protected_follower(self):
        module,hero,follower,escort=self.threat_fixture();escort=dict(escort,target=3)
        self.assertEqual(module.engaged_threats(hero,{0:hero,3:follower,6:escort},3),[escort])
        for change in (dict(active=0),dict(health=0),dict(owner=1),dict(type=10)):
            with self.subTest(change=change):
                self.assertEqual(module.engaged_threats(hero,{0:hero,3:dict(follower,**change),6:escort},3),[])
        self.assertEqual(module.engaged_threats(hero,{0:hero,6:escort},3),[])

    def test_engaged_escort_interrupts_post_refill_staging_before_exact_waypoint(self):
        module,hero,follower,escort=self.threat_fixture();hero=dict(hero,x=28,y=53)
        actors={0:hero,3:follower,6:escort}
        self.assertTrue(module.route_combat_needed('bridge',True,hero,actors,3))
        for phase,rearmed in [('bridge',False),('power-approach',True),('combat',True)]:
            self.assertFalse(module.route_combat_needed(phase,rearmed,hero,actors,3))
        for change in [dict(target=0xffffffff),dict(x=36,y=53),dict(health=0),dict(owner=0)]:
            with self.subTest(change=change):
                self.assertFalse(module.route_combat_needed('bridge',True,hero,{0:hero,3:follower,6:dict(escort,**change)},3))

    def test_resupply_can_suspend_for_an_affordable_engaged_attacker(self):
        module,hero,follower,escort=self.threat_fixture()
        hero=dict(hero,x=29,y=52,health=113,mana=16*256)
        escort=dict(escort,x=30,y=53)
        actors={0:hero,3:follower,6:escort}
        self.assertTrue(module.route_combat_needed('resupply',False,hero,actors,3))
        self.assertFalse(module.route_combat_needed('resupply',False,dict(hero,mana=5*256),actors,3))
        self.assertFalse(module.route_combat_needed('power-refill',False,hero,actors,3))

    def melee_fixture(self,hero):
        return [[1,2,40,1000,6,11,1,30,hero['slot'],0,0,141,117,1,1,6],
                [2,1,40,1100,0,0,0,1,117,29,52,4,8,14,6,4096]]

    def test_recent_original_melee_protects_a_transiently_cleared_actor_target(self):
        module,hero,follower,escort=self.threat_fixture();escort=dict(escort,target=0xffffffff)
        actors={0:hero,3:follower,6:escort};rows=self.melee_fixture(hero)
        recent=module.recent_melee_attackers(rows,hero,actors,3)
        self.assertEqual(recent,{(6,11,1)})
        self.assertEqual(module.engaged_threats(hero,actors,3),[])
        self.assertEqual(module.engaged_threats(hero,actors,3,recent=recent),[escort])
        self.assertTrue(module.route_combat_needed('resupply',False,hero,actors,3,recent))
        for change in [dict(health=0),dict(type=10),dict(owner=2)]:
            self.assertEqual(module.engaged_threats(hero,{0:hero,3:follower,6:dict(escort,**change)},3,recent=recent),[])

    def test_recent_melee_rejects_stale_wrong_victim_and_unapplied_damage(self):
        module,hero,follower,escort=self.threat_fixture();actors={0:hero,3:follower,6:escort}
        rows=self.melee_fixture(hero)
        for index,value in [(1,1),(6,0),(7,0),(8,9),(9,14),(10,1),(12,141),(13,0)]:
            altered=copy.deepcopy(rows);altered[0][index]=value
            with self.subTest(index=index):self.assertEqual(module.recent_melee_attackers(altered,hero,actors,3),set())
        rows[-1][3]=4001
        self.assertEqual(module.recent_melee_attackers(rows,hero,actors,3),set())

    def test_recent_melee_witness_handles_original_tick_wrap(self):
        module,hero,follower,escort=self.threat_fixture();rows=self.melee_fixture(hero)
        rows[0][3]=0xfffffffe;rows[-1][3]=2
        self.assertEqual(module.recent_melee_attackers(rows,hero,{0:hero,3:follower,6:escort},3),{(6,11,1)})

    def test_escort_burst_observes_damage_before_a_third_cast_and_never_overspends(self):
        module,hero,follower,escort=self.threat_fixture()
        for mana,limit in [(0,0),(5*256,0),(6*256,1),(11*256,1),(12*256,2),(38*256,2)]:
            for paused in [False,True]:
                self.assertEqual(module.fireball_burst_limit(dict(hero,mana=mana),escort,paused),limit)
        self.assertEqual(module.fireball_burst_limit(hero,dict(escort,type=10,owner=2),False),7)

    def test_blocked_snapshot_writer_leaves_producer_free_and_retains_complete_history(self):
        from campaign_outcome_record import AtomicSnapshotWriter
        entered=threading.Event();release=threading.Event();written=[]
        def blocked(payload):
            if not written:
                entered.set()
                if not release.wait(5):raise RuntimeError('Test writer was not released')
            written.append(payload)
        writer=AtomicSnapshotWriter(Path('unused-physical-interchange'),write=blocked)
        try:
            writer.submit(b'{"actions":[]}')
            self.assertTrue(entered.wait(2))
            actions=[]
            for i in range(64):
                actions.append(i)
                writer.submit(json.dumps(dict(actions=actions)).encode())
            # The producer reaches here while the first disk operation is
            # blocked;one pending immutable image contains every action.
            self.assertFalse(release.is_set())
            self.assertEqual(written,[])
            actions.append('after-serialization')
        finally:
            release.set();writer.close()
        self.assertEqual(len(written),2)
        self.assertEqual(json.loads(written[-1])['actions'],list(range(64)))
        self.assertEqual(writer.statistics()['coalesced'],63)
        with self.assertRaises(RuntimeError):writer.submit(b'{}')

    def test_snapshot_failure_and_bounds_cannot_replace_existing_interchange(self):
        from campaign_outcome_record import AtomicSnapshotWriter
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'result.json';path.write_bytes(b'previous')
            def fail(payload):raise OSError('Injected atomic write refusal')
            writer=AtomicSnapshotWriter(path,write=fail)
            with self.assertRaises(ValueError):writer.submit(b'x'*(writer.MAX_BYTES+1))
            writer.submit(b'new')
            with self.assertRaisesRegex(RuntimeError,'Injected atomic write refusal'):writer.close()
            self.assertEqual(path.read_bytes(),b'previous')
            writer=AtomicSnapshotWriter(path)
            writer.submit(b'{"success":false}');writer.close()
            self.assertEqual(json.loads(path.read_bytes()),dict(success=False))
            self.assertFalse(path.with_suffix('.tmp').exists())

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

    def test_boss_volley_also_refuses_living_enemy_interceptors(self):
        spec=importlib.util.spec_from_file_location('blocker_policy',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(slot=0,owner=0,x=40,y=30,z=4,active=1,health=141)
        target=dict(slot=4,type=24,owner=1,x=39,y=27,z=4,active=1,health=100)
        guard=dict(slot=5,type=10,owner=1,x=40,y=29,z=4,active=1,health=110)
        actors={0:hero,4:target,5:guard}
        self.assertEqual(module.firing_line_allies(hero,target,actors),[])
        self.assertEqual(module.firing_line_blockers(hero,target,actors),[5])
        self.assertIsNone(module.camulos_pause_point(hero,target,actors,(640,480)))
        for changes in (dict(health=0),dict(active=0),dict(x=43)):
            self.assertEqual(module.firing_line_blockers(hero,target,{0:hero,4:target,5:dict(guard,**changes)}),[])

    def test_camulos_pause_requires_close_visible_living_clear_target(self):
        spec=importlib.util.spec_from_file_location('pause_policy',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(slot=0,owner=0,type=0,x=40,y=30,z=4,active=1,health=200,mana=36*256)
        target=dict(slot=4,owner=1,type=24,x=39,y=27,z=4,active=1,health=100)
        for size in ((800,600),(640,480)):
            actors={0:hero,4:target}
            self.assertIsNotNone(module.camulos_pause_point(hero,target,actors,size))
            ally=dict(slot=3,owner=0,x=40,y=29,z=4,active=1,health=80)
            self.assertIsNone(module.camulos_pause_point(hero,target,{**actors,3:ally},size))
            for changes in (dict(health=0),dict(active=0),dict(owner=0),dict(type=10),dict(x=6)):
                self.assertIsNone(module.camulos_pause_point(hero,dict(target,**changes),actors,size))
        earlier_hero=dict(hero,x=42,y=31,z=4,health=70)
        earlier_target=dict(target,x=43,y=27,z=5)
        for size in ((800,600),(640,480)):
            self.assertIsNotNone(module.camulos_pause_point(earlier_hero,earlier_target,{0:earlier_hero,4:earlier_target},size))
            east_hero=dict(hero,x=44,y=30)
            self.assertIsNotNone(module.camulos_pause_point(east_hero,earlier_target,{0:east_hero,4:earlier_target},size))
        # Retained 640 before-pause poses from nominal failures: never consume
        # the single Space pair at these projected positions.
        for hp,tp in (((41,33,4),(44,27,4)),((42,35,5),(45,29,4))):
            h=dict(hero,**dict(zip(('x','y','z'),hp)));t=dict(target,**dict(zip(('x','y','z'),tp)))
            self.assertIsNone(module.fireball_point(h,t,(640,480)))
            self.assertIsNone(module.camulos_pause_point(h,t,{0:h,4:t},(640,480)))

    def test_camulos_burst_uses_available_volley_without_overspending(self):
        spec=importlib.util.spec_from_file_location('burst_policy',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        for mana in range(0,51*256,127):
            for paused in (False,True):
                limit=module.fireball_burst_limit(dict(mana=mana),dict(type=24),paused)
                self.assertGreaterEqual(limit,0)
                self.assertLessEqual(limit*6*256,mana)
                self.assertLessEqual(limit,6 if paused else 2)
                if paused and mana>=36*256:self.assertEqual(limit,6)
        self.assertEqual(module.fireball_burst_limit(dict(mana=36*256),dict(type=24),True),6)
        self.assertEqual(module.fireball_burst_limit(dict(mana=36*256),dict(type=10),True),7)

    def test_support_attack_requires_one_living_owned_zombie_and_current_enemy(self):
        spec=importlib.util.spec_from_file_location('support_policy',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(slot=0,type=0,owner=0,active=1,health=100)
        enemy=dict(slot=4,type=24,owner=1,active=1,health=100)
        ally=dict(slot=17,type=14,owner=0,active=1,health=90)
        self.assertEqual(module.support_for_attack(hero,enemy,{0:hero,4:enemy,17:ally}),ally)
        for changes in (dict(health=0),dict(active=0),dict(type=10),dict(owner=1)):
            self.assertIsNone(module.support_for_attack(hero,enemy,{0:hero,17:dict(ally,**changes)}))
        self.assertIsNone(module.support_for_attack(hero,enemy,{0:hero,17:ally,18:dict(ally,slot=18)}))
        self.assertIsNone(module.support_for_attack(hero,dict(enemy,health=0),{17:ally}))
        self.assertIsNone(module.support_for_attack(dict(hero,owner=1),enemy,{17:ally}))

    def test_empty_mana_support_policy_does_not_chase_boss_back_into_range(self):
        spec=importlib.util.spec_from_file_location('support_range',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(x=0,y=0,health=100,mana=0)
        enemy=dict(type=24,x=9,y=0,health=100)
        self.assertEqual(module.combat_choice('victory',hero,enemy,1,1,10,True),'idle')
        enemy['x']=5
        self.assertEqual(module.combat_choice('progression',hero,enemy,1,1,10,True),'retreat')
        enemy['health']=0
        self.assertEqual(module.combat_choice('victory',hero,enemy,1,1,10,True),'portal')
        enemy.update(health=100,x=9)
        self.assertEqual(module.combat_choice('defeat',hero,enemy,1,1,10,True),'move')

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
