#!/usr/bin/env python3
"""Negative admission controls; synthetic fixtures never certify game execution."""
import copy,hashlib,json,sys,unittest,sqlite3,tempfile,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from rendering_normal_acceptance import admit,progress,PREFIX,lighting,fullscreen,world_domain,preferences_boundary,reviewed_driver_sources,DRIVER_SOURCES

class Reader:
    def __init__(self):self.data={};self.sources={'apps/qt-shell/main.cpp':'a'*64}
    def json(self,n):return copy.deepcopy(self.data[n])
    def digest(self,n):return self.sources[n] if n in self.sources else hashlib.sha256(json.dumps(self.data[n],sort_keys=True).encode()).hexdigest()
    def pair(self,scenario,mode=0):
        a=dict(kind='public_native_normal_acquisition',success=True,sources_stable=True,original_manifest_verified_before_after=True,original_pixels_used_as_native_inputs=False,scenario=scenario,
          configuration=dict(software_flag=False,skip_intros=False,terrain_lighting='native',source_chunks=True,native_lighting_capture=False,viewport_scaling='integer-sharp'),exit_code=0,private_wine_stopped=True,
          retirement=dict(state='completed',forwarded=0,graphics_retirement=dict(process_exit=True,resources_released=True,surviving_owners=[],counts=dict(surfaces=0,devices=0))),
          resident=dict(source_chunks_enabled=True,stage_profile=dict(capture_complete=True,capture=dict(format=3)),source_chunks='2',normal_quit_retired=True,pending=False,error='',source_copy_rect_refusals='0',renderer='synthetic provider'),sources=dict(self.sources),flow=dict(preferences_cancel_verified=True,preferences_cancel_original_dialogue=1,preferences_cancel_draft_dialogue=2))
        an=PREFIX+scenario+'-'+str(mode)+'-acquisition.json';self.data[an]=a;ar=dict(record=an,sha256=self.digest(an))
        c=dict(kind='public_native_normal_comparison',success=True,sources_stable=True,original_manifest_verified_before_after=True,original_pixels_used_as_native_inputs=False,acquisition=ar,
          lighting=dict(verified=True),preferences_boundary=dict(verified=True),original_menu_actions=dict(verified=True,owned_save_persisted=True),asset_inputs_stable=True,
          original_reference=dict(source_chunks=2,continuous_from_owned_initial_history=True,native_player_pixels_read=False,original_destination_capture_read=False),
          comparison=dict(success=True,world_presentations=1,pixels_compared=16,mismatching_pixels=0,remaining_surfaces=0,comparisons=[dict(world=1,pixels=16,mismatching_pixels=0,actual_sha256='b'*64,expected_sha256='b'*64)]),observations=dict(world_swaps=1),
          input_acknowledgements=dict(verified=True),fullscreen=dict(verified=True),mini_inputs=dict(verified=True),mini_dispatch=dict(verified=True),world_domain=dict(verified=True,format='RGB565',extents=[[800,600]] if mode==0 else [[640,480]]),sources=dict(self.sources))
        cn=PREFIX+scenario+'-'+str(mode)+'-comparison.json';self.data[cn]=c
        return dict(acquisition=ar,comparison=dict(record=cn,sha256=self.digest(cn)))
    def refresh(self,pair):
        a,c=(pair[k]['record'] for k in ('acquisition','comparison'))
        pair['acquisition']['sha256']=self.digest(a);self.data[c]['acquisition']=copy.deepcopy(pair['acquisition']);pair['comparison']['sha256']=self.digest(c)

