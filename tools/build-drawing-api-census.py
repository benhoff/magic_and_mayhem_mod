#!/usr/bin/env python3
"""Build the separate signature-pinned CALL before/after observer; no CRT."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT, BUILD_SHA256
from drawing_api_journal import trampoline
from drawing_api_contracts import contract

STRUCT='struct ApiSnapshot {u32 argument,bytes,events;};struct ApiHook {u32 address,length,code_length,rest,reloc_count,after_offset,mode,reg,operand;u8 signature[20],code[64];u32 reloc[8];void(*before)(void);void(*after)(void);u32 patch_bytes,formal_words,snapshot_count;struct ApiSnapshot snapshots[4];};'
REGISTERS={'edi':0,'esi':1,'ebp':2,'ebx':4,'edx':5,'ecx':6,'eax':7,'esp':9}
PLANS={'core_api':('research/runtime/drawing-api-v2-core-plan-static-20261011.json','16cb948e61e4f9eed276cc00a962e700a82279ab6c6da73722ea14128174adfe'),
       'api_only':('research/runtime/drawing-api-v2-only-plan-static-20261011.json','8788e8e66d34f1afe93986466b3e2200418e1dc342e1b753d7cc32b52488b1ea')}


def load(name, path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module


def generated(item):
    code,rest=trampoline(item,0,0);call_length=item['call_length'];after=call_length+1
    offsets=[]
    if item['call']['transfer']:offsets.append(1)
    offsets.append(after);cursor=rest
    for row in item['remainder']:
        transfer=row['transfer']
        if transfer:offsets.append(cursor+(2 if transfer['kind']=='conditional' else 1))
        cursor+=6 if transfer and transfer['kind']=='conditional' else 5 if transfer else len(bytes.fromhex(row['bytes']))
    offsets.append(len(code)-4)
    if len(code)>64 or len(offsets)>8 or any(offset+4>len(code) for offset in offsets):raise ValueError('Generated API trampoline exceeds bounds')
    mode=item['target_mode'];kind={'register':1,'register_memory':2,'absolute_memory':3,'direct_relative':4}[mode['kind']]
    operand=mode.get('displacement',mode.get('address',mode.get('target',0)))&0xffffffff
    return {'code':code,'rest':rest,'relocations':offsets,'after_offset':after,'mode':kind,'register':REGISTERS.get(mode.get('register'),7),'operand':operand}


def build(executable, selected='core_api'):
    if selected not in PLANS:raise ValueError('Unsupported API cohort')
    relative_plan,expected_sha256=PLANS[selected];plan_path=ROOT/relative_plan
    paths=['tools/build-drawing-api-census.py','runtime/drawing-api-census/observer.c','runtime/drawing-api-census/entry.S','runtime/drawing-api-census/exception_i386.h','runtime/shadow/win32_min.h','protocols/include/mnm/drawing_api_journal_v1.h','protocols/include/mnm/drawing_api_journal_v2.h','tools/drawing_api_contracts.py','protocols/include/mnm/drawing_api_bindings_v1.h','protocols/include/mnm/drawing_census_v1.h','tools/drawing_api_journal.py','tools/drawing_api_pairs.py','tools/drawing_api_sites.py','tools/drawing_census.py','tools/build-shadow-bridge.py','tools/export-menu-support.py',relative_plan]
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    plan_raw=plan_path.read_bytes()
    if hashlib.sha256(plan_raw).hexdigest()!=expected_sha256:raise ValueError('Approved immutable API splice plan hash mismatch')
    plan=json.loads(plan_raw);raw=executable.read_bytes()
    if plan.get('cohort')!=selected or (plan['core_hook_count']==0)!=(selected=='api_only'):raise ValueError('Static plan/core cohort mismatch')
    if hashlib.sha256(raw).hexdigest()!=BUILD_SHA256 or plan['original_sha256']!=BUILD_SHA256 or plan['input_unchanged'] is not True or plan['sources_stable'] is not True:raise ValueError('Unsupported/stale original CALL plan')
    for p,digest in plan['sources'].items():
        if hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=digest:raise ValueError('Planning input changed: '+p)
    hooks=plan['hooks'];offset=load('api_pe','tools/export-menu-support.py').image_offset
    if [r['tag'] for r in hooks]!=list(range(len(hooks))) or not 0<len(hooks)<=512:raise ValueError('Invalid bounded API hook census')
    for item in hooks:
        expected=bytes.fromhex(item['signature']);start=offset(raw,item['site'],len(expected))
        if len(expected)!=item['length'] or raw[start:start+len(expected)]!=expected:raise ValueError('Original API signature mismatch')
    output=ROOT/'working/build/drawing-api-census';output.mkdir(parents=True,exist_ok=True)
    imports=dict(load('api_imports','tools/build-shadow-bridge.py').IMPORTS,GetCurrentThreadId=0,GetTickCount=0,Sleep=4,CreateFileMappingA=24,MapViewOfFile=20,UnmapViewOfFile=4,GetProcAddress=8,AddVectoredExceptionHandler=8)
    (output/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(output/'kernel32.def'),'-l',str(output/'kernel32.lib'),'--kill-at'],check=True)
    header=[f'#define API_HOOKS {len(hooks)}',STRUCT,'static const u8 original_sha256[32]={'+','.join(str(v) for v in bytes.fromhex(BUILD_SHA256))+'};']
    for item in hooks:header += [f'extern void api_before_{item["tag"]}(void);',f'extern void api_after_{item["tag"]}(void);']
    header.append('static const struct ApiHook api_hooks[API_HOOKS]={')
    for item in hooks:
        g=generated(item);detail=contract(item);item['details']=detail;slots=','.join('{%d,%d,%d}'%(r['argument'],r['bytes'],r['events']) for r in detail['snapshots']) or '{0,0,0}';header.append('{%d,%d,%d,%d,%d,%d,%d,%d,%d,{%s},{%s},{%s},api_before_%d,api_after_%d,%d,%d,%d,{%s}},'% (item['site'],item['length'],len(g['code']),g['rest'],len(g['relocations']),g['after_offset'],g['mode'],g['register'],g['operand'],','.join(str(v) for v in bytes.fromhex(item['signature'])),','.join(str(v) for v in g['code']),','.join(str(v) for v in g['relocations']),item['tag'],item['tag'],item.get('patch_bytes',item['length']),detail['formal_words'],len(detail['snapshots']),slots))
    header.append('};');(output/'api-hooks.inc').write_text('\n'.join(header)+'\n')
    (output/'entry.S').write_text((ROOT/'runtime/drawing-api-census/entry.S').read_text()+'\n'+'\n'.join(f'api_pair {r["tag"]}' for r in hooks)+'\n')
    objects=[]
    for source,name in [(ROOT/'runtime/drawing-api-census/observer.c','observer'),(output/'entry.S','entry')]:
        obj=output/(name+'.obj');objects.append(str(obj));subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I',str(output),'-c',str(source),'-o',str(obj)],check=True)
    dll=output/'MnmDrawingApiCensus.dll';subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(dll),*objects,str(output/'kernel32.lib')],check=True)
    if hashlib.sha256(executable.read_bytes()).hexdigest()!=BUILD_SHA256 or sources!={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}:raise ValueError('Original/API build inputs changed')
    report={'schema':1,'journal_version':2,'cohort':selected,'original_sha256':BUILD_SHA256,'sources':sources,'sources_stable':True,'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'hooks':hooks,'refused':plan['refused'],'scope':f'Compiled separate observer for{len(hooks)} exact original CALL sites in{selected}cohort. No original API execution, universal ABI/transparency/timing, output/resource lifetime or DRAW-01 closure.'}
    (output/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'dll':str(dll),'hooks':len(hooks),'refused':len(plan['refused'])}));return dll


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--executable',type=Path,required=True);parser.add_argument('--api-only',action='store_true');args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:build(args.executable,'api_only' if args.api_only else 'core_api')
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
