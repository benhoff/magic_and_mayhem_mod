#!/usr/bin/env python3
"""Freeze the exact new hotspot player, oracle and independent six-start budget."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from cursor_hotspot import ROOT,PREFIX,PLAN,MAX_REQUESTS,MAX_GAMES,acceptance_points as points,descriptors,bitmap_words
from rendering_pilot import require


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',default=PLAN);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not(ROOT/a.report).exists(),'Fresh owned hotspot plan required')
    spec=importlib.util.spec_from_file_location('hotspot_campaign',ROOT/'tools/test-native-campaign.py')
    campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
    with Reader(ROOT) as r:
        oracle_parent='research/runtime/rendering-interaction-results-20261107/input-plan-07.json'
        parent=r.json(oracle_parent)
        require(all(r.digest(n)==h for n,h in parent['oracle']['native_sources'].items()),'Unchanged retained oracle source closure differs')
        for item in parent['oracle']['binaries']:
            raw=r.read_bytes(item['record']);require(hashlib.sha256(raw).hexdigest()==item['sha256'],'Lost exact retained oracle')
            target=ROOT/item['path'];target.parent.mkdir(parents=True,exist_ok=True)
            require(not target.exists() or target.read_bytes()==raw,'Refuse a different oracle executable')
            if not target.exists():target.write_bytes(raw);target.chmod(0o755)
        compiled={n:r.digest(n) for n in sorted(campaign.linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT))}
        binary='working/build/qt-shell/mnm-qt-shell';raw=(ROOT/binary).read_bytes()
        player=dict(path=binary,record=a.report.removesuffix('.json')+'-player.bin',sha256=write_report(ROOT,a.report.removesuffix('.json')+'-player.bin',raw))
        live={'tools/prepare-rendering-cursor-hotspot.py','tools/run-rendering-cursor-hotspot.py','tools/cursor_hotspot.py',
              'tools/run-rendering-interaction.py','tools/rendering_interaction_budget.py','tools/rendering_interaction.py',
              'tools/rendering_interaction_live.py','tools/rendering_interaction_stability.py'}
        cfg=(ROOT/'working/game-nocd/CFG/cursors.cfg').read_bytes()
        bmp=(ROOT/'working/game-nocd/Bitmaps/cursors.bmp').read_bytes()
        desc=descriptors(cfg);words=bitmap_words(bmp)
        result=dict(schema=1,kind='cursor_hotspot_prospective_plan',success=True,
            sources={n:r.digest(n) for n in sorted(live)},compiler_sources=compiled,player_binary=player,
            oracle=dict(parent=dict(record=oracle_parent,sha256=r.digest(oracle_parent)),**parent['oracle']),
            runtime_sources={n:r.digest(n) for n in sorted(native_runtime_source_paths(ROOT)|live)},
            bounds=dict(maximum_requests=MAX_REQUESTS,maximum_original_games=MAX_GAMES,primary_games=4,
                stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),
            domains=[dict(software=s,size=z,scaling=['sharp','integer'],candidates=points(z))
                     for s in (False,True) for z in ([800,600],[640,480])],
            assets=dict(config_sha256=hashlib.sha256(cfg).hexdigest(),bitmap_sha256=hashlib.sha256(bmp).hexdigest(),
                rgb565_sha256=hashlib.sha256(words.astype('<u2').tobytes()).hexdigest(),descriptors=desc),
            required_nonzero_classes=[6,7,9],minimum_displayed_generations_per_state=3,
            phase='Four-point bounded correction after retained pilot exit95 and diagnostic truncation. No request/start refund or original caller admission; stable actual class observations replace long fixed dwells.',
            independence='A new hotspot/background question. No picking/rebuild/recovery/attachment request or original start is erased or reused. Their prior exhausted caps remain intact. This plan permits no same-World rebuild or cast/rotation route.',
            request_cap_approval='User explicitly approved Allow9requests; keep6game starts after five requests/two starts. No previous charge or failure is retired.',
            scope=f'Physical prescribed edge/center moves, original descriptor/point/save-under observations and current native COPY/provenance diagnostics, complete independent original-history viewport comparison and normal owner retirement. {MAX_GAMES} total actual starts/{MAX_REQUESTS} requests maximum including failures. Other cursor/provider/ABI/lifetime domains and both final decisions remain separate.')
    digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,record=a.report,sha256=digest,compiler_sources=len(compiled),player=player['sha256'])),flush=True)


if __name__=='__main__':raise SystemExit(main())
