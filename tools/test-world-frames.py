#!/usr/bin/env python3
"""Compare whole native World canvases with isolated original raster execution and live output."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
BUILD_HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def reference_anchor(path):
    raw=path.read_bytes();pe=struct.unpack_from('<I',raw,60)[0]
    count=struct.unpack_from('<H',raw,pe+6)[0];optional=struct.unpack_from('<H',raw,pe+20)[0]
    rva=0x57e1b0-0x400000
    for i in range(count):
        size,va,stored,offset=struct.unpack_from('<4I',raw,pe+24+optional+i*40+8)
        if va<=rva and rva+8<=va+stored:
            anchor=raw[offset+rva-va:offset+rva-va+8].hex()
            if anchor!='83ec1453558bc156':raise ValueError('Original reference entry signature changed')
            return {'0x57e1b0':anchor}
    raise ValueError('Original reference entry is not file-backed')

def sources():
    paths=set()
    for directory in ['assets','renderer','compat/legacy','runtime/scene']:
        for path in (ROOT/directory).rglob('*'):
            if path.is_file() and (path.suffix in ['.cpp','.hpp','.c','.h','.S'] or path.name in ['CMakeLists.txt','README.md']):paths.add(path)
    paths.update(ROOT/p for p in ['protocols/include/mnm/world_frame_v1.h','protocols/include/mnm/scene_snapshot_v1.h','runtime/shadow/win32_min.h','tests/world-frame-test.cpp','tests/world-frame-reference.cpp','tests/sprite-binary-reference.cpp','tests/resource-fixtures.hpp','tools/test-world-frames.py','tools/capture-scene-game.py','tools/build-scene-observer.py','tools/prepare-scene-observer.py','tools/scene-game-runner.py','research/formats/world-frame-v1.md','research/runtime/native-world-frame-rendering.md'])
    return {str(path.relative_to(ROOT)):sha(path) for path in sorted(paths)}

def identity(raw,indexed):
    raw=bytearray(raw);raw[28:32]=bytes(4)
    return hashlib.sha256(bytes([indexed])+raw).hexdigest()

def records(raw):
    if len(raw)<80 or raw[:8]!=b'MNMWRLD1' or struct.unpack_from('<I',raw,16)[0]!=len(raw) or struct.unpack_from('<I',raw,40)[0]:raise ValueError('Refused/incomplete World capture')
    at=80;result=[]
    for _ in range(struct.unpack_from('<I',raw,36)[0]):
        header=struct.unpack_from('<16I',raw,at);size=header[0];n=header[8]
        if size<64+n or at+size>len(raw):raise ValueError('World record extent')
        result.append((header,raw[at+64:at+64+n]));at+=size
    if at!=len(raw):raise ValueError('Trailing World capture bytes')
    return result

def pinned_bindings(assetroot,needed):
    bindings=[];found=set()
    for path in sorted(assetroot.rglob('*')):
        if path.suffix.lower()!='.spr':continue
        raw=path.read_bytes()
        if len(raw)<24 or struct.unpack_from('<I',raw,8)[0]!=4:continue
        frames,palettes=struct.unpack_from('<2I',raw,12);table=24+palettes*768;base=table+frames*4;matched=set()
        for i in range(frames):
            at=base+struct.unpack_from('<I',raw,table+i*4)[0];size=struct.unpack_from('<I',raw,at)[0]
            key=identity(raw[at:at+size],int(bool(palettes)))
            if key in needed:matched.add(key)
        if matched:
            relative=path.relative_to(assetroot).as_posix();bindings.append({'id':'world/'+hashlib.sha256(relative.encode()).hexdigest(),'sprite':relative,'sha256':sha(path)});found|=matched
    if found!=needed:raise ValueError('Unmapped native World frames: '+str(len(needed-found)))
    return bindings

def encoded_frame(width,height,origin=(0,0),masked=False):
    header=bytearray(40+height*8);runs=bytearray();pixels=bytearray();rows=[]
    for y in range(height):
        rows.append((40+height*8+len(runs),len(pixels)))
        if masked:runs+=bytes([1,width-2,1]);pixels+=bytes((x+y*7)%256 for x in range(width-2))
        else:runs+=bytes([0,width]);pixels+=bytes((x+y*17)%256 for x in range(width))
    first=40+height*8+len(runs)
    for y,(delta,pixel) in enumerate(rows):struct.pack_into('<2I',header,40+y*8,delta,first+pixel)
    struct.pack_into('<5I',header,0,len(header)+len(runs)+len(pixels),width,height,origin[0]&0xffffffff,origin[1]&0xffffffff)
    return bytes(header+runs+pixels)

def wire_record(frame,op,x,y,clip,colours,backend=3,offsets=None,phase=0):
    payload=struct.pack('<16I',*offsets) if op==3 else struct.pack('<256H',*colours)
    period=16 if op==3 else 0
    return struct.pack('<16I',64+len(frame)+len(payload),op,x&0xffffffff,y&0xffffffff,*clip,len(frame),1,len(payload),8 if op==3 else 0,phase,backend,0,period)+frame+payload

def world_wire(draws,width=32,height=12):
    return b'MNMWRLD1'+struct.pack('<18I',1,80,80+sum(map(len,draws)),1,width,height,width,len(draws),0,0x40209ca7,0,0,width,height,1,1,0,0)+b''.join(draws)

def sprite(raw):
    return struct.pack('<6I',0x00525053,28+768+len(raw),4,1,1,0)+bytes(768)+bytes(4)+raw

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--capture-report',type=Path);parser.add_argument('--root',type=Path,default=ROOT/'working/game-clean');args=parser.parse_args()
    if not os.environ.get('DISPLAY'):parser.error('Run under xvfb-run -a')
    args.root=args.root.resolve()
    parent=ROOT/'working/tests/world-frames';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all')
    report={'success':False,'sources':sources(),'source_executable_sha256':BUILD_HASH,'frames':[],'fixtures':[],
        'scope':'Complete bounded native RGB565 World canvases from owned raster inputs/asset identities; independent isolated original pixels and captured live outputs; no continuous presentation or original bypass'}
    inputs={};build=ROOT/'working/build/world-frame';preview=build/'mnm-world-frame-preview';reference=build/'world-frame-reference'
    def run(name,command,expected=0):
        with (out/(name+'.log')).open('x') as log:result=subprocess.run([str(c) for c in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
        if result.returncode!=expected:raise RuntimeError(name+' exit '+str(result.returncode)+'; see '+str(out/(name+'.log')))
    def verify(phase):run('original-'+phase,[ROOT/'tools/original-manifest.sh','verify'])
    verify('before')
    try:
        executable=ROOT/'working/game-nocd/Chaos.exe'
        if sha(executable)!=BUILD_HASH:raise ValueError('Unsupported original executable')
        inputs[executable]=BUILD_HASH;report['entry_anchors']=reference_anchor(executable)
        run('configure',['cmake','-S',ROOT/'compat/legacy','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','mnm-world-frame-preview','world-frame-test','scene-snapshot-test','scene-renderer-test','resource-cache-test','-j4'])
        run('ctest',['ctest','--test-dir',build,'-R','^(native-world-frame|native-scene-snapshot|native-shared-scene-renderer|native-resource-cache)$','--output-on-failure'])
        run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/world-frame-reference.cpp','-o',reference])
        def compare(name,path,root,bindings,live=None):
            prefix=out/name;inputs[path]=sha(path);inputs[bindings]=sha(bindings)
            run(name+'-native',[preview,'--root',root,'--snapshot',path,'--bindings',bindings,'--output',prefix])
            metadata=json.loads(prefix.with_suffix('.json').read_text());actual=prefix.with_suffix('.565').read_bytes()
            original=out/(name+'-original.565');run(name+'-original',[reference,executable,path,original])
            independent=original.read_bytes()
            if actual!=independent:raise RuntimeError(name+' native differs from independent original raster execution by '+str(sum(a!=b for a,b in zip(struct.unpack('<'+'H'*(len(actual)//2),actual),struct.unpack('<'+'H'*(len(independent)//2),independent)))))
            result={'name':name,'width':metadata['width'],'height':metadata['height'],'draws':metadata['records'],'operations':metadata['operations'],'pixels':len(actual)//2,'native_sha256':sha(prefix.with_suffix('.565')),'original_sha256':sha(original),'independent_original_pixels_match':True,'native_readbacks':metadata['native_readbacks'],'driver':metadata['driver']}
            if live:
                inputs[live]=sha(live);raw=path.read_bytes();stride=struct.unpack_from('<I',raw,32)[0];width=metadata['width'];height=metadata['height'];oracle=live.read_bytes()
                if len(oracle)!=stride*height*2:raise ValueError('Live canvas extent')
                cropped=b''.join(oracle[y*stride*2:(y*stride+width)*2] for y in range(height))
                if actual!=cropped:raise RuntimeError(name+' native differs from complete live original canvas by '+str(sum(a!=b for a,b in zip(struct.unpack('<'+'H'*(len(actual)//2),actual),struct.unpack('<'+'H'*(len(cropped)//2),cropped)))))
                result['live_original_pixels_match']=True;result['live_sha256']=sha(live)
            return result
        fixture=out/'assets';fixture.mkdir();base=encoded_frame(32,12);mask=encoded_frame(6,5,(1,-1),True)
        for name,raw in [('base',base),('mask',mask)]: (fixture/(name+'.spr')).write_bytes(sprite(raw))
        bindings=out/'fixture-bindings.json';bindings.write_text(json.dumps([{'id':name,'sprite':name+'.spr','sha256':sha(fixture/(name+'.spr'))} for name in ['base','mask']],indent=2)+'\n')
        colours=[(i*7919)&65535 for i in range(256)]
        # Every operation reads a coloured preceding native layer. Odd-sized
        # masks, opaque zero, negative origins, clips and displacement ordering.
        for op in range(6):
            for clip_index,clip in enumerate([(0,0,32,12),(6,0,14,12),(0,7,32,10)]):
                draw=wire_record(mask,op,5,3,clip,list(reversed(colours)),offsets=[abs(8-i) if clip_index==1 else i%3 for i in range(16)])
                path=out/('fixture-'+str(op)+'-'+str(clip_index)+'.bin');path.write_bytes(world_wire([wire_record(base,0,0,0,(0,0,32,12),colours),draw]))
                report['fixtures'].append(compare(path.stem,path,fixture,bindings))
        if args.capture_report:
            capture_path=args.capture_report.resolve();capture=json.loads(capture_path.read_text());inputs[capture_path]=sha(capture_path)
            if not capture['success'] or capture['source_executable_sha256']!=BUILD_HASH or not capture.get('world_frames'):raise ValueError('Missing successful full-input capture')
            for source,digest in capture['sources'].items():
                if sha(ROOT/source)!=digest:raise ValueError('Stale capture source '+source)
            directory=Path(capture['experiment']);paths=sorted(directory/p for p in capture['world_frames'] if p.endswith('.bin'))
            for relative,digest in capture['world_frames'].items():
                if sha(directory/relative)!=digest:raise ValueError('Captured World input/oracle changed: '+relative)
            needed=set()
            for path in paths:
                if sha(path)!=capture['world_frames'][str(path.relative_to(directory))]:raise ValueError('World capture changed')
                needed.update(identity(raw,h[9]) for h,raw in records(path.read_bytes()))
            bindings=out/'live-bindings.json';data=pinned_bindings(args.root,needed);bindings.write_text(json.dumps(data,indent=2)+'\n')
            for binding in data:inputs[args.root/binding['sprite']]=binding['sha256']
            for path in paths:report['frames'].append(compare(path.stem,path,args.root,bindings,path.with_suffix('.after.565')))
            report.update(capture_report=str(capture_path.relative_to(ROOT)),captured_frame_identities=len(needed),native_asset_files=len(data),live_original_pixels_match=True)
        if sources()!=report['sources']:raise RuntimeError('Validation sources changed')
        if any(sha(path)!=digest for path,digest in inputs.items()):raise RuntimeError('Validation inputs changed')
        report.update(sources_stable=True,ctests_passed=4,independent_original_pixels_match=True,original_work_bypassed=False,original_pixels_used_as_native_inputs=False,success=True)
        report['inputs']={str(path.relative_to(ROOT)):digest for path,digest in inputs.items()}
        report['binaries']={str(path.relative_to(ROOT)):sha(path) for path in [preview,reference]}
    except Exception as error:report['error']=str(error)
    finally:
        verify('after');report['original_manifest_verified_before_after']=True
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'report.json',flush=True)
    if not report['success']:raise RuntimeError(report['error'])
if __name__=='__main__':main()
