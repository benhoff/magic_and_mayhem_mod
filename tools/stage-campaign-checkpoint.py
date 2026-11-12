#!/usr/bin/env python3
"""Copy a verified original checkpoint from SQLite into this checkout's save store."""
import argparse
import hashlib
import json
from pathlib import Path
import tempfile
from coverage_evidence import Reader,write_report
from campaign_checkpoint import save_name
from menu_save_store import stage,commit
from rendering_progression import require

ROOT=Path(__file__).resolve().parents[1]


def checkpoint_acquisition(report):
    if report.get('kind')=='public_native_checkpoint_diagnostic_acquisition':return True
    mode=report.get('configuration',{}).get('checkpoint_mode')
    return report.get('kind')=='public_native_campaign_lightweight_diagnostic' and mode in ('capture','restore') and report.get('scenario')=='checkpoint-'+mode


def import_exact(raw,name,store,temporary_root):
    """Use the save store's existing locked, conflict-checked copy transaction."""
    name=save_name(name)+'.sav';expected=hashlib.sha256(raw).hexdigest()
    require(0<len(raw)<=16*1024**2,'Checkpoint input exceeds the bounded save policy')
    temporary_root.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='checkpoint-import-',dir=temporary_root) as directory:
        game=Path(directory);state=stage(game,store);before=state['baseline']
        require(name not in before or before[name]==expected,'Existing checkpoint name has different bytes; preserved without overwrite')
        target=game/'Save'/name
        if name not in before:
            with target.open('xb') as out:out.write(raw)
        merged=commit(state)
        require(all(merged['files'].get(n)==h for n,h in before.items()),'Existing owned saves changed during import')
        require(merged['files'][name]==expected,'Imported checkpoint hash differs')
        return dict(name=name,sha256=expected,bytes=len(raw),store=str(store),changed=merged['changed'],
                    existing_saves_unchanged=before,files=merged['files'])


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition',required=True)
    p.add_argument('--restore-proof',help='Optional fresh-process restore acquisition for the same exact save')
    p.add_argument('--name',help='New checkpoint- stem; defaults to the original saved name')
    p.add_argument('--store',type=Path,default=ROOT/'working/user-saves/native-menu')
    p.add_argument('--report',required=True)
    a=p.parse_args();store=a.store.resolve()
    require(store.is_relative_to(ROOT/'working') and not a.store.is_symlink(),'Store must be an owned directory under this checkout working/')
    require(a.report.startswith('research/runtime/') and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'New immutable research report required')
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in ['tools/stage-campaign-checkpoint.py','tools/campaign_checkpoint.py','tools/menu_save_store.py']}
    report=dict(sources=sources,schema=1,kind='original_checkpoint_byte_copy',success=False,original_game_executed=False,
                original_serialization_replaced=False,scope=__doc__)
    try:
        with Reader(ROOT) as reader:
            acquired=reader.json(a.acquisition)
            report['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
            require(checkpoint_acquisition(acquired) and acquired.get('success') is True
                    and acquired.get('checkpoint',{}).get('verified') is True and acquired.get('private_wine_stopped') is True
                    and acquired.get('artifact_retention_complete') is True and acquired.get('sources_stable') is True
                    and acquired.get('original_manifest_verified_before_after') is True and acquired.get('player_binary_stable') is True,'Successful retained checkpoint acquisition required')
            expected=acquired['checkpoint']['save_sha256'];source_name=acquired['configuration']['checkpoint_name']
            if a.restore_proof:
                restored=reader.json(a.restore_proof)
                require(checkpoint_acquisition(restored) and restored.get('success') is True and restored.get('scenario')=='checkpoint-restore'
                        and restored.get('private_wine_stopped') is True and restored.get('artifact_retention_complete') is True
                        and restored.get('checkpoint',{}).get('save_sha256')==expected and restored['checkpoint'].get('verified') is True,
                        'Fresh-process restore proof does not match this checkpoint')
                report['restore_proof']=dict(record=a.restore_proof,sha256=reader.digest(a.restore_proof))
            matches=[v for v in acquired['artifacts'] if v['path'].endswith('/game/Save/'+source_name+'.sav') and v['sha256']==expected]
            require(len(matches)==1,'Missing or ambiguous retained original save bytes')
            source=matches[0];raw=reader.read_bytes(source['record'])
            require(hashlib.sha256(raw).hexdigest()==expected==reader.digest(source['record']),'Retained checkpoint hash differs')
            report['original_save']=source
        report['import']=import_exact(raw,a.name or source_name,store,ROOT/'working/tests')
        report['success']=True
    except Exception as e:report['error']=str(e)
    finally:
        report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],error=report.get('error'),record=a.report,sha256=h)),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
