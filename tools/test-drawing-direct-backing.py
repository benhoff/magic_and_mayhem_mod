#!/usr/bin/env python3
"""Execute direct provider leases with actual PE32 mapping and observer hooks."""
import argparse,hashlib,importlib.util,json,os,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch();(out/'tmp').mkdir();os.environ['TMPDIR']=str(out/'tmp')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in register['behaviors'] if b['id']=='NR.direct-provider-readback');builds={b['id']:b for b in register['builds']}
 paths=required_sources(b)|{'tools/test-drawing-direct-backing.py','tools/drawing_fixture_prefix.py','tools/test-drawing-physical-owners.py','tools/build-shadow-bridge.py'}
 # Preserve complete reviewed compiler dependencies before executing this build.
 import ast
 module=ast.parse((ROOT/'tools/build-drawing-session.py').read_text())
 for node in ast.walk(module):
  if isinstance(node,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='sources' for t in node.targets):paths.update(ast.literal_eval(node.value))
 sources={n:sha(ROOT/n) for n in sorted(paths)};claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds))]
 report=dict(schema=1,success=False,sources=sources,claims=claims,cases=[],original_game_executed=False,actual_foreign_driver_executed=False,scope='Actual PE32 direct provider leases and worker/fence hooks, nonmappable sources and unchanged original bindings with registered controlled producers. Shared/file/imported classes use real aliased section mappings; GPU readback/fence producer is controlled. No installed foreign producer or hardware acceptance claim.')
 def run(name,cmd,env=None,expected=0):
  r=subprocess.run([str(v) for v in cmd],cwd=ROOT,env=env,capture_output=True,text=True,timeout=120);(out/(name+'.log')).write_text(r.stdout+r.stderr)
  if r.returncode!=expected:raise ValueError(f'{name}: {r.returncode}, expected {expected}')
  return r
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};prefix=out/'wineprefix';env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all')
 try:
  run('copy-prefix',['cp','-a','--reflink=auto',a.prefix_template.resolve(),prefix])
  spec=importlib.util.spec_from_file_location('backing_build',ROOT/'tools/build-drawing-session.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);dll=m.build(True)
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
  run('compile',flags+['-c',ROOT/'tests/drawing-direct-backing-test.c','-o',out/'fixture.obj']);binary=out/'fixture.exe'
  run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),out/'fixture.obj',dll.parent/'packet.obj',dll.parent/'kernel32.lib'])
  run('wineboot',['wineboot','-i'],env)
  env.update(MNM_DRAW_SESSION_MODE='physical',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RESOURCE_COPY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1',MNM_DRAW_WINE_SOURCE_MAP='2',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_WINE_SOURCE_VERSION='1',MNM_DRAW_WINE_GPU_COMPLETION='1',MNM_DRAW_WINE_SOURCE_RESERVATIONS='1',MNM_DRAW_WINE_PHYSICAL_OWNERS='1',MNM_DRAW_WINE_SOURCE_HANDOFF='1',MNM_DRAW_WINE_SOURCE_OWNERSHIP='2')
  from drawing_source_handoff import analyze
  from drawing_source_reservations import decode as reservations
  import struct
  for kind in range(1,5):
   for fault in [0,1,2,3,4,5,6,7,8,10,11,12,13]+([9] if kind==4 else []):
    target=out/f'kind-{kind}-fault-{fault}';target.mkdir();trial=dict(env,MNM_TEST_BACKING=str(kind),MNM_TEST_BACKING_FAULT=chr(48+fault),MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
    run(f'kind-{kind}-fault-{fault}',['wine',binary],trial,0 if fault==0 else 99)
    if not fault:
     res=reservations((target/'wine-source-reservations.bin').read_bytes());analysis=analyze(target,dict(wine_source_reservation_observation=res));assert analysis['direct_provider_leases']==1 and analysis['bindings'][0]['provider']['kind']==kind and not analysis['private_copy_value_ownership_closed'] and analysis['source_binding_valid_before_api_return']
     from drawing_source_ownership import analyze as ownership
     strict=ownership(target,dict(wine_source_reservation_observation=res));assert strict['direct_provider_leases']==1 and not strict['private_copy_value_ownership_closed']
     report['cases'].append(dict(kind=kind,fault=fault,exit=0,analysis=analysis))
    else:
     if fault==5:
      from drawing_wine_source_map import decode as mapping
      raw=(target/'wine-source-map.bin').read_bytes();assert raw[:8]==b'MNMWMS05' and struct.unpack_from('<64I',raw,64)[37]==0
      report['cases'].append(dict(kind=kind,fault=fault,exit=99,unsupported_without_provider=True));continue
     raw=(target/'wine-source-handoff.bin').read_bytes();last=struct.unpack_from('<64I',raw,len(raw)-256);assert last[1]==9
     reason=4 if fault==7 else 64 if fault in (8,13) else 61
     assert last[16]==reason,(kind,fault,last[16],reason)
     report['cases'].append(dict(kind=kind,fault=fault,exit=99,reason=reason,journal_sha256=hashlib.sha256(raw).hexdigest()))
  target=out/'private-profile-refuses';target.mkdir();run('private-profile-refuses',['wine',binary],dict(env,MNM_TEST_BACKING='4',MNM_DRAW_WINE_SOURCE_OWNERSHIP='1',MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92))))
  report['private_profile_refused_v2_registration']=True
  
  for name in ['source-handoff','source-ownership','physical-owners','wine-source-map']:run('parser-'+name,['python3','-B',ROOT/('tests/test-drawing-'+name+'.py')])
  report['binary_sha256']=sha(binary);report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  if prefix.exists():subprocess.run(['wineserver','-k'],env=env,capture_output=True,timeout=10)
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],cases=len(report['cases']),error=report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
