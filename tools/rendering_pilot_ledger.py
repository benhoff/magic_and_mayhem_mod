"""Append-only EP03 receipts. No final journey, gate or recovery promotion."""
from datetime import datetime,timezone
import json,sqlite3
from pathlib import Path
import re
from coverage_claims import fingerprint
from rendering_acceptance import validate,requirements
from rendering_pilot import require,number

def encoded(value):return json.dumps(value,sort_keys=True,separators=(',',':'),allow_nan=False)
def ref(reader,value):
 require(type(value) is dict and set(value)=={'record','sha256'} and type(value['record']) is str and value['record'].startswith('research/runtime/rendering-acceptance-pilot-results-20261031/') and '..' not in Path(value['record']).parts and re.fullmatch('[0-9a-f]{64}',value['sha256']),'Invalid pilot evidence identity')
 require(reader.digest(value['record'])==value['sha256'],'Changed pilot evidence');return reader.json(value['record'])
def payload(reader,value):
 require(set(value)=={'kind','reason','acquisition','comparison','software_probe'} and value['kind']=='EP03.pipeline' and type(value['reason']) is str and value['reason'].strip(),'Invalid pilot receipt payload')
 a,c,p=[ref(reader,value[k]) for k in ('acquisition','comparison','software_probe')]
 require(a['kind']=='public_native_pilot_acquisition' and c['kind']=='public_native_pilot_comparison' and p['kind']=='public_native_pilot_acquisition','Wrong collector type')
 require(all(v['success'] is True and v['sources_stable'] is True and v['original_manifest_verified_before_after'] is True for v in (a,c,p)),'Incomplete execution, source or manifest evidence')
 require(c['acquisition']==value['acquisition'],'Comparison bound to a different acquisition')
 require(a['configuration']['software_flag'] is False and a['configuration']['skip_intros'] is False and p['configuration']['software_flag'] is True,'Actual default/software flag missing')
 require(a['private_wine_stopped'] is True and p['private_wine_stopped'] is True and a['retirement']['state']==p['retirement']['state']=='completed','Normal process/owner retirement missing')
 require(c['input_acknowledgements']['verified'] is True and c['original_menu_actions']['verified'] is True and c['asset_inputs_stable'] is True,'Action or asset evidence incomplete')
 r=c['comparison'];require(r['success'] is True and number(r['world_presentations'],1)>0 and number(r['pixels_compared'],1)>0 and number(r['mismatching_pixels'])==0 and number(r['remaining_surfaces'])==0,'Presented plane/history mismatch')
 require(r['comparisons'] and all(number(v['mismatching_pixels'])==0 and v['actual_sha256']==v['expected_sha256'] for v in r['comparisons']),'Incomplete native comparison cohort')
 require(c['original_reference']['continuous_from_owned_initial_history'] is True and c['original_reference']['native_player_pixels_read'] is False and c['original_reference']['original_destination_capture_read'] is False,'Original destination seeded the reference')
 require(all(v['original_pixels_used_as_native_inputs'] is False for v in (a,c,p)),'Oracle/native separation missing')
 require(number(r['world_presentations'])==sum(number(v['world'])>0 for v in r['comparisons']) and number(r['pixels_compared'])==sum(number(v['pixels'],1) for v in r['comparisons']),'Incomplete comparison counts')
 require(number(c['observations']['world_swaps'],1)==r['world_presentations'],'Uncompared actual World swap')
 bound={}
 for execution in (a,c,p):
  require(type(execution['sources']) is dict and execution['sources'],'Execution source closure missing')
  for name,digest in execution['sources'].items():
   path=Path(name);require(path.parts and not path.is_absolute() and '..' not in path.parts and path.parts[0] not in ('original','working','.git') and re.fullmatch('[0-9a-f]{64}',digest),'Invalid execution source binding')
   require(name not in bound or bound[name]==digest,'Conflicting source phases');bound[name]=digest
 return dict(world_presentations=r['world_presentations'],pixels_compared=r['pixels_compared'],original_worlds=c['original_reference']['worlds'],original_draws=c['original_reference']['draws'],current_sources={n:h for v in (a,c,p) for n,h in v['sources'].items()},scope=r['scope'])

