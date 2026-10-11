#!/usr/bin/env python3
"""Run real separate CALL observer/stubs against synthetic PE32 bodies."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_census import ROOT,BUILD_SHA256


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--prefix-template',type=Path,required=True);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    claims=json.loads(args.claims.read_text());sources=claims['sources']
    if any(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=digest for p,digest in sources.items()):raise ValueError('Prospective ABI sources changed')
    parent=ROOT/'working/tests/drawing-api-census-abi';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree,mshtml=d')
    def run(name,command,seconds=90):
        with (out/(name+'.log')).open('x') as log:subprocess.run([str(v) for v in command],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=seconds,check=True)
    spec=importlib.util.spec_from_file_location('abi_builder',ROOT/'tools/build-drawing-api-census.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
    imports=dict(builder.load('abi_imports','tools/build-shadow-bridge.py').IMPORTS,GetCurrentThreadId=0,GetTickCount=0,Sleep=4,CreateFileMappingA=24,MapViewOfFile=20,UnmapViewOfFile=4)
    (out/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
    run('imports',['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',out/'kernel32.def','-l',out/'kernel32.lib','--kill-at'])
    header=['#define API_HOOKS 10',builder.STRUCT,'static const u8 original_sha256[32]={'+','.join(str(v) for v in bytes.fromhex(BUILD_SHA256))+'};']
    for tag in range(10):header += [f'extern void api_before_{tag}(void);',f'extern void api_after_{tag}(void);']
    header += ['static struct ApiHook api_hooks[API_HOOKS]={']+[f'{{0,0,0,0,0,0,0,0,0,{{0}},{{0}},{{0}},api_before_{tag},api_after_{tag}}},' for tag in range(10)]+['};']
    (out/'api-hooks.inc').write_text('\n'.join(header)+'\n');(out/'entry.S').write_text((ROOT/'runtime/drawing-api-census/entry.S').read_text()+'\n'+'\n'.join(f'api_pair {tag}' for tag in range(10))+'\n')
    objects=[]
    for index,source in enumerate([ROOT/'tests/drawing-api-census-abi.c',ROOT/'tests/canvas-startup-call.S',out/'entry.S']):
        obj=out/(f'part{index}.obj');objects.append(obj);run(f'compile{index}',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I',out,'-c',source,'-o',obj])
    run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'fixture.exe'),*objects,out/'kernel32.lib'])
    run('prefix-copy',['cp','-a','--reflink=auto',args.prefix_template.resolve(),out/'wineprefix'])
    try:run('forwarding',['xvfb-run','-a','wine',out/'fixture.exe'])
    finally:run('stop-prefix',['wineserver','-k'],10)
    stable=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==digest for p,digest in sources.items())
    if not stable:raise ValueError('ABI inputs changed during execution')
    report={'schema':1,'scope':'Real separate CALL stubs/observer and dynamic splices against10synthetic PE32 bodies only. No original API/graphics or arbitrary machine-context/transparency/output/lifetime validation.','success':True,'sources':sources,'sources_stable':stable,'claims':claims['claims'],'callee_pop_bytes':[0,4,8,12,16,20,24],'call_modes':['absolute_memory','register','register_memory','direct_relative'],'synthetic_bodies':10,'register_flags_x87_sse_last_error_preserved':True,'pre_call_stack_and_target_checked':True,'negative_eax_actual_cleanup_checked':True,'short_conditional_remainder_executed':True,'frame_exhaustion_phase_record_cap_io_failure_forward':True,'same_thread_lifo_and_mismatch_loss_checked':True,'original_renderer_executed':False,'original_splice_manifest_executed':False,'binaries':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'fixture.exe',out/'part0.obj',out/'part1.obj',out/'part2.obj']},'logs':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(out.glob('*.log'))}}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'success':True,'report':str(out/'report.json')}),flush=True)


if __name__=='__main__':main()
