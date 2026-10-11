#!/usr/bin/env python3
"""Execute PE32 forwarding/LastError/x87 and full-plane observer fixtures under private Wine."""
import argparse,importlib.util,json,os,shutil,subprocess,tempfile
from pathlib import Path
from drawing_api_session import inspect,sha
ROOT=Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--report',type=Path,required=True);p.add_argument('--claims',type=Path);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    names=['runtime/drawing/session.c','runtime/drawing/selftest.c','runtime/shadow/win32_min.h','tools/build-drawing-session.py','tools/build-shadow-bridge.py','tools/test-drawing-api-session.py','tools/drawing_api_session.py']
    claims=json.loads(a.claims.read_text()) if a.claims else None
    sources={n:sha((ROOT/n).read_bytes()) for n in names};sources.update(claims['sources'] if claims else {})
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-api-session';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));capture=run/'capture';capture.mkdir()
    s=importlib.util.spec_from_file_location('build',ROOT/'tools/build-drawing-session.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);dll=m.build(True);binary=dll.parent/'selftest.exe'
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/','\\'))
    try:
        with (run/'wineboot.log').open('x') as f:subprocess.run(['wineboot','-u'],env=env,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=90)
        execution=subprocess.run(['wine',str(binary)],cwd=dll.parent,env=env,capture_output=True,timeout=90);(run/'fixture.log').write_bytes(execution.stdout+execution.stderr);execution.check_returncode()
        observed=inspect(capture)
        if len(observed['calls'])!=32 or len(observed['frames'])!=8 or len(observed['actions'])!=2:raise ValueError('Fixture failed exact fault/route/phase accounting')
        faults=[0,0x887601c2,0x887601ae,0x80004005]
        for i,c in enumerate(observed['calls']):
            if c['hresult']&0xffffffff!=faults[i%4] or c['last_error']!=0xa000+i//4 or c['entry_error']!=0xbeef:raise ValueError('Original callback result/error was changed')
        expected=b''.join(((i*997)&65535).to_bytes(2,'little') for i in range(48))
        if any(f['sha256']!=sha(expected) for f in observed['frames']):raise ValueError('Full-plane observer mismatch')
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Fixture sources changed during execution')
        report={'schema':1,'success':True,'sources':sources,'sources_stable':True,'scripted_com_cases':32,'primary_planes':8,'menu_phases':2,'hresult_last_error_x87_preserved':True,'original_game_executed':False,'scope':'Synthetic PE32 COM callbacks at eight primary mutator methods with success/lost/busy/generic failure; exact original callback result/LastError/x87 and single application call, independent unthrottled full-plane readback. No actual engine/driver failure or physical/global presentation equivalence.','binary_sha256':sha(binary.read_bytes()),'dll_sha256':sha(dll.read_bytes()),'artifacts':str(run.relative_to(ROOT)),'observation':observed}
        if claims:report['claims']=claims['claims']
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'success':True,'scripted_com_cases':32,'primary_planes':8}))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
if __name__=='__main__':main()
