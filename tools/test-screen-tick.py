#!/usr/bin/env python3
"""Recover the shared screen tick's closed local CFG and compare whole-body execution."""
import argparse,hashlib,json,os,shlex,subprocess,resource
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import read_json,evidence_digest,write_report
from original_x86 import PE32,Decoder
ROOT=Path(__file__).resolve().parents[1]
ID='RS.screen-tick-control-flow'
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def flow(image):
 start,end=0x5595d0,0x55981e;decoder=Decoder();todo=[start];rows={};edges=[]
 try:
  while todo:
   at=todo.pop()
   if at in rows:continue
   if not start<=at<end:raise ValueError('Local flow escapes recovered extent')
   r=decoder.decode(image.read(at,min(15,end-at)),at);rows[at]=r;nxt=at+r['size'];m=r['mnemonic'];op=r['operand']
   if m.startswith('ret'):continue
   targets=[]
   if m.startswith('j'):
    if not op.startswith('0x'):raise ValueError('Unresolved internal branch')
    targets.append(int(op,16))
    if m!='jmp':targets.append(nxt)
   else:targets.append(nxt)
   for target in targets:edges.append([at,target]);todo.append(target)
  occupied={a for r in rows.values() for a in range(r['address'],r['address']+r['size'])}
  if len(occupied)!=sum(r['size'] for r in rows.values()) or occupied!=set(range(start,end)):raise ValueError('Overlapping or unassigned local bytes')
  calls=[r for r in rows.values() if r['mnemonic']=='call']
  return dict(entry=start,end=end,bytes_sha256=hashlib.sha256(image.read(start,end-start)).hexdigest(),entry_anchors={hex(start):image.read(start,16).hex()},instructions=sorted(rows.values(),key=lambda r:r['address']),edges=sorted(edges),calls=sorted(calls,key=lambda r:r['address']),closed_local_cfg=True,external_helpers_recovered=False,indirect_services_complete=False)
 finally:decoder.close()
def main():
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new owned output')
 out.mkdir(parents=True);(out/'.retain').write_text('Whole shared screen tick original/native execution and CFG witnesses.\n')
 j=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT);b=next(b for b in j['behaviors'] if b['id']==ID)
 sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(required_sources(b))}
 report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=ID,contract_sha256=behavior_contract(b,{v['id']:v for v in j['builds']}))],commands=[],original_sha256=BUILD,live_replacement=False,external_services_authored=True,scope=b['scope'])
 def run(label,cmd):
  print(label,flush=True);log=out/(label+'.log')
  with log.open('x') as f:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=os.environ|dict(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1'),stdout=f,stderr=subprocess.STDOUT,timeout=180)
  text=log.read_text();report['commands'].append(dict(label=label,exit=r.returncode,sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
  if r.returncode:raise ValueError(label+': '+text[-2500:])
  return text
 try:
  run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);image=ROOT/'original/Arcane_Nocd/Chaos.exe';pe=PE32(image.read_bytes());assert pe.sha256==BUILD;report['flow']=flow(pe);report['original_entry_anchors']=report['flow']['entry_anchors']
  inventory_name=j['builds'][0]['inventory'];inventory=read_json(ROOT/inventory_name,root=ROOT)
  report['inventory_review']=dict(baseline=inventory_name,baseline_sha256=evidence_digest(ROOT/inventory_name,root=ROOT),focused_reexport=dict(entry=0x5595d0,ranges=[[0x5595d0,0x55981e]],sha256=report['flow']['bytes_sha256']),overlapping_discovered_functions=[f['entry'] for f in inventory['functions'] if any(a<0x55981e and b>0x5595d0 for a,b in f['ranges'])],retained_unassigned_intersections=[[max(a,0x5595d0),min(b,0x55981e)] for a,b in inventory['unassigned_executable_ranges'] if a<0x55981e and b>0x5595d0],global_inventory_rewritten=False,external_indirect_targets_closed=False)
  if report['inventory_review']['overlapping_discovered_functions']:raise ValueError('Focused caller overlaps an inventoried function')
  for label,flags in [('original',['-m32','-O2','-DMNM_ORIGINAL_REFERENCE']),('sanitized',['-m32','-O1','-DMNM_ORIGINAL_REFERENCE','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('native64',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   run(label+'-build',['g++','-std=c++17','-mstackrealign','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',*flags,'-MMD','-MF',out/(label+'.d'),ROOT/'tests/screen-tick-reference.cpp','-o',out/label])
   for token in shlex.split((out/(label+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
    path=Path(token).resolve()
    if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiler source '+token)
   report[label]=json.loads(run(label,[out/label,*([image] if label!='native64' else [])]))
  if not report['original']==report['sanitized']==report['native64']:raise ValueError('Execution totals/checksums differ')
  report['success']=True
 except (OSError,ValueError,AssertionError,subprocess.SubprocessError) as e:report['error']=str(e)
 finally:
  try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except (OSError,ValueError,subprocess.SubprocessError) as e:report.update(success=False,manifest_error=str(e))
  report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items());report['success']&=report['sources_stable'];sha=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],sha256=sha,error=report.get('error'))),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
