"""Original callback and native public-input proof for an offered Quick Battle map."""
import importlib.util
from pathlib import Path

def validate_quick(session,flow,map_row):
    if flow.get('success') is not True or [s['screen'] for s in flow['states']]!=[3,22,14,25,14,7,26,26,22,3]:
        raise ValueError('Incomplete public Quick Battle menu sequence')
    if flow.get('selected_map_row')!=map_row or not flow.get('selected_map_id') or not flow.get('selected_map_name'):
        raise ValueError('Requested offered map was not selected')
    if flow['selected_map_id']!=str(map_row+1) or flow['states'][4].get('map')!=map_row+1:
        raise ValueError('Original setup did not acknowledge the selected map ordinal')
    if flow.get('world_visits')!=2 or len({s['thread'] for s in flow['states']})!=1:
        raise ValueError('Missing two original Quick Battle World visits on one menu thread')
    if not all(flow.get(k) is True for k in ('focus_release','resize','escape_results','input_via_qt_viewport','rapid_duplicate_clicks')) or flow.get('native_fallback') is not False or flow.get('native_error') or flow.get('recoveries')!=0:
        raise ValueError('Quick Battle input/presentation failed')
    if min(flow.get('native_frames',0),flow.get('native_paints',0))<=0 or len(flow.get('world_presentation',[]))!=2:
        raise ValueError('Missing both native World presentation witnesses')
    root=Path(__file__).resolve().parents[1]
    spec=importlib.util.spec_from_file_location('player_quick_events',root/'tools/test-menu-observer.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    rows=module.decode((session/'events.bin').read_bytes())
    actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
    expected=[(3,2),(22,2),(14,2),(25,0),(14,1),(7,12),(26,1),(26,2),(22,3),(3,4)]
    if actions!=expected:raise ValueError('Original Quick Battle callbacks changed: '+str(actions))
    return dict(verified=True,selected_map_row=map_row,selected_map_id=flow['selected_map_id'],selected_map_name=flow['selected_map_name'],original_callbacks=actions,world_visits=2,
        scope='Selected offered map, two normal World visits, public input and original Quit callbacks. No other-map, whole-output or original-driver equivalence claim.')