class AdmissionTests(unittest.TestCase):
    def test_software_requires_explicit_target_flag_and_actual_mesa_provider(self):
        reader=Reader();pair=reader.pair('normal-quick');a=reader.data[pair['acquisition']['record']]
        with self.assertRaises(ValueError):admit(reader,pair,configuration='player-software')
        a['configuration']['software_flag']=True;a['resident']['renderer']='llvmpipe (controlled provider)';reader.refresh(pair)
        self.assertEqual(admit(reader,pair,configuration='player-software')['provider'],a['resident']['renderer'])
        with self.assertRaises(ValueError):admit(reader,pair)
        for provider in ['NVIDIA', '', False, 'synthetic software flag only']:
            a['resident']['renderer']=provider;reader.refresh(pair)
            with self.subTest(provider=provider),self.assertRaises(ValueError):admit(reader,pair,configuration='player-software')
        with self.assertRaises(ValueError):admit(reader,pair,configuration='unreviewed-provider')

    def test_success_flag_cannot_override_a_recorded_collector_error(self):
        for key in ['acquisition','comparison']:
            reader=Reader();pair=reader.pair('normal-quick')
            reader.data[pair[key]['record']]['error']='Late fullscreen or collector validation failed'
            reader.refresh(pair)
            with self.subTest(key=key),self.assertRaises(ValueError):admit(reader,pair)

    def test_native_worker_outcome_requires_real_applied_work_without_fallback(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);session=root/'working/session';(session/'terrain-light-session').mkdir(parents=True)
            source=root/'worker.cpp';source.write_text('synthetic compiler source')
            policy=dict(mode='native',capture='none',sources={'worker.cpp':hashlib.sha256(source.read_bytes()).hexdigest()})
            manifest=session/'manifest.json';manifest.write_text(json.dumps({'terrain_light_session':policy}))
            outcome=session/'terrain-light-session/outcome.sqlite3';db=sqlite3.connect(outcome);db.execute('create table results(payload text)')
            result=dict(success=True,mode='native',worker_exit=0,worker_exited_early=False,source_bound=True,statistics=dict(applied=2,fallback=0,refused=0,rejected=0,error=0))
            for key,value in [(None,None),('fallback',1),('refused',1),('error',1),('applied',0)]:
                row=copy.deepcopy(result)
                if key:row['statistics'][key]=value
                with db:db.execute('delete from results');db.execute('insert into results values(?)',(json.dumps(row),))
                artifacts={p.relative_to(root).as_posix():dict(sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in (manifest,outcome)}
                if key:
                    with self.subTest(key=key),self.assertRaises(ValueError):lighting(root,dict(session=str(session)),artifacts)
                else:self.assertTrue(lighting(root,dict(session=str(session)),artifacts)['verified'])
            db.close()
    def test_preferences_materialization_accepts_deduplicated_diagnostics(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);session=root/'working/session';(session/'lock-capture').mkdir(parents=True)
            path=session/'lock-capture/lifecycle.log'
            def check(values):
                path.write_text('resident_preferences_materialized '+' '.join(f'{v:08x}' for v in values)+'\n')
                artifacts={path.relative_to(root).as_posix():dict(sha256=hashlib.sha256(path.read_bytes()).hexdigest())}
                return preferences_boundary(root,dict(session=str(session)),artifacts)
            good=[0,312,0,2,1]+[0]*14
            self.assertTrue(check(good)['verified'])
            for bad in (good+[0],good[:4]+[0]+good[5:],good[:3]+[0]+good[4:]):
                with self.subTest(values=bad),self.assertRaises(ValueError):check(bad)

    def test_fullscreen_needs_world_swaps_inside_each_actual_transition(self):
        flow=dict(fullscreen_transitions=[dict(fullscreen=full,accepted=True,paint_before=str(i*3),screen_width=1280,screen_height=1024) for i,full in enumerate([True,False]*2)])
        observed=dict(swaps=[1,7],paints={1:({'world':'1','width':1280,'height':1024},b''),7:({'world':'2','width':1280,'height':1024},b'')})
        self.assertTrue(fullscreen(flow,observed)['verified'])
        for bad in [dict(swaps=[1],paints=observed['paints']),dict(swaps=[1,7],paints={1:({'world':'0'},b''),7:({'world':'0'},b'')})]:
            with self.assertRaises(ValueError):fullscreen(flow,bad)
    def test_only_proved_default_leaves_change_and_decisions_stay_pending(self):
        reader=Reader();pairs=[reader.pair('normal-quick'),reader.pair('normal-storage'),reader.pair('normal-storage',1)]
        plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text());r=progress(plan,reader,pairs)
        self.assertEqual((len(r['requirements']),r['passed_requirements'],r['pending_requirements']),(156,6,150))
        self.assertTrue(all(row['state']=='pending' for row in r['requirements'] if row['configuration']=='player-software' or row['kind']=='gate'))
        self.assertEqual(r['decisions'],{'supported-player':'pending','original-recovery':'pending'})
        self.assertEqual(len(r['scope_conditions']),8)
    def test_storage_requires_actual_worlds_at_both_configured_sizes(self):
        reader=Reader();pairs=[reader.pair(s) for s in ('normal-quick','normal-storage')]
        plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text());r=progress(plan,reader,pairs)
        self.assertEqual(r['passed_requirements'],2)
        self.assertTrue(all(row['state']=='pending' for row in r['requirements'] if row['id'].startswith('journey.save-load.')))
    def test_world_format_is_bound_to_actual_imports_not_a_configuration_label(self):
        def stream(red):
            def command(op,seq,body):return struct.pack('<III',op,seq,len(body))+body
            return b'MNMCMD05'+struct.pack('<II',5,16)+command(1,1,struct.pack('<7I',7,640,480,16,red,0x7e0,31))+command(19,2,struct.pack('<5I',7,1,0,0,0))+command(7,3,struct.pack('<I',7))+command(8,4,b'')
        self.assertEqual(world_domain(dict(commands=stream(0xf800)))['extents'],[[640,480]])
        with self.assertRaises(ValueError):world_domain(dict(commands=stream(0x7c00)))
    def test_execution_and_configuration_failures_refuse(self):
        mutations=[('kind','public_native_watchdog_diagnostic_acquisition'),('success',False),('private_wine_stopped',False),('exit_code',1),('sources_stable',False),('original_pixels_used_as_native_inputs',True)]
        for key,value in mutations:
            with self.subTest(key=key):
                reader=Reader();pair=reader.pair('normal-quick');reader.data[pair['acquisition']['record']][key]=value;reader.refresh(pair)
                with self.assertRaises(ValueError):admit(reader,pair)
        for key,value in [('diagnostic_only',True),('injected_fault',{'kind':'gui-stall'}),('software_flag',True),('skip_intros',True),('terrain_lighting','original'),('source_chunks',False),('native_lighting_capture',True),('viewport_scaling','smooth')]:
            reader=Reader();pair=reader.pair('normal-quick');reader.data[pair['acquisition']['record']]['configuration'][key]=value;reader.refresh(pair)
            with self.subTest(key=key),self.assertRaises(ValueError):admit(reader,pair)
    def test_diagnostic_comparison_cannot_grant_nominal_acceptance(self):
        reader=Reader();pair=reader.pair('normal-quick')
        reader.data[pair['comparison']['record']]['kind']='public_native_watchdog_diagnostic_comparison';reader.refresh(pair)
        with self.assertRaises(ValueError):admit(reader,pair)
    def test_independence_and_cohort_corruption_refuse(self):
        for field in ('native_player_pixels_read','original_destination_capture_read'):
            reader=Reader();pair=reader.pair('normal-quick');reader.data[pair['comparison']['record']]['original_reference'][field]=True;reader.refresh(pair)
            with self.subTest(field=field),self.assertRaises(ValueError):admit(reader,pair)
        for field,value in [('world_presentations',2),('pixels_compared',17),('mismatching_pixels',1),('remaining_surfaces',1)]:
            reader=Reader();pair=reader.pair('normal-quick');reader.data[pair['comparison']['record']]['comparison'][field]=value;reader.refresh(pair)
            with self.subTest(field=field),self.assertRaises(ValueError):admit(reader,pair)
    def test_missing_action_proof_and_surviving_owner_refuse(self):
        for scenario,field in [('normal-quick','fullscreen'),('normal-quick','input_acknowledgements'),('normal-storage','mini_inputs'),('normal-storage','mini_dispatch'),('normal-storage','lighting'),('normal-storage','preferences_boundary')]:
            reader=Reader();pair=reader.pair(scenario);reader.data[pair['comparison']['record']][field]['verified']=False;reader.refresh(pair)
            with self.subTest(field=field),self.assertRaises(ValueError):admit(reader,pair)
        reader=Reader();pair=reader.pair('normal-quick');reader.data[pair['acquisition']['record']]['retirement']['graphics_retirement']['counts']['surfaces']=1;reader.refresh(pair)
        with self.assertRaises(ValueError):admit(reader,pair)
    def test_auxiliary_reuse_cannot_waive_low_or_engine_changes(self):
        reader=Reader();pair=reader.pair('normal-storage');acquired=reader.data[pair['acquisition']['record']]
        changes={}
        for name in DRIVER_SOURCES:
            acquired['sources'][name]='1'*64;reader.sources[name]='2'*64;old=PREFIX+'before-'+Path(name).name+'.bin';reader.sources[old]='1'*64
            changes[name]=dict(before='1'*64,after='2'*64,before_record=old)
        reader.refresh(pair)
        name=PREFIX+'review.json';review=dict(kind='scoped_rendering_driver_branch_review',success=True,native_binary_changes_allowed=False,eligible_acquisitions=[pair['acquisition']],sources={n:'2'*64 for n in DRIVER_SOURCES},changes=changes)
        reader.data[name]=review;reference=dict(record=name,sha256=reader.digest(name));facts=dict(world_extents=[[800,600]],presented_world_image_extents=[[800,600]])
        result=reviewed_driver_sources(reader,reference,pair['acquisition'],acquired,facts);self.assertTrue(all(result[n]=='2'*64 for n in DRIVER_SOURCES))
        for key,value in [('native_binary_changes_allowed',True),('success',False),('eligible_acquisitions',[])]:
            bad=copy.deepcopy(review);bad[key]=value;reader.data[name]=bad;reference['sha256']=reader.digest(name)
            with self.subTest(key=key),self.assertRaises(ValueError):reviewed_driver_sources(reader,reference,pair['acquisition'],acquired,facts)
        reader.data[name]=review;reference['sha256']=reader.digest(name)
        with self.assertRaises(ValueError):reviewed_driver_sources(reader,reference,pair['acquisition'],acquired,dict(world_extents=[[640,480]],presented_world_image_extents=[[640,480]]))
        bad=copy.deepcopy(review);bad['changes']['runtime/render/bridge.c']=next(iter(changes.values()));reader.data[name]=bad;reference['sha256']=reader.digest(name)
        with self.assertRaises(ValueError):reviewed_driver_sources(reader,reference,pair['acquisition'],acquired,facts)

    def test_successful_leaf_does_not_retire_a_retained_failure(self):
        reader=Reader();pair=reader.pair('normal-quick');name=PREFIX+'retained-failure.json'
        reader.data[name]=dict(kind='public_native_normal_acquisition',success=False,original_manifest_verified_before_after=True,scenario='normal-quick',configuration=dict(skip_intros=False),error='producer cancelled',experiments=['owned failed session'],sources=dict(reader.sources))
        reference=dict(record=name,sha256=reader.digest(name));plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        result=progress(plan,reader,[pair],[reference])
        self.assertEqual(result['passed_requirements'],2)
        self.assertEqual(result['negative_executions'][0]['resolution'],'unresolved')
        self.assertEqual(result['negative_executions'][0]['blocks_final_decisions'],['supported-player'])
        self.assertTrue(all(r['state']=='pending' for r in result['requirements'] if r['kind']=='gate'))

    def test_receipt_tampering_source_drift_and_duplicates_refuse(self):
        reader=Reader();pair=reader.pair('normal-quick');reader.data[pair['comparison']['record']]['scope']='changed without a new receipt'
        with self.assertRaises(ValueError):admit(reader,pair)
        reader=Reader();pair=reader.pair('normal-quick');reader.sources['apps/qt-shell/main.cpp']='c'*64
        plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        with self.assertRaises(ValueError):progress(plan,reader,[pair])
        reader=Reader();pair=reader.pair('normal-quick')
        with self.assertRaises(ValueError):progress(plan,reader,[pair,pair])



class SustainedCloseoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        import importlib.util
        spec=importlib.util.spec_from_file_location('existing_acceptance_join',ROOT/'tools/join-rendering-normal-acceptance.py')
        cls.join=importlib.util.module_from_spec(spec);spec.loader.exec_module(cls.join)

    def command_bytes(self,rows):
        return b'MNMCMD05'+struct.pack('<II',5,16)+b''.join(struct.pack('<III',op,i,len(body))+body for i,(op,body) in enumerate(rows,1))

    def resource_rows(self):
        create=(1,struct.pack('<7I',1,640,480,16,0xf800,0x7e0,31)+bytes(640*480*2))
        return [create,*[(19,struct.pack('<5I',1,g,0,1,2)) for g in (1,2,3)],(7,struct.pack('<I',1)),(8,b'')]

    def test_existing_command_lifetimes_conserve_each_world_and_end(self):
        result=self.join.resource_history(self.command_bytes(self.resource_rows()))
        self.assertTrue(result['verified']);self.assertEqual((result['creates'],result['deletes']),(1,1))
        self.assertEqual([r['generation'] for r in result['worlds']],[1,2,3])
        self.assertTrue(all(r['logical_surface_bytes']==640*480*2 for r in result['worlds']))
        self.assertEqual(result['created_bytes'],result['deleted_bytes'])

    def test_missing_end_leak_unknown_delete_alias_and_generation_gap_refuse(self):
        original=self.resource_rows()
        cases=[original[:-1],original[:4]+original[5:],original[:4]+[(7,struct.pack('<I',9)),(8,b'')],
               original[:1]+original,original[:2]+[(19,struct.pack('<5I',1,4,0,1,2))]+original[3:],
               original[:2]+[(19,struct.pack('<5I',9,2,0,1,2))]+original[3:]]
        for rows in cases:
            with self.subTest(rows=len(rows)),self.assertRaises(ValueError):self.join.resource_history(self.command_bytes(rows))

    def test_palette_generation_and_lifetime_are_conserved(self):
        rows=self.resource_rows();palette=(12,struct.pack('<2I',3,7)+bytes(256*3));release=(15,struct.pack('<2I',3,7))
        good=rows[:1]+[palette]+rows[1:-1]+[release,rows[-1]]
        r=self.join.resource_history(self.command_bytes(good));self.assertEqual((r['palette_creates'],r['palette_deletes']),(1,1))
        for bad in (good[:-2]+[rows[-1]],good[:-2]+[(15,struct.pack('<2I',3,8)),rows[-1]]):
            with self.assertRaises(ValueError):self.join.resource_history(self.command_bytes(bad))

    def test_only_three_named_leaves_close_for_the_complete_matrix(self):
        plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        runs=[dict(verified=True,domain=(c,size),identity=str(i),pair={},provider='controlled') for i,(c,size) in enumerate((c,z) for c in ('player-default','player-software') for z in ((640,480),(800,600)))]
        rows=self.join.close_rows(plan,runs)
        self.assertEqual(sum(r['state']=='passed' for r in rows),6)
        self.assertTrue(all(r['state']=='pending' for r in rows if r['id'] not in self.join.SUSTAINED_LEAVES))
        for bad in (runs[:-1],runs+[runs[0]],runs[:3]+[dict(runs[-1],identity='0')],runs[:3]+[dict(runs[-1],verified=False)]):
            with self.assertRaises(ValueError):self.join.close_rows(plan,bad)

    def test_each_actual_swap_occurrence_is_preserved(self):
        observed=dict(swaps=[1,2,2,3],paints={1:(dict(world='0'),b''),2:(dict(world='1'),b''),3:(dict(world='2'),b'')})
        matches=[dict(paint=2,world=1),dict(paint=2,world=1),dict(paint=3,world=2)]
        self.assertEqual(len(self.join.presented_world_interval(observed,dict(comparisons=matches))),3)
        for bad in (matches[1:],matches+[matches[-1]],[dict(matches[0],world=3),*matches[1:]]):
            with self.assertRaises(ValueError):self.join.presented_world_interval(observed,dict(comparisons=bad))

    def test_sustained_admission_keeps_the_existing_native_guards(self):
        for where,path,value in [('a',('configuration','diagnostic_only'),True),('a',('configuration','skip_intros'),True),('a',('resident','pending'),True),('a',('retirement','forwarded'),1),('a',('resident','source_copy_rect_refusals'),'1'),('c',('lighting','verified'),False),('c',('input_acknowledgements','verified'),False),('c',('comparison','mismatching_pixels'),1),('c',('original_reference','native_player_pixels_read'),True)]:
            reader=Reader();pair=reader.pair('normal-quick');record=reader.data[pair['acquisition' if where=='a' else 'comparison']['record']]
            target=record
            for key in path[:-1]:target=target[key]
            target[path[-1]]=value;reader.refresh(pair)
            with self.subTest(path=path),self.assertRaises(ValueError):self.join.sustained_admit(reader,pair)

    def test_lossless_capture_metadata_cannot_fake_completeness(self):
        reader=Reader();pair=reader.pair('normal-quick');a=reader.data[pair['acquisition']['record']]
        raw=b'MNMPROF4'+b'controlled-prefix-payload';name=PREFIX+'controlled-worlds.bin'
        reader.read_bytes=lambda n:raw
        artifact=dict(path='working/owned/worlds.bin',record=name,sha256=hashlib.sha256(raw).hexdigest(),bytes=len(raw))
        a['artifacts']=[artifact];a['resident']['completed']='3';a['resident']['stage_profile']['capture']=dict(format=4,codec='zstd-prefix-sha256-v1',complete=True,finished=True,truncated=False,error='',queue_bytes=0,accepted_worlds=3,written_worlds=3,accepted_source_chunks=2,written_source_chunks=2,file_bytes=len(raw))
        reader.refresh(pair);self.assertEqual(self.join.sustained_admit(reader,pair)['scenario'],'normal-quick')
        for key,value in [('complete',False),('finished',False),('truncated',True),('error','codec refusal'),('queue_bytes',1),('written_worlds',2),('written_source_chunks',1),('file_bytes',len(raw)+1),('format',5)]:
            capture=a['resident']['stage_profile']['capture'];old=capture[key];capture[key]=value;reader.refresh(pair)
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.sustained_admit(reader,pair)
            capture[key]=old
        raw=b'MNMPROF3'+raw[8:];artifact['sha256']=hashlib.sha256(raw).hexdigest();reader.refresh(pair)
        with self.assertRaises(ValueError):self.join.sustained_admit(reader,pair)


class CleanupStorageCloseoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        SustainedCloseoutTests.setUpClass();cls.join=SustainedCloseoutTests.join
    # Use the existing suite and wire layouts; no new execution fixture.
    def cleanup(self):
        a=dict(session='owned/session',private_wine_stopped=True,resident=dict(pending=False,normal_quit_retired=True,imports='3',completed='3',verified_returns='3'))
        shell=dict(schema=1,policy='native-only-rendering-v1',session=a['session'],state='completed',normal_quit=True,renderer_retired=True,fallback=False,error='',launcher_exit=0,restart_requested=False,force_stop_verified=False)
        admission=dict(session=a['session'],native_only=dict(policy=shell['policy']),prefix='owned/prefix')
        process=dict(original_exit_code=0,cleanup_after_original_exit=True,prefix=admission['prefix'])
        def wire(magic,words,size):
            raw=bytearray(size);raw[:8]=magic
            struct.pack_into('<'+str(len(words))+'I',raw,8,*words)
            return bytes(raw)
        inputs=wire(b'MNMINK01',[1,1088,2,0],1088)
        ring=wire(b'MNMRDC03',[3,1048640,7,25,2,0,1,25],1048640)
        control=wire(b'MNMRCV01',[1,576,7,1,3,0,1,1,1,0,2],576)
        head=[0x434d4e4d,0x31574f52,1,64,0x40209ca7,18,32768]+[0]*9
        install=[1,1]+[0]*14;end=[2,10,0,0,0,1,3,2,1]+[0]*7
        cursor=struct.pack('<48I',*(head+install+end))
        graphics=struct.pack('<16I',0x444d4e4d,0x31544552,1,64,1,*([0]*6),2,3,0,0,0)
        return [a,shell,admission,process,inputs,ring,control,cursor,graphics]

    def test_cleanup_needs_all_terminal_owners_and_balanced_work(self):
        result=self.join.cleanup_state(*self.cleanup())
        self.assertTrue(result['verified']);self.assertEqual(result['cursor']['queue_cancelled'],1)
        self.assertEqual(result['published'],result['acknowledged'])

    def test_exit_alone_does_not_certify_cleanup(self):
        for index,key,value in [(0,'private_wine_stopped',False),(1,'renderer_retired',False),(1,'normal_quit',False),(1,'state','failed'),(1,'fallback',True),(1,'restart_requested',True),(1,'force_stop_verified',True),(1,'session','other'),(2,'session','other'),(3,'original_exit_code',1),(3,'cleanup_after_original_exit',False),(3,'prefix','other')]:
            args=self.cleanup();args[index][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.cleanup_state(*args)
        for key,value in [('pending',True),('imports','4'),('verified_returns','2'),('normal_quit_retired',False)]:
            args=self.cleanup();args[0]['resident'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.cleanup_state(*args)

    def test_input_must_be_inactive_stable_and_released(self):
        for offset,value in [(8,2),(12,1084),(16,3),(20,1),(64,0x80000001)]:
            args=self.cleanup();raw=bytearray(args[4]);struct.pack_into('<I',raw,offset,value);args[4]=bytes(raw)
            with self.subTest(offset=offset),self.assertRaises(ValueError):self.join.cleanup_state(*args)

    def test_channel_stop_ack_identity_and_drain_are_required(self):
        for index,offset,value in [(5,20,26),(5,24,1),(5,28,3),(5,32,0),(6,16,8),(6,24,1),(6,32,0),(6,36,2),(6,40,0),(6,48,1),(6,64,1)]:
            args=self.cleanup();raw=bytearray(args[index]);struct.pack_into('<I',raw,offset,value);args[index]=bytes(raw)
            with self.subTest(index=index,offset=offset),self.assertRaises(ValueError):self.join.cleanup_state(*args)

    def test_cursor_and_graphics_must_quiesce_before_reclamation(self):
        for index,offset,value in [(7,128+12,1),(7,128+16,1),(7,128+20,0),(7,128+24,4),(8,20,1),(8,52,1)]:
            args=self.cleanup();raw=bytearray(args[index]);struct.pack_into('<I',raw,offset,value);args[index]=bytes(raw)
            with self.subTest(index=index,offset=offset),self.assertRaises(ValueError):self.join.cleanup_state(*args)

    def test_sixteen_rows_require_distinct_complete_storage_and_cleanup_domains(self):
        plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        quick=[dict(verified=True,domain=(c,size),identity='q'+str(i),pair={},provider='controlled',cleanup=dict(verified=True)) for i,(c,size) in enumerate((c,z) for c in ('player-default','player-software') for z in ((640,480),(800,600)))]
        storage=[dict(r,identity='s'+str(i)) for i,r in enumerate(quick)]
        rows=self.join.cleanup_storage_rows(plan,quick,storage)
        self.assertEqual(sum(r['state']=='passed' for r in rows),16)
        allowed={**self.join.SUSTAINED_LEAVES,**self.join.CLEANUP_LEAVES,**self.join.STORAGE_LEAVES}
        self.assertTrue(all(r['state']=='pending' for r in rows if r['id'] not in allowed))
        for bad in (storage[:-1],storage+[storage[0]],storage[:3]+[dict(storage[-1],identity=quick[0]['identity'])]):
            with self.assertRaises(ValueError):self.join.cleanup_storage_rows(plan,quick,bad)
        quick[-1]['cleanup']['verified']=False
        with self.assertRaises(ValueError):self.join.cleanup_storage_rows(plan,quick,storage)


class LightingCloseoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        SustainedCloseoutTests.setUpClass();cls.join=SustainedCloseoutTests.join

    def state(self):
        cap=8*1024*1024;nonce=b'12345678';seq=3
        inp=bytearray(305);inp[:8]=b'MNMLIT01'
        for at,n in {8:1,12:128,16:len(inp),20:seq,32:1,36:1,40:1,44:1,104:11,108:0x46b8fc,112:1}.items():struct.pack_into('<I',inp,at,n)
        inp[24:32]=nonce;struct.pack_into('<I',inp,120,self.join.lighting_checksum(inp))
        reply=bytearray(85);reply[:8]=b'MNMLRP01';lo,hi=struct.unpack('<2I',nonce)
        struct.pack_into('<6I',reply,8,1,len(reply),seq,lo,hi,struct.unpack_from('<I',inp,120)[0])
        channel=bytearray(128+3*cap);channel[:8]=b'MNMLS001'
        words=[1,cap,6,len(inp),len(reply),0,seq,lo,hi,1,0,0]+[0]*18
        struct.pack_into('<30I',channel,8,*words)
        for index,n in {14:3,15:3,19:3,23:10,24:25,25:0x46b8fc,26:11}.items():struct.pack_into('<I',channel,index*4,n)
        channel[128:128+len(inp)]=inp;channel[128+cap:128+cap+len(reply)]=reply
        names=['calls','applied','fallback','refused','disabled','processed','rejected','compared','mismatches','max_wait_ms','max_compute_us','last_caller','last_clock','last_changed','error','records','trace_active']
        result=dict(success=True,error=None,mode='native',source_bound=True,worker_exit=0,worker_exited_early=False,
                    statistics=dict(zip(names,struct.unpack_from('<17I',channel,56))))
        return [dict(nonce=nonce.hex(),mode='native',capture='none'),result,self.outcome(result),channel]

    def outcome(self,result,duplicate=False):
        db=sqlite3.connect(':memory:')
        db.execute('CREATE TABLE results(payload TEXT)');db.execute('CREATE TABLE lifecycle(pid INTEGER,start_ticks INTEGER,phase TEXT)')
        db.execute('INSERT INTO lifecycle VALUES (123,999,?)',('started',))
        db.execute('INSERT INTO results VALUES (?)',(json.dumps(result),))
        if duplicate:db.execute('INSERT INTO results VALUES (?)',(json.dumps(result),))
        raw=db.serialize();db.close();return raw

    def test_lighting_counts_final_exchange_and_worker_outcome_agree(self):
        row=self.join.lighting_state(*self.state())
        self.assertTrue(row['verified']);self.assertEqual(row['sequence'],3);self.assertEqual(row['terminal_phase'],'closed')
        self.assertEqual(row['caller'],0x46b8fc)

    def test_fallback_missing_application_and_abnormal_worker_do_not_close_lighting(self):
        for field,value in [('success',False),('error','timeout'),('source_bound',False),('mode','observe'),('worker_exit',1),('worker_exited_early',True)]:
            args=self.state();args[1][field]=value;args[2]=self.outcome(args[1])
            with self.subTest(field=field),self.assertRaises(ValueError):self.join.lighting_state(*args)
        for index,name,value in [(14,'calls',4),(15,'applied',2),(16,'fallback',1),(17,'refused',1),(18,'disabled',1),(19,'processed',2),(20,'rejected',1),(21,'compared',1),(28,'error',1),(30,'trace_active',1)]:
            args=self.state();struct.pack_into('<I',args[3],index*4,value);args[1]['statistics'][name]=value;args[2]=self.outcome(args[1])
            with self.subTest(field=name),self.assertRaises(ValueError):self.join.lighting_state(*args)

    def test_foreign_session_pending_request_and_diagnostic_capture_refuse(self):
        for at,value in [(8,2),(16,1),(36,0),(44,0),(48,1),(52,1),(124,1)]:
            args=self.state();struct.pack_into('<I',args[3],at,value)
            with self.subTest(offset=at),self.assertRaises(ValueError):self.join.lighting_state(*args)
        args=self.state();args[2]=self.outcome(args[1],duplicate=True)
        with self.assertRaises(ValueError):self.join.lighting_state(*args)

    def test_final_request_or_reply_mutation_cannot_inherit_application_counts(self):
        cap=8*1024*1024
        for at in [128+20,128+24,128+104,128+108,128+120,128+300,128+cap+16,128+cap+20,128+cap+28,128+cap+32,128+cap+36]:
            args=self.state();args[3][at]^=255
            with self.subTest(offset=at),self.assertRaises(ValueError):self.join.lighting_state(*args)

    def test_selection_and_both_original_resolution_callbacks_are_required(self):
        menu=dict(campaign_selection=dict(verified=True,callbacks=[[3,0],[18,65537],[18,0]],thread=1),display_rebuilds=dict(verified=True,dimensions=[[640,480],[800,600]],callback_sequences=[10,20]))
        self.assertEqual(self.join.lighting_menu_actions(menu,(800,600))[0]['thread'],1)
        for section,key,value in [('campaign_selection','verified',False),('campaign_selection','callbacks',[[3,0]]),('campaign_selection','thread',0),('display_rebuilds','verified',False),('display_rebuilds','dimensions',[[800,600],[640,480]]),('display_rebuilds','callback_sequences',[10,10])]:
            bad=copy.deepcopy(menu);bad[section][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.lighting_menu_actions(bad,(800,600))

    def rows(self):
        plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        quick=[dict(verified=True,domain=(c,z),identity='q'+str(i),pair={},provider='controlled',cleanup=dict(verified=True)) for i,(c,z) in enumerate((c,z) for c in ('player-default','player-software') for z in ((640,480),(800,600)))]
        storage=[dict(r,identity='s'+str(i)) for i,r in enumerate(quick)]
        parent=dict(dossier_identity=self.join.fingerprint(self.join.requirements(plan)),requirements=self.join.cleanup_storage_rows(plan,quick,storage),passed_requirements=16)
        return plan,parent,storage

    def test_exact_twenty_two_rows_preserve_parent_and_leave_other_requirements_pending(self):
        plan,parent,runs=self.rows();before=copy.deepcopy(parent);rows=self.join.lighting_menu_rows(plan,parent,runs)
        self.assertEqual(sum(r['state']=='passed' for r in rows),22);self.assertEqual(parent,before)
        for old,row in zip(parent['requirements'],rows):
            if old['id'] not in self.join.LIGHTING_MENU_LEAVES:self.assertEqual(row,old)

    def test_missing_duplicate_unverified_domain_or_expanded_parent_refuse(self):
        plan,parent,runs=self.rows()
        for bad in [runs[:-1],runs+[runs[0]],runs[:3]+[runs[0]],runs[:3]+[dict(runs[-1],verified=False)],runs[:3]+[dict(runs[-1],identity=runs[0]['identity'])]]:
            with self.assertRaises(ValueError):self.join.lighting_menu_rows(plan,parent,bad)
        parent['requirements'][0]['state']='passed'
        with self.assertRaises(ValueError):self.join.lighting_menu_rows(plan,parent,runs)


class PreferencesResumeCloseoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        SustainedCloseoutTests.setUpClass();cls.join=SustainedCloseoutTests.join

    def case(self):
        def ready(stage,screen,ack,generation):return dict(stage=stage,screen=screen,ack=ack,generation=generation,thread=1)
        def click(stage,action,t):return dict(event='click',stage=stage,action=action,time_ms=t,capture_begin_ms=t,capture_end_ms=t+1,injected_capture_delay_ms=0)
        def marker(stage,n,name):return dict(event='mini-input',stage=stage,source_generation=str(n),record=name)
        states=[ready(21,10,4,16),click(21,'preferencesCancel',1),marker(21,10,'cancel1.json'),ready(22,17,5,22),
                ready(23,10,6,26),click(23,'preferencesCancel',20),marker(23,30,'cancel2.json'),ready(4,17,7,32),
                ready(15,10,14,69),click(15,'preferencesOk',40),ready(16,3,15,73),ready(17,10,16,77),click(17,'preferencesOk',50),
                ready(18,3,17,81),ready(8,12,18,85),click(8,'loadGameLoad',55),marker(9,80,'reload.json'),ready(10,17,19,93)]
        flow=dict(success=True,normal_quit=True,resident_world_requested=True,preferences_cancel_verified=True,display_rebuilds=2,reloaded_from_main=True,native_command_fallback=False,native_error='',native_recoveries=0,states=states,checkpoints=[dict(phase='after-recreate-load',native_active=True,fallback=False,error='',source_generation='80',source_base='60',frames=1000,frame_base=900)])
        records={}
        for name,t in [('cancel1.json',10),('cancel2.json',30),('reload.json',60)]:
            events=[dict(event='escape',down=True,time_ms=t,input_words={'4':10,'5':1}),dict(event='escape',down=False,time_ms=t+1,input_words={'4':12,'5':0}),dict(event='complete',time_ms=t+2,input_words={'4':14,'5':0})]
            records[name]=dict(success=True,mode='mini',skip_click=True,input_active_checked=True,escape_sent=True,viewport_pixels=[800,600],tutorial_actions=[],events=events)
        compared=[dict(world=n,paint=n,pixels=480000,mismatching_pixels=0,actual_sha256='a'*64,expected_sha256='a'*64) for n in [10,11,12,30,31,32,80,81,82]]
        resources=[dict(generation=i,width=800,height=600,creates=1,deletes=0,surfaces=1,created_bytes=960000,deleted_bytes=0,logical_surface_bytes=960000,palette_creates=0,palette_deletes=0,palettes=0) for i in range(1,83)]
        return [flow,records,compared,resources,(800,600)]

    def test_all_three_returns_bind_original_input_fresh_output_and_resource_history(self):
        result=self.join.preferences_resume_state(*self.case())
        self.assertTrue(result['verified']);self.assertEqual(result['presentations'],9)
        self.assertEqual([p['source_marker'] for p in result['returns']],[10,30,80])
        self.assertEqual([p['serialized_source_baseline'] for p in result['returns']],[None,None,60])

    def test_physical_edges_active_input_and_no_scene_click_are_required(self):
        for field,value in [('success',False),('skip_click',False),('input_active_checked',False),('escape_sent',False),('viewport_pixels',[640,480]),('tutorial_actions',[{'action':'click'}])]:
            args=self.case();args[1]['cancel1.json'][field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):self.join.preferences_resume_state(*args)
        for index,key,value in [(0,'5',0),(0,'4',11),(2,'4',8)]:
            args=self.case();args[1]['cancel1.json']['events'][index]['input_words'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.preferences_resume_state(*args)
        args=self.case();args[1]['cancel1.json']['events'][1]['down']=True
        with self.assertRaises(ValueError):self.join.preferences_resume_state(*args)

    def test_original_ack_thread_and_both_cancel_apply_paths_are_required(self):
        for index,key,value in [(3,'ack',4),(3,'thread',2),(7,'screen',3),(17,'ack',18)]:
            args=self.case();args[0]['states'][index][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.preferences_resume_state(*args)
        for index in [1,5,9,12,14,16]:
            args=self.case();args[0]['states'].pop(index)
            with self.subTest(index=index),self.assertRaises((ValueError,KeyError)):self.join.preferences_resume_state(*args)

    def test_cached_partial_or_mismatched_presentations_do_not_close_resume(self):
        args=self.case();args[2].pop()
        with self.assertRaises(ValueError):self.join.preferences_resume_state(*args)
        for key,value in [('world',9),('mismatching_pixels',1),('pixels',0),('actual_sha256','b'*64)]:
            args=self.case();args[2][0][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.preferences_resume_state(*args)
        args=self.case();args[0]['checkpoints'][0]['source_base']='80'
        with self.assertRaises(ValueError):self.join.preferences_resume_state(*args)
        args=self.case();args[0]['resident_world_requested']=False
        with self.assertRaises(ValueError):self.join.preferences_resume_state(*args)
        args=self.case();args[0]['checkpoints'][0]['source_base']='-1'
        with self.assertRaises(ValueError):self.join.preferences_resume_state(*args)

    def test_resumed_resource_generation_and_conservation_must_match(self):
        for key,value in [('generation',8),('width',640),('surfaces',0),('logical_surface_bytes',1),('palettes',1)]:
            args=self.case();args[3][9][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.join.preferences_resume_state(*args)

    def rows(self):
        plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        allowed=set(self.join.SUSTAINED_LEAVES)|set(self.join.CLEANUP_LEAVES)|set(self.join.STORAGE_LEAVES)|set(self.join.LIGHTING_MENU_LEAVES)
        parent=dict(dossier_identity=self.join.fingerprint(self.join.requirements(plan)),passed_requirements=22,requirements=[dict(r,state='passed' if r['id'] in allowed else 'pending') for r in self.join.requirements(plan)])
        runs=[dict(identity=str(i),domain=(c,z),resume=dict(verified=True),pair={},provider='controlled') for i,(c,z) in enumerate((c,z) for c in ('player-default','player-software') for z in ((640,480),(800,600)))]
        return plan,parent,runs

    def test_only_two_resume_rows_are_added_and_all_parent_rows_stay_exact(self):
        plan,parent,runs=self.rows();before=copy.deepcopy(parent);rows=self.join.preferences_resume_rows(plan,parent,runs)
        self.assertEqual(sum(r['state']=='passed' for r in rows),24);self.assertEqual(parent,before)
        for a,b in zip(parent['requirements'],rows):
            if a['id']!=self.join.RESUME_LEAF:self.assertEqual(a,b)

    def test_domain_aliasing_unverified_proof_and_wrong_parent_inventory_refuse(self):
        plan,parent,runs=self.rows()
        for bad in [runs[:-1],runs+[runs[0]],runs[:3]+[runs[0]],runs[:3]+[dict(runs[-1],resume=dict(verified=False))]]:
            with self.assertRaises(ValueError):self.join.preferences_resume_rows(plan,parent,bad)
        passed=next(v for v in parent['requirements'] if v['state']=='passed');pending=next(v for v in parent['requirements'] if v['state']=='pending')
        passed['state']='pending';pending['state']='passed'
        with self.assertRaises(ValueError):self.join.preferences_resume_rows(plan,parent,runs)

if __name__=='__main__':unittest.main()
