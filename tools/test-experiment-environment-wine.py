#!/usr/bin/env python3
"""Check real Wine registry isolation, pooled reuse and retained-state restore."""
import argparse
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import tarfile

import experiment_environment as pool
from coverage_evidence import write_report, read_bytes, evidence_digest
from terrain_validation_display import private_display

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--run',type=Path,required=True)
    parser.add_argument('--report',required=True)
    parser.add_argument('--raw',required=True)
    parser.add_argument('--bind-output',action='store_true',help='Exercise a same-device bind-mounted output group')
    parser.add_argument('--baseline-raw',help='Verified earlier baseline bundle used only to deduplicate unchanged input bytes')
    args=parser.parse_args()
    run=args.run.absolute()
    if not run.is_relative_to(ROOT/'working') or run.exists():
        parser.error('Require a new task-owned run under working/')
    spec=importlib.util.spec_from_file_location('environment_claims',ROOT/'tools/draft-coverage-claims.py')
    declaration=importlib.util.module_from_spec(spec);spec.loader.exec_module(declaration)
    claims=declaration.declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),['HOST.experiment-environment-reuse'],[])
    run.mkdir(parents=True)
    inherited={}
    if args.baseline_raw:
        data=read_bytes(ROOT/args.baseline_raw,root=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data),mode='r:gz') as archive:
            metadata=archive.extractfile('pool.sqlite3').read()
        temporary=run/'baseline-input.sqlite3';temporary.write_bytes(metadata)
        database=sqlite3.connect(temporary.as_uri()+'?mode=ro',uri=True)
        try:
            payload,digest=database.execute('select payload,sha256 from baseline').fetchone()
            if hashlib.sha256(payload).hexdigest()!=digest:raise ValueError('Baseline input manifest changed')
            inherited=json.loads(payload)
        finally:database.close()
    project=run/'project';source=project/'working/template';source.parent.mkdir(parents=True)
    mountpoint=project/'working/tests'
    if args.bind_output:
        data_directory=run/'mounted-output';data_directory.mkdir();mountpoint.mkdir()
        subprocess.run(['mount','--bind',str(data_directory),str(mountpoint)],check=True)
    report=dict(bind_output=args.bind_output,schema=1,success=False,sources=claims['sources'],claims=claims['claims'],original_game_executed=False)
    key=r'HKCU\Software\MnmEnvironmentReuse'
    with private_display(run/'xvfb.log') as display:
        env={k:v for k,v in display.items() if not k.startswith('MNM_')}
        env.update(WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d')
        report['wine_version']=subprocess.check_output(['wine','--version'],env=env,text=True).strip()
        def command(name,prefix,words,expected=0):
            with (run/(name+'.log')).open('xb') as log:
                result=subprocess.run(words,env=dict(env,WINEPREFIX=str(prefix)),stdout=log,stderr=subprocess.STDOUT,timeout=120)
            if (expected==0 and result.returncode) or (expected!=0 and result.returncode==0):
                raise ValueError(name+' returned '+str(result.returncode))
            return result.returncode
        def quiet(prefix):
            local=dict(env,WINEPREFIX=str(prefix))
            result=subprocess.run(['wineserver','-k'],env=local,timeout=15,check=False,capture_output=True)
            if result.returncode not in (0,1):raise ValueError('Owned Wine server stop failed')
            subprocess.run(['wineserver','-w'],env=local,timeout=15,check=True)
        command('bootstrap',source,['wineboot','-i']);quiet(source)
        before=pool._scan(source)[0]
        # Select a regular immutable input, avoiding external Wine symlinks.
        files=[name for name,row in before.items() if row[0]=='file' and name.startswith('drive_c/windows/')]
        retained=max(files,key=lambda n:(source/n).stat().st_size)
        paths=[]
        for name in ('first','second'):
            parent=project/'working/tests'/name;parent.mkdir(parents=True);paths.append(parent/'wineprefix')
        first=pool.acquire(project,paths[0],source)
        inode=(first.target/retained).stat().st_ino
        command('first-add',first.target,['wine','reg','add',key,'/v','Marker','/t','REG_SZ','/d','first-run-only','/f'])
        quiet(first.target);first.close()
        second=pool.acquire(project,paths[1],source)
        report['reused_inode']=(second.target/retained).stat().st_ino==inode
        report['second_registry_clean']=command('second-query',second.target,['wine','reg','query',key,'/v','Marker'],expected=1)!=0
        quiet(second.target);second.close()
        restored=pool.restore(first.receipt)
        command('restored-query',restored,['wine','reg','query',key,'/v','Marker']);quiet(restored)
        report['restored_registry_value']='first-run-only' in (run/'restored-query.log').read_text(errors='replace')
        report['source_unchanged']=pool._scan(source)[0]==before
        report['retained_file']=retained
        # Preserve original baseline bytes, final-state deltas and command logs;
        # the remaining Wine trees are temporary execution output.
        raw=io.BytesIO()
        def retain_new_bytes(info):
            name=info.name.removeprefix('pristine/')
            if info.isfile() and info.name.startswith('pristine/') and name in inherited and inherited[name][0]=='file' and before[name][3]==inherited[name][3]:
                return None
            return info
        if args.baseline_raw:
            report['baseline_parent_record']=args.baseline_raw
            report['baseline_parent_sha256']=evidence_digest(ROOT/args.baseline_raw,root=ROOT)
        with tarfile.open(fileobj=raw,mode='w:gz') as archive:
            archive.add(first.family/'pristine',arcname='pristine',filter=retain_new_bytes)
            archive.add(first.family/'pool.sqlite3',arcname='pool.sqlite3')
            for lease in (first,second):archive.add(lease.receipt,arcname=lease.target.parent.name+'/'+lease.receipt.name)
            for log in sorted(run.glob('*.log')):archive.add(log,arcname='logs/'+log.name)
            for reg in sorted(restored.glob('*.reg')):archive.add(reg,arcname='post-query/'+reg.name)
        report['raw_sha256']=write_report(ROOT,args.raw,raw.getvalue())
        report['raw_record']=args.raw
        report['sources_stable']=all(hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest for name,digest in claims['sources'].items())
        report['success']=all(report[name] for name in ('reused_inode','second_registry_clean','restored_registry_value','source_unchanged','sources_stable'))
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        if args.bind_output:subprocess.run(['umount',str(mountpoint)],check=True)
        print(json.dumps({k:v for k,v in report.items() if k not in ('sources','claims')},indent=2))
        return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
