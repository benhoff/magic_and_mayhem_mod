#!/usr/bin/env python3
"""Compare bounded refresh callers, local scheduler gates and actual light cycles."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import write_report, read_bytes
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS=['TL.script-relation-production']

def sha(path):
    return hashlib.sha256(read_bytes(path,root=ROOT) if path.is_relative_to(ROOT) else path.read_bytes()).hexdigest()

def image_slice(pe,address,count):
    head=int.from_bytes(pe[0x3c:0x40],'little');opt=head+24
    start=opt+int.from_bytes(pe[head+20:head+22],'little')
    for i in range(int.from_bytes(pe[head+6:head+8],'little')):
        s=start+i*40;rva=int.from_bytes(pe[s+12:s+16],'little');n=int.from_bytes(pe[s+16:s+20],'little');offset=int.from_bytes(pe[s+20:s+24],'little')
        d=address-0x400000-rva
        if 0<=d and d+count<=n:return pe[offset+d:offset+d+count]
    raise ValueError('Address outside raw executable sections: '+hex(address))

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--main-root',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True)
    a=p.parse_args();main_root=a.main_root.resolve();out=a.output.resolve();report_path=a.report.absolute()
    if out.exists() or report_path.exists() or not report_path.is_relative_to(ROOT/'research/runtime'):p.error('Require new output and immutable research report paths')
    out.mkdir(parents=True);(out/'.retain').write_text('Refresh caller validation outputs; retain until durable evidence verified.\n');(out/'tmp').mkdir()
    env=dict(os.environ,TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behaviors=[b for b in reg['behaviors'] if b['id'] in IDS];assert len(behaviors)==1
    builds={b['id']:b for b in reg['builds']};scenarios=[]
    sources=set().union(*(required_sources(b)|set(b['documents']) for b in behaviors));sources.update(['tools/coverage_claims.py','tools/coverage_evidence.py'])
    before={name:sha(ROOT/name) for name in sorted(sources)}
    report=dict(schema=1,success=False,original_sha256=BUILD,sources=before,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={s['id']:scenario_contract(s) for s in scenarios if b['id'] in s['behaviors']}) for b in behaviors],commands=[],results=[],live_replacement=False,
                scope='; '.join(b['scope'] for b in behaviors))
    pe_path=main_root/'original/Arcane_Nocd/Chaos.exe';verified_before=False
    def run(label,cmd,timeout=180):
        r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=timeout)
        (out/(label+'.log')).write_text(r.stdout+r.stderr);report['commands'].append(dict(label=label,exit_code=r.returncode,log_sha256=sha(out/(label+'.log'))));r.check_returncode();return r.stdout
    try:
        run('original-before',[main_root/'tools/original-manifest.sh','verify']);verified_before=True
        assert sha(pe_path)==BUILD,'Original executable hash mismatch';pe=pe_path.read_bytes()
        fixture=(ROOT/'tests/terrain-light-script-reference.cpp').read_text()
        addresses=sorted({int(v,16) for v in re.findall(r'REDIRECT\((0x[0-9a-f]+),',fixture)})
        adapters={hex(v):image_slice(pe,v,8).hex() for v in addresses}
        ranges={0x56a8d0:0x56b1da}
        protected={v:image_slice(pe,v,end-v) for v,end in ranges.items()}
        report['original_entry_anchors']={hex(v):raw[:16].hex() for v,raw in protected.items()};report['adapters']=adapters
        report['protected_original_bodies']={hex(v):dict(end=ranges[v],sha256=hashlib.sha256(raw).hexdigest()) for v,raw in protected.items()}
        table='#include <map>\nstatic const std::map<U,Bytes> adapterBytes={\n'+''.join('{'+hex(v)+', {'+','.join('0x'+raw[i:i+2] for i in range(0,len(raw),2))+'}},\n' for v in addresses for raw in [adapters[hex(v)]])+'};\nstatic const std::map<U,Bytes> protectedBytes={\n'+''.join('{'+hex(v)+', {'+','.join(str(b) for b in raw)+'}},\n' for v,raw in protected.items())+'};\n'
        (out/'refresh-adapter-bytes.hpp').write_text(table);report['adapter_header_sha256']=sha(out/'refresh-adapter-bytes.hpp')
        models=[ROOT/'reconstruction/rendering'/n for n in ['terrain_lighting.cpp','terrain_creature_lighting.cpp','terrain_static_lighting.cpp','terrain_lighting_cycle.cpp']]
        common=['g++','-std=c++17','-I'+str(ROOT/'reconstruction/rendering'),'-I'+str(out)]
        for name,flags in [('original',['-O2','-m32','-fno-pie','-no-pie','-DORIGINAL_REFERENCE']),('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
            print(name,flush=True);binary=out/name;output=out/(name+'.bin');deps=out/(name+'.d')
            run(name+'-build',[*common,*flags,'-MMD','-MF',deps,ROOT/'tests/terrain-light-script-reference.cpp',*models,'-o',binary])
            # Independent preprocessing records the fixture's header closure;
            # compiler depfiles for each linked model are also checked below.
            dependencies=set()
            for i,unit in enumerate([ROOT/'tests/terrain-light-script-reference.cpp',*models]):
                listing=run(name+'-deps-'+str(i),[*common,*flags,'-MM',unit])
                for token in listing.replace('\\\n',' ').split(':',1)[1].split():
                    dep=Path(token).resolve()
                    if dep==out/'refresh-adapter-bytes.hpp':continue
                    if dep.is_relative_to(ROOT):dependencies.add(dep.relative_to(ROOT).as_posix())
            assert dependencies<=set(before),'Unfingerprinted dependencies: '+repr(dependencies-set(before))
            stats=json.loads(run(name,[binary,*([pe_path] if name=='original' else []),output]))
            report['results'].append(dict(variant=name,stats=stats,output_sha256=sha(output),output_bytes=output.stat().st_size,binary_sha256=sha(binary),dependencies=sorted(dependencies)))
        first=report['results'][0]
        report['all_match']=all((r['stats'],r['output_sha256'],r['output_bytes'])==(first['stats'],first['output_sha256'],first['output_bytes']) for r in report['results'])
        if not report['all_match']:
            left=(out/'original.bin').read_bytes();right=(out/'native.bin').read_bytes();at=next((i for i,(x,y) in enumerate(zip(left,right)) if x!=y),min(len(left),len(right)))
            report['first_mismatch']=dict(offset=at,original=left[max(0,at-16):at+32].hex(),native=right[max(0,at-16):at+32].hex());raise ValueError('Original/native/sanitized output mismatch')
        report['success']=True
    except Exception as exc:report['error']=str(exc)
    finally:
        try:run('original-after',[main_root/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=verified_before
        except Exception as exc:report['manifest_error']=str(exc);report['success']=False
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in before.items());report['inputs_stable']=pe_path.is_file() and sha(pe_path)==BUILD
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        h=write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode());assert hashlib.sha256(read_bytes(report_path,root=ROOT)).hexdigest()==h
        print(json.dumps(dict(success=report['success'],report=str(report_path),sha256=h,error=report.get('error'),first_mismatch=report.get('first_mismatch'),results=report['results'])),flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
