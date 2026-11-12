#!/usr/bin/env python3
"""Compare owned screen providers and failure policies in bounded local hosts."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import shlex
import subprocess
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import evidence_digest, read_json, write_report
from original_x86 import PE32, Decoder

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RS.screen-control-resource-providers','RS.screen-bitmap-resource-providers',
       'RS.screen-provider-failure-paths','NP.screen-owned-provider-recovery']
# Unknown indirect flows are refused. These three tables are the recovered
# layout-type dispatch and the two compressed shade-count dispatches.
TABLES = {0x558649:(0x5591fc,11),0x5844a5:(0x584b78,9),0x5892f1:(0x5897b4,9)}


def flow(pe, entry, ranges):
    decoder = Decoder()
    todo, rows, tails, tables = [entry], {}, [], []
    try:
        while todo:
            at = todo.pop()
            if at in rows: continue
            if not any(a <= at < b for a,b in ranges):
                raise ValueError('Unassigned local flow '+hex(at))
            row=decoder.decode(pe.read(at,15),at);rows[at]=row
            name,operand=row['mnemonic'],row['operand']
            if name.startswith('ret'): continue
            if name.startswith('j'):
                if operand.startswith('0x'):
                    target=int(operand,16)
                    if any(a<=target<b for a,b in ranges):todo.append(target)
                    elif name=='jmp':tails.append(dict(at=at,target=target))
                    else:raise ValueError('Conditional edge escapes '+hex(at))
                elif at in TABLES:
                    address,count=TABLES[at];data=pe.read(address,count*4)
                    targets=[int.from_bytes(data[i:i+4],'little') for i in range(0,len(data),4)]
                    todo.extend(targets);tables.append(dict(address=address,targets=targets,sha256=hashlib.sha256(data).hexdigest()))
                else:raise ValueError('Unresolved indirect flow '+hex(at))
            if name!='jmp':todo.append(at+row['size'])
        return dict(entry=entry,instructions=sorted(rows.values(),key=lambda x:x['address']),
                    tables=tables,tail_endpoints=tails,local_edges_reviewed=True)
    finally:decoder.close()


def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',required=True)
    args=parser.parse_args();out=args.output.resolve();target=(ROOT/args.report).resolve()
    temporary=Path(os.environ.get('TMPDIR','/tmp')).resolve()
    if (out.exists() or target.exists() or Path(args.report).is_absolute()
        or not target.is_relative_to(ROOT/'research/runtime') or target.suffix!='.json'
        or '.git' in out.parts or out.is_relative_to((ROOT/'original').resolve())
        or not (out.is_relative_to(ROOT/'working') or out.is_relative_to(temporary))):
        parser.error('Require new owned working/RAM output and immutable research/runtime JSON report')
    out.mkdir(parents=True);(out/'.retain').write_text('Screen providers: immutable inputs and execution diagnostics retained in SQLite.\n')
    register=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
    behaviors={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(set().union(*(required_sources(behaviors[i]) for i in IDS)))}
    report=dict(schema=1,success=False,original_sha256=PIN,sources=sources,
      claims=[dict(behavior=i,contract_sha256=behavior_contract(behaviors[i],builds)) for i in IDS],
      live_replacement=False,windows_seh_dispatch=False,physical_driver_equivalence=False,
      platform_file_profile_heap_com_codec_endpoints_supplied=True,commands=[])
    scenario=next(s for s in register['scenarios'] if s['id']=='screen-providers-owned-failures-20261026')
    for claim in report['claims']:claim['scenarios']={scenario['id']:scenario_contract(scenario)}
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1:handle_segv=0:handle_sigfpe=0',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    def run(label,command):
        print(label,flush=True)
        result=subprocess.run(list(map(str,command)),cwd=ROOT,env=env,capture_output=True,timeout=900)
        data=result.stdout+b'\n'+result.stderr
        digest=write_report(ROOT,str(Path(args.report).with_suffix(''))+'-'+label+'.log',data)
        report['commands'].append(dict(label=label,exit=result.returncode,sha256=digest,argv=list(map(str,command))))
        if result.returncode:raise ValueError(label+' failed: '+data.decode(errors='replace')[-4000:])
        return result.stdout.decode()
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        image=ROOT/'original/Arcane_Nocd/Chaos.exe';pe=PE32(image.read_bytes())
        if pe.sha256!=PIN:raise ValueError('Original hash differs')
        inventory=read_json(ROOT/builds['nocd-40209ca7']['inventory'],root=ROOT)
        functions={f['entry']:f for f in inventory['functions']}
        discoveries={}
        for name in ['discovery01.json','discovery02.json']:
            discoveries.update(read_json(ROOT/'research/runtime/screen-provider-results-20261026'/name,root=ROOT)['functions'])
        entries=sorted({int(o['address']) for i in IDS for o in behaviors[i].get('original',[]) if o.get('relation','code')=='code'}-{0x5977d0})
        cold={0x5c0510:0x5c051e,0x5c051e:0x5c052c,0x5bf610:0x5bf61e,0x5bf61e:0x5bf62c,0x5bf62c:0x5bf63a,0x5bf63a:0x5bf648,0x5c1b50:0x5c1b58}
        report['flow']=[];report['original_entry_anchors']={hex(a):pe.read(a,16).hex() for a in entries}
        body={};owners={}
        for entry in entries:
            if entry in functions:ranges=functions[entry]['ranges']
            elif entry in cold:ranges=[[entry,cold[entry]]]
            else:
                recovered=[x for x in register.get('recovered_ranges',[]) if x['build']=='nocd-40209ca7' and x['start']==entry]
                ranges=[[entry,max(x['end'] for x in recovered)]] if recovered else [[entry,entry+0x4000]]
            item=flow(pe,entry,ranges);report['flow'].append(item)
            for row in item['instructions']:
                body[row['address']]=pe.read(row['address'],row['size']);owners[row['address']]=entry
            for table in item['tables']:
                body[table['address']]=pe.read(table['address'],len(table['targets'])*4);owners[table['address']]=entry
        spans=[];payload=[]
        for address,data in sorted(body.items()):
            spans.append((address,len(payload),len(data),int(owners[address] in {0x58b660,0x584430,0x589280})))
            payload.extend(data)
        header=['#pragma once','static void guardBodies(bool controlledLower=false){',
                'static const unsigned char bytes[]={'+','.join(map(str,payload))+'};',
                'static const unsigned spans[][4]={'+','.join('{'+','.join(map(str,row))+'}' for row in spans)+'};',
                'for(const auto& row:spans)if(!(controlledLower&&row[3]))check(!std::memcmp(reinterpret_cast<const void*>(std::uintptr_t(row[0])),bytes+row[1],row[2]),"Original provider body changed");','}']
        (out/'screen-provider-protected.hpp').write_text('\n'.join(header)+'\n')
        report['protected_header_sha256']=hashlib.sha256((out/'screen-provider-protected.hpp').read_bytes()).hexdigest()
        configurations=[('original',['-m32','-O2','-DMNM_ORIGINAL_REFERENCE','-DMNM_SCREEN_PROTECTED']),
          ('sanitized',['-m32','-O1','-DMNM_ORIGINAL_REFERENCE','-DMNM_SCREEN_PROTECTED','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
          ('native64',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
        roots={'control':'tests/screen-provider-reference.cpp','surface':'tests/screen-surface-reference.cpp',
               'palette':'tests/screen-palette-reference.cpp','layout':'tests/screen-layout-reference.cpp',
               'world-failure':'tests/screen-world-failure-reference.cpp'}
        report['executions']={}
        for config,flags in configurations:
            report['executions'][config]={}
            for suite,cpp in roots.items():
                binary=out/(config+'-'+suite);dep=out/(config+'-'+suite+'.d')
                command=['g++','-std=c++17','-g','-mstackrealign','-Wall','-Wextra','-Werror','-Wno-unused-function','-fno-pie','-no-pie',*flags,'-I',out,'-MMD','-MF',dep,ROOT/cpp]
                object_file=out/(config+'-'+suite+'.o')
                run(config+'-'+suite+'-compile',command+['-c','-o',object_file])
                link=['g++','-fno-pie','-no-pie',*flags,object_file]
                if config!='native64':link += [ROOT/'tests/actor-services-call.S',ROOT/'tests/screen-provider-call.S']
                run(config+'-'+suite+'-link',link+['-o',binary])
                for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                    path=Path(token).resolve()
                    if path.is_relative_to(ROOT) and path!=out/'screen-provider-protected.hpp' and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared source '+str(path))
                report['executions'][config][suite]=json.loads(run(config+'-'+suite,[binary,*([image] if config!='native64' else [])]))
        if report['executions']['original']!=report['executions']['sanitized'] or report['executions']['original']!=report['executions']['native64']:raise ValueError('Cross-configuration mismatch')
        for suite,cpp,extra in [('exceptions','tests/screen-provider-exceptions-test.cpp',[]),('composition','tests/screen-composed-resources-test.cpp',['reconstruction/rendering/palette_effects.cpp','reconstruction/rendering/palette_shading.cpp','reconstruction/rendering/terrain_selection.cpp'])]:
            binary=out/suite;objects=[]
            flags=['-std=c++17','-g','-O1','-Wall','-Wextra','-Werror','-Wno-unused-function','-fsanitize=address,undefined','-fno-omit-frame-pointer']
            for n,unit in enumerate([cpp]+extra):
                if unit not in sources:raise ValueError('Undeclared translation unit '+unit)
                obj=out/(suite+'-'+str(n)+'.o');dep=out/(suite+'-'+str(n)+'.d');objects.append(obj)
                run(suite+'-'+str(n)+'-compile',['g++',*flags,'-MMD','-MF',dep,'-c',ROOT/unit,'-o',obj])
                for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                    path=Path(token).resolve()
                    if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared source '+str(path))
            run(suite+'-link',['g++',*flags,*objects,'-o',binary])
            report[suite]=json.loads(run(suite,[binary]))
        build=out/'native'
        run('native-configure',['cmake','-S',ROOT/'assets/resources','-B',build,'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer'])
        run('native-build',['cmake','--build',build,'--target','screen-resources-test','resource-manager-test','resource-preparation-test','-j2'])
        for dep in build.rglob('*.o.d'):
            for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(token).resolve()
                if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared native build source '+str(path))
        report['native']=json.loads(run('native-owner',[build/'screen-resources-test']))
        run('native-regressions',['ctest','--test-dir',build,'-R','native-resource-(manager|preparation)','--output-on-failure'])
        report['success']=True
    except (OSError,ValueError,KeyError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        except (OSError,ValueError,KeyError,subprocess.SubprocessError) as exc:report.update(success=False,manifest_error=str(exc))
        report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items())
        current=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
        cb={b['id']:b for b in current['behaviors']};bb={b['id']:b for b in current['builds']}
        ss=next(s for s in current['scenarios'] if s['id']==scenario['id'])
        report['contracts_stable']=all(behavior_contract(cb[c['behavior']],bb)==c['contract_sha256'] and c['scenarios'][scenario['id']]==scenario_contract(ss) for c in report['claims'])
        report['success'] &= report['sources_stable'] and report['contracts_stable']
        if not report['sources_stable'] or not report['contracts_stable']:report.setdefault('error','Source or contract declarations changed during execution')
        sha=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],sha256=sha,error=report.get('error'))),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
