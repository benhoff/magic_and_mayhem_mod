#!/usr/bin/env python3
"""Generated whole-map assembly, original geometry/queues and offline scene pixels."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result

def assemble(recipe,maps,blocks,ttd):
    """Copy original-selected crops; independent Python rotation/projection."""
    side=maps[0][0][1]//maps[0][0][6];layers=max(v[0][3] for v in maps)
    w,h=recipe['columns']*side,recipe['rows']*side
    cells=[[0,65535,65535,65535,0,0] for _ in range(w*h*layers)];occupied=set()
    for b in blocks:
        v,raw=maps[b['source']];sx,sy,rotation=b['source_x'],b['source_y'],b['rotation'];tx,ty=b['column'],b['row']
        assert (tx,ty) not in occupied and sx+side<=v[1] and sy+side<=v[2];occupied.add((tx,ty))
        for z in range(v[3]):
            for y in range(side):
                for x in range(side):
                    c=list(struct.unpack_from('<6H',raw,76+12*((z*v[2]+sy+y)*v[1]+sx+x)));c[1:4]=[65535]*3;c[5]&=0xfff7
                    if rotation:
                        c[0]=struct.unpack_from('<H',ttd,16+356*c[0]+0x94-4*rotation)[0]
                        for _ in range(rotation):c[4]=(c[4]&0xc3ff)|((c[4]&0x1c00)<<1)|((c[4]&0x2000)>>3)
                    px,py=[(x,y),(side-1-y,x),(side-1-x,side-1-y),(y,side-1-x)][rotation]
                    cells[(z*h+ty*side+py)*w+tx*side+px]=c
    assert len(occupied)==recipe['columns']*recipe['rows']
    header=bytearray(76);struct.pack_into('<6I',header,0,6,w,h,layers,w*h,len(cells))
    return bytes(header)+b''.join(struct.pack('<6H',*c) for c in cells)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--source-root',type=Path,default=ROOT)
    parser.add_argument('--preview',type=Path,required=True);parser.add_argument('--sanitized',type=Path,required=True)
    args=parser.parse_args();source=args.source_root.resolve();root=ROOT/'working/game-clean'
    parent=ROOT/'working/tests/terrain-generated-scenes';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,name,**kwargs):
        result=subprocess.run(command,capture_output=True,text=True,timeout=240,**kwargs)
        (out/(name+'.stdout')).write_text(result.stdout);(out/(name+'.stderr')).write_text(result.stderr)
        if result.returncode:raise RuntimeError(name+' failed: '+result.stdout[-500:]+result.stderr[-1800:])
        return result.stdout
    def verify(phase):run([str(ROOT/'tools/original-manifest.sh'),'verify'],'original-'+phase)
    sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest();inputs={}
    def track(path):inputs[path]=sha(path)
    verify('before')
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe';assert sha(pe)==HASH
        builds=[args.preview.resolve(),args.sanitized.resolve()]
        for path in [pe,Path(__file__),*builds]:track(path)
        for folder in ['assets','renderer','renderer/sprites','reconstruction/rendering','apps/terrain-preview']:
            for path in (source/folder).glob('*'):
                if path.is_file():track(path)
        for name in ['tests/terrain-generated-scene-reference.cpp','tests/terrain-generated-reference.cpp','tests/world-terrain-reference.cpp','tests/terrain-map-reference.cpp','tools/test-terrain-region.py','tools/test-terrain-map.py','tools/test-terrain-preview.py','tools/decode-cfg.py']:track(source/name)
        if (source/'source-baseline.json').exists():track(source/'source-baseline.json')
        codec=module('generated_scene_codec',source/'tools/decode-cfg.py');recipes=module('generated_scene_recipes',source/'tools/test-terrain-region.py');geometry=module('generated_scene_geometry',source/'tools/test-terrain-map.py');sprites=module('generated_scene_sprites',source/'tools/test-terrain-preview.py')
        configs={};decode_cache={}
        for realm in ['Celtic','Greek','Medieval']:
            path=root/f'Realms/{realm}/{realm}.cfg';track(path);data=path.read_bytes();configs[realm]=recipes.recipe_rows(data if data.startswith(b';') else codec.decode(data)[1])
        include='-I'+str(source/'reconstruction/rendering');models=[str(source/'reconstruction/rendering'/name) for name in ['terrain_generated.cpp','terrain_catalog.cpp','terrain_generation.cpp','terrain_specific.cpp','terrain_solver.cpp','terrain_constraints.cpp','terrain_selection.cpp']]
        helper=out/'generation-reference';run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie',include,str(source/'tests/terrain-generated-scene-reference.cpp'),*models,'-o',str(helper)],'compile-generation')
        world=out/'world-reference';run(['g++','-m32','-std=c++17',include,str(source/'tests/world-terrain-reference.cpp'),'-o',str(world)],'compile-world')
        surface=out/'geometry-reference';run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie',include,str(source/'tests/terrain-map-reference.cpp'),str(source/'reconstruction/rendering/terrain_map.cpp'),'-o',str(surface)],'compile-geometry')
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0')
        selections=[('Celtic',1,0),('Celtic',1,123),('Greek',1,0),('Greek',1,123),('Medieval',16,0),('Medieval',16,123),('Celtic',6,None)]
        layouts=[];images=[];geometry_fixtures=[];first_source=None;first_realm=None
        for index,(realm,id,seed) in enumerate(selections):
            recipe=next(r for r in configs[realm] if r['id']==id);entries=[list(q)+[1] for q in recipe['specific']]+[[q[0],-1,-1,-1,q[1]] for q in recipe['random']]
            maps=[];paths=[];fixture=struct.pack('<6I',1,id,recipe['columns'],recipe['rows'],len(recipe['specific']),len(recipe['random']))
            for entry in entries:
                requested=recipe['path']+'/'+recipe['prefix']+f'{entry[0]:02}.map';path=root/requested.replace('\\','/');track(path);paths.append(requested)
                if path not in decode_cache:decode_cache[path]=codec.decode(path.read_bytes())[1]
                raw=decode_cache[path];maps.append((struct.unpack_from('<19I',raw),raw));fixture+=struct.pack('<5i',*entry)+raw[:76]
                if first_source is None and maps[-1][0][6:8]==(1,1):first_source=path
            fp=out/f'recipe-{index}.bin';fp.write_bytes(fixture)
            original=json.loads(run([str(helper),str(pe),str(fp),str(seed or 0)],f'layout-{index}'));assert original['complete']
            directory=root/recipe['sprite_path'].replace('\\','/');ttd=directory/'Terrain.ttd';spr=directory/'Terrain.spr';track(ttd);track(spr)
            if first_realm is None:first_realm=directory
            raw=assemble(recipe,maps,original['blocks'],ttd.read_bytes());base,final=geometry.geometry(raw,ttd.read_bytes())
            payloads=[out/f'map-{index}-{kind}.bin' for kind in ['raw','base','final']]
            for path,data in zip(payloads,[raw,base,final]):path.write_bytes(data)
            geometry_fixtures.extend([str(ttd),*map(str,payloads)])
            w,h,layers=struct.unpack_from('<3I',final,4);camera=[w//2,h//2,min(20,w,h),layers];asset=dict(sprite=spr.read_bytes(),frames={})
            expected_blocks=[{**{k:v for k,v in b.items() if k!='source'},'path':paths[b['source']]} for b in original['blocks']]
            source_cells=[c for _,raw_map in maps for c in struct.iter_unpack('<6H',raw_map[76:])]
            projected_objects=sum(bool(c[5]&8) for c in source_cells);projected_references=sum(ref!=65535 for c in source_cells for ref in c[1:4])
            layouts.append(dict(realm=realm,region=id,seed=seed,dimensions=[w,h,layers],source_layers=sorted({v[0][3] for v in maps}),multi_block_sources=sum(v[0][6]*v[0][7]>1 for v in maps),blocks=len(expected_blocks),original=original,initialized_cells_sha256=hashlib.sha256(final[76:]).hexdigest()))
            for view in range(4):
                for visibility in [False,True]:
                    number=len(images);expected=json.loads(run([str(world),str(pe),str(ttd),str(spr),str(payloads[2]),*map(str,camera),'256','64',str(int(visibility)),str(view)],f'world-{number}'))
                    pixels=[0x2124]*(512*256)
                    for draw in expected['queue']:
                        if draw['kind']==-2:continue
                        ox,oy,values=sprites.frame(asset,draw['frame'])
                        for dx,dy,value in values:
                            x,y=draw['x']-ox+dx,draw['y']-oy+dy
                            if 0<=x<512 and 0<=y<256:pixels[y*512+x]=value
                    words=struct.pack('<'+'H'*len(pixels),*pixels);rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in pixels)
                    for build,binary in enumerate(builds):
                        prefix=out/f'frame-{number}-{build}';command=[str(binary),'--root',str(root),'--region-config',f'realms\\{realm.upper()}\\{realm.lower()}.CFG','--region-id',str(id),'--world','--initialize-terrain','--view',str(view),'--output',str(prefix)]
                        if seed is not None:command+=['--generation-seed',str(seed)]
                        if visibility:command+=['--visibility']
                        run(command,f'native-{number}-{build}',env=env);actual=json.loads(prefix.with_suffix('.json').read_text());queue=[]
                        for draw in actual['queue']:
                            d={k:v for k,v in draw.items() if k not in ['tile','role']};d['cell']=actual['tiles'][draw['tile']]['cell'];queue.append(d)
                        info=actual['map']['recipe'];assert sorted(info['blocks'],key=lambda b:(b['row'],b['column']))==expected_blocks
                        assert info['projected_source_objects']==projected_objects and info['projected_source_references']==projected_references
                        assert actual['map']['cells_sha256']==hashlib.sha256(final[76:]).hexdigest() and [actual['map'][k] for k in ['width','height','layers']]==[w,h,layers]
                        if seed is not None:
                            gen=info['generation'];assert gen['seed']==seed and gen['next_seed']==original['next_seed'] and gen['complete'] and len(gen['attempts'])==original['attempts']
                        else:assert 'generation' not in info
                        assert queue==expected['queue'] and actual['remaining_surfaces']==0 and prefix.with_suffix('.565').read_bytes()==words and actual['rgba_sha256']==hashlib.sha256(rgba).hexdigest()
                        for tile,owner in zip(actual['tiles'],actual['owners']):
                            c=struct.unpack_from('<6H',final,76+12*tile['cell']);assert (tile['definition'],tile['flags8'],tile['flags10'])==(c[0],c[4],c[5]);assert owner==expected['owners'][tile['cell']]
                        images.append(dict(realm=realm,region=id,seed=seed,view=view,visibility=visibility,build=build,draws=len(queue),hidden=sum(d['kind']==-2 for d in queue),sha256=sha(prefix.with_suffix('.565'))))
            print(realm,id,seed,'complete scene matches',flush=True)
        geometry_result=json.loads(run([str(surface),str(pe),*geometry_fixtures],'geometry'))
        # A complete but unsatisfiable one-source recipe verifies that exhaustion
        # stops before geometry/rendering and writes no output files.
        fixture_root=out/'exhausted-assets';fixture_root.mkdir();assert first_source and first_realm
        for name in ['Terrain.ttd','Terrain.spr']:shutil.copy2(first_realm/name,fixture_root/name)
        shutil.copy2(first_source,fixture_root/'Part01.map')
        (fixture_root/'Realm.cfg').write_text('[REGION0]\nName=Exhausted\nPath=.\nSpritePath=.\nSectionPrefix=Part\nMapSize=(5,5)\nSpecific=\nRandom=(1_1)\n')
        rejections=[];cases=[('seed-without-config',['--generation-seed','0']),('negative-seed',['--region-config','Realms/Celtic/Celtic.cfg','--region-id','1','--generation-seed','-1']),('overflow-seed',['--region-config','Realms/Celtic/Celtic.cfg','--region-id','1','--generation-seed','4294967296']),('missing-map',['--region-config','Realms/Medieval/Medieval.cfg','--region-id','0','--generation-seed','0'])]
        for build,binary in enumerate(builds):
            for name,flags in cases+[('exhausted',['--region-config','Realm.cfg','--region-id','0','--generation-seed','0'])]:
                prefix=out/f'rejected-{build}-{name}';selected_root=fixture_root if name=='exhausted' else root
                result=subprocess.run([str(binary),'--root',str(selected_root),'--world','--initialize-terrain','--output',str(prefix),*flags],capture_output=True,text=True,timeout=60,env=env)
                assert result.returncode==1 and 'Sanitizer' not in result.stderr and not any(prefix.with_suffix(s).exists() for s in ['.565','.png','.json'])
                if name=='exhausted':assert 'exhausted 10 attempts; next seed 5000' in result.stderr
                rejections.append(dict(build=build,case=name,error=result.stderr.splitlines()[-1]))
        assert all(sha(path)==digest for path,digest in inputs.items()),'Inputs changed'
        report=dict(all_match=True,live_validated=False,executable_sha256=HASH,layouts=layouts,images=images,rejections=rejections,geometry=geometry_result,artifacts=str(out.relative_to(ROOT)),source_and_input_sha256={str(p):v for p,v in inputs.items()},helper_sha256={p.name:sha(p) for p in [helper,world,surface]},scope='Original post-CFG assignments, independent whole-map ordinary projection/rotation, original geometry pass and traversal/queue/visibility, independent unshaded SPR pixels; native complete CFG/MAP loading and scene presentation. No original whole-scene renderer, entities, lighting, water or live replacement.')
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(dict(layouts=len(layouts),images=len(images),rejections=len(rejections),geometry=geometry_result),flush=True)
    finally:verify('after')
if __name__=='__main__':main()
