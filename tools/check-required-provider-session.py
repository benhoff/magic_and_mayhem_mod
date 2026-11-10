#!/usr/bin/env python3
"""Join actual provider calls/populated owners with existing local body proofs."""
import argparse,hashlib,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
from rendering_menu_acceptance import artifact
from required_boundary_observation import PREFIX,decode,storage_summary
from campaign_map_resources import assess as resource_assess
ROOT=Path(__file__).resolve().parents[1]

def provider_lifetimes(d):
    require(all(d['counts'][n]>0 for n in ('main-prepare','main-release','report-prepare','report-release',
            'world-enter','world-exit','realm-enter','realm-exit','region-prepare','region-release')),
            'Missing required actual screen-provider route')
    main=[];reports=[]
    for e,r in d['pairs']:
        if e[2]==2:
            require(r[7]==0x5c63e4 and r[56]==6 and all(r[57:62]) and r[62]==0
                    and r[80] and r[83]==r[6] and r[82]==0x4a7850,
                    'Main preparation lacks populated controls/owned callback')
            main.append(dict(entry=e[0],returned=r[0],owner=r[6],controls=list(r[57:63]),callback=r[52]))
        elif e[2]==4:
            require(not any(r[j]for j in (40,50,52,54,55)) and r[43]==r[44]
                    and r[56]==6 and not any(r[57:63]),'Main release leaves live members')
        elif e[2]==10:
            require(r[7]==0x5c5ebc and r[56]==21 and sum(bool(v)for v in r[57:78])>=20
                    and r[84] and r[87]==r[6] and r[86]==0x4747a0,
                    'Report preparation lacks labels/owned callback')
            reports.append(dict(entry=e[0],returned=r[0],owner=r[6],labels=list(r[57:78]),callback=r[54]))
        elif e[2]==12:
            require(not any(r[j]for j in (50,52,53,54,89,90)) and r[43]==r[44],
                    'Report release leaves live labels/button/callback/temporary arrays')
    require(main and reports,'No real populated provider pairs')
    return dict(main=main,reports=reports,counts=d['counts'],normal_pairs=len(d['pairs']),
        realm='Entry-only tail observations corroborated by original coordinator callback journal; no inferred indirect return cleanup',
        scope='Required observed Main/Realm/World/Report path only; original helper endpoints remain permitted suppliers. No all-path Windows exception or physical-driver recovery.')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not(ROOT/a.report).exists(),'Fresh analysis required')
    result=dict(schema=1,kind='required_provider_session_analysis',success=False,original_game_runs=0,
        new_pixel_comparisons=0,scope=__doc__)
    with Reader(ROOT)as reader:
        q=reader.json(a.acquisition);plan=reader.json(q['prospective_plan'])
        result['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
        sources=q['sources']|{n:reader.digest(n)for n in ('tools/check-required-provider-session.py','tools/required_boundary_observation.py','tools/campaign_map_resources.py')}
        result['sources']=sources
        try:
            require(q['success'] and q['sources_stable'] and q['private_wine_stopped']
                    and q.get('prefix_pool',{}).get('returned') and q['original_game_runs']==1,
                    'Incomplete provider acquisition/retirement')
            require(all(reader.digest(n)==h for n,h in sources.items()),'Provider source phase changed')
            require(q['configuration']['evidence_mode']=='diagnostic' and q['semantic_route']['semantic_route_verified']
                    and q['original_callbacks']['verified'],'Missing actual original route')
            d=decode(artifact(reader,q,'required-boundaries-v1.bin'))
            result['lifetimes']=provider_lifetimes(d)
            resources=resource_assess(artifact(reader,q,'map-resource-boundaries.bin'),q['flow'])
            require(resources['verified'] and resources['world_epochs']==2,'Missing actual populated World retire/reacquire')
            result['world_resources']=resources;result['storage']=storage_summary(d)
            retired=q['retirement']['graphics_retirement']
            require(retired['process_exit'] and retired['resources_released'] and not retired['surviving_owners']
                    and all(v==0 for v in retired['counts'].values()),'Surviving native provider owners')
            result['proof']=dict(verified=True,profile=q['configuration']['id'],normal_quit=True,
                populated_owner_lifetimes=True,pixel_admission=False)
            result['success']=True
        except Exception as e:result['error']=str(e)
        finally:
            result['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
            result['success'] &= result['sources_stable']
            h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'))))
    return not result['success']
if __name__=='__main__':raise SystemExit(main())
