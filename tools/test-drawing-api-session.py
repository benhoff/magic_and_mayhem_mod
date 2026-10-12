#!/usr/bin/env python3
"""Execute PE32 forwarding/LastError/x87 and full-plane observer fixtures under private Wine."""
import argparse,importlib.util,json,os,shutil,subprocess,tempfile
from pathlib import Path
from drawing_api_session import inspect,sha
ROOT=Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--report',type=Path,required=True);p.add_argument('--claims',type=Path);p.add_argument('--physical',action='store_true');a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    names=['tests/test-drawing-call-intervals.py','runtime/drawing/call_intervals.h','runtime/drawing/physical_fence.h','runtime/drawing/session.c','runtime/drawing/selftest.c','runtime/shadow/win32_min.h','tools/build-drawing-session.py','tools/build-shadow-bridge.py','tools/test-drawing-api-session.py','tools/drawing_call_intervals.py','tools/drawing_api_session.py','tools/drawing_api_physical.py','tests/drawing-api-physical-peer.py','tools/drawing_presented_session.py']
    claims=json.loads(a.claims.read_text()) if a.claims else None
    sources={n:sha((ROOT/n).read_bytes()) for n in names};sources.update(claims['sources'] if claims else {})
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-api-session';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));capture=run/'capture';capture.mkdir()
    s=importlib.util.spec_from_file_location('build',ROOT/'tools/build-drawing-session.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);dll=m.build(True);binary=dll.parent/'selftest.exe'
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/','\\'))
    peer=None
    if a.physical:
        env['MNM_DRAW_SESSION_MODE']='physical'
        peer=subprocess.Popen(['python3',str(ROOT/'tests/drawing-api-physical-peer.py'),str(capture),str(run/'physical-peer.json')])
    try:
        with (run/'wineboot.log').open('x') as f:subprocess.run(['wineboot','-u'],env=env,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=90)
        execution=subprocess.run(['wine',str(binary)],cwd=dll.parent,env=env,capture_output=True,timeout=90);(run/'fixture.log').write_bytes(execution.stdout+execution.stderr);execution.check_returncode()
        if peer:
            peer.wait(timeout=10)
            if peer.returncode:raise ValueError('Synthetic physical peer failed')
        observed=inspect(capture)
        if len(observed['calls'])!=(40 if a.physical else 32) or len(observed['frames'])!=8 or len(observed['actions'])!=(2 if a.physical else 18):raise ValueError('Fixture failed exact fault/route/phase accounting')
        faults=[0,0x887601c2,0x887601ae,0x80004005]
        original_calls=[c for c in observed['calls'] if c['object']==observed['frames'][0]['object']]
        if len(original_calls)!=32:raise ValueError('Original callback cases missing')
        competitors=[c for c in observed['calls'] if c not in original_calls]
        if a.physical and (len(competitors)!=8 or any(c['method']!=5 or c['hresult']&0xffffffff!=0x80004005 or c['last_error']!=0xcafe for c in competitors)):raise ValueError('Competing thread callback forwarding changed')
        for i,c in enumerate(original_calls):
            if c['hresult']&0xffffffff!=faults[i%4] or c['last_error']!=0xa000+i//4 or c['entry_error']!=0xbeef:raise ValueError('Original callback result/error was changed')
        expected=b''.join(((i*997)&65535).to_bytes(2,'little') for i in range(48))
        if a.physical:
            from drawing_presented_session import rgba565
            expected=rgba565(expected)
        if any(f['sha256']!=sha(expected) for f in observed['frames']):raise ValueError('Full-plane observer mismatch')
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Fixture sources changed during execution')
        report={'schema':1,'success':True,'sources':sources,'sources_stable':True,'scripted_com_cases':32,'primary_planes':8,'menu_phases':2 if a.physical else 18,'driver_reentrant_threads':8,'physical_fence_enabled':a.physical,'diagnostic_lock_unlock_calls':0 if a.physical else 16,'hresult_last_error_x87_preserved':True,'original_game_executed':False,'scope':'Synthetic PE32 COM callbacks at eight primary mutator methods with success/lost/busy/generic failure; exact original callback result/LastError/x87 and single application call, independent unthrottled full-plane readback. No actual engine/driver failure or physical/global presentation equivalence.','binary_sha256':sha(binary.read_bytes()),'dll_sha256':sha(dll.read_bytes()),'artifacts':str(run.relative_to(ROOT)),'observation':observed}
        if a.physical:
            from drawing_api_physical import analyze
            report['physical_witness_admission']=analyze(capture,run/'physical-peer.json')
            from drawing_call_intervals import inspect_intervals
            report['call_intervals']=inspect_intervals(capture)
            if report['call_intervals']['frames_with_other_thread_mutations']!=8:raise ValueError('Competing writes escaped interval accounting')
            report['competing_thread_calls']=8
            report['scope']='Synthetic32PE32 original callback cases plus eight separate failed mutators on competing driver callback threads/eight successful primary API fences, exact result/LastError/x87 and single original callback with zero observer Lock/Unlock calls. Synthetic ACK/pixels only; no original game, real X11 capture, source ownership or full journey.'
        if claims:report['claims']=claims['claims']
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'success':True,'scripted_com_cases':32,'primary_planes':8}))
    finally:
        if peer and peer.poll() is None:peer.terminate();peer.wait(timeout=5)
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
if __name__=='__main__':main()
