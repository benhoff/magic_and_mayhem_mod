#!/usr/bin/env python3
"""Freeze installed font scout inputs and independent two-start budget."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
from coverage_evidence import Reader,write_report
from check_inputs import provision,paths
from rendering_build_binding import native_runtime_source_paths

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-lifetime-results-20261108/'
PARENT='research/runtime/rendering-copy-natural-results-20261108/input-plan-10.json'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source-root',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('Fresh owned plan required')
    q=dict(schema=1,kind='installed_font_lifetime_plan',success=False,original_game_runs=0,newly_passed_requirements=0)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        selected=paths(a.source_root)
        if set(selected)!={'game','assets','prefix'}:raise ValueError('Missing verified shared inputs')
        q['shared_inputs']=provision(ROOT,selected)
        game=ROOT/'working/game-nocd'
        if not game.is_symlink() or game.resolve()!=selected['game']:raise ValueError('Private source must replace only the newly provisioned alias')
        game.unlink();shutil.copytree(selected['game'],game)
        original=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if hashlib.sha256(original.read_bytes()).hexdigest()!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':raise ValueError('Original input differs')
        shutil.copyfile(original,game/'Chaos.exe')
        with Reader(ROOT) as reader:
            old=reader.json(PARENT)
            if any(reader.digest(n)!=h for n,h in old['compiler_sources'].items()):raise ValueError('Retained player compiler drift')
            player=old['player_binary'];binary=ROOT/player['path'];binary.parent.mkdir(parents=True,exist_ok=True)
            raw=reader.read_bytes(player['record'])
            if hashlib.sha256(raw).hexdigest()!=player['sha256']:raise ValueError('Retained tested player differs')
            if binary.exists():raise ValueError('Existing tested binary must be retained')
            binary.write_bytes(raw);binary.chmod(0o755)
            names=set(old['compiler_sources'])|native_runtime_source_paths(ROOT)|{n.relative_to(ROOT).as_posix() for n in (ROOT/'tools').glob('*.py')}
            q.update(success=True,compiler_sources=old['compiler_sources'],player_binary=player,
                     sources={n:reader.digest(n) for n in sorted(names)},parent=dict(record=PARENT,sha256=reader.digest(PARENT)),
                     bounds=dict(maximum_requests=4,maximum_charged_original_games=2,stop_on_game_failure=True,case_deadline_seconds=600,retained_artifact_bytes=2*1024**3),
                     configurations=[dict(software=s,size=[800,600]) for s in (False,True)],
                     diagnostic_scope='all',reviewed_failures=[],repair_of=dict(record=None),
                     first_question='Actual High-to-Low installed font loads, glyph byte-use and normal destructor order. Pixel/DC output proof remains separate.',
                     source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip())
    except Exception as e:q.update(success=False,error=str(e))
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(success=q['success'],record=a.report,sha256=digest,error=q.get('error'))),flush=True)
    return int(not q['success'])

if __name__=='__main__':raise SystemExit(main())
