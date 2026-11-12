#!/usr/bin/env python3
"""Run bounded synthetic admission, observer lifetime and PE32 acknowledgement controls."""
import argparse,hashlib,importlib.util,json,os,shlex,shutil,struct,subprocess,sys
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(name):
 spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/name);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
 if not a.report.startswith('research/runtime/rendering-acceptance-pilot-results-20261031/') or (ROOT/a.report).exists():p.error('New owned immutable report required')
 out=ROOT/'working/pilot'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False)
 names=['tools/test-rendering-pilot-controls.py','tools/rendering_pilot.py','tools/rendering_pilot_ledger.py','tests/test-rendering-pilot.py','tests/test-rendering-pilot-ledger.py','tests/rendering-pilot/observer.cpp','tests/rendering-pilot/CMakeLists.txt','tests/rendering-pilot/input-ack.c']
 report=dict(schema=1,kind='synthetic_pilot_controls',success=False,live_game_validated=False,sources={n:sha(ROOT/n) for n in names},checks=[])
 server=None;env=dict(os.environ)
 def run(label,command,**kw):
  with (out/(label+'.log')).open('xb') as log:subprocess.run([str(v) for v in command],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=240,**kw)
 try:
  run('python-capture-history',[sys.executable,'-B','tests/test-rendering-pilot.py']);report['checks'].append('nine capture/history controls including PRESENT snapshot and malformed streams')
  run('python-ledger',[sys.executable,'-B','tests/test-rendering-pilot-ledger.py']);report['checks'].append('three admission and reopen/restore controls including wrong types, source conflicts and decision forgery')
  build=ROOT/'working/build/pilot-controls'
  run('configure',['cmake','-S','tests/rendering-pilot','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'])
  run('build',['cmake','--build',build,'--target','pilot-observer','--parallel','4'])
  for dependency in build.glob('**/*.o.d'):
   for token in shlex.split(dependency.read_text().replace('\\\n',' ')):
    source=Path(token).resolve()
    if source.is_file() and source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):report['sources'][str(source.relative_to(ROOT))]=sha(source)
  run('observer',[build/'pilot-observer'],env=dict(env,QT_QPA_PLATFORM='offscreen'));report['checks'].append('actual NativeRenderTarget observer registration and replaced-owner destruction')
  dll=load('build-render-bridge.py').build(True,out/'pe32')
  report['sources'].update(json.loads((dll.parent/'manifest.json').read_text())['compiled_sources'])
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
  run('compile-input',flags+['-c','tests/rendering-pilot/input-ack.c','-o',out/'input.obj'])
  run('link-input',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'input.exe'),out/'input.obj',dll.parent/'render.lib',dll.parent/'kernel32.lib'])
  shutil.copy2(dll,out/dll.name)
  prefix=out/'wineprefix';shutil.copytree(ROOT/'working/fixtures/prefix-seed',prefix,symlinks=True)
  state=out/'input.bin';raw=bytearray(1088);raw[:16]=b'MNMINK01'+struct.pack('<II',1,1088);struct.pack_into('<6I',raw,16,2,1,400,300,800,600);struct.pack_into('<I',raw,64+39*4,0x80000001);state.write_bytes(raw)
  sys.path.insert(0,str(ROOT/'protocols/python'));from mnm_protocols import frame_v1
  frame=out/'frame.bin'
  with frame.open('wb') as f:f.write(frame_v1.initial_header());f.truncate(frame_v1.SIZE)
  host=out/'host';host.mkdir();server,owner,_=load('run-native-owned.py').start_display(host,allow_glx=True)
  env={k:v for k,v in env.items() if not k.startswith(('MNM_','WINE'))}
  env.update(DISPLAY=owner['display'],XAUTHORITY=owner['authority'],WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_ACCEPTANCE_PILOT='1',MNM_RENDER_INPUT='Z:'+str(state).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'))
  run('input',['wine',out/'input.exe'],env=env);run('wine-stopped',['wineserver','-w'],env=env)
  ack=Path(str(state)+'.acceptance').read_bytes();assert ack[:8]==b'MNMIAK01' and len(ack)==256
  rows=list(struct.iter_unpack('<12I',ack[64:]));assert [r[3] for r in rows]==[0x8001,0,0x8001,0x1234] and [r[4] for r in rows]==[1,1,1,0]
  report['checks'].append('PE32 transition observer preserves callback results and LastError with trace I/O enabled, including fallback')
  report['input_ack_rows']=[list(r) for r in rows];report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  if server:server.terminate();server.wait(timeout=10)
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items());report['success'] &= report['sources_stable']
  report['artifacts']=[]
  for f in sorted(out.rglob('*')):
   if f.is_file() and not f.is_symlink() and 'wineprefix' not in f.relative_to(out).parts and f.name!='Xauthority':
    name=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(out)).encode()).hexdigest()[:20]+'.bin';data=f.read_bytes();h=write_report(ROOT,name,data);report['artifacts'].append(dict(path=str(f.relative_to(out)),record=name,sha256=h,bytes=len(data)))
  h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],error=report.get('error'),record=a.report,sha256=h)))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
