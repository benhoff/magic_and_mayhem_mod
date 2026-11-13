#!/usr/bin/env python3
"""Verify and real-restore an unchanged controller-owned RAM acquisition; keep it held."""
import argparse,hashlib,json,tempfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from evidence_workspace import scan,idle,materialize
from broader_delegated_plan import CONTROLLER_RAM
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--validation',required=True);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh controller RAM proof required')
 with Reader(ROOT)as r:
  v=r.json(args.validation);require(v['success']and v['sources_stable']and v['lifetime'],'Complete current World validation required');a=r.json(v['acquisition']['record']);require(r.digest(v['acquisition']['record'])==v['acquisition']['sha256']and a['success']and a['private_wine_stopped']and a['prefix_pool']['returned'],'Foreign or live acquisition');family=v['family'];require(family in CONTROLLER_RAM,'Unreviewed controller RAM family');phase=r.json(a['broader_phase']['record']);require(phase['capture_storage']['enabled']is False and phase['capture_storage']['policy']=='unchanged-controller-owned-tmpfs-v1'and phase['capture_storage']['paired_real_restore_required'],'Undeclared controller RAM policy');storage=a['provider_adaptation']['capture_storage'];require(storage['enabled']is False and storage['entries']==[],'Outer RAM adapter overlaid the controller');source=Path(a['ram_capture_root']);require(source.parent==Path('/tmp')and source.name.startswith(CONTROLLER_RAM[family])and not any(p.is_symlink()for p in(source,*source.parents)),'Unowned/aliased controller RAM source');out=Path(a['temporary_root']);require(out.is_absolute()and out.is_relative_to(ROOT/'working')and out.name==Path(v['acquisition']['record']).stem,'Foreign logical acquisition root');idle(source);members,identity,allocated=scan(source);require(members and all(x['kind']in('file','directory')for x in members),'Unexpected controller RAM member');verified=[]
  for item in members:
   if item['kind']!='file'or item['path']=='.retain':continue
   logical=out/item['path'];names={str(logical),str(logical.relative_to(ROOT))};found=[x for x in a['artifacts']if x['path']in names];require(len(found)==1,'Missing/ambiguous controller RAM artifact');ref=found[0];require(item['size']==ref['bytes']and item['sha256']==ref['sha256']==r.digest(ref['record']),'Different retained controller RAM bytes');verified.append(ref)
  require(verified and any(Path(x['path']).name=='worlds.bin'for x in verified)and any(Path(x['path']).name.startswith('commands-')for x in verified),'Incomplete controller source/command history');vr=dict(record=args.validation,sha256=r.digest(args.validation));sources={n:r.digest(n)for n in('tools/retain-broader-controller-ram.py','tools/broader_delegated_plan.py')}
 with configured(ROOT)as pair:
  idle(source);require(scan(source)[1]==identity,'Controller RAM changed before custody')
  with pair.batch():
   rows,after,_=scan(source,pair);require(rows==members and after==identity,'Controller RAM changed during custody');manifest=pair.root('held_validated_controller_RAM',source,dict(schema=1,kind='execution_workspace',source=str(source),members=rows,pin_scope='Preserve every original controller RAM byte/path/pin; source remains physically held.'))
  with tempfile.TemporaryDirectory(prefix='controller-RAM-realrestore-',dir=source.parent)as tmp:materialize(pair,manifest,Path(tmp)/'restored',replica=1)
  require(scan(source)[1]==identity,'Held controller RAM changed');result=dict(source=str(source),manifest=manifest,real_restore_verified=True,replicas_verified=2,retired=False,physical_source_held=True,allocated_bytes=allocated);pair.root('validated_controller_RAM_verified',source,result)
 q=dict(schema=1,kind='broader_validated_controller_owned_RAM_handoff',success=True,original_game_runs=0,validation=vr,acquisition=v['acquisition'],verified_artifacts=verified,results=[result],sources=sources,sources_stable=True,scope='The unchanged caller/picking controller owns its existing RAMcapture/sourcehistory. An outer adapter must not recreate that logical symlink. Every exact file must match its already retained acquisition artifact before two-store verification and a real restore. Preserve all original files,pins and physical paths; nothing is removed and no source/status/hash is refreshed.')
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,retained_files=len(verified),physical_source_held=True)),flush=True)
if __name__=='__main__':main()
