#!/usr/bin/env python3
"""Independent complete displayed pixels plus passive boundary session facts."""
import importlib.util,sys
from pathlib import Path
from rendering_pilot import require
from required_boundary_observation import PREFIX,CONTROLS,decode,storage_summary
ROOT=Path(__file__).resolve().parents[1]

def assess(reader,a,c,observed):
    require(a['success'] and a['sources_stable'] and a['private_wine_stopped']
            and a['prefix_pool']['returned'] and a['original_game_runs']==1,'Incomplete session')
    require(a['resident']['normal_quit_retired'] and not a['resident']['pending']
            and not a['resident']['error'],'Incomplete normal retirement')
    if a['configuration']['software_flag']:
        require(a['resident']['renderer'].lower().startswith(('llvmpipe','softpipe')),'Missing actual software provider')
    transitions=a['required_boundaries']['transitions'];require(len(transitions)==12,'Missing fullscreen transition')
    matches={r['paint']:r for r in c['comparison']['comparisons']}
    for i,t in enumerate(transitions):
        require(t['size']==[[800,600],[640,480],[800,600]][i//4]
                and t['fullscreen']==(i%2==0) and t['toolbar']==(i%4==0),'Wrong fullscreen schedule')
        p=t['paint'];require(int(p['paint']) in observed['swaps'] and int(p['paint']) in matches
                and matches[int(p['paint'])]['mismatching_pixels']==0,'Uncompared fullscreen paint')
        require(t['geometry_after']['focus']['host_focused']
                and int(t['after']['world'])>int(t['before']['world']),'Lost focus/progress')
        if i%2:
            require(t['geometry_after']['viewport']==transitions[i-1]['geometry_before']['viewport']
                    and t['geometry_after']['origin']==transitions[i-1]['geometry_before']['origin'],
                    'Window geometry not restored')
    trace=[v for v in a['artifacts'] if v['path'].endswith('/required-boundaries-v1.bin')]
    require(len(trace)==1,'Missing/ambiguous boundary trace')
    d=decode(reader.read_bytes(trace[0]['record']));storage=storage_summary(d)
    require({tuple(r[:2]) for r in storage['extents']}=={(800,600),(640,480)},'Missing actual resolution recreation')
    return dict(verified=True,normal_quit=True,software=a['configuration']['software_flag'],
        fullscreen_transitions=12,rebuilds=a['required_boundaries']['rebuilds'],storage=storage,
        trace=trace[0],mismatching_pixels=c['comparison']['mismatching_pixels'],
        scope='Actual Qt/X11/Mesa fullscreen at the three finite extents with normal retirement; passive provider identities and every-World state. No global caller unreachability or unobserved provider-path claim.')

def main():
    spec=importlib.util.spec_from_file_location('boundary_pixel_comparison',ROOT/'tools/check-rendering-viewport-cursor.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    module.PREFIX=PREFIX;module.CONTROLS=CONTROLS;module.assess=assess
    return module.main()
if __name__=='__main__':raise SystemExit(main())
