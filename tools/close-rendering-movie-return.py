#!/usr/bin/env python3
"""Natural movie EOF to painted, focused Main; no movie swap-pixel claim."""
import argparse,hashlib,importlib.util,json,struct,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require
from rendering_menu_acceptance import artifact,validate_artifacts
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocols/python'))
from mnm_protocols import media_v1 as wire

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-movie-return-results-20261106/'
LEAF='journey.enabled-movies.return-to-menu'
REFERENCE_EVIDENCE='NR.movie-frame-production.native-integrated-20261010'
REFERENCE_TOOL='tools/test-native-movie-frames.py'


def parent(reader,evidence,behavior):
    root=reader.root
    result=subprocess.run([sys.executable,str(root/'tools/evidence-applicability.py'),'plan','--root',str(root),
        '--evidence',evidence,'--behavior',behavior,'--require-applicable'],cwd=root,capture_output=True,text=True)
    require(result.returncode==0,'Accepted parent impacted: '+result.stderr)
    plan=json.loads(result.stdout);require(plan['current_scope_preserved'] is True and not plan['source_actions'],'Parent needs targeted revalidation')
    ref=plan['retained_input'];data=reader.json(ref['record'])
    require(reader.digest(ref['record'])==ref['sha256'] and data['success'] is True,'Accepted parent identity differs')
    return data,plan


def references(reader):
    reg=reader.json('research/runtime/coverage/register.json');e=next(v for v in reg['evidence'] if v['id']==REFERENCE_EVIDENCE)
    raw=reader.json(e['record']);require(reader.digest(e['record'])==e['sha256'] and raw['success'] is True
        and raw['original_manifest_verified_before_after'] is True and raw['inputs_stable'] is True
        and raw['original_pixels_used_as_native_inputs'] is False,'Invalid retained independent reference data')
    require(reader.digest(REFERENCE_TOOL)==raw['sources'][REFERENCE_TOOL],'Independent reference algorithm changed')
    selected={}
    for case in raw['cases']:
        name=Path(case['movie']).name.lower()
        if name not in ('intro0.avi','intro1.avi'):continue
        require(name not in selected and case['max_channel_difference']==case['different_channels']==case['tolerance']==0
                and case['native_rgba_sha256']==case['reference_rgba_sha256'] and case['frames']>0,'Incomplete reference sequence')
        selected[name]=dict(input_sha256=raw['inputs'][case['movie']],frames=case['frames'],width=case['width'],height=case['height'],
            rgba_sha256=case['reference_rgba_sha256'],container_slots=case['container_frame_slots'])
    require(set(selected)=={'intro0.avi','intro1.avi'},'Missing independent intro reference')
    return selected,dict(record=e['record'],sha256=e['sha256'],evidence=e['id'],reference_tool_sha256=raw['sources'][REFERENCE_TOOL],
        role='Retained independent expected decoded-output data only. Old native binary/status/source freshness is not promoted. Current delivered sequences are compared to these fixed hashes for identical encoded inputs.')


def media_inputs(reader,a):
    return [json.loads(artifact(reader,a,'native-media-report.json')),artifact(reader,a,'native-media.bin'),
            json.loads(artifact(reader,a,'native-audio-launch-policy.json'))]


