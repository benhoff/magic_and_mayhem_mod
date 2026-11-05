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

if __name__=='__main__':unittest.main()
