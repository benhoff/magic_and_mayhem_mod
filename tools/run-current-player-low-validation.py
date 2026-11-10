#!/usr/bin/env python3
"""Run two prospective low-resolution cases through the public Preferences store."""
import argparse
import hashlib
import importlib.util
import json
import sqlite3
import sys
import time
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
from menu_preferences_store import load
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/whole-player-validation-results-20261110/'
N='research/runtime/rendering-normal-results-20261031/'


def seed_preferences(path,values):
    require(not path.exists() and not path.is_symlink(),'Fresh bounded Preferences input required')
    require(values==[0,-250,1,0,1,0,0],'Unexpected non-resolution preference change')
    body=(json.dumps(dict(schema=1,source_sha256='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168',values=values),indent=2)+'\n').encode()
    path.parent.mkdir(parents=True,exist_ok=True)
    with path.open('xb')as stream:stream.write(body)
    require(load(path)['values']==values,'Public Preferences input was refused')
    return body


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--software',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
    label='software-low-01'if a.software else'default-low-01'
    require(a.report==N+'wp10-'+label+'-acquisition.json'and not(ROOT/a.report).exists(),'Fresh planned low case required')
    with Reader(ROOT)as r:
        plan=r.json(P+'plan-02.json');built=r.json(P+'build-02.json')
        require(plan['success']is built['success']is True and all(r.digest(n)==h for n,h in plan['sources'].items()),'Changed prospective low plan/build')
        require(hashlib.sha256((ROOT/'working/game-nocd/CFG/prefs.cfg').read_bytes()).hexdigest()==plan['original_preferences_sha256'],'Original configured Preferences input changed')
        names=native_runtime_source_paths(ROOT)|{'tools/run-current-player-low-validation.py'}
        before={n:r.digest(n)for n in sorted(names)}
        plan_ref=dict(record=P+'plan-02.json',sha256=r.digest(P+'plan-02.json'));build_ref=dict(record=P+'build-02.json',sha256=r.digest(P+'build-02.json'))
    db=sqlite3.connect(ROOT/'working/whole-player/requests.sqlite3')
    db.execute('BEGIN IMMEDIATE');prior=list(db.execute('SELECT report,state FROM requests'))
    require(4<=len(prior)<6 and all(state=='compared'for _,state in prior),'Six-start total budget or unreviewed predecessor')
    require(not any(('software-low'if a.software else'default-low')in name for name,_ in prior),'Repeated low profile refused')
    db.execute('INSERT INTO requests VALUES(?,?,?,?,?,NULL,NULL)',(a.report,'quick-low',int(a.software),time.time_ns(),'requested'));db.commit()
    spec=importlib.util.spec_from_file_location('current_low_collector',ROOT/'tools/run-rendering-acceptance-pilot.py');collector=importlib.util.module_from_spec(spec);spec.loader.exec_module(collector)
    original_writer=collector.write_report;fixture=None
    def writer(root,path,raw):
        nonlocal fixture
        if path==a.report.removesuffix('.json')+'-player-build.bin':
            target=ROOT/'working/pilot'/Path(a.report).stem/'config/mnm-qt-shell/engine-preferences.json'
            body=seed_preferences(target,plan['low_preferences_values'])
            record=P+label+'-preferences-input.bin';fixture=dict(path=str(target),record=record,sha256=write_report(ROOT,record,body),values=plan['low_preferences_values'],consumer='menu_preferences_store.stage through the ordinary --preferences-store launch option',seeded_before_qt_launch=True)
        if path==a.report:
            q=json.loads(raw);stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in before.items())
            q.setdefault('sources',{}).update(before);q.update(native_runtime_sources=sorted(native_runtime_source_paths(ROOT)),current_player_plan=plan_ref,current_player_build=build_ref,current_player_scenario='quick-low',current_player_profile='player-software'if a.software else'player-default',runtime_sources_stable=stable)
            q['configuration']['initial_preferences_fixture']=fixture
            q['success']=q['success']and stable and fixture is not None;q['sources_stable']=q.get('sources_stable',True)and stable
            raw=(json.dumps(q,indent=2)+'\n').encode();sha=original_writer(root,path,raw)
            db.execute('UPDATE requests SET state=?,result_sha256=? WHERE report=?',('acquired'if q['success']else'failed',sha,path));db.commit();return sha
        return original_writer(root,path,raw)
    collector.write_report=writer
    sys.argv=[str(ROOT/'tools/run-rendering-acceptance-pilot.py'),'--scenario','normal-quick','--report',a.report]+(['--software']if a.software else[])
    try:return collector.main()
    finally:db.close()

if __name__=='__main__':raise SystemExit(main())