def completed_media(a,rows,raw,policy,expected):
    require(set(expected)=={'intro0.avi','intro1.avi'},'Missing expected intro sequence')
    require(policy.get('schema')==1 and all(policy.get(k) is True for k in
        ('native_audio_factory','native_media','native_media_startup_probe','wine_directsound_disabled')),'Wrong native media admission policy')
    require(len(rows)==3 and all(type(v.get('request')) is int and type(v.get('operation')) is int for v in rows)
            and [(v['request'],v['operation'],v.get('status')) for v in rows]==[(1,4,3),(2,1,3),(3,1,3)]
            and rows[0].get('native_pipeline_available') is True,'Missing/extra/reordered/failed media requests')
    movies=[]
    for row,name in zip(rows[1:],('intro0.avi','intro1.avi')):
        ref=expected[name];input_name='FMV/Intro'+name[5]+'.avi'
        require(a['installation_inputs'][input_name]['sha256']==ref['input_sha256'],'Encoded intro input differs from reference')
        path=Path(a['session'])/'game'/'FMV'/('Intro'+name[5]+'.avi')
        require(row.get('asset')==row.get('path')==str(path),'Movie belongs to another session or asset')
        require(row.get('accepted') is True and row.get('error')=='' and row.get('result')==3
                and row.get('video_presentation_requested') is True and row.get('caller_paused') is False,
                'Movie did not complete visible delivery naturally')
        require(type(row.get('video_frames')) is int and row['video_frames']==row.get('presented_video_frames')==ref['frames']
                and (row.get('width'),row.get('height'))==(ref['width'],ref['height'])
                and row.get('video_rgba_sha256')==ref['rgba_sha256'],'Incomplete/changed delivered decoded frame sequence')
        require(row.get('audible_output_requested') is True and row.get('audio_device_available') is True
                and row.get('initial_silent_output') is False and row.get('waiting_for_audio_output') is False
                and row.get('audio_frames',0)>0 and row.get('audio_bytes')==4*row['audio_frames']
                and all(row.get(k)==0 for k in ('output_losses','output_recoveries','default_output_changes')),
                'Unadmitted silence, output loss or incomplete PCM consumption')
        movies.append(dict(request=row['request'],name=name,input_sha256=ref['input_sha256'],decoded_frames=row['video_frames'],
            delivered_frames=row['presented_video_frames'],rgba_sha256=row['video_rgba_sha256'],width=ref['width'],height=ref['height'],
            audio_frames=row['audio_frames'],audio_bytes=row['audio_bytes'],normal_eof=True))
    require(wire.valid_header(raw,len(raw)),'Invalid media retirement channel')
    word=lambda name:struct.unpack_from('<I',raw,getattr(wire,name+'_OFFSET'))[0]
    require(word('REQUEST_SEQUENCE')==6 and word('RESPONSE_SEQUENCE')>0 and word('RESPONSE_SEQUENCE')%2==0
            and word('REQUEST_ID')==word('RESPONSE_ID')==word('ACCEPTED_ID')==3
            and word('OPERATION')==wire.OPERATION_MOVIE and word('RESPONSE_STATUS')==wire.STATUS_COMPLETE
            and word('FLAGS')==word('CANCELLED_REQUEST_ID')==word('SOUND_CANCEL_GENERATION')==0 and word('HEARTBEAT')>0,
            'Unacknowledged/cancelled/hidden/unfinished media request')
    n=word('PATH_LENGTH');require(0<n<=wire.MAX_PATH_LENGTH,'Invalid retained media path')
    path=raw[wire.PATH_OFFSET:wire.PATH_OFFSET+n].decode('ascii').replace('\\','/').lower()
    require(path in ('./fmv/intro1.avi','fmv/intro1.avi') and not any(raw[36:44]+raw[52:64]+raw[324:512]+raw[532:]),
            'Wrong final intro path or unreviewed media wire fields')
    return dict(verified=True,movies=movies,last_request=3,response_sequence=word('RESPONSE_SEQUENCE'),cancelled_request=0,
        decoded_frames=sum(v['decoded_frames'] for v in movies),decoded_pixels=sum(v['decoded_frames']*v['width']*v['height'] for v in movies),
        movie_gpu_output_proved=False,scope='Both native intro requests naturally complete and deliver every decoded RGBA frame; full ordered digests match retained independent expected output for identical encoded assets. Delivery counts are callbacks, not physical movie swaps or whole-viewport pixel comparison.')


