#!/usr/bin/env python3
"""Compare owned selected kind8 sprite callback traces with unchanged original methods."""
import argparse,hashlib,json,struct,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1];BID='RS.world-kind8-sprite-methods';BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=json.load(open(ROOT/'research/runtime/coverage/register.json'));b=next(b for b in r['behaviors'] if b['id']==BID);s=next(s for s in r['scenarios'] if s['id']=='native-kind8-sprite-methods');paths=required_sources(b)|set(s['tests'])
    return [dict(behavior=BID,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s)})],{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);claims,sources=declaration();exe=ROOT/'original/Arcane_Nocd/Chaos.exe';report=dict(schema=1,success=False,claims=claims,sources=sources,original_sha256=BUILD,scope='Selected5492b0/549730bank/player/RNG/owned frame selection and word/generic/AX1clipped callback traces. Synthetic RNG/raster bindings; no pixels, actual RNG implementation, malformed original pointers/negative counts, native live class/caller admission or replacement.',live_replacement=False,raster_callbacks_synthetic=True,random_callback_synthetic=True)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,cmd):
        with (out/(name+'.stdout')).open('xb') as f,(out/(name+'.stderr')).open('xb') as e:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=f,stderr=e,check=True,timeout=300)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        if sha(exe)!=BUILD:raise ValueError('Unsupported original')
        raw=exe.read_bytes();pe=struct.unpack_from('<I',raw,60)[0];sections=struct.unpack_from('<H',raw,pe+6)[0];optional=struct.unpack_from('<H',raw,pe+20)[0]
        def at(address,size):
            for i in range(sections):
                row=pe+24+optional+i*40;_,rva,rsz,offset=struct.unpack_from('<4I',raw,row+8);delta=address-0x400000-rva
                if 0<=delta and delta+size<=rsz:return raw[offset+delta:offset+delta+size]
            raise ValueError('Unbacked discovery address')
        discovery=[]
        for table,method,assignment,bank,init,push,name in [(0x5c74e4,0x5492b0,0x5491cb,0x6f2150,0x549210,0x54921b,0x5eb48c),(0x5c74fc,0x549730,0x549951,0x6f2268,0x549690,0x54969b,0x5eb4a0)]:
            if at(table+12,4)!=struct.pack('<I',method) or at(assignment,6)!=b'\xc7\x06'+struct.pack('<I',table) or at(init,5)!=b'\xa1'+struct.pack('<I',bank) or at(push,5)!=b'\x68'+struct.pack('<I',name):raise ValueError('Pinned constructor/bank/resource binding changed')
            discovery.append(dict(vtable=table,method=method,constructor_assignment=assignment,bank_global=bank,resource_string=at(name,32).split(b'\0')[0].decode('ascii')))
        anchors={};ranges=[]
        for start,end,bank in [(0x5492b0,0x549375,0x6f2150),(0x549730,0x549815,0x6f2268)]:
            if at(start,6)!=b'\x8b\x15'+struct.pack('<I',bank) or at(end-3,3)!=b'\xc2\x08\x00' or at(end,5)!=b'\x90'*5:raise ValueError('Recovered method extent changed')
            anchors[f'{start:08x}']=at(start,16).hex();ranges.append(dict(start=start,end=end,sha256=hashlib.sha256(at(start,end-start)).hexdigest()))
        report.update(class_bindings_verified=True,discovery=discovery,anchors=anchors,recovered_method_ranges=ranges)
        for name in ['native','original']:
            run(name+'-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',*(['-DMNM_ORIGINAL_REFERENCE'] if name=='original' else []),ROOT/'tests/kind8-sprite-method-test.cpp','-o',out/name])
            run(name,[out/name,*([exe] if name=='original' else [])])
        native=(out/'native.stdout').read_bytes();original=(out/'original.stdout').read_bytes()
        if len(native)!=72000*140 or native!=original:
            first=next((i for i,(x,y) in enumerate(zip(native,original)) if x!=y),min(len(native),len(original)))
            raise ValueError('Callback trace mismatch at byte'+str(first)+' lengths'+str((len(native),len(original))))
        if (out/'native.stderr').read_text()!='native_controls=8\n':raise ValueError('Incomplete native refusals')
        counts=dict(no_raster=0,word=0,generic=0,clipped=0,random_calls=0)
        for row in struct.iter_unpack('<35I',native):
            if row[0] or row[1]>2 or row[2]>2:raise ValueError('Invalid original return/call count')
            counts['random_calls']+=row[2]
            if not row[1]:counts['no_raster']+=1
            for i in range(row[1]):counts[['word','generic','clipped'][row[3+i*8]]]+=1
        if declaration()!=(claims,sources) or sha(exe)!=BUILD:raise ValueError('Source/input changed')
        report.update(success=True,sources_stable=True,original_wrapper_instructions_unchanged=True,callback_traces_match=True,object_preservation_compared=True,original_cases=72000,native_controls=8,trace_sha256=sha(out/'native.stdout'),trace_counts=counts)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
