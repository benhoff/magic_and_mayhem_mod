#!/usr/bin/env python3
"""Retire only a validated RAM capture duplicate after two copies and a restore."""
import argparse,hashlib,json,os,shutil,stat,sys,tempfile,uuid
from pathlib import Path
from broader_capture_storage import BASE,PIN
from coverage_evidence import Reader,write_report
from evidence_objects import configured,_sync
from evidence_workspace import scan,idle,materialize,MARKER
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--validation',required=True);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh capture handoff report required')
    with Reader(ROOT)as r:
        v=r.json(args.validation);require(v['success']is True and v['sources_stable']is True,'Validated current case required')
        ar=v['acquisition'];require(r.digest(ar['record'])==ar['sha256'],'Acquisition identity changed');a=r.json(ar['record']);require(a['success']is True,'Failed acquisition needs its own negative retention review')
        if 'comparison'in v:
            cr=v['comparison'];require(r.digest(cr['record'])==cr['sha256'],'Comparison identity changed');c=r.json(cr['record']);require(c['success']is True,'Complete independent comparison required')
        storage=a['provider_adaptation'].get('capture_storage',dict(enabled=False,entries=[]))
        entries=storage['entries'];require(not storage['enabled']or storage['captured_bytes_preserved'],'Missing captured-byte custody')
        refs={n['path']:n for n in a['artifacts']};plans=[]
        for entry in entries:
            physical,logical=Path(entry['physical']),Path(entry['logical'])
            require(physical.parent==BASE and len(physical.name)==24 and all(ch in'0123456789abcdef'for ch in physical.name)
                    and not any(p.is_symlink()for p in[physical,*physical.parents]),'Refuse external or aliased RAM source')
            require((physical/'.retain').read_bytes()==PIN and entry['copied']and logical.is_relative_to(ROOT/'working'),
                    'Only the owned explicitly temporary duplicate may retire')
            idle(physical);rows,identity,allocated=scan(physical)
            require(all(row['kind']in('directory','file')for row in rows),'Unexpected RAM member type')
            expected={item['name']:item for item in entry['files']}
            require({row['path']for row in rows if row['kind']=='file'}==set(expected),'RAM member set changed')
            for row in rows:
                if row['kind']!='file':continue
                item=expected[row['path']];require(row['sha256']==item['sha256']and row['size']==item['bytes'],'RAM capture changed')
                # Retained original collectors use both relative and absolute
                # logical artifact paths. Exactly one must identify each file.
                names=[str(logical/row['path']),str((logical/row['path']).relative_to(ROOT))]
                matches=[refs[n]for n in names if n in refs];require(len(matches)==1,'Missing unique captured artifact')
                ref=matches[0];require(ref['sha256']==row['sha256']and r.digest(ref['record'])==row['sha256'],'Captured artifact identity differs')
            plans.append(dict(entry=entry,members=rows,identity=identity,allocated_bytes=allocated))
        vr=dict(record=args.validation,sha256=r.digest(args.validation))
    results=[]
    with configured(ROOT)as pair:
        for plan in plans:
            source=Path(plan['entry']['physical']);idle(source)
            require(scan(source)[1]==plan['identity'],'RAM duplicate changed before sealing')
            with pair.batch():
                rows,identity,allocated=scan(source,pair)
                require(rows==plan['members']and identity==plan['identity'],'RAM duplicate changed during sealing')
                manifest=pair.root('execution_workspace',source,dict(schema=1,kind='execution_workspace',source=str(source),members=rows,
                    pin_scope='Exact task-owned temporary diagnostic duplicate; all original pins and metadata retained. The acquisition has a separate exact logical Cold copy.'))
            with tempfile.TemporaryDirectory(prefix='mm-capture-restore-',dir=BASE.parent)as tmp:materialize(pair,manifest,Path(tmp)/'restored',replica=1)
            idle(source);require(scan(source)[1]==identity,'RAM duplicate changed before retirement')
            result=dict(source=str(source),manifest=manifest,real_restore_verified=True,replicas_verified=2,allocated_bytes=allocated,
                validation=vr,logical_closed_copy=plan['entry']['logical'])
            pair.root('temporary_capture_copy_verified',source,result)
            quarantine=source.with_name(source.name+'.handoff-'+uuid.uuid4().hex);source.rename(quarantine)
            try:
                idle(quarantine);require(scan(quarantine)[0]==rows,'RAM duplicate changed after quarantine')
                source.mkdir(mode=0o700);(source/MARKER).write_text(json.dumps(manifest,separators=(',',':'))+'\n')
                (source/'.retain').write_text('Exact original temporary capture bytes and pin retained in two verified stores with a real restore.\nManifest: '+manifest['sha256']+'\nThe normal logical capture copy is also retained by the acquisition and execution handoff.\n')
                _sync(source/MARKER);_sync(source/'.retain')
            except BaseException:
                if source.exists()and set(p.name for p in source.iterdir())<={MARKER,'.retain'}:
                    for member in source.iterdir():member.unlink()
                    source.rmdir()
                if not source.exists():quarantine.rename(source)
                raise
            shutil.rmtree(quarantine);result['retired']=True;result['reclaimed_allocated_bytes']=allocated-sum(p.stat().st_blocks*512 for p in source.iterdir())
            pair.root('temporary_capture_copy_retired',source,result);results.append(result)
    q=dict(schema=1,kind='broader_validated_temporary_capture_copy_handoff',success=True,original_game_runs=0,validation=vr,acquisition=ar,
        results=results,source=dict(path='tools/retire-broader-capture-copy.py',sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()),
        scope='Only these validated, closed, explicitly temporary RAM duplicate captures retire. Exact raw bytes, metadata, original pins and historical paths survive in two verified stores and a real restore. The unchanged logical capture copy and acquisition remain independently retained. No source checkout, original input, build, environment, unknown/pinned historical output or acceptance status is removed or changed.')
    h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,retired_RAM_duplicates=len(results))),flush=True)

if __name__=='__main__':main()