def return_state(a,menu,media):
    require(a['success'] is True and a['sources_stable'] is True and a['original_game_runs']==1 and a['exit_code']==0
            and a['private_wine_stopped'] is True and a.get('forced_private_prefix_stop',False) is False
            and a['original_manifest_verified_before_after'] is True
            and a['configuration']['skip_intros'] is False and menu['verified'] is media['verified'] is True,
            'Incomplete nominal natural-return execution')
    state=a['states'][0]
    require(state['screen']==3 and state['ack']==0 and state['ready'] is True and state['thread']>0
            and menu['original_callbacks'][:2]==[[3,3],[10,1]],'Missing initial original Main/Preferences callback return')
    ready=[v for v in menu['main_ready'] if v['label']=='main-ready-initial']
    shown=[v for v in menu['visible_main_presentations'] if v['label']=='main-ready-initial']
    require(len(ready)==len(shown)==1 and ready[0]['state_index']==0 and ready[0]['sha256']==shown[0]['sha256']
            and all(ready[0]['target'][k] is True for k in ('focused','enabled','showing')),'No painted, ready focused initial Main return')
    first=[v for v in a['actions'] if v['action']=='click' and v['name']=='Preferences']
    require(len(first)==1 and first[0]['physical']['down_up'] is True and first[0]['physical']['host_focus']['host_focused'] is True
            and first[0]['host_ns']>=a['screenshots'][0]['host_ns'],'Initial Main public input did not follow the return capture')
    require(menu['original_quit'] is True and menu['unintended_worlds']==0 and a['resident']['pending'] is False
            and a['resident']['normal_quit_retired'] is True
            and int(a['resident']['imports'])==int(a['resident']['completed'])==int(a['resident']['verified_returns'])==0,
            'Menu return started World work or failed normal retirement')
    return dict(verified=True,media=media,initial_main=ready[0],painted_initial_main=shown[0],first_public_input=first[0],
        original_first_callbacks=menu['original_callbacks'][:2],original_thread=state['thread'],normal_quit=True,unintended_worlds=0,
        movie_gpu_output_proved=False,skip_return_proved=False,
        scope='Natural enabled Intro0/Intro1 completion returns to the initial painted, focused native Main, with actual physical Preferences input and original callback effects, then normal Quit with no World. Initial checkpoint output is verified; exact EOF/first-paint simultaneity, movie GPU output, skip-induced return and original DirectShow/audio-device equivalence are separate.')


def close_rows(plan,previous,runs):
    validate(plan);template=requirements(plan)
    require(previous['dossier_identity']==fingerprint(template) and len(previous['requirements'])==156
            and previous['passed_requirements']==sum(v['state']=='passed' for v in previous['requirements'])==36
            and {v['id'] for v in previous['requirements'] if v['state']=='passed'}==set(previous['closed_leaf_ids'])
            and [(v['configuration'],v['id']) for v in previous['requirements']]==[(v['configuration'],v['id']) for v in template],
            'Changed accepted36row inventory')
    require(len(runs)==4 and all(type(v['configuration'][k]) is bool for v in runs for k in ('software_flag','high_resolution'))
            and {(v['configuration']['software_flag'],v['configuration']['high_resolution']) for v in runs}=={(s,h) for s in (False,True) for h in (False,True)}
            and len({v['identity'] for v in runs})==4 and all(v['proof']['verified'] is True for v in runs),'Missing/duplicate natural movie-return domain')
    rows=[]
    for old in previous['requirements']:
        row=dict(old)
        if row['id']==LEAF:
            require(row['state']=='pending','Movie return already passed')
            row.update(state='passed',admitted_execution_evidence=[dict(reason=v['proof']['scope'],execution=v['case'],native_ui_layout=[800,600],original_resolution_selector=int(not v['configuration']['high_resolution']),scope='Native menu High/Low original contexts; no World is entered and no new World-size or movie GPU-output proof follows.') for v in runs if v['configuration']['software_flag'] is (row['configuration']=='player-software')])
        rows.append(row)
    require(sum(v['state']=='passed' for v in rows)==38,'Movie-return closeout scope changed');return rows


