#!/usr/bin/env python3
"""Execute actual PE32 private backing, alias, VM and worker/fence controls."""
import argparse,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from drawing_api_session import sha
from drawing_physical_owners import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--handoff',action='store_true');p.add_argument('--ownership',action='store_true');p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--shared-fixture-prefix',type=Path);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.ownership:a.handoff=True
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective fixture sources changed')
    stable();parent=ROOT/'working/tests/drawing-physical-owners';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Physical owner PE32 controls and negative journals.\n')
    spec=importlib.util.spec_from_file_location('builder',ROOT/'tools/build-drawing-session.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder);dll=builder.build(True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-physical-owners-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(dll.parent/'packet.obj'),str(dll.parent/'kernel32.lib')],check=True)
    from drawing_fixture_prefix import prepare
    prefix,lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW_SESSION_MODE='physical',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RESOURCE_COPY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1',MNM_DRAW_WINE_SOURCE_MAP='2',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_WINE_SOURCE_VERSION='1',MNM_DRAW_WINE_GPU_COMPLETION='1',MNM_DRAW_WINE_SOURCE_RESERVATIONS='1',MNM_DRAW_WINE_PHYSICAL_OWNERS='1')
    cases=[('0',0,None),('1',99,'fault'),('2',99,3),('3',99,12),('4',99,12),('5',99,6),('6',99,3),('7',99,14),('8',99,4),('9',0,None),('a',99,7),('b',0,None),('c',99,8),('d',0,None),('e',0,None),('f',99,7),('g',99,7),('h',0,'preflight'),('i',0,'preflight'),('j',0,'off'),('k',0,None),('l',0,None),('m',99,'fault'),('n',99,18),('o',99,18),('p',99,18),('q',99,12),('r',99,12),('s',99,12),('t',99,12),('u',99,19),('v',0,None),('w',99,3),('x',0,None),('y',99,18),('[',0,None)]
    if a.ownership:env['MNM_DRAW_WINE_SOURCE_OWNERSHIP']='1'
    if a.handoff:
        env['MNM_DRAW_WINE_SOURCE_HANDOFF']='1'
        cases=[('0',0,None),('l',0,None),('z',0,None),('2',0,None),('1',99,'fault'),('3',99,'handoff-6'),('a',99,'handoff-40'),('p',99,'handoff-5'),('{',99,'handoff-15'),('c',99,'handoff-9'),('U',99,'handoff-3'),('V',99,'handoff-3'),('X',0,None),('Y',0,'preflight'),('Z',0,'preflight'),('[',0,None)]
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-i' if a.shared_fixture_prefix else '-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        registry=run/'registry';registry.mkdir()
        registry_env=dict(env,MNM_TEST_PHYSICAL='W',MNM_DRAW_SESSION_DIRECTORY='Z:'+str(registry).replace('/',chr(92)))
        registry_env.pop('MNM_DRAW_WINE_SOURCE_OWNERSHIP',None)
        with (registry/'wine.log').open('x') as out:subprocess.run(['wine',str(binary)],env=registry_env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=30)
        results=[]
        for case,expected,reason in cases:
            target=run/('case-'+case);target.mkdir();trial=dict(env,MNM_TEST_PHYSICAL=case,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
            if reason=='preflight':trial.pop('MNM_DRAW_WINE_SOURCE_OWNERSHIP',None)
            if reason=='off':trial.pop('MNM_DRAW_WINE_PHYSICAL_OWNERS')
            with (target/'wine.log').open('x') as out:x=subprocess.run(['wine',str(binary)],env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if x.returncode!=expected:raise ValueError(f'Case {case}: exit {x.returncode}, expected {expected}; {target}')
            raw=(target/'wine-physical-owners.bin').read_bytes() if reason!='off' else b'';analysis=None
            if reason is None:analysis=decode(raw)
            elif reason=='fault':
                fault=(target/'wine-physical-owner-fault.bin').read_bytes()
                if fault[:8]!=b'MNMPFLT1' or len(fault)!=64:raise ValueError('Missing actual raw CPU fault')
            elif isinstance(reason,str) and reason.startswith('handoff-'):
                handoff_raw=(target/'wine-source-handoff.bin').read_bytes();last=struct.unpack_from('<64I',handoff_raw,len(handoff_raw)-256)
                if last[1]!=9 or last[16]!=int(reason.split('-')[1]):raise ValueError(f'Unexpected handoff refusal: {case} {last[:21]}')
            elif isinstance(reason,int):
                last=struct.unpack_from('<32I',raw,len(raw)-128)
                if last[1]!=5 or last[7]!=reason:raise ValueError(f'Case {case}: incorrect refusal {last[:8]}')
            if reason not in (None,'off'):
                try:decode(raw)
                except ValueError:pass
                else:raise ValueError('Incomplete/failed ownership accepted')
            results.append(dict(case=case,exit_code=x.returncode,refusal=reason,journal_sha256=sha(raw) if raw else None,analysis=analysis))
        window_controls=[]
        if a.handoff:
            for mode,expected,reason in [(1,99,'handoff-3'),(2,99,'handoff-3'),(3,99,'handoff-3'),(4,0,None),(5,99,'handoff-3'),(6,0,None),(7,99,'fault'),(8,99,'fault'),(9,99,'handoff-6'),(10,99,'physical-4'),(11,99,'physical-3'),(12,99,'handoff-7'),(13,99,'handoff-6')]:
                target=run/('window-'+str(mode));target.mkdir();trial=dict(env,MNM_TEST_PHYSICAL='0',MNM_TEST_RETIRE=chr(48+mode),MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
                with (target/'wine.log').open('x') as out:x=subprocess.run(['wine',str(binary)],env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
                if x.returncode!=expected:raise ValueError(f'Ownership window {mode}: {x.returncode}, expected {expected}: {target}')
                if reason=='fault':
                    fault=(target/'wine-physical-owner-fault.bin').read_bytes()
                    if fault[:8]!=b'MNMPFLT1' or len(fault)!=64:raise ValueError('Missing protected staging fault')
                elif reason and reason.startswith('physical-'):
                    raw=(target/'wine-physical-owners.bin').read_bytes();last=struct.unpack_from('<32I',raw,len(raw)-128)
                    if last[1]!=5 or last[7]!=int(reason.split('-')[1]):raise ValueError('Wrong staging alias refusal')
                elif reason:
                    raw=(target/'wine-source-handoff.bin').read_bytes();last=struct.unpack_from('<64I',raw,len(raw)-256)
                    if last[1]!=9 or last[16]!=int(reason.split('-')[1]):raise ValueError('Wrong retirement/VM refusal')
                window_controls.append(dict(case=mode,exit_code=x.returncode,refusal=reason))
        admission_controls=[]
        if a.ownership:
            for mode in ['0','5','6','7','8','9','HANDOFF','PHYSICAL_OWNERS','SOURCE_VERSION','GPU_COMPLETION','BLIT_WORKER','BLIT_PACKET','SUBMISSION','DIRECTORY','INVALID']:
                target=run/('admission-'+mode);target.mkdir();trial=dict(env,MNM_TEST_PHYSICAL='0',MNM_TEST_OWNERSHIP=mode if len(mode)==1 else 'I',MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
                if mode=='DIRECTORY':trial.pop('MNM_DRAW_SESSION_DIRECTORY')
                elif mode=='INVALID':trial['MNM_DRAW_WINE_SOURCE_OWNERSHIP']='0'
                elif len(mode)>1:trial.pop('MNM_DRAW_WINE_'+('SOURCE_HANDOFF' if mode=='HANDOFF' else mode))
                with (target/'wine.log').open('x') as out:x=subprocess.run(['wine',str(binary)],env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
                expected=99 if mode in ['5','6','7','8','9'] else 0
                if x.returncode!=expected:raise ValueError(f'Ownership admission {mode}: {x.returncode}, expected {expected}: {target}')
                admission_controls.append(dict(case=mode,exit_code=x.returncode))
        from drawing_physical_owners import analyze
        from drawing_source_reservations import decode as reserve_decode
        import copy,shutil
        positive=run/'case-0';reservation=reserve_decode((positive/'wine-source-reservations.bin').read_bytes());joined=analyze(positive,dict(wine_source_reservation_observation=reservation));mutations=[]
        for label,name,offset,value in [('gpu-status','wine-gpu-completion.bin',64+8*4,5),('release-thread','wine-physical-owners.bin',None,999),('reservation-command','wine-source-reservations.bin',64+2*128+12*4,999),('packet-reserved','wine-blit-packet.bin',0,0)]:
            target=run/('mutation-'+label);target.mkdir()
            for artifact in ('wine-physical-owners.bin','wine-physical-owner-fault.bin','wine-source-reservations.bin','wine-gpu-completion.bin','wine-blit-packet.bin'):shutil.copyfile(positive/artifact,target/artifact)
            data=bytearray((target/name).read_bytes())
            if offset is None:
                offset=next(64+i*128+2*4 for i,r in enumerate(struct.iter_unpack('<32I',data[64:])) if r[1]==3)
            struct.pack_into('<I',data,offset,value);(target/name).write_bytes(data)
            try:analyze(target,dict(wine_source_reservation_observation=reservation))
            except ValueError:mutations.append(dict(case=label,refused=True))
            else:raise ValueError('Forged physical ownership join accepted: '+label)
        forged=copy.deepcopy(reservation);forged['bindings'][0]['command']=999
        try:analyze(positive,dict(wine_source_reservation_observation=forged))
        except ValueError:mutations.append(dict(case='forged-report',refused=True))
        else:raise ValueError('Forged report accepted')
        parser=subprocess.run(['python3',str(ROOT/'tests/test-drawing-physical-owners.py')],capture_output=True,text=True,check=True)
        handoff_parser=subprocess.run(['python3',str(ROOT/'tests/test-drawing-source-handoff.py')],capture_output=True,text=True,check=True)
        handoffs=None
        if a.handoff:
            from drawing_source_handoff import analyze as analyze_handoff
            handoffs={}
            for label in ('0','l','z','2','X','['):
                target=run/('case-'+label);r=reserve_decode((target/'wine-source-reservations.bin').read_bytes());handoffs[label]=analyze_handoff(target,dict(wine_source_reservation_observation=r))
        ownership_analysis=None
        if a.ownership:
            from drawing_source_ownership import analyze as analyze_ownership
            ownership_analysis=analyze_ownership(positive,dict(wine_source_reservation_observation=reservation))
            subprocess.run(['python3','-B',str(ROOT/'tests/test-drawing-source-ownership.py')],check=True)
        stable();a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,heap_collision_control_passed=True,handoff_mode=a.handoff,ownership_mode=a.ownership,window_controls=window_controls,admission_controls=admission_controls,ownership_analysis=ownership_analysis,handoff_analyses=handoffs,raw_join=joined,join_mutations=mutations,parser_checks=parser.stderr,handoff_parser_checks=handoff_parser.stderr,original_game_executed=False,scope='Restricted private Wine system-memory allocation policy; real PE32 page faults and cached NtProtect/NtFree hooks, distinct-resource client alias refusal, exact worker/fence lease retirement. No unknown external backing or global host/driver alias proof.'),indent=2)+'\n');print(json.dumps(dict(success=True,cases=len(results))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if lock is not None:lock.close()


if __name__=='__main__':main()
