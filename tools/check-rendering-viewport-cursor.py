#!/usr/bin/env python3
"""Independently compare every viewport and validate exact edge/rebuild witnesses."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import tarfile
import numpy as np
from comparison_process import Processes
from coverage_evidence import Reader, write_report
from rendering_interaction import decode, signed
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, compare, inflate, require
from resident_capture import records
from viewport_cursor import PREFIX, CONTROLS, SIZES, edge_points, cursor_ready, physical_point, hidden_copy_witness

ROOT = Path(__file__).resolve().parents[1]


def module(name):
    spec=importlib.util.spec_from_file_location('viewport_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def assess(reader, a, c, observed):
    require(a['success'] and a['sources_stable'] and a['private_wine_stopped']
            and a['prefix_pool']['returned'] and not a.get('forced_private_prefix_stop')
            and a['original_game_runs']==1, 'Incomplete nominal acquisition')
    require(a['configuration']['actual_size_schedule']==SIZES
            and not a['configuration'].get('diagnostic_only',False)
            and a['configuration']['skip_intros'] is False
            and a['configuration']['terrain_lighting']=='native'
            and ('--software-rendering' in a['command']) is a['configuration']['software_flag'],
            'Wrong requested configuration')
    cleanup=module('join-rendering-normal-acceptance').cleanup_proof(reader,a)
    require(cleanup['verified'] and cleanup['normal_quit'], 'Normal native owner retirement is missing')
    resident=a['resident']
    require(resident['normal_quit_retired'] and not resident['pending'] and not resident['error']
            and int(resident['source_copy_rect_refusals'])==0,
            'Unresolved original/native copy or resident failure')
    if a['configuration']['software_flag']:
        require(resident['renderer'].lower().startswith(('llvmpipe','softpipe')),
                'Actual software rendering provider is missing')
    hidden=hidden_copy_witness(artifact(reader,a,'game/native-hidden-spellbox-copy.bin'))
    rows=decode(artifact(reader,a,'frame-input.bin'))['rows']
    producer=module('inspect-canvas-producers'); cameras={}; extents={}; source_count=chunks=0
    for generation, chunk, stream, request in records(Path(a['temporary_root'])/'worlds.bin'):
        _, source_rows=producer.decode(stream,complete=False)
        if chunk: chunks+=1;continue
        source_count+=1
        camera=[struct.unpack_from('<37I',b) for f,b in source_rows if f[2]==24]
        if camera:
            require(all(m[9]==camera[0][9] and m[3:5]==camera[0][3:5] for m in camera),
                    'Source orientation/center disagrees between producers')
            cameras[generation]=dict(view=camera[0][9],center=list(camera[0][3:5]),
                                     map_size=list(camera[0][1:3]))
    world_rows=c['world_domain']['worlds']
    require(source_count==len(world_rows)==c['original_reference']['worlds']
            and chunks==c['original_reference']['source_chunks'], 'Incomplete original source history')
    extents={r['generation']:[r['width'],r['height']] for r in world_rows}
    matches={r['paint']:r for r in c['comparison']['comparisons']}
    pointers=[v for v in a['actions'] if v['action']=='pointer']
    phases=[];previous=0
    windows=a['viewport_cursor']['windows']
    for label,size in zip(('high-initial','low','high-returned'),SIZES):
        checked=[]
        for mode in ('sharp','integer'):
            for edge,point in edge_points(size):
                name=label+'-'+mode+'-edge-'+edge
                found=[w for w in windows if w['name']==name]
                require(len(found)==1, 'Missing/duplicate edge action: '+name)
                w=found[0];begin,released,settled=(w[k] for k in ('begin','released','settled'))
                require(previous<=int(begin['host_us'])<=int(released['host_us'])<int(settled['host_us']),
                        'Reordered viewport action')
                previous=int(settled['host_us'])
                chosen=[v for v in pointers if int(begin['host_us'])<=v['host_ns']//1000<=int(released['host_us'])]
                require(len(chosen)==1 and chosen[0]['logical']==point and w['size']==size,
                        'Missing exact physical edge action')
                require(len(w['witnesses'])==3 and len({v['generation'] for v in w['witnesses']})==3,
                        'Three distinct displayed edge generations required')
                baseline=rows[w['baseline_row']-1]
                for fact in w['witnesses']:
                    paint_id=fact['paint'];require(paint_id in observed['swaps'],'Edge paint was not swapped')
                    paint,body=observed['paints'][paint_id]
                    require(int(released['host_us'])<int(paint['host_us'])<=int(settled['host_us'])
                            and paint['scaling']==(0 if mode=='sharp' else 2)
                            and extents[fact['generation']]==size, 'Wrong edge paint time/mode/extent')
                    actual=cursor_ready(rows[:fact['ordinal']],paint,point,baseline)
                    require(actual==fact and physical_point(chosen[0],paint,size)==point,
                            'Original pointer/source witness differs')
                    match=matches[paint_id]
                    raw=body() if callable(body) else body
                    require(match['actual_sha256']==match['expected_sha256']==hashlib.sha256(raw).hexdigest()
                            and match['mismatching_pixels']==0 and fact['generation'] in cameras,
                            'Edge lacks complete independent displayed output/camera')
                checked.append(dict(label=edge,scaling=mode,verified=True,
                                    generations=[v['generation'] for v in w['witnesses']],witnesses=w['witnesses']))
        phases.append(dict(size=size,windows=checked))
    rebuilt=[]
    require(len(a['viewport_cursor']['rebuilds'])==2,'Two actual display rebuilds required')
    for old,new,r in zip(SIZES,SIZES[1:],a['viewport_cursor']['rebuilds']):
        x,y=r['original_before'],r['original_after'];paint=r['confirmed_paint']
        require(r['before']==old and r['after']==new and r['same_original_process']
                and r['same_world'] is False and x['thread']==y['thread']
                and x['frame']['nonce']==y['frame']['nonce']
                and x['frame']['generation']<y['frame']['generation']
                and x['high_resolution']==int(old[0]==800) and y['high_resolution']==int(new[0]==800)
                and [paint['frame_width'],paint['frame_height']]==new
                and int(paint['paint']) in observed['swaps']
                and r['preferences']['screen']==10 and r['returned']['screen']==3,
                'Rebuild is not bound to original selector/source/display continuity')
        rebuilt.append(dict(verified=True,before=old,after=new,
                            first_generation=y['frame']['generation'],paint=paint['paint']))
    limits=[]
    require(len(a['viewport_cursor']['camera_limits'])==8,'Four camera directions at both sizes required')
    for window in a['viewport_cursor']['camera_limits']:
        before,after=window['before'],window['after']
        samples=[r for r in rows if r['kind']==1 and r['frame']
                 and before['ordinal']<=r['ordinal']<=after['ordinal']]
        require(len(samples)>=3 and before['camera']!=after['camera'],'Camera direction did not move')
        for row in samples:
            context=cameras.get(row['frame']['generation'])
            require(context and context['view']==row['camera'][0], 'Camera limit source orientation differs')
            cells=list(map(signed,row['camera'][1:3]))
            require(all(-5<=cells[i]<=context['map_size'][i]+5 for i in (0,1)),
                    'Camera exceeded the recovered finite map bounds')
        limits.append(dict(name=window['name'],samples=len(samples),
                           minimum=[min(signed(r['camera'][i+1]) for r in samples) for i in (0,1)],
                           maximum=[max(signed(r['camera'][i+1]) for r in samples) for i in (0,1)]))
    return dict(verified=True,software=a['configuration']['software_flag'],phases=phases,
                rebuilds=rebuilt,camera_limits=limits,normal_quit=True,cleanup=cleanup,hidden_redraw=hidden,
                source_generations=source_count,source_chunks=chunks,
                mismatching_pixels=c['comparison']['mismatching_pixels'],
                scope='Finite physical full-frame edges/corners under Sharp/Integer at800/640/800, original completed cursor polls and fresh compared displays across actual Main Preferences recreation in one process. Four directional camera samples stay inside recovered map bounds. No same-World continuity, arbitrary clamp saturation, picking, physical scanout or original-driver claim.')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True)
    args=p.parse_args()
    require(args.report.startswith(PREFIX) and not(ROOT/args.report).exists(),'Fresh immutable comparison required')
    out=ROOT/'working/viewport-cursor-comparisons'/Path(args.report).stem
    out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
    result=dict(schema=1,kind='public_native_interaction_comparison',success=False,
                original_pixels_used_as_native_inputs=False)
    processes=Processes();before=False
    with Reader(ROOT) as reader:
        a=reader.json(args.acquisition);plan=reader.json(a['prospective_plan'])
        result['claims']=plan['claims']
        result['acquisition']=dict(record=args.acquisition,sha256=reader.digest(args.acquisition))
        sources=a['sources']|plan['oracle']['native_sources']|plan['sources']
        sources.update({n:reader.digest(n) for n in CONTROLS})
        result['sources']=sources
        def execute(label,argv):
            with (out/(label+'.log')).open('xb') as log:
                processes.run(list(map(str,argv)),cwd=ROOT,stdout=log,timeout=900)
        try:
            require(a['success'] and a['sources_stable'] and all(reader.digest(n)==h for n,h in sources.items()),
                    'Incomplete/changed acquisition source phase')
            for binary in [a['player_binary'],*plan['oracle']['binaries']]:
                require(reader.digest(binary['record'])==binary['sha256']==
                        hashlib.sha256((ROOT/binary['path']).read_bytes()).hexdigest(),'Changed tested executable')
            streams=[artifact(reader,a,Path(v['path']).name) for v in a['artifacts']
                     if '/capture/' in v['path'] and v['path'].endswith('.bin')]
            observed=acquisition(streams,a['session_identity'])
            result['captured_command_stream_sha256']=hashlib.sha256(observed['commands']).hexdigest()
            result['observations']=observed['observations']
            result['world_domain']=module('rendering_normal_acceptance').world_domain(observed)
            result['lighting']=module('rendering_normal_acceptance').lighting(
                ROOT,a,{v['path']:v for v in a['artifacts']})
            source=Path(a['temporary_root'])/'worlds.bin'
            require(source.read_bytes()==artifact(reader,a,'worlds.bin'),'Source capture changed')
            execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
            assets=Path(a['session'])/'game'
            extensions={'.spr','.jpg','.jpeg','.bmp','.pcx','.sft','.fp','.ttd','.wav'}
            asset_inputs={str(f.relative_to(assets)):hashlib.sha256(f.read_bytes()).hexdigest()
                          for f in sorted(assets.rglob('*')) if f.is_file() and f.suffix.lower() in extensions}
            require(asset_inputs and all(a['installation_inputs'][n]['sha256']==h
                                        for n,h in asset_inputs.items()),'Reference asset differs')
            result['asset_inputs']=asset_inputs
            reference=out/'reference'
            execute('original-history',[ROOT/'working/build/qt-shell/mnm-acceptance-world-history',source,
                assets,ROOT/'working/original-reference',ROOT/'original/Arcane_Nocd/Chaos.exe',reference])
            index=json.loads((reference/'worlds.json').read_text())
            require(index['success'] and not index['native_player_pixels_read']
                    and not index['original_destination_capture_read'],'Dependent reference')
            worlds={}
            for row in index['worlds']:
                generation=int(row['generation'])
                require(generation==len(worlds)+1 and Path(row['path']).name==row['path'],'Original World order')
                raw=inflate((reference/row['path']).read_bytes())
                worlds[generation]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
            result['original_reference']=dict(worlds=len(worlds),source_chunks=int(index.get('source_chunks','0')),
                continuous_from_owned_initial_history=True,native_player_pixels_read=False,original_destination_capture_read=False)
            result['comparison']=compare(observed,worlds)
            require(result['comparison']['success'],'Independent viewport pixels differ')
            result['proof']=assess(reader,a,result,observed)
            result['asset_inputs_stable']=all(hashlib.sha256((assets/n).read_bytes()).hexdigest()==h
                                              for n,h in asset_inputs.items())
            require(result['asset_inputs_stable'],'Reference assets changed during comparison')
            result['success']=result['proof']['verified']
        except Exception as exc:
            result['error']=str(exc)
        finally:
            if before:
                execute('original-after',[ROOT/'tools/original-manifest.sh','verify'])
                result['original_manifest_verified_before_after']=True
            result['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
            result['comparison_processes_stopped']=processes.stopped()
            result['success'] &= result['sources_stable'] and result['comparison_processes_stopped']
            files=sorted(f for f in out.rglob('*') if f.is_file())
            bundle=out/'outputs.tar.gz'
            with tarfile.open(bundle,'w:gz',compresslevel=1) as archive:
                for f in files: archive.add(f,arcname=str(f.relative_to(out)),recursive=False)
            name=args.report.removesuffix('.json')+'-outputs.bin';raw=bundle.read_bytes()
            result['artifacts']=[dict(record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw),encoding='tar-gzip')]
            digest=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
            print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=digest)),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
