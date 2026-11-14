"""Read the actual command-capture policy independently of a launch receipt."""
import json,struct
from pathlib import Path
from rendering_pilot import require
from retained_artifacts import read_artifact
POLICY=dict(flag='MNM_ACCEPTANCE_COMMAND_HISTORY',value='1',version=4,policy='command-history-lossless-v1',row_limit=65536,raw_limit_bytes=8*1024**3,stored_limit_bytes=1024**3,queue_limit_bytes=256*1024**2,queue_limit_rows=4096,compression_workers=8,native_limits_changed=False)
def header(raw,session):
 require(raw[:8]==b'MNMACE01'and len(raw)>=16,'Command capture envelope missing')
 m,n=struct.unpack_from('<II',raw,8)
 require(1<=m<=65536 and n==0 and len(raw)>=16+m,'Command open row extent')
 row=json.loads(raw[16:16+m]);require(row['index']==0 and row['kind']=='open'and row['role']=='commands'and row['session']==session,'Foreign command open row')
 require(all(row.get(k)==POLICY[k]for k in('version','policy','row_limit','raw_limit_bytes','stored_limit_bytes','queue_limit_bytes','queue_limit_rows','compression_workers')),'Actual recorder policy differs from unchanged bounded v4')
 return row

def capture_selection(reader,acquired,phase):
 require(phase['command_history_policy']==POLICY,'Changed prospective command-history policy')
 selected=acquired['provider_adaptation']['command_history_selection'];require(selected['native_code_or_limits_changed']is False and selected['game_actions_or_deadlines_changed']is False and selected['old_capture_completed']is False,'Recorder selection broadened its claim')
 require(len(selected['launches'])==1,'Ambiguous history player launch');launch=selected['launches'][0]
 require(all(launch.get(k)==v for k,v in dict(flag='1',policy=POLICY['policy'],version=4,row_limit=65536,raw_limit_bytes=8*1024**3,queued_byte_limit=256*1024**2,queued_job_limit=4096).items()),'Launch selection differs from actual fixed policy')
 streams=[v for v in acquired['artifacts']if '/capture/'in v['path']and Path(v['path']).name.startswith('commands')and v['path'].endswith('.bin')]
 require(len(streams)==1,'Ambiguous current command capture');item=streams[0]
 raw=read_artifact(reader,acquired,item['path'],max_bytes=1024**3+8)
 row=header(raw,acquired['session_identity'])
 return dict(verified=True,capture_sha256=item['sha256'],open=row,old_capture_completed=False,native_limits_changed=False,complete_history_must_also_pass_independent_comparison=True)