def analyze(reader):
    accepted,impact=parent(reader,'WX06.normal-world-exit-closed01','NP.normal-world-exit-closeout')
    menus,menu_impact=parent(reader,'MS06.main-setup-closed01','NP.main-setup-menu-closeout')
    previous=reader.json(accepted['inputs']['progress']['record'])
    require(reader.digest(accepted['inputs']['progress']['record'])==accepted['inputs']['progress']['sha256'],'Accepted progress hash differs')
    selection=menus['inputs']['selection'];selected=reader.json(selection['record'])
    require(reader.digest(selection['record'])==selection['sha256'] and selected['success'] is True,'Unaccepted public case selection')
    expected,reference=references(reader);runs=[];sources={n:reader.digest(n) for n in accepted['sources']}
    spec=importlib.util.spec_from_file_location('movie_return_events',ROOT/'tools/test-menu-observer.py');decoder=importlib.util.module_from_spec(spec);spec.loader.exec_module(decoder)
    for case in selected['cases']:
        a=reader.json(case['record']);require(reader.digest(case['record'])==case['sha256'],'Changed accepted public case')
        require(all(reader.digest(n)==h for n,h in (a['sources']|a['compiler_sources']).items()),'Current movie producer/compiled source changed')
        require(('--software-rendering' in a['command']) is a['configuration']['software_flag'],'Movie provider flag differs')
        calls=[[v['menu_id'],v['argument']] for v in decoder.decode(artifact(reader,a,'events.bin')) if v['event']==3]
        menu=validate_artifacts(reader,a,calls);media=completed_media(a,*media_inputs(reader,a),expected)
        proof=return_state(a,menu,media);runs.append(dict(identity=a['session'],configuration=a['configuration'],case=case,menu_proof=menu,proof=proof))
    for n in ('tools/close-rendering-movie-return.py','tests/test-rendering-movie-return-closeout.py',REFERENCE_TOOL,
              'tools/native_media_acceptance.py','tests/test-native-player-media.py','tests/test-native-media-launch.py'):
        sources[n]=reader.digest(n)
    require(all(reader.digest(n)==h for n,h in sources.items()),'Mixed movie-return source phase')
    rows=close_rows(reader.json('docs/native-rendering-closure.json'),previous,runs)
    return dict(schema=1,kind='native_rendering_natural_movie_return_closeout',success=True,sources=sources,
        accepted_parent=dict(evidence='WX06.normal-world-exit-closed01',binding=impact['retained_input'],applicability=impact),
        menu_parent=dict(evidence='MS06.main-setup-closed01',binding=menu_impact['retained_input'],applicability=menu_impact),
        case_selection=selection,reference=reference,expected_movies=expected,executions=runs,dossier_identity=previous['dossier_identity'],
        requirements=rows,closed_leaf_ids=sorted(set(previous['closed_leaf_ids'])|{LEAF}),passed_requirements=38,pending_requirements=118,
        inherited_current_rows=36,new_movie_return_rows=2,original_game_runs=0,new_decodes=0,new_per_pixel_comparisons=0,
        new_native_code=False,new_game_fixture=False,natural_main_returns=4,complete_movie_requests=8,
        matched_decoded_frames=sum(v['proof']['media']['decoded_frames'] for v in runs),
        decoded_pixels_covered_by_digests=sum(v['proof']['media']['decoded_pixels'] for v in runs),
        initial_main_painted_captures=4,movie_gpu_pixels_compared=0,skip_return_proved=False,
        historical_failures_retired=False,decisions=previous['decisions'],
        scope='Close2natural movie-return rows across4retained public default/software High/Low contexts, preserving36accepted rows. Both intro requests reach normal EOF with exact delivered decoded-frame counts/digests for unchanged inputs and completed uncancelled media acknowledgements; initial Main is painted/focused and accepts physical input with actual original callbacks, no World and normal Quit. Native UI logical800x600. No new game/decode/per-pixel comparison/native code/build/fixture. Movie GPU output, skip return, exact EOF/first-paint simultaneity, original DirectShow/audio-device equivalence, other118rows, both final decisions and known Save admission/historical failures remain separate.')


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh owned result required')
    result=dict(schema=1,kind='native_rendering_natural_movie_return_closeout',success=False,original_game_runs=0)
    try:
        with Reader(ROOT) as reader:result=analyze(reader)
    except Exception as error:result['error']=str(error)
    sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=sha,passed=result.get('passed_requirements'))),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
