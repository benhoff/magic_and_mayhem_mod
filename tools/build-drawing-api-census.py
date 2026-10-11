#!/usr/bin/env python3
"""Build the separate signature-pinned CALL before/after observer; no CRT."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import struct
from drawing_census import ROOT, BUILD_SHA256
from drawing_api_journal import trampoline
from drawing_api_contracts import contract

STRUCT='struct ApiSnapshot {u32 argument,bytes,events;};struct ApiHook {u32 address,length,code_length,rest,reloc_count,after_offset,mode,reg,operand;u8 signature[20],code[64];u32 reloc[8];void(*before)(void);void(*after)(void);u32 patch_bytes,formal_words,snapshot_count;struct ApiSnapshot snapshots[4];};'
REGISTERS={'edi':0,'esi':1,'ebp':2,'ebx':4,'edx':5,'ecx':6,'eax':7,'esp':9}
PLANS={'core_api':('research/runtime/drawing-api-virtual-core-splices-static-20261011.json','9314a6621ef6172c3fade2bf3954e8d3a9f7ea758fe934b1d5582b57c03879ba'),
       'api_only':('research/runtime/drawing-api-virtual-only-splices-static-20261011.json','09c2f679e68fd74a5248e60d1a8e6ec8cc8945139e30e08b500926d5c19d34c8')}


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


def compact_cpu_hooks(hooks, selected):
    """Compact diagnostics cannot silently truncate a graphics client capture."""
    from drawing_cpu_sites import SLOTS
    if selected != 'core_api' or not hooks:
        raise ValueError('Compact CPU journal requires original-active CPU/core hooks')
    for hook in hooks:
        mode = hook['target_mode']; raw = bytes.fromhex(hook['call']['bytes'])
        absolute = mode.get('kind') == 'absolute_memory' and mode.get('address') in SLOTS
        callback = hook['site'] == 0x59d98a and mode == {'kind':'register','register':'eax'}
        if not (absolute and raw == b'\xff\x15'+struct.pack('<I',mode['address']) or callback and raw == b'\xff\xd0'):
            raise ValueError('Compact CPU hook is outside exact global/CRT CALL namespace')
        if hook.get('exact_import'):
            raise ValueError('Compact CPU diagnostics cannot claim an imported client')
        details = contract(hook,2)
        if 'details' in hook and hook['details'] != details:
            raise ValueError('Compact CPU metadata differs from its untyped contract')
        if details['formal_words'] != 0xffffffff or details['snapshots']:
            raise ValueError('Compact CPU hooks cannot carry typed formals or snapshots')


def build(executable, selected='core_api', compact_cpu=False):
    if type(compact_cpu) is not bool:raise ValueError('Compact CPU profile must be explicit boolean')
    if selected not in PLANS:raise ValueError('Unsupported API cohort')
    relative_plan,expected_sha256=PLANS[selected];plan_path=ROOT/relative_plan
    paths=['tools/build-drawing-api-census.py','runtime/drawing-api-census/observer.c','runtime/drawing-api-census/entry.S','runtime/drawing-api-census/exception_i386.h','runtime/shadow/win32_min.h','protocols/include/mnm/drawing_api_journal_v1.h','protocols/include/mnm/drawing_api_journal_v2.h','tools/drawing_api_contracts.py','tools/drawing_api_surface_contracts.py','research/runtime/drawing-surface-api-layouts-20261011.json','protocols/include/mnm/drawing_api_bindings_v1.h','protocols/include/mnm/drawing_census_v1.h','tools/drawing_api_journal.py','tools/drawing_api_pairs.py','tools/drawing_api_sites.py','tools/drawing_census.py','tools/build-shadow-bridge.py','tools/export-menu-support.py',relative_plan]
    if compact_cpu:paths += ['tools/drawing_cpu_sites.py']
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    plan_raw=plan_path.read_bytes()
    if hashlib.sha256(plan_raw).hexdigest()!=expected_sha256:raise ValueError('Approved immutable API splice plan hash mismatch')
    plan=json.loads(plan_raw);raw=executable.read_bytes()
    if plan.get('cohort')!=selected or (plan['core_hook_count']==0)!=(selected=='api_only'):raise ValueError('Static plan/core cohort mismatch')
    if hashlib.sha256(raw).hexdigest()!=BUILD_SHA256 or plan['original_sha256']!=BUILD_SHA256 or plan['input_unchanged'] is not True or plan['sources_stable'] is not True:raise ValueError('Unsupported/stale original CALL plan')
    for p,digest in plan['sources'].items():
        if hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=digest:raise ValueError('Planning input changed: '+p)
    hooks=plan['hooks'];offset=load('api_pe','tools/export-menu-support.py').image_offset
    if compact_cpu:compact_cpu_hooks(hooks,selected)
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
        g=generated(item);detail=contract(item,2);item['details']=detail;slots=','.join('{%d,%d,%d}'%(r['argument'],r['bytes'],r['events']) for r in detail['snapshots']) or '{0,0,0}';header.append('{%d,%d,%d,%d,%d,%d,%d,%d,%d,{%s},{%s},{%s},api_before_%d,api_after_%d,%d,%d,%d,{%s}},'% (item['site'],item['length'],len(g['code']),g['rest'],len(g['relocations']),g['after_offset'],g['mode'],g['register'],g['operand'],','.join(str(v) for v in bytes.fromhex(item['signature'])),','.join(str(v) for v in g['code']),','.join(str(v) for v in g['relocations']),item['tag'],item['tag'],item.get('patch_bytes',item['length']),detail['formal_words'],len(detail['snapshots']),slots))
    header.append('};');(output/'api-hooks.inc').write_text('\n'.join(header)+'\n')
    (output/'entry.S').write_text((ROOT/'runtime/drawing-api-census/entry.S').read_text()+'\n'+'\n'.join(f'api_pair {r["tag"]}' for r in hooks)+'\n')
    objects=[]
    for source,name in [(ROOT/'runtime/drawing-api-census/observer.c','observer'),(output/'entry.S','entry')]:
        obj=output/(name+'.obj');objects.append(str(obj));subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',*(['-DMNM_COMPACT_CPU_JOURNAL=1'] if compact_cpu else []),'-I',str(output),'-c',str(source),'-o',str(obj)],check=True)
    dll=output/'MnmDrawingApiCensus.dll';subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(dll),*objects,str(output/'kernel32.lib')],check=True)
    if hashlib.sha256(executable.read_bytes()).hexdigest()!=BUILD_SHA256 or sources!={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}:raise ValueError('Original/API build inputs changed')
    report={'schema':1,'journal_version':2,'contract_revision':2,'cohort':selected,'original_sha256':BUILD_SHA256,'sources':sources,'sources_stable':True,'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),'hooks':hooks,'refused':plan['refused'],'scope':f'Compiled separate observer for{len(hooks)} exact original CALL sites in{selected}cohort. No original API execution, universal ABI/transparency/timing, output/resource lifetime or DRAW-01 closure.'}
    if compact_cpu:
        report.update(journal_version=1,compact_cpu_journal=True,call_boundary_role='cpu_backend_dispatch',diagnostic_stack_words=8,formal_arguments_recorded=False,buffer_snapshots_recorded=False)
    (output/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'dll':str(dll),'hooks':len(hooks),'refused':len(plan['refused'])}));return dll


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--executable',type=Path,required=True);parser.add_argument('--api-only',action='store_true');args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:build(args.executable,'api_only' if args.api_only else 'core_api')
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
