"""Join a completed cursor observation to the existing delayed-swap grace."""
from rendering_interaction_assessment import presented_interval as original
from rendering_pilot import require
POLICY='unchanged-state-source-completion-within-existing-500ms-swap-grace-v1'
class Completion:
    def __init__(self,allowed):self.diagnostics=[];self.allowed=set(allowed)
    def __call__(self,rows,paints,released,settled):
        # Select by source identity/state/time only, never by cursor result/hit.
        end=settled['frame_input_row'];baseline=rows[end-1]
        if (released['frame_input_row'],end,released['host_us'],settled['host_us'])not in self.allowed:return original(rows,paints,released,settled)
        if 'cursor'not in baseline:return original(rows,paints,released,settled)
        selected_end=end
        for row in rows[end:]:
            if row['kind']!=1:break
            if(not row['frame']or not baseline['frame']or row['thread']!=baseline['thread']
                or row['frame']['nonce']!=baseline['frame']['nonce']
                or(row['tick']-baseline['tick'])%2**32>500):break
            # A different original camera/selection/mode/actor state ends the
            # completion interval even if the host still displays an old frame.
            if any(row[k]!=baseline[k]for k in('camera','mode','tracked','history','position')):break
            selected_end=row['ordinal']
        if selected_end==end:return original(rows,paints,released,settled)
        # The original function keeps its existing host cutoff +500000us and
        # scaling filter. Extend only the stopped source-row bound, not time.
        bound=dict(settled,frame_input_row=selected_end)
        tail,shown=original(rows,paints,released,bound)
        if any(r['ordinal']>end for r in tail):
            self.diagnostics.append(dict(released_row=released['frame_input_row'],settled_row=end,completion_row=selected_end,
                host_settled_us=settled['host_us'],host_grace_us=500000,source_grace_ms=500,
                original_tail_rows=[r['ordinal']for r in tail],source_frames=[r['frame']['generation']for r in tail],
                cursor_results_used_for_selection=False,additional_World_credit=0))
        return tail,shown


def away_rotation_intervals(markers):
    require(len(markers)%3==0,'Incomplete interaction marker triples')
    groups=[markers[i:i+3]for i in range(0,len(markers),3)];allowed=[]
    for index,group in enumerate(groups[:-1]):
        name=group[0]['action'];following=groups[index+1][0]['action']
        if name.endswith('-away-2')and '-view-'in name:
            prefix,view=name.removesuffix('-away-2').rsplit('-view-',1)
            require(following==prefix+'-rotate-'+view,'Away completion is not immediately followed by its rotation')
            require([m['phase']for m in group]==['begin','released','settled']and all(m['action']==name for m in group),'Changed away action markers')
            released,settled=group[1:];allowed.append((released['frame_input_row'],settled['frame_input_row'],released['host_us'],settled['host_us']))
    require(len(allowed)==8,'All eight declared view-away/rotation boundaries required')
    return allowed