class Workspace:
 def __init__(self,root,plan):
  self.root=Path(root);spec=validate(plan);self.dossier=fingerprint(dict(spec=spec,requirements=requirements(plan)))
  self.path=self.root/'working/coverage/rendering-pilot.sqlite3';self.path.parent.mkdir(parents=True,exist_ok=True);(self.path.parent/'.retain').touch(exist_ok=True)
  self.db=sqlite3.connect(self.path);self.db.executescript("""pragma synchronous=FULL;
   create table if not exists events(id text primary key,dossier text not null,payload text not null,observed_at text not null);
   create trigger if not exists events_no_update before update on events begin select raise(abort,'Immutable pilot receipt');end;
   create trigger if not exists events_no_delete before delete on events begin select raise(abort,'Immutable pilot receipt');end;""")
 def close(self):self.db.close()
 def append(self,reader,value):
  result=payload(reader,value);require(all(reader.digest(n)==h for n,h in result['current_sources'].items()),'Pilot execution source changed before admission')
  key=fingerprint(dict(dossier=self.dossier,payload=value));stamp=datetime.now(timezone.utc).isoformat()
  with self.db:self.db.execute('insert or ignore into events values(?,?,?,?)',(key,self.dossier,encoded(value),stamp))
  return key
 def snapshot(self,reader):
  events=[]
  for key,dossier,body,stamp in self.db.execute('select id,dossier,payload,observed_at from events order by rowid'):
   value=json.loads(body);payload(reader,value);require(dossier==self.dossier and key==fingerprint(dict(dossier=dossier,payload=value)),'Pilot ledger identity')
   instant=datetime.fromisoformat(stamp);require(instant.tzinfo is not None and instant.utcoffset().total_seconds()==0 and instant.isoformat()==stamp,'Invalid pilot timestamp')
   events.append(dict(id=key,dossier=dossier,payload=value,observed_at=stamp))
  return dict(schema=1,dossier=self.dossier,events=events,decisions={'supported-player':'pending','original-recovery':'pending'},closes_full_requirement_rows=0)
 def restore(self,reader,snapshot):
  require(set(snapshot)=={'schema','dossier','events','decisions','closes_full_requirement_rows'} and type(snapshot['schema']) is int and snapshot['schema']==1 and snapshot['dossier']==self.dossier and snapshot['decisions']=={'supported-player':'pending','original-recovery':'pending'} and type(snapshot['closes_full_requirement_rows']) is int and snapshot['closes_full_requirement_rows']==0,'Invalid pilot snapshot binding or decision')
  require(type(snapshot['events']) is list and len(snapshot['events'])<=128,'Pilot event budget')
  rows=[]
  for event in snapshot['events']:
   require(set(event)=={'id','dossier','payload','observed_at'} and event['dossier']==self.dossier,'Invalid pilot event')
   payload(reader,event['payload']);require(event['id']==fingerprint(dict(dossier=self.dossier,payload=event['payload'])),'Restored pilot event identity')
   instant=datetime.fromisoformat(event['observed_at']);require(instant.tzinfo is not None and instant.utcoffset().total_seconds()==0 and instant.isoformat()==event['observed_at'],'Invalid pilot timestamp')
   rows.append((event['id'],event['dossier'],encoded(event['payload']),event['observed_at']))
  require(len({r[0] for r in rows})==len(rows),'Duplicate pilot events')
  with self.db:
   for row in rows:
    old=self.db.execute('select id,dossier,payload,observed_at from events where id=?',(row[0],)).fetchone();require(old is None or old==row,'Conflicting restored pilot event');self.db.execute('insert or ignore into events values(?,?,?,?)',row)
