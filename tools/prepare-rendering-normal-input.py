#!/usr/bin/env python3
"""Prepare/restore the owned low-resolution EP04 input with exact byte receipts."""
import argparse,hashlib,importlib.util,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_normal_acceptance import PREFIX
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def sha(path):
    with path.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('operation',choices=['prepare','restore']);p.add_argument('--acquisition',required=True);p.add_argument('--preparation');p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'New owned immutable report required')
    game=ROOT/'working/game-nocd';require(game.is_dir() and not game.is_symlink(),'Owned physical staged input required')
    report=dict(schema=1,kind='native_normal_input_transition',success=False,operation=a.operation)
    before_verified=False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before_verified=True
        with Reader(ROOT) as reader:
            acquired=reader.json(a.acquisition)
            require(acquired.get('private_wine_stopped') is True and (a.operation=='restore' or acquired['success'] is True),'Stopped owned consumer required; preparation also requires success')
            sources={n:reader.digest(n) for n in ['tools/prepare-rendering-normal-input.py','tools/run-opengl-game.py','tools/encode-cfg.py','tools/decode-cfg.py']}
            report.update(sources=sources,acquisition=dict(record=a.acquisition,sha256=reader.digest(a.acquisition)))
            if a.operation=='prepare':
                require(acquired['scenario']=='normal-storage' and not acquired['configuration']['skip_intros'],'Full high-mode storage acquisition required')
                before=[]
                for name in ['CFG/prefs.cfg','CFG/Encrypted/prefs.cfg']:
                    path=game/name
                    if not path.exists():continue
                    require(not path.is_symlink() and path.stat().st_nlink==1 and sha(path)==acquired['installation_inputs'][name]['sha256'],'Staged preference changed since high-mode execution')
                    raw=path.read_bytes();record=str(Path(a.report).with_suffix(''))+'-before-'+name.replace('/','-')+'.bin';h=write_report(ROOT,record,raw);before.append(dict(path=name,sha256=h,record=record))
                report['before']=before
                spec=importlib.util.spec_from_file_location('normal_input_patch',ROOT/'tools/run-opengl-game.py');helper=importlib.util.module_from_spec(spec);spec.loader.exec_module(helper)
                changes=helper.patch_staged_preferences(game,'VIDEO',{'IsHighRes':'FALSE'})
                report.update(before=before,changes=changes,setting=dict(section='VIDEO',IsHighRes='FALSE'))
            else:
                require(a.preparation is not None,'Preparation receipt required for exact restoration')
                preparation=reader.json(a.preparation)
                require(preparation['success'] is True and preparation['operation']=='prepare','Invalid preparation')
                require(all(reader.digest(n)==h for n,h in preparation['sources'].items()),'Changed input patch sources')
                changes={r['path']:r for r in preparation['changes']};restored=[]
                for row in preparation['before']:
                    name=row['path'];require(name in {'CFG/prefs.cfg','CFG/Encrypted/prefs.cfg'},'Unsafe restoration path')
                    path=game/name;require(not path.is_symlink() and path.stat().st_nlink==1 and sha(path)==changes[name]['after_sha256']==acquired['installation_inputs'][name]['sha256'],'Owned low input changed before restoration')
                    raw=reader.read_bytes(row['record']);require(hashlib.sha256(raw).hexdigest()==row['sha256'],'Preserved original preference bytes differ')
                    path.write_bytes(raw);require(sha(path)==row['sha256'],'Restoration verification failed');restored.append(row)
                report.update(preparation=dict(record=a.preparation,sha256=reader.digest(a.preparation)),restored=restored)
        report.update(success=True,scope='Owned staged input bytes only; original files/code remain pinned. Actual RGB565 World extent is established by independent execution, not this setting.')
    except Exception as error:report['error']=str(error)
    finally:
        if before_verified:
            try:
                subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
                report['original_manifest_verified_before_after']=True
            except Exception as error:report.update(success=False,original_verification_error=str(error))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report.get('sources',{}).items())
        report['success'] &= report['sources_stable']
        h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],record=a.report,sha256=h,error=report.get('error'))))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
