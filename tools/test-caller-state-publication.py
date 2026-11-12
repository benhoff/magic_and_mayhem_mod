#!/usr/bin/env python3
"""Exercise publication replay with retained original Add observations and mutants."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from coverage_evidence import Reader,write_report

ROOT=Path(__file__).resolve().parents[1]
MAIN=(ROOT/subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--git-common-dir'],text=True).strip()).resolve().parent
PREFIX='research/runtime/caller-state-results-20261108/'


def need(value,message):
    if not value:raise ValueError(message)


def semantic(fx):
    out=bytearray(224);out[:5]=fx[:5];out[8:12]=fx[24:28];top=(fx[3]>>3)&7
    for i in range(8):
        if fx[4]&(1<<((top+i)&7)):out[12+i*10:22+i*10]=fx[32+i*16:42+i*16]
    out[92:220]=fx[160:288];return out


def packet(observed,fx):
    source=observed[3];variant=observed[5];format=observed[1];clip=observed[2]
    width=max(64,observed[36]+16);stride=width+3+source%5
    row=[0]*212;entry=observed[12]-12
    row[:16]=[observed[0],1,26,7,0x54ab16,entry,entry+12,1,0,1,1,2,1234,1234,0,0x700000]
    before=[observed[15],observed[14],observed[13],entry,observed[11],0xd1d2d3d4,0x710000,0xa1a2a3a4,[0x202,0xed7,0x602,0xa97][variant%4]]
    after=[observed[15],observed[14],observed[13],observed[12],observed[11],observed[10],observed[9],observed[8],observed[24]]
    row[16:25]=before;row[25:34]=after;row[34:36]=observed[34:36]
    row[46:62]=observed[48:64];row[62:78]=observed[48:64]
    row[190:192]=1,0x5006b2;row[192:212]=[1,0,1,0,1,0,1,0,123,0,1,1,1,0,0,0,0,4,format,stride]
    state=bytearray(struct.pack('<212I',*row));state[312:536]=semantic(fx);state[536:760]=semantic(fx)
    fields=[0]*24;fields[0]=116;fields[1]=1;fields[2]=14;fields[3]=99;fields[5:8]=width,32,stride
    fields[8:10]=observed[34:36];fields[10:14]=[2 if clip else 0,2 if clip else 0,width-1,31];fields[18]=20;fields[22]=0x54ab16
    shape=[observed[36],observed[37],source%32,source%(32 if format else 64),source*7%32]
    cps=struct.pack('<24I',*fields)+struct.pack('<5I',*shape)
    return bytearray(struct.pack('<8I',32+848+116,116,0,0,0xffffffff,0xffffffff,0xffffffff,0xcd5)+state+cps)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--name',required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--model-build',required=True);a=p.parse_args()
    target=PREFIX+a.name+'.json'
    if not a.name or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-'for c in a.name)or(ROOT/target).exists():p.error('Fresh owned result required')
    declaration=json.loads(a.claims.read_text());sha=lambda data:hashlib.sha256(data).hexdigest()
    sources={n:(ROOT/n).read_bytes()for n in declaration['sources']}
    if any(sha(sources[n])!=h for n,h in declaration['sources'].items()):raise ValueError('Prospective sources changed')
    out=Path(tempfile.mkdtemp(prefix='mnm-publication-controls-',dir='/tmp'))
    report=dict(schema=1,kind='caller_state_publication_controls',success=False,claims=declaration['claims'],sources=declaration['sources'],
        scope='Replay64retained original additive return-state observations across formats/clips/geometry/FP seeds, plus actual byte-corruption controls and36source-identity/conditional-liveness/retained-call/cleanup/expected-byte guards. No new original raster/game, glyph history, installed or whole-row equivalence.',
        original_game_runs=0,new_original_raster_executions=0,newly_passed_requirements=0,commands=[],temporary_directory=str(out))
    def retain(label,data):
        path=PREFIX+a.name+'-'+label;write_report(ROOT,path,data);return dict(path=path,sha256=sha(data),bytes=len(data))
    def run(label,argv,raw=None):
        if raw is not None:retain(label+'-request.bin',raw)
        r=subprocess.run(list(map(str,argv)),input=raw,cwd=ROOT,capture_output=True,timeout=180)
        report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=r.returncode,
            stdout=retain(label+'-stdout.bin',r.stdout),stderr=retain(label+'-stderr.log.bin',r.stderr)))
        return r
    report['source_preimages']={n:retain('source-'+sha(n.encode())[:16]+'.bin',data)for n,data in sources.items()
        if subprocess.run(['git','show','HEAD:'+n],cwd=ROOT,capture_output=True).stdout!=data}
    try:
        r=run('original-before',[MAIN/'tools/original-manifest.sh','verify']);r.check_returncode()
        with Reader(ROOT)as reader:
            build=reader.json(a.model_build)
            need(build['success']and all(reader.digest(n)==h for n,h in build['sources'].items()),'Model build source changed')
            binary=Path(build['physical_binary']);need(sha(binary.read_bytes())==build['binary']['sha256'],'Model binary changed')
            report['model_build']=dict(record=a.model_build,sha256=reader.digest(a.model_build));report['model_binary']=build['binary']
            parent=PREFIX+'add-state-02.json';proof=reader.json(parent);need(proof['success'],'Retained original comparison failed')
            artifact=proof['artifacts']['states.bin'];raw=reader.read_bytes(artifact['path'])
            need(raw[:16]==b'MNMRECT1'+struct.pack('<II',1,1344),'Original additive wire contract')
            report['retained_original_observations']=dict(record=parent,sha256=reader.digest(parent),wire=artifact)
        count=(len(raw)-16)//1344;selected=[i*(count-1)//63 for i in range(64)];packets=[]
        for i in selected:
            at=16+i*1344;observed=struct.unpack_from('<80I',raw,at);need(observed[6]==63,'Unaccepted original additive observation')
            packets.append(packet(observed,raw[at+320:at+832]))
        request=b'MNMCSR01'+struct.pack('<II',1,len(packets))+b''.join(packets)
        original=MAIN/'original/Arcane_Nocd/Chaos.exe'
        result=run('retained-original-cases',[binary,original],request)
        need(result.returncode==0 and len(result.stdout)==16+64*380,'Incomplete publication replay')
        need(result.stdout[:16]==b'MNMCSO01'+struct.pack('<II',1,64),'Publication result contract')
        need(all(struct.unpack_from('<5I',result.stdout,16+i*380+12)==(1,1,1,1,1)for i in range(64)),'Retained original state publication differs')
        corruptions=[]
        for label,offset in [('nonvolatile',32+25*4),('return',32+32*4),('flags',32+33*4),('workspace',32+62*4),('floating',32+536+2),('last-error',32+13*4)]:
            altered=bytearray(packets[0]);altered[offset]^=1
            x=run('mutant-'+label,[binary,original],b'MNMCSR01'+struct.pack('<II',1,1)+altered)
            need(x.returncode==0 and len(x.stdout)==396 and not all(struct.unpack_from('<5I',x.stdout,28)),'Undetected publication corruption '+label)
            corruptions.append(label)
        x=run('source-controls',['python3',ROOT/'tests/test-rendering-caller-state-join.py'])
        need(x.returncode==0 and b'Ran 36 tests'in x.stderr,'Missing source/conditional/retained-call/cleanup/expected-byte controls')
        report.update(success=True,retained_original_cases=64,publication_corruptions=corruptions,source_join_controls=36)
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError,KeyError)as error:report['error']=str(error)
    finally:
        try:run('original-after',[MAIN/'tools/original-manifest.sh','verify']).check_returncode()
        except (OSError,subprocess.SubprocessError)as error:report.update(success=False,manifest_error=str(error))
        report['sources_stable']=all((ROOT/n).read_bytes()==data for n,data in sources.items());report['success'] &= report['sources_stable']
        write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report[k]for k in('success','error','retained_original_cases','publication_corruptions','source_join_controls')if k in report}),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
