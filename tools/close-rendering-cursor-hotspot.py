#!/usr/bin/env python3
"""Close only finite nonzero-hotspot rows after all four actual domains pass."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
from coverage_evidence import Reader,write_report
from cursor_hotspot import ROOT,PREFIX,BID,PARENT,LEAF,REQUIRED_CLASSES,MAX_REQUESTS,MAX_GAMES
from rendering_pilot import require
from rendering_menu_acceptance import artifact
from rendering_interaction import decode as frame_decode
from campaign_input_observation import decode as input_decode
from rendering_gameplay_acceptance import bind_markers
from rendering_selection import cursor_accounting
from native_copy_routes import decode as routes_decode
from copy_caller_provenance import decode as callers_decode


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--comparison',action='append',required=True);p.add_argument('--report',required=True)
    args=p.parse_args();require(args.report.startswith(PREFIX) and not(ROOT/args.report).exists(),'Fresh owned report required')
    result=dict(schema=1,kind='cursor_hotspot_acceptance',success=False,historical_failures_retired=False,cases=[])
    with Reader(ROOT) as reader:
        reg=reader.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors'] if b['id']==BID)
        sources={n:reader.digest(n) for n in required_sources(b)};result['analysis_sources']=sources
        try:
            spec=importlib.util.spec_from_file_location('hotspot_parent',ROOT/'tools/close-rendering-world-exit.py')
            module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            parent,impact=module.parent_plan(ROOT,reader,'SL08.selection-highlight01','NP.selection-highlight-closeout')
            require(parent==reader.json(PARENT) and parent['passed_requirements']==104,'Frozen104-row parent differs')
            domains=set();identities=set()
            for name in args.comparison:
                c=reader.json(name);a=reader.json(c['acquisition']['record']);proof=c['proof'];cfg=a['configuration']
                require(c['success'] and c['sources_stable'] and proof['verified']
                    and c['original_manifest_verified_before_after'] and c['comparison_processes_stopped']
                    and c['comparison']['mismatching_pixels']==0 and proof['cleanup']['verified']
                    and all(reader.digest(n)==h for n,h in c['sources'].items()),'Unaccepted/stale actual hotspot comparison')
                require(reader.digest(c['acquisition']['record'])==c['acquisition']['sha256'],'Changed acquisition')
                for source_name,h in c['sources'].items():
                    require(source_name not in sources or sources[source_name]==h,'Conflicting current cursor execution/analysis source phases')
                    sources[source_name]=h

                frame=frame_decode(artifact(reader,a,'frame-input.bin'))
                inputs=input_decode(artifact(reader,a,'input-admission.bin'))
                bind_markers(a['camera_actions'],inputs)
                require(frame['nonce']==a['resident']['nonce'] and frame['owner']==inputs['owner_thread']
                    and all(v['thread']==frame['owner'] for v in a['states']),'Original input/menu/source owners differ')
                require(c['world_domain']['verified'] and c['world_domain']['extents']==[cfg['actual_size']],
                    'Actual source size differs from the declared domain')
                if cfg['software_flag']:
                    require(a['resident']['renderer'].lower().startswith(('llvmpipe','softpipe')),'Software provider unproved')
                cursor=cursor_accounting(artifact(reader,a,'native-cursor-lifetime.bin'))
                path=Path(a['session'])/'lock-capture/copy-routes-v2.bin'
                require(path.read_bytes()==artifact(reader,a,'copy-routes-v2.bin'),'Changed scalar COPY interchange')
                rh,routes=routes_decode(path)
                layouts=reader.json('research/runtime/copy-caller-results-20261102/layouts01.json')['wrappers']
                for v,length in zip(layouts,[5,5,6,6,7,8,8]):v['hook_length']=length
                _,callers=callers_decode(artifact(reader,a,'copy-callers-v3.bin'),rh,routes,layouts)
                lo,hi=proof['copy_history']['required_copy_interval']
                active=[v[0] for v in routes if v[40] and lo<=v[40]<=hi]
                require(active,'No original calls in cursor interval')
                selected=[v for v,k in zip(routes,callers) if k[17]==3 and min(active)<=v[0]<=max(active)]
                require(selected and all(v[40] and lo<=v[40]<=v[41]<=hi for v in selected),
                    'A scoped original cursor call lacked complete emitted output')
                require(len({g for v in proof['windows'] for g in v['generations']})==42,
                    'Hotspot windows reuse source generations')
                require(all(v['viewport_stencil']['viewport_pixels']>0 and
                    v['viewport_stencil']['actual_sha256']==v['viewport_stencil']['expected_sha256']
                    for v in proof['nonzero']),'Missing displayed original cursor stencil')
                domain=('player-software' if cfg['software_flag'] else 'player-default',tuple(cfg['actual_size']))
                require(domain not in domains and a['session_identity'] not in identities,'Repeated actual domain/session')
                domains.add(domain);identities.add(a['session_identity'])
                require(len(proof['windows'])==14 and len(proof['nonzero'])==18 and
                    all(REQUIRED_CLASSES=={v['cursor_class'] for v in proof['nonzero'] if v['mode']==mode}
                        for mode in ('sharp','integer')),'Missing actual nonzero-hotspot state')
                require(reader.json(name)['acquisition']==c['acquisition'] and name.endswith('.json'),
                    'Emitted comparison reference does not resolve to the checked acquisition')
                result['cases'].append(dict(verified=True,identity=a['session_identity'],domain=[domain[0],list(domain[1])],
                    pair=dict(acquisition=c['acquisition'],comparison=dict(record=name,sha256=reader.digest(name))),
                    provider=a['resident']['renderer'],nonzero_states=18,settled_source_generations=42,
                    world_presentations=c['comparison']['world_presentations'],viewport_pixels=c['comparison']['pixels_compared'],
                    copy_history=proof['copy_history'],cleanup=proof['cleanup'],cursor_journal=cursor,
                    complete_cursor_calls=len(selected),cursor_stencil_pixels=sum(v['viewport_stencil']['viewport_pixels'] for v in proof['nonzero'])))
            require(len(result['cases'])==4 and domains=={(p,s) for p in ('player-default','player-software')
                for s in ((640,480),(800,600))},'Four unique actual domains required')
            attempts=[]
            for path in sorted((ROOT/PREFIX).glob('*-acquisition.json')):
                a=reader.json(path.relative_to(ROOT));require(type(a['original_game_runs']) is int,'Unknown original start')
                attempts.append(dict(record=str(path.relative_to(ROOT)),sha256=reader.digest(path.relative_to(ROOT)),
                    success=a['success'],original_games=a['original_game_runs'],repair_of=a.get('repair_of')))
            require(len(attempts)<=MAX_REQUESTS and sum(v['original_games'] for v in attempts)<=MAX_GAMES,'Original attempt cap exceeded')
            rows=[]
            for old in parent['requirements']:
                row=dict(old)
                if row['id']==LEAF:
                    require(row['state']=='pending','Hotspot row already accepted')
                    row.update(state='passed',admitted_execution_evidence=[dict(execution=v['pair'],
                        actual_world_extents=[v['domain'][1]],provider=v['provider'],reason=b['scope'])
                        for v in result['cases'] if v['domain'][0]==row['configuration']])
                rows.append(row)
            require(sum(r['state']=='passed' for r in rows)==106,'Hotspot promotion exceeds two rows')
            negative=next(e for e in reg['evidence'] if e['id']=='CR07.diagnosis01')
            require(reader.digest(negative['record'])==negative['sha256'],'Lost invalid-rectangle negative')
            require(all(reader.digest(n)==h for n,h in sources.items()),'Closeout sources changed')
            result.update(success=True,analysis_sources_stable=True,scope=b['scope'],attempts=attempts,
                original_game_runs=sum(v['original_games'] for v in attempts),original_requests=len(attempts),
                sources=parent['sources']|{n:h for n,h in sources.items() if n not in parent['sources']},
                claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)
                    for s in reg['scenarios'] if BID in s['behaviors']})],
                parent=dict(record=PARENT,sha256=reader.digest(PARENT)),accepted_parent_applicability=impact,
                historical_invalid_requests=dict(id=negative['id'],record=negative['record'],sha256=negative['sha256']),
                rejected_caller57e957=dict(record=PREFIX+'pilot-diagnosis-01.json',sha256=reader.digest(PREFIX+'pilot-diagnosis-01.json'),fixed=False),
                requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),passed_requirements=106,pending_requirements=50,
                newly_passed_requirements=2,verified_domains=4,required_domains=4,nonzero_source_states=72,
                settled_source_generations=168,mismatching_pixels=0,current_whole_player_proved=False)
        except Exception as error:result['error']=str(error)
    h=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=h)),flush=True)
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
