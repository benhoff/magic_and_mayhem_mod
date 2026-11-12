#!/usr/bin/env python3
"""Freeze three missing clipping domains after the retained Low input/output refusal."""
import importlib.util
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-clipped-operations-results-20261107/'
PARENT=PREFIX+'input-plan-11.json'


def main():
    target=PREFIX+'input-plan-12.json'
    require(not (ROOT/target).exists(),'Fresh immutable clipping plan required')
    spec=importlib.util.spec_from_file_location('clip_campaign',ROOT/'tools/test-native-campaign.py')
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
    build=ROOT/'working/build'
    with Reader(ROOT) as r:
        plan=r.json(PARENT)
        plan['parent']=dict(record=PARENT,sha256=r.digest(PARENT))
        negative=PREFIX+'default-640-08-missing-scope-02.json';q=r.json(negative)
        require(q['analysis_completed'] and not q['success'] and q['actual_geometric_reductions']==0,
                'Missing exact Low input/output refusal')
        plan['repair_basis']=dict(record=negative,sha256=r.digest(negative))
        accepted=PREFIX+'default-800-07-comparison-01.json';good=r.json(accepted)
        require(good['success'] and good['native_clip_regions_verified'],'Missing retained High proof')
        plan['retained_accepted_case']=dict(record=accepted,sha256=r.digest(accepted))
        controls=PREFIX+'distinct-copy-controls-02.json';c=r.json(controls)
        require(c['success'] and c['sources_stable'] and c['result']['distinct_inputs']['success']
                and all(r.digest(n)==h for n,h in c['sources'].items()),'Current bounded-selector controls missing')
        plan['diagnostic_controls']=dict(record=controls,sha256=r.digest(controls))
        helper_name=PREFIX+'window-exposure-namespace-controls-03.json';helper=r.json(helper_name)
        require(helper['success'] and all(r.digest(n)==h for n,h in helper['sources'].items()),'Existing Win32 helper changed')
        binary=ROOT/'working/clip-window-namespace-controls-03/exposure.exe'
        plan['exposure_helper']=dict(path=str(binary.relative_to(ROOT)),sha256=r.digest(binary.relative_to(ROOT)))
        plan['game_count_reviews']=[dict(record=(count:=PREFIX+f'default-640-{i:02}-game-count-review-01.json'),sha256=r.digest(count)) for i in (9,10)]
        plan['exposure_helper_build']=dict(record=helper_name,sha256=r.digest(helper_name))
        plan['compiler_sources']={n:r.digest(n) for n in sorted(m.linked_compiler_sources(build,ROOT))}
        body=(build/'mnm-qt-shell').read_bytes()
        require(b'MNM_NATIVE_COPY_DISTINCT_INPUTS' in body and b'distinct-copy-inputs' in body,'Selected player lacks bounded diagnostic implementation')
        name=target.removesuffix('.json')+'-player.bin'
        plan['player_binary']=dict(path='working/build/mnm-qt-shell',record=name,sha256=write_report(ROOT,name,body))
        plan['runtime_sources']={n:r.digest(n) for n in sorted(native_runtime_source_paths(ROOT))}
        names=set(plan['sources'])|{'tools/native_copy_window_exposure.py','tests/native-copy-window-exposure.c'}
        plan['sources']={n:r.digest(n) for n in sorted(names)}
        require(all(r.digest(n)==h for n,h in plan['clip_oracle_sources'].items()),'Frozen scalar clip/pixel oracle changed')
    plan.update(kind='finite_public_native_distinct_clipping_plan',scope=__doc__)
    plan['bounds']=dict(maximum_requests=14,maximum_original_games=8,new_games=1,
        prior_zero_game_preparations=5,prior_original_games=7,accepted_retained_games=3,
        stop_on_first_failure=True,successful_repaired_low_comparison_before_expansion=True)
    plan['domains']=[dict(software=True,size=[640,480])]
    plan['targeted_recollection']=dict(reason='SoftwareLow13failed with EAGAIN while the shared prefix-pool lock was busy, before any target prefix or original start. Bounded waiting now also covers that transient lock only while the target remains absent; partial/symlink targets and the120sdeadline refuse. Preserve three admitted domains and collect only missing softwareLow. Keep the eight-start game cap; add only one preparation request. Low10stopped during an unnecessary pre-admission accessibility scan; its missed session/count is corrected independently from exact admission and original wait/exit bytes. Restore ordinary owner-journal polling, discover late session in cleanup, retain every failed stage, and keep the existing finite startup deadline. Actual corrected CMake player contains the bounded mode. Low09helper ran outside private /tmp and could not reach its desktop; exact count review includes that interrupted original start. No-game control03now proves outside refusal and validated descendant namespace success, including complete post-move geometry. Stage exact helper under session because private /tmp hides host RAM binaries. Low08has no geometric reductions: an X-server move left all410CPU/84GPU RGNDATA lists equal to the full requested destination. Its all-copy prefix also truncates. Use the existing ordinary Win32 helper on Low at first Main; keep that declared position through normal Quit. Preserve and reanalyze accepted High data without another High/default game. Collect only the missing Low/default and two software domains.',native_draw='Unchanged optional RGNDATA observer/provider; preserve its20off/on native controls.',diagnostic='New opt-in distinct-input observer only; draw execution is unchanged. Max8COPY words/lifetimes/ancestry signatures per ancestry/key class and256MiB retained planes; retain every selected complete plane. Skipped duplicates/budgets receive no output credit. Existing512MiB queue/8192COPY/32GiBreadback limits stay.')
    plan['temporary_storage']=dict(C5='Existing1GiB per-file tmpfs bound',C3='Fresh regular compressed source file in tmpfs,2GiB retained bound',
        durable_disk_reservation_gib=2,minimum_disk_free_gib=90,minimum_tmpfs_available_gib=8,
        estimate_basis='Measured1.1GiB staged installation plus at most256MiB selected four-plane files, bounded metadata/logs and immutable replication overhead. Both large captures remain in RAM. Completed stages hand off only after paired real-restore verification.')
    plan['input_policy']=dict(movies='Enabled; existing physical Escape; no movie-output credit',map='Existing public Quick first map; no map/save/game-dispatch/pixel fixture',
        desktop='High ordinary windowed client clipping. Low: existing Win32 SetWindowPos helper at first ready Main, half left of private desktop, keep through normal Quit. No X-only move or fabricated clip list. The private window is destroyed by normal cleanup.',
        output='Actual operation RGNDATA, caller/result/live identity and meaningful complete selected CPU/GPU WORD planes with preserved exterior. Never infer all-copy or arbitrary repeated-value equivalence from the subset.',
        dimensions='Actual World CREATE/import/source640or800 is separate from primary/window/Qt presentation size.',
        resources='Fourteen total requests/eight total original games, including every prior refusal. Only one new game; the three accepted domains are retained. Stop after another failed acquisition/comparison. No reexecution of accepted default800.')
    print(write_report(ROOT,target,(json.dumps(plan,indent=2)+'\n').encode()))


if __name__=='__main__':main()
