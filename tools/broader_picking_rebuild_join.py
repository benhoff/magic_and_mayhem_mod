"""Join rebuilt actual swaps to the exact Original generation, under one deadline."""
from types import CodeType
import dis
from rendering_pilot import require

POLICY='same-generation-original-rebuild-paint-v1'
TRACE_LIMIT=2048
INPUT_LIMIT=16+8192*384

def rebuild_code(main):
    found=[]
    def visit(code):
        if code.co_qualname=='main.<locals>.PickingJourney.rebuild':found.append(code)
        for value in code.co_consts:
            if isinstance(value,CodeType):visit(value)
    visit(main.__code__)
    require(len(found)==1,'Missing unique frozen rebuild code')
    code=found[0]
    require(sum(i.opname in ('LOAD_ATTR','LOAD_METHOD') and i.argval=='latest'
                for i in dis.get_instructions(code))==2,'Frozen rebuild latest callsites changed')
    return code

def select(paints,rows,before,size,old_world,low,now,deadline):
    if now>=deadline:return None
    by_generation={r['frame']['generation']:r for r in rows if r['kind']==1 and r['frame']}
    for paint in reversed(paints):
        generation=int(paint['world']);row=by_generation.get(generation)
        if ([paint['frame_width'],paint['frame_height']]!=list(size)
            or generation<old_world+5 or generation<=before['frame']['generation']
            or not all(paint.get(k)is True for k in ('native_gpu_frame','gpu_read_complete','pixels_captured'))
            or paint.get('presentation_owner')!='gui' or row is None):continue
        if (row['high_resolution']!=int(not low) or row['thread']!=before['thread']
            or row['frame']['nonce']!=before['frame']['nonce']):continue
        return paint,row
    return None

def receipt(before,paint,row,old_world,low,deadline,selected):
    return dict(original_before_ordinal=before['ordinal'],original_after=row,paint=paint,
                old_world=old_world,low=low,deadline=deadline,selected_monotonic=selected)
