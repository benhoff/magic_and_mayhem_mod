#!/usr/bin/env python3
"""Supplement the unchanged approved 8x6 overlap mechanism on actual NVIDIA GL."""
import argparse,ast,hashlib,json,os,signal,subprocess,types
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_native_host import start_nvidia,gpu_environment
from native_render_driver import require_nvidia
from rendering_surface_loss import linked_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def hardware_code(raw,filename):
 tree=ast.parse(raw,filename=filename);expected=ast.parse("dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')",mode='eval').body;found=0
 for node in ast.walk(tree):
  if isinstance(node,ast.Assign)and len(node.targets)==1 and isinstance(node.targets[0],ast.Name)and node.targets[0].id=='env':
   require(ast.dump(node.value)==ast.dump(expected),'Unexpected overlap environment binding');node.value=ast.copy_location(ast.Name(id='BROADER_HARDWARE_ENV',ctx=ast.Load()),node.value);found+=1
 require(found==1,'Missing/duplicate overlap environment binding')
 return compile(ast.fix_missing_locations(tree),filename,'exec'),hashlib.sha256(ast.dump(tree,include_attributes=False).encode()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh hardware control required')
 out=ROOT/'working/broader-player'/Path(args.report).stem;out.mkdir();(out/'.retain').touch();build=Path('/dev/shm/mnm-broader-capture-worker-20261110/native-controls-07/renderer')
 with Reader(ROOT)as r:
  parent=r.json(P+'overlap-controls-02.json');require(parent['success']and all(r.digest(n)==h for n,h in parent['sources'].items()),'Controlled native source changed')
  sources=dict(parent['sources']);sources.update({n:r.digest(n)for n in('tools/check-broader-copy-hardware-overlap.py','tools/broader_native_host.py','tools/native_render_driver.py')})
  for binary in('render-surface-copy-fixtures','mnm-render-commands'):sources.update({n:r.digest(n)for n in linked_sources(build,ROOT,binary)})
  for binary in('render-surface-copy-fixtures','mnm-render-commands'):
   row=next(v for v in parent['artifacts']if v['path']==str(build/binary));require(hashlib.sha256((build/binary).read_bytes()).hexdigest()==row['sha256']==r.digest(row['record']),'Exact controlled binary changed')
  raw=r.read_bytes('tools/check-native-surface-overlap.py')
 q=dict(schema=1,kind='broader_current_hardware_overlap_controls',success=False,original_game_runs=0,approved_scope_amendment_unchanged=True,original_padding_or_physical_driver_equivalence=False,sources=sources);server=None
 try:
  for label in('before',):
   run=subprocess.run([ROOT/'tools/original-manifest.sh','verify'],capture_output=True);(out/('original-'+label+'.log')).write_bytes(run.stdout+run.stderr);require(run.returncode==0,'Immutable originals differ')
  host=out/'host';host.mkdir();server,receipt,cookie=start_nvidia(host);q['host']=receipt
  env=gpu_environment(os.environ);env.update(DISPLAY=receipt['display'],XAUTHORITY=receipt['authority'],QT_QPA_PLATFORM='xcb',LP_NUM_THREADS='4')
  code,sha=hardware_code(raw,str(ROOT/'tools/check-native-surface-overlap.py'));module=types.ModuleType('broader_hardware_overlap_core');module.__file__=str(ROOT/'tools/check-native-surface-overlap.py');module.BROADER_HARDWARE_ENV=env;exec(code,module.__dict__)
  result=module.check(build,out/'interchange.json');require(result['success']and result['cases']==1536 and result['policy_tests']==20 and result['refused_streams']==6,'Incomplete hardware controlled case set');q['actual_hardware']=require_nvidia([result['renderer']]);q['comparison']=result;q['bound_execution_ast_sha256']=sha;q['only_environment_binding_changed']=True;q['success']=True
 except Exception as e:q['error']=repr(e)
 finally:
  if server is not None:
   if server.poll()is None:os.killpg(server.pid,signal.SIGTERM)
   server.wait(timeout=10)
  q['owned_server_stopped']=server is None or server.poll()is not None
  after=subprocess.run([ROOT/'tools/original-manifest.sh','verify'],capture_output=True);(out/'original-after.log').write_bytes(after.stdout+after.stderr);q['original_manifest_verified_before_after']=after.returncode==0
  q['artifacts']=[];files=[f for f in out.rglob('*')if f.is_file()]
  if q.get('comparison'):files +=[f for f in(ROOT/q['comparison']['artifacts']).rglob('*')if f.is_file()]
  for file in sorted(set(files)):
   body=file.read_bytes();name=args.report.removesuffix('.json')+'-'+hashlib.sha256(str(file).encode()).hexdigest()[:20]+'.bin';q['artifacts'].append(dict(path=str(file),record=name,sha256=write_report(ROOT,name,body),bytes=len(body)))
  with Reader(ROOT)as r:q['sources_stable']=all(r.digest(n)==h for n,h in sources.items())
  q['success']&=q['sources_stable']and q['original_manifest_verified_before_after']and q['owned_server_stopped'];q['scope']='Supplemental actual NVIDIA controlled native-backend and v3-replay checks against the same retained original 8x6 opaque RGB565 inputs/outputs. The owner-approved original/Mesa amendment, shared-profile mapping, natural reachability boundary and other154rows remain unchanged. No new game, natural overlap, production live-alias discovery, Windows/hardware original-driver equivalence or whole-player acceptance.'
  h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=h)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
