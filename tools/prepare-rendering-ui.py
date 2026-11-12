#!/usr/bin/env python3
"""Freeze a bounded HUD/tooltip/pause matrix using the unchanged accepted player."""
import argparse
import hashlib
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
from rendering_ui_acceptance import PREFIX
ROOT=Path(__file__).resolve().parents[1]
CONTROL=['tools/prepare-rendering-ui.py','tools/run-rendering-ui.py','tools/rendering_ui_acceptance.py',
         'tools/run-rendering-interaction.py','tools/run-rendering-camera-acceptance.py',
         'tools/run-rendering-menu-acceptance.py','tools/rendering_menu_accessibility.py']


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh owned plan required')
    with Reader(ROOT) as r:
        old_name='research/runtime/rendering-enabled-intros-results-20261107/input-plan-03.json';old=r.json(old_name)
        require(all(r.digest(n)==h for n,h in old['compiler_sources'].items()),'Accepted player compiler source drift')
        b=old['player_binary'];raw=r.read_bytes(b['record']);require(hashlib.sha256(raw).hexdigest()==b['sha256'],'Changed player payload')
        path=ROOT/'working/build/ui-player/mnm-qt-shell';path.parent.mkdir(parents=True,exist_ok=True)
        require(not path.exists() or path.read_bytes()==raw,'Preserve differing player');path.write_bytes(raw);path.chmod(0o755)
        oracle=r.json('research/runtime/rendering-interaction-results-20261107/input-plan-07.json')['oracle']
        require(all(r.digest(n)==h for n,h in oracle['native_sources'].items()),'Original reference source drift')
        for item in oracle['binaries']:
            dest=ROOT/item['path'];payload=r.read_bytes(item['record']);require(hashlib.sha256(payload).hexdigest()==item['sha256'],'Changed reference payload')
            dest.parent.mkdir(parents=True,exist_ok=True);require(not dest.exists() or dest.read_bytes()==payload,'Preserve differing oracle')
            dest.write_bytes(payload);dest.chmod(0o755)
        result=dict(schema=1,success=True,kind='bounded_hud_tooltip_pause_plan',
            parent=dict(record=old_name,sha256=r.digest(old_name)),compiler_sources=old['compiler_sources'],
            player_binary=dict(b,path=str(path.relative_to(ROOT))),oracle=oracle,
            runtime_sources={n:r.digest(n) for n in sorted(native_runtime_source_paths(ROOT))},
            sources={n:r.digest(n) for n in CONTROL},
            supersedes=dict(record=PREFIX+'input-plan-06.json',sha256=r.digest(PREFIX+'input-plan-06.json'),
                reason='All seven requests/five game starts remain charged. Hermes dialogue blocked the campaign summon. Use existing bounded DialogueReadiness with actual native viewport OCR and physical centre click before HUD/tooltip and cast, rather than assuming Return attempts clear dialogue. Cumulative ceilings remain12requests/9starts; four required cases fit, no repair starts remain.'),
            bounds=dict(maximum_requests=12,maximum_original_games=9,primary_games=4,repair_games=5,
                        stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),
            domains=[dict(software=s,size=z) for s in (False,True) for z in ([800,600],[640,480])],
            scope='Six pending HUD-update/tooltip/pause rows only. Physical input, read-only original observations, source-only raster/history comparison, full native output, normal Quit and identity admission. No deadline/balance/configuration changes, second battle/resize, fault or final-player claim.')
        print(write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode()))


if __name__=='__main__':main()
