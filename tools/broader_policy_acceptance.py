"""Verify native finite-edge certificates without granting raw-original equivalence."""
import hashlib,io,json,tarfile
from rendering_pilot import require
POLICY='native-retain-outside-displacement-v1'

def assess(reader,comparison):
    policy=comparison['native_edge_policy']
    require(policy['id']==POLICY and policy['all_viewport_pixels_compared'] is True and policy['ignored_pixels']==0
            and policy['raw_original_result_preserved'] is True and policy['original_equivalence_for_retained_taps'] is False
            and policy['production_renderer_changed'] is False and policy['original_game_runs_added']==0,
            'Unknown or weakened native edge comparison')
    def retained(ref):
        require(reader.digest(ref['record'])==ref['sha256'],'Changed edge-policy dependency')
        return reader.json(ref['record'])
    raw=retained(policy['raw_original_comparison']);build=retained(policy['oracle_build']);controls=retained(policy['controls'])
    require(raw['acquisition']==comparison['acquisition'] and raw['sources_stable'] and raw['comparison_processes_stopped']
            and raw['original_manifest_verified_before_after'] and len(raw['epochs'])==2,'Unbound raw-original result')
    require(build['success'] and controls['success'] and controls['cases']==26 and controls['original_game_runs']==0
            and controls['guard_sha256']==reader.digest('tools/broader_finite_edge_oracle.hpp')
            and all(reader.digest(n)==h for n,h in build['sources'].items()),'Stale or untested policy oracle')
    require(reader.digest(build['binary']['record'])==build['binary']['sha256'],'Policy oracle binary not retained')
    require(policy['raw_original_mismatching_pixels']==sum(e['comparison']['mismatching_pixels']for e in raw['epochs']),
            'Raw-original negative evidence count changed')
    artifacts=comparison['artifacts'];require(len(artifacts)==1 and artifacts[0]['encoding']=='tar-gzip','Expected exact comparison archive')
    archive=artifacts[0];require(0<=archive['bytes']<=512*1024**2,'Policy archive budget')
    body=reader.read_bytes(archive['record']);require(len(body)==archive['bytes'] and hashlib.sha256(body).hexdigest()==archive['sha256'],'Changed comparison archive')
    members={v['path']:v for v in archive['members']};require(len(members)==len(archive['members']),'Duplicate comparison archive identity')
    certificates=[]
    with tarfile.open(fileobj=io.BytesIO(body),mode='r:gz')as source:
        actual_members=source.getmembers();require(len(actual_members)==len(members) and len({v.name for v in actual_members})==len(members) and {v.name for v in actual_members}==members.keys(),'Comparison archive membership differs')
        for index,(epoch,parent)in enumerate(zip(comparison['epochs'],raw['epochs'])):
            require(epoch['root']==parent['root'] and epoch['world_domain']==parent['world_domain']
                    and epoch['observations']==parent['observations'] and epoch['asset_inputs']==parent['asset_inputs']
                    and epoch['comparison']['success'] and epoch['comparison']['mismatching_pixels']==0,
                    'Policy comparison changed acquisition scope or contains mismatches')
            reference=epoch['original_reference'];require(reference['reference_model']=='original-rasters-with-explicit-native-finite-edge'
                    and reference['original_equivalence_for_padding'] is False,'Policy oracle labeled raw-original equivalence')
            count=reference['worlds'];affected=[];total=0
            index_name='epoch-'+str(index)+'/reference/worlds.json';index_entry=members[index_name]
            require(index_entry['bytes']<=4*1024**2,'Policy reference index budget')
            index_stream=source.extractfile(index_name);require(index_stream is not None,'Missing policy reference index')
            index_bytes=index_stream.read(4*1024**2+1)
            require(len(index_bytes)==index_entry['bytes'] and hashlib.sha256(index_bytes).hexdigest()==index_entry['sha256'],'Changed policy reference index')
            index_value=json.loads(index_bytes);worlds=index_value['worlds']
            require(index_value['success'] and len(worlds)==count and [int(v['generation'])for v in worlds]==list(range(1,count+1))
                    and len(epoch['world_domain']['extents'])==1 and all([v['width'],v['height']]==epoch['world_domain']['extents'][0]for v in worlds),'Changed policy source extent/order')
            names={n for n in members if n.startswith('epoch-'+str(index)+'/reference/')and n.endswith('.finite-edge.json')}
            require(names=={'epoch-'+str(index)+'/reference/world-%06d.565.finite-edge.json'%g for g in range(1,count+1)},'Missing or additional policy World certificate')
            for generation in range(1,count+1):
                name='epoch-'+str(index)+'/reference/world-%06d.565.finite-edge.json'%generation;entry=members[name]
                require(entry['bytes']<=1024**2,'Policy certificate budget');stream=source.extractfile(name);require(stream is not None,'Missing policy certificate bytes');data=stream.read(1024**2+1)
                require(len(data)==entry['bytes'] and hashlib.sha256(data).hexdigest()==entry['sha256'],'Changed policy certificate')
                value=json.loads(data);require(value['schema']==1 and value['policy']==POLICY and value['original_padding_equivalence_claimed'] is False,'Unknown certificate policy')
                world=worlds[generation-1];size=world['width']*world['height'];seen=set()
                for tap in value['taps']:
                    ordinal,destination,sample,word=(tap[n]for n in('draw','destination','outside_sample','retained_word'))
                    require(all(type(v)is int for v in(ordinal,destination,sample,word)) and 1<=ordinal<=12320
                            and 0<=destination<size<=sample and 0<=sample-destination<=16 and 0<=word<=65535
                            and (ordinal,destination)not in seen,'Invalid finite-edge coverage certificate');seen.add((ordinal,destination))
                total+=len(value['taps'])
                if value['taps']:affected.append(dict(world=generation,taps=value['taps']))
            certificates.append(dict(epoch=index,world_certificates=count,retained_taps=total,affected_worlds=affected))
    require(certificates==policy['certificates'],'Policy summary differs from retained per-draw certificates')
    return dict(verified=True,id=POLICY,raw_original_comparison=policy['raw_original_comparison'],
                raw_original_mismatching_pixels=policy['raw_original_mismatching_pixels'],
                original_equivalence_for_retained_taps=False,ignored_pixels=0,certificates=certificates,
                oracle_sources=build['sources'])
