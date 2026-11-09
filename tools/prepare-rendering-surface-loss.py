#!/usr/bin/env python3
"""Provision private mutable inputs and freeze the independent bounded loss phase."""
import argparse,hashlib,io,json,os,shlex,shutil,subprocess,tarfile
from pathlib import Path
from check_inputs import paths,provision
from coverage_evidence import Reader,write_report
from coverage_claims import required_sources
from rendering_build_binding import native_runtime_source_paths
from rendering_surface_loss import linked_sources,FAULT
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/surface-loss-restore-results-20261109/'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source-root',type=Path,required=True);p.add_argument('--report',required=True)
    p.add_argument('--controls',required=True);p.add_argument('--parent');p.add_argument('--repair-review');a=p.parse_args()
    require(bool(a.parent)==bool(a.repair_review),'Repair phase needs exact parent and retained diagnosis')
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh bounded plan required')
    q=dict(schema=1,kind='native_surface_loss_prospective_plan',success=False,original_game_runs=0,fault_policy=FAULT)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        with Reader(ROOT) as reader:
            parent=reader.json(a.parent) if a.parent else None
            repair=reader.json(a.repair_review) if a.repair_review else None
            if parent:
                require(repair['success'] and repair['kind']=='native_surface_loss_primary_provider_repair_review' and
                    repair['parent_plan']==dict(record=a.parent,sha256=reader.digest(a.parent)) and repair['charged_before']==2 and
                    repair['bounds_preserved']==dict(maximum_requests=6,maximum_original_games=10,maximum_repair_cases=1),
                    'Unreviewed/expanded provider repair')
        selected=paths(a.source_root);require(set(selected)=={'game','assets','prefix'},'Missing shared immutable inputs')
        q['shared_inputs']=provision(ROOT,selected,aliases=not bool(parent))
        game=ROOT/'working/game-nocd'
        if parent:require(game.is_dir() and not game.is_symlink(),'Repair requires the same owned private game source')
        else:
            require(game.is_symlink() and game.resolve()==selected['game'],'Private game destination already exists')
            game.unlink();shutil.copytree(selected['game'],game)
        original=ROOT/'original/Arcane_Nocd/Chaos.exe'
        require(hashlib.sha256(original.read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Original NoCD input differs')
        if parent:require(hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest()==hashlib.sha256(original.read_bytes()).hexdigest(),'Private repair executable drift')
        else:
            shutil.copyfile(original,game/'Chaos.exe');(game/'.retain').write_text('SL09 private mutable source; preserve while experiments/references consume it.\n')
        with Reader(ROOT) as r:
            controls=r.json(a.controls);require(controls['success'] and controls['sources_stable'] and
                all(r.digest(n)==h for n,h in controls['sources'].items()),'Current native target/worker controls required')
            build=ROOT/'working/build/qt-shell';compiler=linked_sources(build,ROOT,'mnm-qt-shell')
            oracle=linked_sources(build,ROOT,'mnm-acceptance-world-history')
            for token in shlex.split((ROOT/'working/original-reference.d').read_text().replace('\\\n',' ')):
                path=Path(token).resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):oracle.add(path.relative_to(ROOT).as_posix())
            require({'tests/acceptance-world-reference.cpp','tests/acceptance-world-history.cpp'}<=oracle,'Incomplete independent oracle source graph')
            b=next(v for v in r.json('research/runtime/coverage/register.json')['behaviors'] if v['id']=='NP.surface-loss-restore-acceptance')
            native=native_runtime_source_paths(ROOT);names=compiler|oracle|native|required_sources(b)|{'tools/check-native-surface-loss.py'}
            hashes={n:r.digest(n) for n in sorted(names)}
            def binary(path,label):
                raw=(ROOT/path).read_bytes();record=a.report.removesuffix('.json')+'-'+label+'.bin'
                return dict(path=path,record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw))
            unique=[]
            for name in sorted(names):
                old=subprocess.run(['git','show','HEAD:'+name],cwd=ROOT,capture_output=True)
                if old.returncode or hashlib.sha256(old.stdout).hexdigest()!=hashes[name]:unique.append(name)
            phase=io.BytesIO()
            with tarfile.open(fileobj=phase,mode='w:gz') as tar:
                for name in unique:
                    body=(ROOT/name).read_bytes();info=tarfile.TarInfo(name);info.size=len(body);info.mode=(ROOT/name).stat().st_mode&0o777
                    tar.addfile(info,io.BytesIO(body))
            record=a.report.removesuffix('.json')+'-unique-sources.tar.gz.bin'
            source_bundle=dict(record=record,sha256=write_report(ROOT,record,phase.getvalue()),members=unique,
                source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                reconstruction='Use exact Git base for unchanged hashed source paths, replace only the listed archive members; verify complete compiler/runtime/controller hashes before executing.')
            pilot='default640-02' if parent else 'default640-01'
            reports={PREFIX+name+'-acquisition.json':dict(software=software,size=size)
                for name,software,size in [(pilot,False,[640,480]),('software640-01',True,[640,480]),
                    ('default800-01',False,[800,600]),('software800-01',True,[800,600])]}
            if parent:
                import sqlite3
                with sqlite3.connect((ROOT/'working/surface-loss/attempts.sqlite3').as_uri()+'?mode=ro',uri=True) as db:
                    rows=list(db.execute('SELECT report,repair_of,charged_games FROM attempts'))
                require(rows==[(repair['failed_acquisition']['record'],None,2)],'Repair cannot reset/replace the existing budget')
                q.update(parent_plan=dict(record=a.parent,sha256=r.digest(a.parent)),repair_review=dict(record=a.repair_review,sha256=r.digest(a.repair_review)),
                    retained_requests=[repair['failed_acquisition']],charged_original_starts_before=2,repair_of=repair['failed_acquisition']['record'])
            else:require(not (ROOT/'working/surface-loss/attempts.sqlite3').exists(),'Independent loss budget already exists')
            q.update(success=True,compiler_sources={n:hashes[n] for n in sorted(compiler)},runtime_sources={n:hashes[n] for n in sorted(native)},
                sources={n:hashes[n] for n in sorted(required_sources(b))},source_phase=source_bundle,
                player_binary=binary('working/build/qt-shell/mnm-qt-shell','player'),
                oracle=dict(native_sources={n:hashes[n] for n in sorted(oracle)},binaries=[binary('working/original-reference','original-reference'),
                    binary('working/build/qt-shell/mnm-acceptance-world-history','original-history')]),
                current_controls=dict(record=a.controls,sha256=r.digest(a.controls)),
                bounds=dict(maximum_requests=6,maximum_original_games=10,primary_cases=4,primary_original_starts=8,
                    repair_cases=1,maximum_repair_original_starts=2,setup_only_requests=1,stop_on_first_failure=True,
                    successful_pilot_comparison_before_expansion=True,case_deadline_seconds=600),
                execution_paths=dict(schema=1,output_root='working/surface-loss/acceptance',budget='working/surface-loss/attempts.sqlite3',reports=reports),
                domains=list(reports.values()),prior_requests=[],pilot=PREFIX+pilot+'-acquisition.json',
                original_pixels_used_as_native_inputs=False,newly_passed_requirements=0,
                scope='Actual ordinary native target loss and terminal/public fresh-generation policy in two profiles/two frame sizes; independent failed-prefix/fresh output. No physical hardware reset, legacy driver recovery, arbitrary race or old-session continuation. Separate original-recovery and supported-player decisions stay pending.',
                failure_policy='Stop after any acquisition or comparison failure. A repair requires the retained failure and concrete reviewed source/configuration preimages; retain every charge and the cumulative6-request/10-start cap. Unknown starts retain the reservation and never earn zero-game credit.')
    except Exception as e:q.update(success=False,error=str(e))
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    sha=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=sha)),flush=True)
    return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
