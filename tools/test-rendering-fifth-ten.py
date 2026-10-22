#!/usr/bin/env python3
"""Execute ten bounded native rendering adapter contracts."""
import argparse
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
BEHAVIORS=['NR.drawing-wine-texture-entry-observation', 'NR.drawing-source-worker-lease', 'NR.drawing-storage-query-candidates', 'NR.drawing-wine-resource-copy', 'NR.drawing-original-cursor-point', 'NR.drawing-wine-source-descriptor', 'NR.drawing-wine-source-map', 'NR.drawing-wine-source-map-bounded', 'NR.drawing-source-write-trace', 'NR.drawing-gpu-completion', 'NR.drawing-semantic-state-observation', 'NR.drawing-wine-submission-generations']
SCENARIO='NR.fifth-ten.adapters-20261021'


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
    parent=ROOT/'working/tests/rendering-fifth-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Fifth rendering batch raw execution, binaries and failed attempts.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('fifth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    # Pin both production compilers' full source closure before execution.
    todo=set(declaration['sources'])|{'tools/test-rendering-fifth-ten.py','tools/coverage_evidence.py',
      'tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/build-drawing-session.py',
      'tools/build-scene-observer.py','tools/build-menu-observer.py','tools/build-shadow-bridge.py',
      'tools/drawing_api_session.py','tools/drawing_fixture_prefix.py','tools/drawing_lifetime_index.py',
      'tests/test-rendering-fifth-ten.py','tests/drawing-source-limit-test.c'}
    for directory in ('runtime/drawing','runtime/scene','runtime/menu'):
        todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).iterdir() if p.suffix in {'.c','.h','.S'})
    todo.update('reconstruction/rendering/'+name for name in ('glyph_backend.c','glyph_backend_state.c','glyph_backend_x87.S'))
    visited=set()
    while todo:
        name=todo.pop()
        if name in visited:continue
        visited.add(name);path=ROOT/name;declaration['sources'][name]=sha(path)
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
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
        probes=['drawing-wine-texture-entry','drawing-source-worker-lease','drawing-storage-transitions',
          'drawing-wine-resource-copy','drawing-input-point','drawing-wine-source-descriptor',
          'drawing-wine-source-map','drawing-source-write-trace','drawing-gpu-completion',
          'drawing-semantic-state','drawing-source-limit']
        import shutil
        shutil.copy2(drawing/'MnmDrawing.dll',out/'MnmDrawing.dll')
        full={'drawing-source-worker-lease','drawing-wine-resource-copy','drawing-wine-source-descriptor','drawing-wine-source-map','drawing-source-limit'}
        for name in probes:
            obj=out/(name+'.obj');binary=out/(name+'.exe');dep=out/(name+'.d')
            run(name+'-compile',[*flags,'-MD','-MF',dep,'-c',ROOT/('tests/'+name+'-test.c'),'-o',obj])
            dependencies(dep)
            libs=([drawing/'packet.obj'] if name in full else [])+([drawing/'drawing.lib'] if name=='drawing-input-point' else [])
            run(name+'-link',['lld-link','/machine:x86','/base:0x18000000','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),obj,*libs,out/'kernel32.lib'])
            report['controls'][name]=dict(binary_sha256=sha(binary))
        def pe(name,label=None,extra=None,expected=0):
            target=out/(label or name);target.mkdir()
            values=dict(MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_SESSION_MODE='physical',
              MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/','\\'),MNM_FIXTURE_DIRECTORY='Z:'+str(target).replace('/','\\'))
            if name=='drawing-input-point':
                values.pop('MNM_DRAW_WINE_SUBMISSION');values.pop('MNM_DRAW02_PRODUCER_BIND')
            run(label or name,['wine',out/(name+'.exe')],expected,{**values,**(extra or {})},target,timeout=45)
            return target
        import struct
        from drawing_wine_lifetime import decode as lifetime_decode
        target=pe('drawing-wine-texture-entry')
        value=lifetime_decode((target/'wine-lifetime.bin').read_bytes())
        if len(value['events'])!=4 or value['observed_generations']!=2 or value['remaining_observed_objects']:raise ValueError('Texture hook closure')
        report['controls']['drawing-wine-texture-entry']['result']=value
        from drawing_source_worker_lease import decode as worker_decode
        worker=dict(MNM_DRAW_SOURCE_BYTES='1',MNM_DRAW_SOURCE_LEASE='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_SOURCE_WORKER_LEASE='1')
        cases=[]
        for mode in ('d','e','n','u','t','a','l','b'):
            expected=0 if mode in ('d','e') else 99
            target=pe('drawing-source-worker-lease','worker-'+mode,dict(worker,MNM_FIXTURE_MODE=mode),expected)
            raw=(target/'api-source-worker-lease.bin').read_bytes()
            value=worker_decode(raw) if not expected else None
            if value and (value['completed_commands']!=1 or value['accepted_leases']!=1):raise ValueError('Worker lease completion')
            cases.append(dict(mode=mode,exit_code=expected,result=value,journal_sha256=hashlib.sha256(raw).hexdigest()))
        report['controls']['drawing-source-worker-lease']['cases']=cases
        from drawing_storage_transitions import decode as storage_decode
        from drawing_storage_query_returns import bind
        target=out/'drawing-storage-transitions'
        target=pe('drawing-storage-transitions',extra=dict(MNM_DRAW_STORAGE_TRANSITIONS='1',MNM_SCENE_DIR='Z:'+str(target).replace('/','\\')))
        value=storage_decode((target/'canvas-storage-transitions-0000.bin').read_bytes())
        if not value['closed'] or value['calls']!=8 or value['observed_lock_return_epochs']!=5 or [q['status'] for q in value['queries']]!=[2,1,3,0,0,0,0,0]:raise ValueError('Storage transition closure')
        bound=bind(value)
        if len(bound['bindings'])!=1 or bound['bindings'][0]['query']!=6 or value['queries'][-2]['input_address']:raise ValueError('Storage query binding')
        report['controls']['drawing-storage-transitions'].update(result=value,bindings=bound)
        from drawing_wine_resource_copy import decode as copy_decode
        from drawing_wine_resource_mapping import decode as resource_decode
        target=pe('drawing-wine-resource-copy',extra=dict(MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RESOURCE_COPY='1'))
        value=copy_decode((target/'wine-resource-copy.bin').read_bytes(),resource_decode((target/'wine-resource-map.bin').read_bytes()))
        if len(value['calls'])!=3:raise ValueError('Resource copy closure')
        report['controls']['drawing-wine-resource-copy']['result']=value
        from drawing_input_point import decode as point_decode
        from drawing_api_session import inspect
        target=out/'drawing-input-point'
        target=pe('drawing-input-point',extra=dict(MNM_DRAW_SESSION_MODE='inputs',MNM_DRAW_INPUT_POINT='1',MNM_DRAW_POINT_FIXTURE_FILE='Z:'+str(target/'points.bin').replace('/','\\')))
        raw=(target/'points.bin').read_bytes()
        if len(raw)!=320:raise ValueError('Cursor point count')
        points=[point_decode(raw[i:i+64]) for i in range(0,len(raw),64)];value=inspect(target)
        if len(value['inputs'])!=18 or value['frames'] or value['calls'] or any(x['method']!=3 for x in value['inputs']):raise ValueError('Cursor callback count')
        by_record={x['ordinal']:x for x in value['inputs']}
        for point in points:
            if point['cursor_record']:
                row=by_record.get(point['cursor_record'])
                if row is None or row['thread']!=point['owner_thread'] or row['cursor']!=point['cursor']:raise ValueError('Cursor exact binding')
        if [p['cursor_record'] for p in points]!=[1,0,2,3,2]:raise ValueError('Cursor sequence')
        report['controls']['drawing-input-point'].update(points=points,api_observation=value,cross_page_refusal_and_acceptance=True)
        from drawing_wine_source_descriptor import decode as descriptor_decode
        descriptor=dict(MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1')
        target=pe('drawing-wine-source-descriptor',extra=descriptor)
        value=descriptor_decode((target/'wine-source-descriptor.bin').read_bytes())
        if len(value['samples'])!=6 or value['known_descriptors']!=1 or value['extra_public_queries']!=7 or [r['status'] for r in value['samples']]!=[0,1,2,3,4,1]:raise ValueError('Descriptor closure')
        report['controls']['drawing-wine-source-descriptor']['result']=value
        from drawing_wine_source_map import decode as map_decode
        maps=[]
        for bounded in (False,True):
            extra=dict(descriptor,MNM_DRAW_WINE_SOURCE_MAP='2' if bounded else '1')
            if bounded:extra.update(MNM_DRAW_WINE_SOURCE_MAP_ATTEMPTS='8',MNM_DRAW_WINE_SOURCE_MAP_BYTES='4194304')
            target=pe('drawing-wine-source-map','map-bounded' if bounded else 'map-legacy',extra)
            value=map_decode((target/'wine-source-map.bin').read_bytes(),target)
            statuses=[0,0,0,0,2,2,0,0,3,0,0,3] if bounded else [0,2,2,4,5,6,6,6,6,7,0,0,2]+[0]*8+[3,1,1,1,1]
            blits,payloads,attempts=(12,8,[8]) if bounded else (27,11,[16,1])
            if [r['status'] for r in value['samples']]!=statuses or value['retained_payloads']!=payloads or value['map_attempts']!=attempts:raise ValueError('Mapping closure')
            if list(struct.iter_unpack('<I',(target/'fixture-forwards.bin').read_bytes()))!=[(i,) for i in range(1,blits+1)]:raise ValueError('Map forward count')
            for payload in value['payloads']:
                expected=bytes((x*3+y*5+7)&255 for y in range(payload['height']) for x in range(payload['width']*2))
                if (target/payload['path']).read_bytes()!=expected:raise ValueError('Dense mapped bytes changed')
            maps.append(dict(bounded=bounded,result=value,original_blits=blits))
        for fatal,status in [('1',9),('2',10)]:
            target=pe('drawing-wine-source-map','map-fatal-'+fatal,dict(descriptor,MNM_DRAW_WINE_SOURCE_MAP='1',MNM_TEST_SOURCE_MAP_FATAL=fatal),99)
            raw=(target/'wine-source-map.bin').read_bytes()
            if len(raw)!=320 or struct.unpack_from('<64I',raw,64)[10]!=status or (target/'fixture-forwards.bin').read_bytes():raise ValueError('Fatal map forwarding guard')
            try:map_decode(raw,target)
            except ValueError:pass
            else:raise ValueError('Fatal map admitted')
        report['controls']['drawing-wine-source-map']['cases']=maps
        pe('drawing-source-limit')
        report['controls']['drawing-source-limit']['environment_cases']=15
        from drawing_source_write_trace import snapshot,marker,fault,resolve_writer
        cases=[]
        for label,mode,refused in [('normal','0',False),('direct','1',True),('exact-alias','2',True),('partial-alias','3',True),('upper-page-halo','4',True),('arming-window','5',True),('unrelated-same-reservation','6',False),('second-virtual-mapping-escape','7',False),('lower-page-halo','8',True),('default-off','0',False),('default-off-fault','1',True),('invalid-flag','9',False)]:
            extra=dict(MNM_DRAW_SOURCE_LEASE='1',MNM_DRAW_SOURCE_WRITE_TRACE='1',MNM_WRITE_TEST=mode)
            if label.startswith('default-off'):extra.pop('MNM_DRAW_SOURCE_WRITE_TRACE')
            if label=='invalid-flag':extra['MNM_DRAW_SOURCE_WRITE_TRACE']='x'
            target=pe('drawing-source-write-trace','write-'+label,extra,99 if refused else 0)
            value=None;attribution=None
            if label.startswith('default-off') or label=='invalid-flag':
                if list(target.glob('source-write-*.bin')):raise ValueError('Disabled write trace emitted')
            else:
                value=snapshot((target/'source-write-trace-0001.bin').read_bytes())
                if len(value['canvases'])!=6:raise ValueError('Trace canvases')
                if label=='arming-window':
                    if (target/'source-write-armed-0001.bin').exists():raise ValueError('Arming fault admitted')
                else:marker((target/'source-write-armed-0001.bin').read_bytes(),value,2)
                if refused:
                    record=fault((target/'source-write-fault-0001.bin').read_bytes(),value)
                    expected=[1,2,3] if label=='partial-alias' else [4] if label=='upper-page-halo' else [] if label=='lower-page-halo' else [1,2]
                    if not record['foreign_thread'] or record['phase']!=(1 if label=='arming-window' else 2) or record['recorded_pixel_aliases']!=expected or record['target_is_source_pixel']!=(label not in ('upper-page-halo','lower-page-halo')):raise ValueError('Write fault identity')
                    if (target/'source-write-done-0001.bin').exists():raise ValueError('Faulted lease closed')
                    attribution=resolve_writer(record,[out/'drawing-source-write-trace.exe'])
                    if not attribution['resolved']:raise ValueError('Writer attribution failed')
                    value['fault']=record
                else:
                    marker((target/'source-write-done-0001.bin').read_bytes(),value,3)
                    if label=='second-virtual-mapping-escape' and (target/'fixture-shared-read.bin').read_bytes()!=struct.pack('<2H',0x1111,0xabcd):raise ValueError('Alias escape missing')
            if refused:
                raw=(target/'source-lease-fault-0001.bin').read_bytes()
                if len(raw)!=48 or raw[:8]!=b'MNMSLFT1':raise ValueError('Legacy fault record')
            cases.append(dict(case=label,result=value,writer_attribution=attribution))
        report['controls']['drawing-source-write-trace']['cases']=cases
        from drawing_gpu_completion import decode as gpu_decode
        cases=[]
        for mode in [0,1,2,3,4,5,6,8,9,10,11,12,13,14,15,16]:
            expected=99 if mode in (3,4,5,6,8,9,15,16) else 0
            extra=dict(MNM_DRAW_WINE_GPU_COMPLETION='1',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_WORKER_PRODUCER='1',MNM_GPU_TEST=chr(ord('0')+mode))
            if mode==10:extra['MNM_DRAW_WINE_GPU_COMPLETION']='x'
            if mode==11:extra.pop('MNM_DRAW_WINE_GPU_COMPLETION')
            target=pe('drawing-gpu-completion','gpu-'+str(mode),extra,expected)
            path=target/'wine-gpu-completion.bin';value=None
            if mode==14:
                try:gpu_decode(path.read_bytes())
                except ValueError as error:
                    if str(error)!='GPU completion terminal':raise
                else:raise ValueError('Midwait closure admitted')
            elif mode in (15,16):
                if len(path.read_bytes())!=64:raise ValueError('Invalid packet fabricated fence')
            elif mode in (10,11):
                if path.exists():raise ValueError('Disabled GPU log')
            else:
                value=gpu_decode(path.read_bytes(),complete=not expected)
                if len(value['calls'])!=1 or (value['calls'][0]['worker'],value['calls'][0]['command'],value['calls'][0]['submission'])!=(1,7,8):raise ValueError('Fence worker identity')
            cases.append(dict(mode=mode,exit_code=expected,result=value))
        report['controls']['drawing-gpu-completion'].update(cases=cases,actual_gl_context_executed=False)
        from drawing_semantic_state import decode as semantic_decode
        target=out/'drawing-semantic-state'
        target=pe('drawing-semantic-state',extra=dict(MNM_DRAW_SEMANTIC_STATE_FILE='Z:'+str(target/'semantic-state.bin').replace('/','\\')))
        value=semantic_decode((target/'semantic-state.bin').read_bytes())
        if len(value['samples'])!=2:raise ValueError('Semantic cadence/owner closure')
        report['controls']['drawing-semantic-state'].update(result=value,foreign_and_zero_owner_refused=True)
        for name in ['wine-submission','wine-lifetime','wine-source-descriptor','wine-source-map','wine-resource-copy','storage-transitions','storage-query-returns','input-point','source-write-trace','gpu-completion','semantic-state']:
            run('decoder-'+name,[sys.executable,'-B',ROOT/('tests/test-drawing-'+name+'.py')])
        run('fifth-ten-controls',[sys.executable,'-B',ROOT/'tests/test-rendering-fifth-ten.py'])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        fresh=module('fifth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])
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
