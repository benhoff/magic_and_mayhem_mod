#!/usr/bin/env python3
"""Execute ten bounded native rendering adapter contracts."""
import argparse
import ast
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile
from coverage_evidence import write_report
from drawing_fixture_prefix import prepare

ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.drawing-producer-source-points', 'NR.drawing-primitive-api-binding', 'NR.drawing-wine-resource-mapping', 'NR.drawing-wine-map-storage-point', 'NR.drawing-wine-client-memory-binding', 'NR.drawing-wine-retained-map-source', 'NR.drawing-wine-blit-packet-binding', 'NR.drawing-wine-egl-worker-context', 'NR.drawing-hud-state-observation', 'NR.drawing-actor-latch-observation']
SCENARIO='NR.sixth-ten.identities-20261022'
DECODER_TESTS=['producer-points','primitive-api','primitive-lifetimes','wine-resource-map','wine-map-storage-point','wine-client-memory','wine-retained-map-source','wine-blit-packet','worker-context','hud-state','actor-latches','actor-latch-binding']


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wine-prefix',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--prefix-owner-root',type=Path,default=ROOT)
    args=parser.parse_args()
    report_path=args.report if args.report.is_absolute() else ROOT/args.report
    if report_path.exists():parser.error('Refusing evidence overwrite')
    parent=ROOT/'working/tests/rendering-sixth-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Sixth rendering batch raw execution, binaries and failed attempts.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('sixth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    # Pin both production compilers' full source closure before execution.
    todo=set(declaration['sources'])|{'tools/test-rendering-sixth-ten.py','tools/coverage_evidence.py',
      'tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/build-drawing-session.py',
      'tools/build-scene-observer.py','tools/build-menu-observer.py','tools/build-shadow-bridge.py',
      'tools/drawing_api_session.py','tools/drawing_fixture_prefix.py','tools/drawing_lifetime_index.py',
      'tests/test-rendering-sixth-ten.py','tests/drawing-source-pair-guards-test.c','tests/drawing-source-export-test.c','tests/drawing-storage-point-test.c','tests/drawing-wine-blit-packet-call.S','tests/drawing-actor-latch-region-test.c','tools/drawing_resource_index.py','tools/test-drawing-egl-worker-context.py','tools/build-drawing-x11-delivery.py','tests/drawing-egl-delivery-test.c','tools/drawing_x11_delivery.py','tests/test-drawing-call-intervals.py','tools/index-coverage-sources.py','tools/check-re-coverage.py'}
    for directory in ('runtime/drawing','runtime/scene','runtime/menu','runtime/display'):
        todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).iterdir() if p.suffix in {'.c','.h','.S'})
    todo.update('reconstruction/rendering/'+name for name in ('glyph_backend.c','glyph_backend_state.c','glyph_backend_x87.S'))
    todo.update('tests/test-drawing-'+name+'.py' for name in DECODER_TESTS)
    visited=set()
    while todo:
        name=todo.pop()
        if name in visited:continue
        visited.add(name);path=ROOT/name;declaration['sources'][name]=sha(path)
        if path.suffix=='.py':
            for node in ast.walk(ast.parse(path.read_text())):
                if isinstance(node,ast.Constant) and isinstance(node.value,str) and node.value.endswith('.py'):
                    for candidate in (ROOT/node.value,path.parent/node.value,ROOT/'tools'/node.value,ROOT/'tests'/node.value):
                        if candidate.is_file() and candidate.resolve().is_relative_to(ROOT):todo.add(candidate.resolve().relative_to(ROOT).as_posix())
                names=([alias.name for alias in node.names] if isinstance(node,ast.Import) else
                  [node.module]+[node.module+'.'+alias.name for alias in node.names] if isinstance(node,ast.ImportFrom) and node.module else [])
                for name in names:
                    relative=Path(*name.split('.'))
                    for base in (ROOT/'tools',ROOT/'protocols/python'):
                        for candidate in (base/relative.with_suffix('.py'),base/relative/'__init__.py'):
                            if candidate.is_file():todo.add(candidate.relative_to(ROOT).as_posix())
        if path.suffix in {'.c','.cpp','.h','.hpp','.S','.inc'}:
            for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
                child=(path.parent/include).resolve()
                if child.is_file() and child.is_relative_to(ROOT):todo.add(child.relative_to(ROOT).as_posix())
    report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],
      commands=[],controls={},scope=__doc__,original_game_executed=False,original_artifacts_consumed=False,
      live_capture_integrated=False,original_comparison_claimed=False,live_replacement=False,
      reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix())
    (out/'tmp').mkdir()
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env['TMPDIR']=str(out/'tmp')
    env.update(WINEPREFIX=str(args.wine_prefix.resolve()),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',
      __GLX_VENDOR_LIBRARY_NAME='mesa',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
    def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=180):
        command=[str(x) for x in command];print(label,flush=True)
        log=out/(label+'.log')
        with log.open('x') as stream:
            result=subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},stdout=stream,stderr=subprocess.STDOUT,timeout=timeout)
        report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=result.returncode,log_sha256=sha(log)))
        if result.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(f'{label}: expected {expected}, got {result.returncode}; {log}')
        return log.read_text()
    def dependencies(path):
        for token in shlex.split(path.read_text().replace('\\\n',' ').split(':',1)[1]):
            p=Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                name=p.relative_to(ROOT).as_posix()
                if name not in report['sources'] or sha(p)!=report['sources'][name]:raise ValueError('Undeclared compiler input: '+name)
    lock=None
    try:
        # Initialization is allowed only in an explicitly marked fixture-suite
        # directory. prepare() checks its owner and exclusive lock before Wine.
        prefix=args.wine_prefix.resolve()
        owner=args.prefix_owner_root.resolve()
        if owner!=ROOT and (owner.parent!=ROOT.parent or not (owner/'.git').is_file()):raise ValueError('Prefix owner must be this worktree or a registered sibling')
        if not prefix.is_relative_to(owner/'working/tests/drawing-fixture-prefix-suite') or prefix.name!='wineprefix':raise ValueError('Use an owned fixture-suite prefix')
        prefix,lock=prepare(owner,out,prefix,prefix)
        report['prefix']=dict(path=str(prefix),exclusive_lock=True,idle_before_use=True)
        report['wine_version']=run('wine-version',['wine','--version']).strip()
        run('wineboot',['wineboot','-u'],timeout=120)
        run('drawing-production',[sys.executable,'-B',ROOT/'tools/build-drawing-session.py'])
        run('drawing-selftest-build',[sys.executable,'-B',ROOT/'tools/build-drawing-session.py','--selftest'])
        run('scene-production',[sys.executable,'-B',ROOT/'tools/build-scene-observer.py'])
        for directory in ('drawing-session','drawing-session-selftest','scene-observer'):
            build=ROOT/'working/build'/directory
            data=json.loads((build/'manifest.json').read_text())
            for name,digest in data['sources'].items():
                if report['sources'].get(name)!=digest:raise ValueError('Unpinned production build input: '+name)
        run('menu-production',[sys.executable,'-B',ROOT/'tools/build-menu-observer.py'])
        menu=json.loads((ROOT/'working/build/menu-observer/manifest.json').read_text())
        for name,digest in menu['compiled_sources'].items():
            if report['sources'].get(name)!=digest:raise ValueError('Unpinned menu input: '+name)
        drawing=ROOT/'working/build/drawing-session-selftest'
        definitions=(drawing/'kernel32.def').read_text()+'GetExitCodeThread@8\nCreateMutexA@12\nReleaseMutex@4\nCreateEventA@16\nSetEvent@4\nDuplicateHandle@28\nSetEnvironmentVariableA@8\n'
        definition=out/'kernel32.def';definition.write_text(definitions)
        run('kernel-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function','-ffunction-sections','-fdata-sections']
        probes=['drawing-source-pair-guards','drawing-source-export','drawing-primitive-lifetimes',
          'drawing-wine-resource-map','drawing-wine-map-storage-point','drawing-wine-client-memory',
          'drawing-wine-retained-map-source','drawing-wine-blit-packet','drawing-wine-worker-context',
          'drawing-hud-state','drawing-actor-latches','drawing-actor-latch-region','drawing-storage-point']
        full={'drawing-source-pair-guards','drawing-wine-resource-map','drawing-wine-map-storage-point',
          'drawing-wine-client-memory','drawing-wine-retained-map-source','drawing-wine-blit-packet','drawing-wine-worker-context'}
        packet_call=out/'packet-call.obj'
        run('packet-call-compile',[*flags,'-c',ROOT/'tests/drawing-wine-blit-packet-call.S','-o',packet_call])
        for name in probes:
            obj=out/(name+'.obj');binary=out/(name+'.exe');dep=out/(name+'.d')
            run(name+'-compile',[*flags,'-MD','-MF',dep,'-c',ROOT/('tests/'+name+'-test.c'),'-o',obj])
            dependencies(dep)
            libs=([drawing/'packet.obj'] if name in full else [])+([packet_call] if name in {'drawing-wine-blit-packet','drawing-wine-worker-context'} else [])
            if name=='drawing-source-export':
                scene=ROOT/'working/build/scene-observer'
                libs += [scene/(n+'.obj') for n in ('canvas_producers.S','drawing_publication.S','glyph_live.c','glyph_leave.S','glyph_route.c','glyph_backend.c','glyph_backend_state.c','glyph_backend_x87.S')]
                libs += [scene/'kernel32.lib']
            run(name+'-link',['lld-link','/machine:x86','/base:0x18000000','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/opt:ref','/out:'+str(binary),obj,*libs,out/'kernel32.lib'])
            report['controls'][name]=dict(binary_sha256=sha(binary))
        def pe(name,label=None,extra=None,expected=0):
            target=out/(label or name);target.mkdir()
            values=dict(MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_SESSION_MODE='physical',
              MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/','\\'),MNM_FIXTURE_DIRECTORY='Z:'+str(target).replace('/','\\'))
            run(label or name,['wine',out/(name+'.exe')],expected,{**values,**(extra or {})},target,timeout=45)
            return target
        import struct
        from drawing_wine_resource_mapping import decode as resource_decode
        from drawing_wine_submission import decode as submission_decode
        from drawing_wine_map_storage_point import decode as storage_decode
        from drawing_wine_client_memory import decode as client_decode
        from drawing_wine_retained_map_source import decode as retained_decode
        from drawing_wine_blit_packet import decode as packet_decode
        from drawing_worker_context import channel
        from drawing_primitive_api import decode as primitive_decode
        from drawing_hud_state import decode as hud_decode
        from drawing_actor_latches import decode as latch_decode
        pe('drawing-source-export');pe('drawing-storage-point')
        pair_cases=[]
        for mode in range(10):
            target=pe('drawing-source-pair-guards','source-pair-'+str(mode),{'MNM_PAIR_FIXTURE_CASE':str(mode)},97 if mode else 0)
            raw=(target/'api-producer-points.bin').read_bytes()
            if mode and len(raw)!=64:raise ValueError('Malformed pair reached journal')
            if not mode and len(raw)!=64+4*192:raise ValueError('Paired source phase closure')
            pair_cases.append(dict(mode=mode,exit_code=97 if mode else 0,journal_sha256=hashlib.sha256(raw).hexdigest()))
        report['controls']['drawing-source-pair-guards']['cases']=pair_cases
        primitive_cases=[]
        for mode,expected in [('normal',0),('invalid',0),('prerequisite',0),('budget',99),('write',99),('x',99),('c',0)]:
            target=out/('primitive-'+mode)
            target=pe('drawing-primitive-lifetimes','primitive-'+mode,dict(MNM_DRAW_PRIMITIVE_LIFETIMES='0' if mode=='invalid' else '1',MNM_FIXTURE_MODE=mode,MNM_SCENE_DIR='Z:'+str(target).replace('/','\\')),expected)
            if mode=='normal':
                value=primitive_decode((target/'api-primitive-identities.bin').read_bytes())
                if value['active_primitive_calls']!=1:raise ValueError('Active primitive not recorded')
                report['controls']['drawing-primitive-lifetimes']['result']=value
            if mode=='c' and len((target/'api-primitive-identities.bin').read_bytes())!=320:raise ValueError('Primitive query wrote after close')
            primitive_cases.append(dict(mode=mode,expected_exit=expected))
        report['controls']['drawing-primitive-lifetimes']['cases']=primitive_cases
        for mode in ('normal','m','p','n'):
            target=pe('drawing-wine-resource-map','resource-'+mode,dict(MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_RESOURCE_FIXTURE_CASE='' if mode=='normal' else mode))
            raw=(target/'wine-resource-map.bin').read_bytes()
            if mode=='m':
                try:resource_decode(raw)
                except ValueError:pass
                else:raise ValueError('Forced midcall resource closure accepted')
            else:
                value=resource_decode(raw)
                if mode=='normal' and (len(value['calls'])!=7 or value['open_map_intervals']):raise ValueError('Resource closure')
                if mode=='n':
                    row=value['calls'][-1]
                    if row['object'] or not row['unknown_unmap'] or row['map_interval'] or row['data']:raise ValueError('Null inherited stale interval')
                report['controls']['drawing-wine-resource-map'][mode]=value
        for mode in ('normal','close','failed','invalid'):
            target=pe('drawing-wine-map-storage-point','storage-'+mode,dict(MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_MAP_STORAGE='1',MNM_STORAGE_FIXTURE_CLOSE_QUERY='1' if mode=='close' else '0',MNM_STORAGE_FIXTURE_FAILED_QUERY='1' if mode=='failed' else '2' if mode=='invalid' else '0'),99 if mode=='invalid' else 0)
            raw=(target/'wine-map-storage-point.bin').read_bytes()
            if mode=='close':
                if len(raw)!=160:raise ValueError('Storage query wrote after close')
            elif mode=='invalid':
                if len(raw)!=64:raise ValueError('Invalid export fabricated point')
            else:
                value=storage_decode(raw,resource_decode((target/'wine-resource-map.bin').read_bytes()))
                if len(value['points'])!=5 or value['matched']!=(0 if mode=='failed' else 1):raise ValueError('Map storage closure')
                if mode=='failed' and any(p['export_status'] or any(p['metadata']) for p in value['points']):raise ValueError('Failed export retained bytes')
                report['controls']['drawing-wine-map-storage-point'][mode]=value
        target=pe('drawing-wine-client-memory',extra=dict(MNM_DRAW_WINE_CLIENT_MEMORY='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1'))
        submissions=(target/'wine-submission.bin').read_bytes();life=(target/'wine-lifetime.bin').read_bytes()
        value=client_decode((target/'wine-client-memory.bin').read_bytes(),submission_decode(submissions,lifetime=life),submissions)
        if len(value['setters'])!=4 or len(value['queries'])!=5 or value['known_source_points']!=3:raise ValueError('Client binding closure')
        report['controls']['drawing-wine-client-memory']['result']=value
        for mode in range(1,5):
            target=pe('drawing-wine-client-memory','client-close-'+str(mode),dict(MNM_DRAW_WINE_CLIENT_MEMORY='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_CLIENT_FIXTURE_CLOSE=str(mode)))
            raw=(target/'wine-client-memory.bin').read_bytes();rows=list(struct.iter_unpack('<48I',raw[64:]))
            if rows[-1][1]!=4 or sum(r[1]==3 for r in rows):raise ValueError('Client query emitted after close')
            if mode!=4 and any(r[1]==2 for r in rows):raise ValueError('Closed setter fabricated return')
        target=pe('drawing-wine-retained-map-source',extra=dict(MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RETAINED_MAP='1'))
        value=retained_decode((target/'wine-retained-map-source.bin').read_bytes(),(target/'wine-resource-map.bin').read_bytes(),(target/'wine-submission.bin').read_bytes(),(target/'wine-lifetime.bin').read_bytes())
        if len(value['points'])!=10 or value['matched']!=5 or value['matched_closed_maps']!=3 or [p['status'] for p in value['points']]!=[1,1,1,0,1,2,1,4,0,3]:raise ValueError('Retained map exact binding')
        report['controls']['drawing-wine-retained-map-source']['result']=value
        worker=dict(MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1')
        for name,flag,modes in [('drawing-wine-blit-packet','MNM_PACKET_FIXTURE_CASE',['n','w','p']),('drawing-wine-worker-context','MNM_CONTEXT_FIXTURE_CASE',['n','w','p','f','s'])]:
            extra=dict(worker)
            if name.endswith('context'):extra['MNM_DRAW_WINE_WORKER_CONTEXT']='1'
            target=pe(name,extra=extra)
            raw=(target/'wine-blit-packet.bin').read_bytes();rows=list(struct.iter_unpack('<16I',raw[64:]))
            if len(rows)!=7 or rows[-1][8] or rows[-1][3]!=2:raise ValueError('Packet binding closure')
            if name.endswith('context'):
                value=channel(struct.unpack('<16I',(target/'wine-worker-context.bin').read_bytes()))
                if value['active'] or value['sequence']!=8:raise ValueError('Context publication retirement')
                report['controls'][name]['result']=value
            for mode in modes:pe(name,name+'-'+mode,{**extra,flag:mode},99)
        target=out/'drawing-hud-state'
        target=pe('drawing-hud-state',extra=dict(MNM_DRAW_HUD_STATE_FILE='Z:'+str(target/'hud.bin').replace('/','\\')))
        value=hud_decode((target/'hud.bin').read_bytes())
        if len(value['samples'])!=4:raise ValueError('HUD owner, cadence and overflow controls')
        report['controls']['drawing-hud-state']['result']=value
        for mode in ('0','1','2'):
            target=out/('latches-'+mode)
            target=pe('drawing-actor-latches','latches-'+mode,dict(MNM_DRAW_ACTOR_LATCH_FILE='Z:'+str(target/'latches.bin').replace('/','\\'),MNM_LATCH_FIXTURE_POINT=mode),99 if mode=='2' else 0)
            if mode!='2':
                value=latch_decode((target/'latches.bin').read_bytes())
                if len(value['samples'])!=7:raise ValueError('Latch sample closure')
                report['controls']['drawing-actor-latches'][mode]=value
        target=out/'drawing-actor-latch-region'
        target=pe('drawing-actor-latch-region',extra=dict(MNM_DRAW_REGION_INPUT_FILE='Z:'+str(target/'region.bin').replace('/','\\'),MNM_DRAW_ACTOR_LATCH_FILE='Z:'+str(target/'actor-latches.bin').replace('/','\\')))
        from drawing_region_input import decode as region_decode
        from drawing_actor_latches import bind_callbacks
        value=region_decode((target/'region.bin').read_bytes());latches=latch_decode((target/'actor-latches.bin').read_bytes())
        if len(value['calls'])!=32 or len(bind_callbacks(latches,value)['phases'])!=64:raise ValueError('Latch Region forwarding join')
        report['controls']['drawing-actor-latch-region']['result']=value
        # The legacy tool requires physical claim/report interchange files;
        # its sole report consumer is this SQLite-bound combined execution.
        claims=out/'egl-claims.json';claims.write_text(json.dumps(declaration)+'\n')
        egl_report=out/'egl-interchange.json'
        private_tmp=out/'display-tmp';private_tmp.mkdir()
        run('software-egl-context',['bwrap','--die-with-parent','--ro-bind','/','/',
          '--bind',ROOT/'working',ROOT/'working','--bind',private_tmp,'/tmp','--dev-bind','/dev','/dev','--proc','/proc',
          sys.executable,'-B',ROOT/'tools/test-drawing-egl-worker-context.py','--claims',claims,'--report',egl_report],timeout=180)
        value=json.loads(egl_report.read_text())
        if not value['success'] or not value['sources_stable']:raise ValueError('Software EGL worker context fixture failed')
        report['controls']['software-egl-context']=value
        for name in DECODER_TESTS:
            run('decoder-'+name,[sys.executable,'-B',ROOT/('tests/test-drawing-'+name+'.py')])
        run('sixth-ten-controls',[sys.executable,'-B',ROOT/'tests/test-rendering-sixth-ten.py'])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        fresh=module('sixth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])
        if fresh['claims']!=declaration['claims']:raise ValueError('Contract drift during execution')
        report['success']=True
    except Exception as error:report['error']=repr(error)
    finally:
        if lock is not None:
            try:
                run('owned-prefix-stop',['wineserver','-k'],expected=(0,1))
                run('owned-prefix-retired',['wineserver','-w'],timeout=20)
            except Exception as error:
                report.update(success=False,retirement_error=repr(error))
            finally:lock.close()
        report['source_inputs']=len(report['sources'])
        report['sha256']=write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=str(report_path),sha256=report['sha256'],error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
