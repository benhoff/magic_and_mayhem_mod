#!/usr/bin/env python3
"""Compare screen selection, cursor selection and input dispatch with pinned PE32 bodies."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import shlex
import subprocess
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import read_json, evidence_digest, write_report
from original_x86 import PE32, Decoder
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS=['RS.screen-selection-service','RS.cursor-selection-service','RS.screen-control-dispatch','RS.screen-service-tick-transitions','RS.screen-minimal-transition-callbacks']
RANGES=[(0x5571e0,0x557368),(0x4a39a0,0x4a3c17),(0x4a5070,0x4a5842)]


def decode(pe,start,end):
    decoder=Decoder();todo=[start];rows={};indirect=[]
    try:
        while todo:
            at=todo.pop()
            if at in rows:continue
            if not start<=at<end:raise ValueError('Flow escapes '+hex(at))
            row=decoder.decode(pe.read(at,min(15,end-at)),at);rows[at]=row
            op,m=row['operand'],row['mnemonic']
            if m.startswith('ret'):continue
            if m.startswith('j'):
                if op.startswith('0x'):todo.append(int(op,16))
                elif at==0x4a3a75:
                    targets=[pe.word(0x4a3c18+i*4) for i in range(20)]
                    indirect.append(dict(address=at,index_domain=[0,19],table=0x4a3c18,targets=targets));todo+=targets
                else:raise ValueError('Unresolved local indirect '+hex(at))
            if m!='jmp':todo.append(at+row['size'])
        occupied={a for r in rows.values() for a in range(r['address'],r['address']+r['size'])}
        if occupied!=set(range(start,end)) or len(occupied)!=sum(r['size'] for r in rows.values()):raise ValueError('Local byte census mismatch')
        return dict(entry=start,end=end,bytes_sha256=hashlib.sha256(pe.read(start,end-start)).hexdigest(),instructions=sorted(rows.values(),key=lambda r:r['address']),resolved_local_switches=indirect,closed_local_cfg=True,external_helpers_complete=False)
    finally:decoder.close()


def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True);parser.add_argument('--report',required=True)
    args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/args.report).exists():parser.error('Require new owned output/report')
    out.mkdir(parents=True);(out/'tmp').mkdir();(out/'.retain').write_text('Screen service raw execution, diagnostics and compiler dependency records.\n')
    reg=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT);bs={b['id']:b for b in reg['behaviors']};builds={b['id']:b for b in reg['builds']}
    sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(set().union(*(required_sources(bs[i]) for i in IDS)))}
    report=dict(schema=1,success=False,original_sha256=BUILD,sources=sources,claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],builds)) for i in IDS],commands=[],live_replacement=False,external_platform_and_world_helpers_authored=True)
    def run(label,command):
        print(label,flush=True)
        result=subprocess.run(list(map(str,command)),cwd=ROOT,env=os.environ|dict(TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'),capture_output=True,timeout=300)
        payload=result.stdout+b'\n'+result.stderr;(out/(label+'.log')).write_bytes(payload)
        report['commands'].append(dict(label=label,exit=result.returncode,log_sha256=hashlib.sha256(payload).hexdigest()))
        if result.returncode:raise ValueError(label+': '+payload.decode(errors='replace')[-3500:])
        return result.stdout.decode()
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        image=ROOT/'original/Arcane_Nocd/Chaos.exe';pe=PE32(image.read_bytes())
        if pe.sha256!=BUILD:raise ValueError('Unsupported original')
        report['flow']=[decode(pe,*r) for r in RANGES]
        report['original_entry_anchors']={hex(a):pe.read(a,16).hex() for a in [*(a for a,b in RANGES),0x4b9b90,0x4b9ba0,0x4b9bb0,0x4b9bc0]}
        inventory_name=reg['builds'][0]['inventory'];inventory=read_json(ROOT/inventory_name,root=ROOT)
        report['inventory_review']=dict(baseline=inventory_name,sha256=evidence_digest(ROOT/inventory_name,root=ROOT),service_functions=[f for f in inventory['functions'] if f['entry'] in [a for a,b in RANGES]],minimal_callbacks_unassigned=True,global_inventory_rewritten=False)
        if any(not any(a<=entry<b for a,b in inventory['unassigned_executable_ranges']) for entry in [0x4b9b90,0x4b9ba0,0x4b9bb0,0x4b9bc0]):raise ValueError('Minimal callback inventory boundary changed')
        # Real shared-tick vtable candidates are explicitly candidates: matching
        # pointers cannot establish constructors, dynamic types or owning parents.
        candidates=[]
        for section in pe.sections:
            if section['executable']:continue
            raw=pe.read(section['address'],section['stored'])
            for offset in range(16,len(raw)-40,4):
                if raw[offset:offset+4]==bytes.fromhex('d0955500'):
                    at=section['address']+offset-16
                    candidates.append(dict(address=at,slots=[pe.word(at+i*4) for i in range(14)],ownership='unrecovered'))
        report['shared_tick_vtable_candidates']=candidates
        report['real_minimal_screen_callbacks']={hex(at):pe.read(at,size).hex() for at,size in [(0x4b9b90,5),(0x4b9ba0,1),(0x4b9bb0,1),(0x4b9bc0,1)]}
        for label,flags in [('original',['-m32','-O2','-DMNM_ORIGINAL_REFERENCE']),('sanitized',['-m32','-O1','-DMNM_ORIGINAL_REFERENCE','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('native64',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            run(label+'-build',['g++','-std=c++17','-mstackrealign','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',*flags,'-MMD','-MF',out/(label+'.d'),ROOT/'tests/screen-services-reference.cpp','-o',out/label])
            for token in shlex.split((out/(label+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(token).resolve()
                if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiler source '+token)
            report[label]=json.loads(run(label,[out/label,*([image] if label!='native64' else [])]))
        if not report['original']==report['sanitized']==report['native64']:raise ValueError('Cross-build results differ')
        report['success']=True
    except (OSError,ValueError,AssertionError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        except (OSError,ValueError,subprocess.SubprocessError) as exc:report.update(success=False,manifest_error=str(exc))
        report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items());report['success'] &= report['sources_stable']
        sha=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],sha256=sha,error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
