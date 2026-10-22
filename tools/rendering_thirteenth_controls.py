"""Bounded native/provider controls and retained-original diagnostic execution."""
import hashlib,importlib.util,json,os,shutil,struct,sys
from pathlib import Path
from coverage_evidence import read_json,evidence_digest
from rendering_twelfth_controls import last_json


def providers(root,out,run,env):
    run('production-drawing',[sys.executable,'-B',root/'tools/build-drawing-session.py'])
    run('selftest-drawing',[sys.executable,'-B',root/'tools/build-drawing-session.py','--selftest'])
    dll=root/'working/build/drawing-session-selftest/MnmDrawing.dll'
    # The linked production packet/kernel objects come from the current builder.
    result=[]
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    from drawing_source_handoff import analyze
    from drawing_source_reservations import decode as reservations
    for direct,source in [(False,'drawing-backing-provider-test.c'),(True,'drawing-direct-backing-test.c')]:
        group=out/('direct-provider' if direct else 'private-provider');group.mkdir();exe=group/'fixture.exe';obj=group/'fixture.obj'
        run(group.name+'-compile',[*flags,'-MD','-MF',group/'fixture.d','-c',root/'tests'/source,'-o',obj])
        run(group.name+'-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(exe),obj,dll.parent/'packet.obj',dll.parent/'kernel32.lib'])
        settings=dict(MNM_DRAW_SESSION_MODE='physical',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RESOURCE_COPY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1',MNM_DRAW_WINE_SOURCE_MAP='2',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_WINE_SOURCE_VERSION='1',MNM_DRAW_WINE_GPU_COMPLETION='1',MNM_DRAW_WINE_SOURCE_RESERVATIONS='1',MNM_DRAW_WINE_PHYSICAL_OWNERS='1',MNM_DRAW_WINE_SOURCE_HANDOFF='1',MNM_DRAW_WINE_SOURCE_OWNERSHIP='2' if direct else '1')
        for kind in range(1,5):
            faults=[0,1,2,3,4,5,6,7,8,10,11,12]+([13] if direct else [])+([9] if kind==4 else [])
            for fault in faults:
                target=group/f'kind-{kind}-fault-{fault}';target.mkdir()
                trial=dict(settings,MNM_TEST_BACKING=str(kind),MNM_TEST_BACKING_FAULT=chr(48+fault),MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
                run(group.name+f'-{kind}-{fault}',['wine',exe],expected=0 if fault==0 else 99,extra=trial,timeout=60)
                item=dict(direct=direct,kind=kind,fault=fault,expected_exit=0 if fault==0 else 99)
                if fault==0:
                    res=reservations((target/'wine-source-reservations.bin').read_bytes());analysis=analyze(target,dict(wine_source_reservation_observation=res))
                    assert analysis['bindings'][0]['provider']['kind']==kind
                    assert analysis['direct_provider_leases' if direct else 'external_provider_leases']==1
                    if direct:
                        assert not analysis['private_copy_value_ownership_closed'] and analysis['source_binding_valid_before_api_return']
                    item['analysis']=analysis
                elif direct and fault==5:
                    raw=(target/'wine-source-map.bin').read_bytes();assert raw[:8]==b'MNMWMS05' and struct.unpack_from('<64I',raw,64)[37]==0
                    item['unsupported_without_provider']=True
                else:
                    raw=(target/'wine-source-handoff.bin').read_bytes();last=struct.unpack_from('<64I',raw,len(raw)-256);assert last[1]==9
                    expected=(4 if fault==7 else 64 if fault in (8,13) else 61) if direct else (40 if fault==5 and kind in (1,3) else 45 if fault==5 and kind==4 else 12 if fault==5 else 14 if fault==7 else 64 if fault==8 else 61)
                    assert last[16]==expected,(direct,kind,fault,last[16],expected);item['refusal_reason']=expected
                result.append(item)
        if direct:
            target=group/'private-profile-refuses';target.mkdir()
            run('private-profile-refuses-v2',['wine',exe],extra=dict(settings,MNM_TEST_BACKING='4',MNM_DRAW_WINE_SOURCE_OWNERSHIP='1',MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92))))
    return dict(cases=result,source_contracts=['private binding transfer/restoration','direct original-binding read lease'],actual_foreign_gpu_or_driver_executed=False)


def diagnostic_inputs(root,out):
    main=Path('/home/hoff/swdev/magic_and_mayhem_mod')
    original_owner=main/'working/worktrees/draw02-end-to-end'
    reference=root/'research/runtime/draw02-aux-save-readback-original120-01.json'
    original=read_json(reference,root=root)
    old_capture=Path(original['experiment'])/'capture';capture=root/old_capture.relative_to(main)
    assert (capture/'canvas-producers.bin').is_file(),'Complete cold restore first'
    manifest=original_owner/'working/reference-input/game-nocd-manifest.json';pin=read_json(manifest)
    template=read_json(root/'research/runtime/draw02-aux-save-readback-checkpoints-01.json',root=root)
    assets_reference=root/'research/runtime'/Path(template['assets_reference']['path']).name;assets_report=read_json(assets_reference,root=root)
    owned_game=out/'retained-game';owned_game.mkdir()
    stage=root/assets_report['artifacts']/'inputs/assets';stage.mkdir(parents=True,exist_ok=True)
    normalized={}
    for name,digest in pin['staged_files'].items():
        key=name.replace('\\','/').casefold()
        if key in normalized:assert normalized[key][1]==digest
        normalized[key]=(name,digest)
    copied={}
    from drawing_scene_corpus import asset_name
    for name,digest in assets_report['source_assets'].items():
        key=asset_name(name).casefold();relative,expected=normalized[key];assert digest==expected
        if key in copied:assert copied[key]==digest;continue
        candidates=[Path(original['experiment'])/'game',main/'working/game-clean']
        found=None
        for base in candidates:
            path=base
            for part in relative.replace('\\','/').split('/'):
                if not path.is_dir():break
                matches=[p for p in path.iterdir() if p.name.casefold()==part.casefold()]
                if len(matches)!=1:break
                path=matches[0]
            else:
                if path.is_file():
                    with path.open('rb') as stream:actual=hashlib.file_digest(stream,'sha256').hexdigest()
                    if actual==digest:found=path;break
        if found is None:raise ValueError('Retained encoded input missing: '+relative)
        for target in (stage/key,owned_game/relative.replace('\\','/')):
            target.parent.mkdir(parents=True,exist_ok=True)
            if not target.exists():shutil.copyfile(found,target)
            with target.open('rb') as stream:assert hashlib.file_digest(stream,'sha256').hexdigest()==digest
        copied[key]=digest
    return dict(reference=reference,assets_reference=assets_reference,input_manifest=manifest,bindings=root/'research/runtime/draw02-aux-save-readback-bindings-01.json',capture=capture,original_capture=old_capture,game=owned_game,original_game=Path(original['experiment'])/'game',encoded_inputs=copied)


def diagnostic_replays(root,out,run,executable,claims,inputs,report_directory):
    # Current commands run over restored identical capture bytes through read-only
    # namespace bindings. No original report or historical capture is rewritten.
    shared=['--reference',inputs['reference'],'--executable',executable,'--claims',claims]
    reports={}
    def execute(label,tool,args):
        report=report_directory/(label+'.json')
        run('diagnostic-'+label,[sys.executable,'-B',root/'tools'/tool,*args,'--report',report],isolated=True,bindings=[(inputs['capture'],inputs['original_capture']),(inputs['game'],inputs['original_game'])],timeout=900)
        value=read_json(report,root=root)
        if value.get('native_exit',value.get('native_exit_code',0))!=0:raise ValueError('Native diagnostic did not complete '+label)
        reports[label]=dict(path=report.relative_to(root).as_posix(),sha256=evidence_digest(report,root=root),success=value['success'],comparisons=len(value.get('comparisons',[])),excluded=len(value.get('excluded',[])),failed=sum(bool(v.get('different_pixels')) for v in value.get('comparisons',[])),native_input_sha256=value.get('native_input_sha256'))
        return report,value
    prediction,_=execute('source-prediction','compare-drawing-source-points.py',[*shared,'--input-manifest',inputs['input_manifest'],'--display-conversion','normalized-nearest'])
    held,_=execute('held-egl','compare-drawing-held-egl.py',[*shared,'--input-manifest',inputs['input_manifest'],'--display-conversion','normalized-nearest'])
    checkpoint,cp=execute('copy-checkpoints','compare-drawing-copy-checkpoints.py',[*shared,'--input-manifest',inputs['input_manifest'],'--assets-reference',inputs['assets_reference']])
    values,v=execute('copy-values','analyze-drawing-copy-order-values.py',[*shared,'--template',checkpoint,'--bindings',inputs['bindings']])
    aux,a=execute('aux-readback','compare-drawing-aux-readback.py',[*shared,'--template',checkpoint,'--bindings',inputs['bindings']])
    if not cp['comparisons'] or not v['success'] or not a['comparisons']:raise ValueError('Missing complete diagnostic cohort')
    # Negative comparisons remain negative; implementation proof checks complete
    # accounting and unchanged native history, never an expected parity outcome.
    assert a['checkpoints_unchanged'] and v['checkpoints_unchanged']
    return reports
