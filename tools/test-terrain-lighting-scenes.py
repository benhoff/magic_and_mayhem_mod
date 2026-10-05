#!/usr/bin/env python3
"""Controlled object cycles through Qt terrain scenes vs original fields/queues/draws."""
import argparse
import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result

def fixture(width,height,layers):
    first=[[0]*8 for _ in range(8)];second=copy.deepcopy(first)
    first[1][0]=255;second[1][0]=1;first[2][0]=1
    text=['MNM_LIGHTING 1','relations',' '.join(str(v) for table in [first,second] for row in table for v in row)]
    for tick in range(12):
        x,y=(width//2+tick%3)%width,(height//2+tick%2)%height
        creatures=[(1,1,0,x,y,0),(1,1,1,0,0,layers-1),(1,1,2,width-1,height-1,0)]
        objects=[(1,32,x,y,0,x,y),(1,8,0,0,layers-1,0,0),(0,33,width-1,height-1,0,0,0)]
        if tick==6:creatures=[]
        if tick==7:objects=[]
        view=(width//2,height//2,min(32,width,height)) if tick%3 else (0,0,min(32,width,height))
        changed=1 if tick in [0,8] else 2 if tick==3 else 0
        text.append('tick '+' '.join(map(str,[1 if tick==5 else 0,changed,int(tick!=4),int(tick!=2),*view,len(creatures)+2,len(objects),len(creatures),len(objects)])))
        text.extend('creature '+' '.join(map(str,c)) for c in creatures)
        text.extend('object '+' '.join(map(str,o)) for o in objects)
    text.append('end');return '\n'.join(text)+'\n'

def main():
    if '--inner' not in sys.argv:
        subprocess.run(['xvfb-run','-a',sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=1200);return
    sys.argv.remove('--inner');parser=argparse.ArgumentParser()
    parser.add_argument('--preview',type=Path,required=True);parser.add_argument('--sanitized-preview',type=Path,required=True)
    parser.add_argument('--source-root',type=Path,default=ROOT);args=parser.parse_args();source=args.source_root.resolve()
    parent=ROOT/'working/tests/terrain-lighting-scenes';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,name,env=None):
        result=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=120,env=env)
        (out/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode();return result.stdout
    def verify(phase):run([ROOT/'tools/original-manifest.sh','verify'],'original-'+phase)
    verify('before')
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe';root=ROOT/'working/game-clean'
        if sha(pe)!=HASH:raise ValueError('Executable hash mismatch')
        binaries=[args.preview.resolve(),args.sanitized_preview.resolve()]
        inputs={p:sha(p) for p in [pe,*binaries,Path(__file__),root/'CFG/Encrypted/chaos.cfg',root/'CFG/prefs.cfg']}
        for folder in ['assets','renderer','renderer/sprites','reconstruction/rendering','apps/terrain-preview','tests']:
            for p in (source/folder).glob('*'):
                if p.is_file():inputs[p]=sha(p)
        for file in ['tools/decode-cfg.py','tools/test-terrain-map.py']:inputs[source/file]=sha(source/file)
        model=source/'reconstruction/rendering';field=out/'field';world=out/'world';palette=out/'palette'
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-I'+str(model),'-I'+str(source/'apps/terrain-preview'),source/'tests/terrain-lighting-scene-reference.cpp',source/'apps/terrain-preview/light_fixture.cpp',*[model/p for p in ['terrain_lighting.cpp','terrain_creature_lighting.cpp','terrain_static_lighting.cpp','terrain_lighting_cycle.cpp']],'-o',field],'compile-field')
        for binary,test,extra in [(world,'world-terrain-reference.cpp',[]),(palette,'palette-shading-reference.cpp',[model/'palette_shading.cpp'])]:
            run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-I'+str(model),source/'tests'/test,*extra,'-o',binary],'compile-'+binary.name)
        codec=module('codec',source/'tools/decode-cfg.py');geometry=module('geometry',source/'tools/test-terrain-map.py')
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
        frames=[];ticks=0;field_bytes=0;fixture_files={};refusals=[]
        selected=[0,1,3,4,5,7,8,11]
        for realm in ['Celtic','Greek','Medieval']:
            directory=next(d for d in sorted((root/'Realms'/realm).iterdir()) if d.is_dir() and (d/'Terrain.ttd').exists() and list(d.glob('*.map')))
            path=sorted(directory.glob('*.map'))[0];ttd=directory/'Terrain.ttd';spr=directory/'Terrain.spr'
            for p in [path,ttd,spr]:inputs[p]=sha(p)
            raw=codec.decode(path.read_bytes())[1];w,h,l=struct.unpack_from('<3I',raw,4)
            final=geometry.geometry(raw,ttd.read_bytes())[1];payload=out/(realm+'.map');payload.write_bytes(final)
            snapshots=out/(realm+'.lighting');snapshots.write_text(fixture(w,h,l));fixture_files[realm]={'path':str(snapshots.relative_to(ROOT)),'sha256':sha(snapshots)}
            original_prefix=out/(realm+'-field');expected=json.loads(run([field,pe,w,h,l,-50,7,snapshots,original_prefix],'field-'+realm))
            ticks+=expected['ticks'];field_bytes+=expected['ticks']*expected['field_bytes']*5
            for tick,state in enumerate(expected['trace']):
                b=(out/f'{realm}-field-{tick}.buffers').read_bytes();n=expected['field_bytes']
                state['buffer_sha256']=[hashlib.sha256(b[i*n:(i+1)*n]).hexdigest() for i in range(5)]
            for tick in selected:
                field_file=out/f'{realm}-field-{tick}.field';field_raw=field_file.read_bytes()
                for view in range(4):
                    hashes=[]
                    for build,binary in enumerate(binaries):
                        name=f'frame-{realm}-{tick}-{view}-{build}';prefix=out/name
                        base=[binary,'--root',root,'--realm',str(directory.relative_to(root)),'--map',str(path.relative_to(root)),'--world','--initialize-terrain','--recovered-camera','--view',view,'--palette-shading','--preferences','cfg\\PREFS.CFG','--lighting-config','CFG/Encrypted/chaos.cfg','--terrain-lighting']
                        run([*base,'--light-fixture',snapshots,'--light-tick',tick,'--output',prefix],name,env)
                        native=json.loads(prefix.with_suffix('.json').read_text());info=native['map'];lighting=info['terrain_light_field']
                        assert lighting['trace']==expected['trace'][:tick+1] and lighting['sha256']==sha(field_file) and lighting['fixture_sha256']==sha(snapshots) and lighting['selected_tick']==tick and lighting['fixture_ticks']==12 and lighting['publication']=='recovered creature then static cycle' and native['remaining_surfaces']==0
                        assert info['cells_sha256']==hashlib.sha256(final[76:]).hexdigest()
                        for tile in native['tiles']:assert tile['light']==struct.unpack_from('<b',field_raw,(tile['level']//2*h+tile['row'])*w+tile['column'])[0]
                        original=json.loads(run([world,pe,ttd,spr,payload,*info['camera'],*info['pan'],0,view,*info['origin_fields'],field_file],'world-'+name))
                        queue=[]
                        for item in native['queue']:
                            q={k:v for k,v in item.items() if k not in ['tile','role']};q['cell']=native['tiles'][item['tile']]['cell'];queue.append(q)
                        assert queue==original['queue'],name
                        qfile=out/(name+'.queue');qfile.write_bytes(b''.join(struct.pack('<Iiii',d['frame'],d['x'],d['y'],d['shade']) for d in original['queue'] if d['kind']!=-2))
                        pixels=out/(name+'-original.565');run([palette,pe,spr,256,qfile,pixels,50,50,2.2,2.2],'pixels-'+name)
                        assert pixels.read_bytes()==prefix.with_suffix('.565').read_bytes(),name
                        hashes.append(sha(pixels))
                    assert hashes[0]==hashes[1];frames.append({'realm':realm,'tick':tick,'view':view,'field_sha256':sha(field_file),'pixels_sha256':hashes[0]})
            print('matched',realm,'object-driven scenes',flush=True)
        # Default final-tick selection must produce the same pixels as tick 11.
        for build,binary in enumerate(binaries):
            name=f'default-last-{build}';prefix=out/name
            run([binary,*base[1:],'--light-fixture',snapshots,'--output',prefix],name,env)
            assert prefix.with_suffix('.565').read_bytes()==(out/f'frame-Medieval-11-3-{build}.565').read_bytes()
        # Refusals occur before output creation, including semantic field bounds.
        variants=[('bad-version',snapshots.read_text().replace('MNM_LIGHTING 1','MNM_LIGHTING 2')),('truncated','MNM_LIGHTING 1'),('bad-owner',snapshots.read_text().replace('creature 1 1 0','creature 1 1 -1',1)),('out-of-field',snapshots.read_text().replace('creature 1 1 0','creature 1 1 0 127 127 31\ncreature 1 1 0',1)),('trailing',snapshots.read_text()+'extra'),('oversized',' '* (1024*1024+1))]
        # Independent invalid-position snapshot keeps token structure valid.
        text=snapshots.read_text().splitlines();at=next(i for i,line in enumerate(text) if line.startswith('creature '));parts=text[at].split();parts[-1]='31';text[at]=' '.join(parts);variants[3]=('out-of-field','\n'.join(text)+'\n')
        for build,binary in enumerate(binaries):
            options=[['--light-tick','0'],['--light-fixture',snapshots,'--light-source','0,0,0,17'],['--light-fixture',snapshots,'--light-tick','12'],['--light-fixture',snapshots,'--light-tick','-1'],['--light-fixture',out/'missing']]
            for label,text in variants:
                p=out/(label+'.lighting');p.write_text(text);options.append(['--light-fixture',p])
            for i,extra in enumerate(options):
                prefix=out/f'refused-{build}-{i}';r=subprocess.run(list(map(str,[binary,*base[1:],*extra,'--output',prefix])),capture_output=True,text=True,env=env,timeout=30)
                assert r.returncode==1 and 'Sanitizer' not in r.stderr and not any(prefix.with_suffix(s).exists() for s in ['.565','.png','.json'])
                refusals.append(r.stderr.splitlines()[-1])
        assert all(sha(p)==digest for p,digest in inputs.items())
        report={'all_match':True,'live_validated':False,'executable_sha256':HASH,'original_ticks':ticks,'five_buffer_byte_comparisons':field_bytes,'native_frames':2*len(frames),'default_final_tick_frames':2,'refusals':refusals,'frames':frames,'fixtures':fixture_files,'source_and_input_sha256':{str(p.relative_to(ROOT)):digest for p,digest in inputs.items()},'helper_sha256':{p.name:sha(p) for p in [field,world,palette]},'boundary':'Owned text snapshots and selected tick prefixes; original whole combined lighting updater, five guarded buffers and eleven state fields; original world traversal/sort queues and indexed draws across installed geometry, three realms/four views; no installed entity production, live clock or replacement'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print({'all_match':True,'native_frames':report['native_frames']},flush=True)
    finally:verify('after')
if __name__=='__main__':main()
