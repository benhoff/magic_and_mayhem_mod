"""PE32 publication callbacks and pinned, retained startup replay inputs."""
import hashlib,json,struct,sys
from pathlib import Path
from coverage_evidence import evidence_digest,read_json


def publication(root,out,run):
    folder=out/'publication';folder.mkdir();objects=[]
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    for name in ('tests/drawing-publication-probe.c','runtime/scene/canvas_producers.S'):
        obj=folder/(Path(name).name+'.obj');run('publication-build-'+Path(name).name,[*flags,'-c',root/name,'-o',obj]);objects.append(obj)
    build=root/'working/build/scene-observer-selftest'
    objects.extend(build/(name+'.obj') for name in ('drawing_publication.S','glyph_live.c','glyph_leave.S','glyph_route.c','glyph_backend.c','glyph_backend_state.c','glyph_backend_x87.S'))
    exe=folder/'probe.exe';run('publication-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(exe),*objects,build/'kernel32.lib'])
    cases=[]
    for mode in range(9):
        target=folder/str(mode);target.mkdir()
        run('publication-case-'+str(mode),['xvfb-run','-a','wine',exe],cwd=target,extra=dict(MNM_PUBLICATION_CASE=str(mode),MNM_SCENE_DIR='Z:'+str(target).replace('/',chr(92))))
        metrics=struct.unpack('<8I',(target/'publication-controls.bin').read_bytes());row=struct.unpack('<24I',(target/'publication.bin').read_bytes())
        if metrics[0]!=mode or metrics[6]!=0x51a7 or row[2]!=metrics[7] or row[3]!=0x51a7 or row[4:10]!=(5,1,2,1,1,0x10 if mode==1 else 0x1000000) or (row[19],row[20])!=(metrics[3],metrics[4]):raise ValueError('Publication result/identity changed')
        if metrics[3] and (target/'publication-oracle-0001.565').read_bytes()!=struct.pack('<6H',1,2,3,4,5,6):raise ValueError('Padded primary rows changed')
        cases.append(dict(mode=mode,lock_calls=metrics[1],unlock_calls=metrics[2],frames=metrics[3],drops=metrics[4],original_hresult=row[2],last_error=row[3],padded_rows_checked=bool(metrics[3])))
    return dict(cases=cases,binary_sha256=evidence_digest(exe,root=root),actual_driver_executed=False,observer_trampoline_abi_claimed=False)


def retained_inputs(root,out):
    from rendering_thirteenth_controls import diagnostic_inputs
    main=Path('/home/hoff/swdev/magic_and_mayhem_mod')
    relative=Path('working/worktrees/draw02-end-to-end/working/experiments/drawing-api-session/run-wskpjhnh/capture')
    restored=Path('/var/tmp/mnm-rendering-thirteenth-ten-20261022')/relative
    if not (restored/'canvas-producers.bin').is_file():raise ValueError('Verified restored original capture is unavailable')
    local=root/relative;local.parent.mkdir(parents=True,exist_ok=True)
    if not local.exists():local.symlink_to(restored,target_is_directory=True)
    if local.resolve()!=restored.resolve():raise ValueError('Retained input owner differs')
    result=diagnostic_inputs(root,out)
    result['restore_provenance']=Path('/var/tmp/mnm-rendering-thirteenth-ten-20261022/working/tests/retained-original-inputs/restore.sqlite3')
    return result
