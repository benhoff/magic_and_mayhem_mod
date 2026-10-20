#!/usr/bin/env python3
"""Repair omitted direct-word inputs in a frozen queue under strict adjacency checks."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def u(raw,at):return struct.unpack_from('<I',raw,at)[0]
def declaration():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='NP.world-word-entry-observation');paths=required_sources(b)
    for directory in ('renderer','assets','compat/legacy','protocols','runtime/scene'):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ('.h','.hpp','.c','.cpp','.S','.inc') or p.name=='CMakeLists.txt'))
    paths.update(['tools/test-world-frames.py','tests/world-frame-reference.cpp','tests/sprite-binary-reference.cpp'])
    return [dict(behavior=b['id'],contract_sha256=behavior_contract(b,{b['id']:b for b in r['builds']}))],{p:sha(ROOT/p) for p in sorted(paths)}
def recover(scene,world):
    if scene[:8]!=b'MNMSCNE1' or u(scene,8)!=1 or u(scene,12)!=64 or u(scene,16)!=len(scene) or u(scene,36)!=1 or u(scene,40)!=0x40209ca7:raise ValueError('Unsupported frozen scene')
    if world[:8]!=b'MNMWRLD1' or u(world,16)!=len(world) or u(world,40) or u(world,64)!=1 or u(scene,20)!=u(world,20) or (u(scene,56),u(scene,60))!=(u(world,24),u(world,28)):raise ValueError('Uncorrelated frozen World')
    rows=[struct.unpack_from('<8I',scene,64+i*32) for i in range(u(scene,24))];blobs={};at=u(scene,52)
    for i in range(u(scene,28)):
        token,size,indexed,reserved=struct.unpack_from('<4I',scene,at)
        if token!=i+1 or size<40 or indexed>1 or reserved or at+16+size>len(scene):raise ValueError('Invalid frozen blob')
        blobs[token]=(indexed,scene[at+16:at+16+size]);at+=16+size
    if at!=len(scene):raise ValueError('Trailing frozen scene bytes')
    records=[];keys={};at=80
    for i in range(u(world,36)):
        size=u(world,at);record=world[at:at+size];n=u(record,32)
        if len(record)!=size or size<64+n or (u(record,4)==0 and u(record,52)==8):raise ValueError('Invalid/already observed word trace')
        records.append(record)
        if n:keys.setdefault((u(record,36),record[64:64+n],u(record,8),u(record,12)),[]).append(i)
        at+=size
    if at!=len(world):raise ValueError('Trailing frozen World bytes')
    def matches(row):
        if row[7] or row[0] not in blobs:return []
        indexed,frame=blobs[row[0]];return keys.get((indexed,frame,row[2],row[3]),[])
    inserts={};proof=[]
    for i,row in enumerate(rows):
        if row[4]!=31 or row[7] or row[0] not in blobs or blobs[row[0]][0]:continue
        if row[6]!=0x8ad08ad0 or i==0 or i+1==len(rows):raise ValueError('Unsupported word row auxiliary state')
        previous,following=matches(rows[i-1]),matches(rows[i+1])
        if len(previous)!=1 or len(following)!=1 or following[0]!=previous[0]+1:raise ValueError('Word insertion lacks unique adjacent raster boundary')
        before,after=records[previous[0]],records[following[0]]
        if before[16:32]!=after[16:32]:raise ValueError('Word insertion has ambiguous clip state')
        frame=blobs[row[0]][1];width,height,ox,oy=struct.unpack_from('<4i',frame,4);x,y=struct.unpack('<2i',struct.pack('<2I',row[2],row[3]));left,top,right,bottom=struct.unpack_from('<4i',before,16)
        if not (left<=x-ox and x-ox+width<=right and top<=y-oy and y-oy+height<=bottom):raise ValueError('Frozen word body/clipping identity is ambiguous')
        record=bytearray(64);struct.pack_into('<4I',record,0,64+len(frame),0,row[2],row[3]);record[16:32]=before[16:32];struct.pack_into('<I',record,32,len(frame));struct.pack_into('<I',record,52,8);record.extend(frame)
        if following[0] in inserts:raise ValueError('Multiple word insertions share a boundary')
        inserts[following[0]]=bytes(record);proof.append(dict(scene_row=i,blob=row[0],insert_before_original_raster=following[0],previous_scene_row=i-1,next_scene_row=i+1,previous_raster=previous[0],next_raster=following[0],frame_sha256=hashlib.sha256(frame).hexdigest(),anchor=[x,y],clip=[left,top,right,bottom]))
    if not inserts:raise ValueError('No omitted frozen word rows')
    out=bytearray(world[:80]+b''.join(inserts.get(i,b'')+record for i,record in enumerate(records)));struct.pack_into('<I',out,16,len(out));struct.pack_into('<I',out,36,len(records)+len(inserts));return bytes(out),proof

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--capture',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--build-dir',type=Path,required=True);args=parser.parse_args();out=args.output.resolve();capture=args.capture.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):parser.error('Require fresh output under working/')
    out.mkdir(parents=True);claims,sources=declaration();report=dict(schema=1,success=False,claims=claims,sources=sources,original_sha256=BUILD,live_replacement=False,historical_capture_sources_refreshed=False,original_pixels_used_as_native_inputs=False,scope='Strict reconstruction of omitted direct-word rows from the already retained scene/World serializations and unique adjacent raster boundaries. Frozen historical selector identity remains unknown. Compare independent native/original zero-base pixels and diagnostic original-before replay with the retained after canvas; no native output seeding or new live observation.')
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,command):
        with (out/(label+'.log')).open('x') as f:subprocess.run([str(c) for c in command],cwd=ROOT,env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'},stdout=f,stderr=subprocess.STDOUT,check=True,timeout=600)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        exe=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(exe)!=BUILD:raise ValueError('Unsupported original')
        raw=capture/'capture';scene=raw/'scene-0001.bin';world=raw/'world-0001.bin';before=raw/'world-0001.before.565';after=raw/'world-0001.after.565';inputs={str(p.relative_to(ROOT)):sha(p) for p in [scene,world,before,after,capture/'report.json']};wire,insertions=recover(scene.read_bytes(),world.read_bytes());fixed=out/'reconstructed.bin';fixed.write_bytes(wire)
        spec=importlib.util.spec_from_file_location('world',ROOT/'tools/test-world-frames.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        rows=module.records(wire);needed={module.identity(frame,h[9]) for h,frame in rows if h[1]<6};bindings=module.pinned_bindings(capture/'game',needed);(out/'bindings.json').write_text(json.dumps(bindings,indent=2)+'\n')
        run('configure',['cmake','-S',ROOT/'compat/legacy','-B',args.build_dir,'-DCMAKE_BUILD_TYPE=Debug']);run('build',['cmake','--build',args.build_dir,'--target','mnm-world-frame-preview','-j4'])
        run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/world-frame-reference.cpp','-o',out/'reference'])
        run('native',['xvfb-run','-a',args.build_dir/'mnm-world-frame-preview','--root',capture/'game','--bindings',out/'bindings.json','--snapshot',fixed,'--output',out/'native'])
        run('original-zero',[out/'reference',exe,fixed,out/'original-zero.565']);run('original-prior',[out/'reference',exe,fixed,out/'original-prior.565',before])
        native=(out/'native.565').read_bytes();zero=(out/'original-zero.565').read_bytes();prior=(out/'original-prior.565').read_bytes();live=after.read_bytes();width,height,stride=struct.unpack_from('<III',wire,24);live=b''.join(live[y*stride*2:(y*stride+width)*2] for y in range(height))
        if native!=zero or prior!=live:raise ValueError('Frozen word reconstruction does not close replay')
        if declaration()!=(claims,sources) or sha(exe)!=BUILD:raise ValueError('Declared source/contract changed')
        for binding in bindings:inputs[str((capture/'game'/binding['sprite']).relative_to(ROOT))]=binding['sha256']
        if any(sha(ROOT/path)!=value for path,value in inputs.items()):raise ValueError('Frozen input or bound asset changed')
        report.update(success=True,sources_stable=True,inputs_stable=True,insertions=insertions,inputs=inputs,pixels_compared=width*height,native_zero_matches_private_original=True,captured_before_replay_matches_live=True,native_zero_matches_live=native==live,draws=len(rows),native_sha256=hashlib.sha256(native).hexdigest(),live_sha256=hashlib.sha256(live).hexdigest())
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
