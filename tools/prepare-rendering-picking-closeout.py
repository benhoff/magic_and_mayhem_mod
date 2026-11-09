#!/usr/bin/env python3
"""Freeze the two-profile picking task after the separately verified VC08 repair."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract, scenario_contract, required_sources
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
from picking_closeout import PREFIX, CONTROLS, BID, SID, preserved_pilot

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    parser.add_argument('--retained-pilot')
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),
            'Fresh owned plan required')
    require(not ROOT.is_relative_to('/tmp'),
            'Isolated session and prefix roots must survive the masked /tmp boundary')
    with Reader(ROOT) as r:
        old_name = 'research/runtime/viewport-cursor-results-20261108/input-plan-05.json'
        old = r.json(old_name)
        for section in (old['compiler_sources'], old['oracle']['native_sources']):
            require(all(r.digest(n) == h for n, h in section.items()),
                    'Retained player/original-reference source changed')
        for b in [old['player_binary'], *old['oracle']['binaries']]:
            require(r.digest(b['record']) == b['sha256'] ==
                    hashlib.sha256((ROOT/b['path']).read_bytes()).hexdigest(),
                    'Exact retained executable differs')
        register = r.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id'] == BID)
        scenario = next(s for s in register['scenarios'] if s['id'] == SID)
        names = native_runtime_source_paths(ROOT) | CONTROLS | required_sources(behavior)
        sources = {n: r.digest(n) for n in sorted(names)}
        preimages = {}
        for name, digest in (sources | old['compiler_sources'] | old['oracle']['native_sources']).items():
            record = PREFIX+'source-'+digest+'.bin'
            require(write_report(ROOT, record, (ROOT/name).read_bytes()) == digest,
                    'Changed source snapshot')
            preimages[name] = dict(record=record, sha256=digest)
        result = dict(schema=1, kind='picking_closeout_prospective_plan', success=True,
            sources={n:sources[n] for n in sorted(CONTROLS)}, runtime_sources=sources,
            source_preimages=preimages, compiler_sources=old['compiler_sources'],
            player_binary=old['player_binary'], oracle=old['oracle'],
            retained_binary_parent=dict(record=old_name, sha256=r.digest(old_name)),
            source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
            claims=[dict(behavior=BID, contract_sha256=behavior_contract(
                behavior, {b['id']:b for b in register['builds']}),
                scenarios={SID:scenario_contract(scenario)})],
            bounds=dict(maximum_requests=4, maximum_original_games=3, primary_games=2,
                stop_on_first_failure=True, successful_pilot_comparison_before_expansion=True),
            domains=[dict(software=s, actual_sizes=[[800,600],[640,480],[800,600]],
                scaling=['sharp','integer'], windows=48, displayed_witnesses=144) for s in (False,True)],
            map='Celtic Forest One (small)', actor=dict(slot=0,type=0,owner=0),
            previous_caps_preserved=True,
            independence='New user-authorized picking closeout after separately accepted VC08 handoff repair. Every PR07/HS08/VC08 failure and exhausted cap remains unchanged. This plan counts every new request and start; software expansion requires a qualified default comparison.',
            scope=behavior['scope'], original_game_runs=0,
            temporary_outputs='Cold-owned source/session/prefix, RAM capture planes, compiler caches, pooled resettable environments and derived reference planes. Source changes, exact binaries, configurations, raw observations, failed attempts and SQLite results must receive a verified durable paired handoff before closeout.')
        if args.retained_pilot:
            compared=r.json(args.retained_pilot)
            acquired=r.json(compared['acquisition']['record'])
            require(r.digest(compared['acquisition']['record'])==compared['acquisition']['sha256'],
                    'Retained pilot acquisition identity differs')
            changes={n:dict(before=acquired['sources'].get(n),after=h)
                     for n,h in sources.items() if acquired['sources'].get(n)!=h}
            preserved_pilot(acquired['sources'],sources,changes,compared)
            old_plan=r.json(acquired['prospective_plan']['record'])
            require(r.digest(acquired['prospective_plan']['record'])==acquired['prospective_plan']['sha256'],
                    'Old prospective plan identity differs')
            for name,change in changes.items():
                before=old_plan['source_preimages'][name]
                require(r.digest(before['record'])==before['sha256']==change['before'],
                        'Missing exact old host source preimage')
                change.update(before_record=before['record'],after_record=preimages[name]['record'])
            # The machine admission compares only the exact before/after pair;
            # separate pointers retain both full source versions for review.
            result['retained_pilot']=dict(acquisition=compared['acquisition'],
                comparison=dict(record=args.retained_pilot,sha256=r.digest(args.retained_pilot)),
                source_changes={n:{k:v[k]for k in('before','after')}for n,v in changes.items()},
                source_preimages=changes,
                scope='Only named post-retirement host/analyzer controls changed. Native/runtime/compiler/oracle bytes, input domains, captures and request/start charges remain unchanged; current retained-data analysis supplies the pilot proof.')
        digest = write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
        # The resolver opened before a new staging database exists cannot see
        # that database; verify through a fresh invocation-scoped reader.
        with Reader(ROOT) as fresh:
            require(fresh.json(args.report) == result, 'Plan read-back differs')
    print(json.dumps(dict(success=True,record=args.report,sha256=digest)))


if __name__ == '__main__': main()
