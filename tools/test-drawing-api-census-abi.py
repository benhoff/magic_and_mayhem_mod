#!/usr/bin/env python3
"""Run real separate CALL observer/stubs against synthetic PE32 bodies."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import struct
import tempfile
from drawing_census import ROOT,BUILD_SHA256
from drawing_api_bindings import decode,physical
from drawing_api_details import trace as detail_trace
from drawing_api_journal import trace as compact_trace


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--prefix-template',type=Path,required=True);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--compact-cpu',action='store_true');args=parser.parse_args()
    claims=json.loads(args.claims.read_text());sources=claims['sources']
    if any(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=digest for p,digest in sources.items()):raise ValueError('Prospective ABI sources changed')
    parent=ROOT/'working/tests/drawing-api-census-abi';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree,mshtml=d',MNM_API_ABI_DIR='Z:'+str(out).replace('/','\\'))
    def run(name,command,seconds=90):
        with (out/(name+'.log')).open('x') as log:subprocess.run([str(v) for v in command],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=seconds,check=True)
    spec=importlib.util.spec_from_file_location('abi_builder',ROOT/'tools/build-drawing-api-census.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
    imports=dict(builder.load('abi_imports','tools/build-shadow-bridge.py').IMPORTS,GetCurrentThreadId=0,GetTickCount=0,Sleep=4,CreateFileMappingA=24,MapViewOfFile=20,UnmapViewOfFile=4,GetProcAddress=8,AddVectoredExceptionHandler=8)
    (out/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    run('imports',['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',out/'kernel32.def','-l',out/'kernel32.lib','--kill-at'])
    header=['#define API_HOOKS 26',builder.STRUCT,'static const u8 original_sha256[32]={'+','.join(str(v) for v in bytes.fromhex(BUILD_SHA256))+'};']
    for tag in range(26):header += [f'extern void api_before_{tag}(void);',f'extern void api_after_{tag}(void);']
    header += ['static struct ApiHook api_hooks[API_HOOKS]={']+[f'{{0,0,0,0,0,0,0,0,0,{{0}},{{0}},{{0}},api_before_{tag},api_after_{tag},0,0,0,{{{{0,0,0}}}}}},' for tag in range(26)]+['};']
    (out/'api-hooks.inc').write_text('\n'.join(header)+'\n');(out/'entry.S').write_text((ROOT/'runtime/drawing-api-census/entry.S').read_text()+'\n'+'\n'.join(f'api_pair {tag}' for tag in range(26))+'\n')
    objects=[]
    for index,source in enumerate([ROOT/'tests/drawing-api-census-abi.c',ROOT/'tests/drawing-api-detail-call.S',out/'entry.S']):
        obj=out/(f'part{index}.obj');objects.append(obj);run(f'compile{index}',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',*(['-DMNM_COMPACT_CPU_JOURNAL=1'] if args.compact_cpu else []),'-I',out,'-c',source,'-o',obj])
    run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'fixture.exe'),*objects,out/'kernel32.lib'])
    run('prefix-copy',['cp','-a','--reflink=auto',args.prefix_template.resolve(),out/'wineprefix'])
    try:run('forwarding',['xvfb-run','-a','wine',out/'fixture.exe'])
    finally:run('stop-prefix',['wineserver','-k'],10)
    raw=(out/'abi-hooks.bin').read_bytes()
    if len(raw)!=3328:raise ValueError('Synthetic hook manifest extent mismatch')
    synthetic=[];registers={0:'edi',1:'esi',2:'ebp',4:'ebx',5:'edx',6:'ecx',7:'eax',9:'esp'}
    for tag in range(26):
        site,length,call_length,mode,register,operand=struct.unpack_from('<6I',raw,tag*128)
        signature=raw[tag*128+24:tag*128+24+length]
        patch_bytes=struct.unpack_from('<I',raw,tag*128+44)[0]
        formal_words,snapshot_count=struct.unpack_from('<2I',raw,tag*128+48)
        if snapshot_count>4 or any(raw[tag*128+56:tag*128+64]):raise ValueError('Synthetic argument metadata bounds/reserved mismatch')
        snapshots=[]
        for index in range(4):
            argument,size,events,reserved=struct.unpack_from('<4I',raw,tag*128+64+index*16)
            if reserved or index>=snapshot_count and any((argument,size,events)):raise ValueError('Synthetic snapshot metadata reserved mismatch')
            if index<snapshot_count:snapshots.append({'argument':argument,'bytes':size,'events':events,'name':f'fixture buffer{index}'})
        if length>20 or any(raw[tag*128+24+length:tag*128+44]):raise ValueError('Synthetic signature/reserved mismatch')
        target={'kind':'absolute_memory','address':operand} if mode==3 else {'kind':'direct_relative','target':operand} if mode==4 else {'kind':'register','register':registers[register]} if mode==1 else {'kind':'register_memory','register':registers[register],'displacement':operand if operand<0x80000000 else operand-0x100000000}
        transfer={'kind':'call','target':operand,'condition':None} if mode==4 else None
        call={'address':site,'bytes':signature[:call_length].hex(),'transfer':transfer};rest=[]
        if tag==7:rest=[{'address':site+2,'bytes':'85c0','transfer':None},{'address':site+4,'bytes':'7502','transfer':{'kind':'conditional','target':site+8,'condition':5}}]
        if tag in (8,24):rest=[{'address':site+2,'bytes':'85c0','transfer':None},{'address':site+4,'bytes':'90','transfer':None}]
        synthetic.append({'details':{'name':f'fixture{tag}','origin':'synthetic_fixture','formal_words':formal_words,'snapshots':snapshots},'patch_mode':'breakpoint' if patch_bytes==1 else 'jump','patch_bytes':patch_bytes,'tag':tag,'site':site,'length':length,'call_length':call_length,'signature':signature.hex(),'target_mode':target,'call':call,'remainder':rest})
    synthetic_manifest={'journal_version':1 if args.compact_cpu else 2,'synthetic_fixture':True,'original_sha256':BUILD_SHA256,'hooks':synthetic,'scope':'Synthetic ABI only; no original splice execution.'}
    (out/'synthetic-manifest.json').write_text(json.dumps(synthetic_manifest,indent=2)+'\n')
    bindings=decode(out/'api-bindings.bin',synthetic_manifest)
    for item in bindings['hooks']:
        mapped=physical(item['physical_call_return'],bindings)
        if mapped.get('original_call_site')!=item['site'] or mapped.get('tag')!=item['tag']:raise ValueError('Synthetic exact binding classification failed')
    details=(compact_trace if args.compact_cpu else detail_trace)(out/'abi-api.bin',synthetic_manifest)
    if details['paired_calls']!=26 or not args.compact_cpu and details['details']['receiver_candidate_calls']!=2:raise ValueError('Native writer/decoder pair/candidate mismatch')
    (out/'decoded-api.json').write_text(json.dumps(details,indent=2)+'\n')
    (out/'decoded-bindings.json').write_text(json.dumps(bindings,indent=2)+'\n')
    stable=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==digest for p,digest in sources.items())
    if not stable:raise ValueError('ABI inputs changed during execution')
    report={'schema':1,'scope':'Real separate CALL stubs/observer/dynamic splices and binding writer/decoder round trip against26synthetic PE32 bodies only. No original API/graphics or arbitrary machine-context/transparency/output/lifetime validation.','success':True,'sources':sources,'sources_stable':stable,'claims':claims['claims'],'callee_pop_bytes':[0,4,8,12,16,20,24,52],'call_modes':['absolute_memory','register','register_memory','direct_relative'],'synthetic_bodies':26,'register_flags_x87_sse_last_error_preserved':True,'pre_call_stack_and_target_checked':True,'negative_eax_actual_cleanup_checked':True,'short_conditional_remainder_executed':True,'frame_exhaustion_phase_record_cap_io_failure_forward':True,'same_thread_lifo_and_mismatch_loss_checked':True,'one_byte_breakpoint_cases':13,'foreign_exception_forwarding_checked':True,'call_operand_and_successor_unchanged_checked':True,'callee_entry_flags_registers_last_error_checked_bodies':22,'full_thirteen_argument_stack_checked':True,'null_unreadable_before_after_output_buffers_checked':True,'candidate_object_no_access_after_call_checked':True,'v2_native_writer_decoder_round_trip':True,'details':details,'exact_binding_round_trip':True,'bindings':bindings,'original_renderer_executed':False,'original_splice_manifest_executed':False,'binaries':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'fixture.exe',out/'part0.obj',out/'part1.obj',out/'part2.obj']},'logs':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(out.glob('*.log'))}}
    report.update(journal_version=synthetic_manifest['journal_version'],compact_cpu_journal=args.compact_cpu)
    if args.compact_cpu:
        report.update(v2_native_writer_decoder_round_trip=False,v1_native_writer_decoder_round_trip=True,
            formal_arguments_recorded=False,buffer_snapshots_recorded=False,
            null_unreadable_before_after_output_buffers_checked=False,
            opaque_output_and_no_access_callee_forwarding_checked=True)
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'success':True,'report':str(out/'report.json')}),flush=True)


if __name__=='__main__':main()
