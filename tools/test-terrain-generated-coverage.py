#!/usr/bin/env python3
"""Bounded all-recipe terrain coverage at zero/wrapping seeds and corner cameras."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--source-root',type=Path,default=ROOT)
    parser.add_argument('--preview',type=Path,required=True);parser.add_argument('--sanitized',type=Path,required=True)
    parser.add_argument('--preview-report',type=Path,required=True,help='Validated source and binary provenance from the preceding scene chunk')
    args=parser.parse_args();source=args.source_root.resolve();root=ROOT/'working/game-clean'
    parent=ROOT/'working/tests/terrain-generated-coverage';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def execute(command,name,**kwargs):
        result=subprocess.run(command,capture_output=True,text=True,timeout=240,**kwargs)
        (out/(name+'.stdout')).write_text(result.stdout);(out/(name+'.stderr')).write_text(result.stderr);return result
    def run(command,name,**kwargs):
        result=execute(command,name,**kwargs)
        if result.returncode:raise RuntimeError(name+' failed: '+result.stdout[-500:]+result.stderr[-1800:])
        return result.stdout
    def verify(phase):run([str(ROOT/'tools/original-manifest.sh'),'verify'],'original-'+phase)
    sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest();inputs={}
    def track(path):inputs[path]=sha(path)
    verify('before')
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe';assert sha(pe)==HASH
        builds=[args.preview.resolve(),args.sanitized.resolve()];prior_path=args.preview_report.resolve();prior=json.loads(prior_path.read_text());assert prior['all_match'] and prior['executable_sha256']==HASH
        for binary in builds:assert sha(binary)==prior['source_and_input_sha256'][str(binary.relative_to(ROOT))]
        # The unchanged production binaries are reused only when all corresponding
        # C++/header/CMake inputs match their previously validated frozen sources.
        prior_sources=prior['source_and_input_sha256'];matched_sources={}
        for path,digest in prior_sources.items():
            prefix='working/tests/terrain-generated-scenes-source-'
            if path.startswith(prefix):
                name=path.split('/',3)[3]
                if Path(name).suffix in ['.cpp','.hpp','.h'] or Path(name).name=='CMakeLists.txt':
                    assert sha(source/name)==digest,name;matched_sources[name]=digest
        assert 'apps/terrain-preview/main.cpp' in matched_sources
        for path in [pe,Path(__file__),prior_path,*builds]:track(path)
        for folder in ['assets','renderer','renderer/sprites','reconstruction/rendering','apps/terrain-preview']:
            for path in (source/folder).glob('*'):
                if path.is_file():track(path)
        for name in ['tests/terrain-generated-scene-reference.cpp','tests/terrain-generated-reference.cpp','tests/world-terrain-reference.cpp','tests/terrain-map-reference.cpp','tools/test-terrain-generated-scenes.py','tools/test-terrain-region.py','tools/test-terrain-map.py','tools/test-terrain-preview.py','tools/decode-cfg.py']:track(source/name)
        if (source/'source-baseline.json').exists():track(source/'source-baseline.json')
        codec=module('coverage_codec',source/'tools/decode-cfg.py');recipes=module('coverage_recipes',source/'tools/test-terrain-region.py');geometry=module('coverage_geometry',source/'tools/test-terrain-map.py');sprites=module('coverage_sprites',source/'tools/test-terrain-preview.py');scene=module('coverage_assembly',source/'tools/test-terrain-generated-scenes.py')
        configs={};decode_cache={}
        for realm in ['Celtic','Greek','Medieval']:
            path=root/f'Realms/{realm}/{realm}.cfg';track(path);data=path.read_bytes();configs[realm]=recipes.recipe_rows(data if data.startswith(b';') else codec.decode(data)[1])
        include='-I'+str(source/'reconstruction/rendering');models=[str(source/'reconstruction/rendering'/name) for name in ['terrain_generated.cpp','terrain_catalog.cpp','terrain_generation.cpp','terrain_specific.cpp','terrain_solver.cpp','terrain_constraints.cpp','terrain_selection.cpp']]
        helper=out/'generation-reference';run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie',include,str(source/'tests/terrain-generated-scene-reference.cpp'),*models,'-o',str(helper)],'compile-generation')
        world=out/'world-reference';run(['g++','-m32','-std=c++17',include,str(source/'tests/world-terrain-reference.cpp'),'-o',str(world)],'compile-world')
        surface=out/'geometry-reference';run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie',include,str(source/'tests/terrain-map-reference.cpp'),str(source/'reconstruction/rendering/terrain_map.cpp'),'-o',str(surface)],'compile-geometry')
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0')
        layouts=[];images=[];failures=[];camera_refusals=[];missing=[];geometry_fixtures=[];selections=[]
        for realm_index,(realm,rows) in enumerate(configs.items()):
            for recipe in rows:
                for seed_index,seed in enumerate([0,0xffffffff]):selections.append((realm_index,realm,recipe,seed_index,seed))
        assert len(selections)==80
        def command(binary,realm,id,seed,prefix):
            return [str(binary),'--root',str(root),'--region-config',f'realms\\{realm.upper()}\\{realm.lower()}.CFG','--region-id',str(id),'--generation-seed',str(seed),'--world','--initialize-terrain','--output',str(prefix)]
        def check_failure(result,prefix,expected=None):
            assert result.returncode==1 and 'Sanitizer' not in result.stderr and not any(prefix.with_suffix(s).exists() for s in ['.565','.png','.json'])
            error=result.stderr.splitlines()[-1]
            if expected is not None:assert expected in error,(error,expected)
            return error
        for index,(realm_index,realm,recipe,seed_index,seed) in enumerate(selections):
            id=recipe['id'];entries=[list(q)+[1] for q in recipe['specific']]+[[q[0],-1,-1,-1,q[1]] for q in recipe['random']]
            paths=[recipe['path']+'/'+recipe['prefix']+f'{entry[0]:02}.map' for entry in entries]
            absent=[path for path in paths if not (root/path.replace('\\','/')).is_file()]
            if absent:
                errors=[]
                for build,binary in enumerate(builds):
                    prefix=out/f'unavailable-{index}-{build}';errors.append(check_failure(execute(command(binary,realm,id,seed,prefix),f'unavailable-{index}-{build}',env=env),prefix,'Test'))
                assert errors[0]==errors[1];missing.append(dict(realm=realm,region=id,seed=seed,paths=absent,errors=errors,original_executed=False));print(realm,id,seed,'unavailable inputs refused',flush=True);continue
            maps=[];fixture=struct.pack('<6I',1,id,recipe['columns'],recipe['rows'],len(recipe['specific']),len(recipe['random']))
            for entry,requested in zip(entries,paths):
                path=root/requested.replace('\\','/');track(path)
                if path not in decode_cache:decode_cache[path]=codec.decode(path.read_bytes())[1]
                raw=decode_cache[path];maps.append((struct.unpack_from('<19I',raw),raw));fixture+=struct.pack('<5i',*entry)+raw[:76]
            fp=out/f'recipe-{index}.bin';fp.write_bytes(fixture)
            generation=execute([str(helper),str(pe),str(fp),str(seed)],f'layout-{index}')
            if generation.returncode:
                reason=generation.stderr.strip();assert reason in ['Region solver rollback before grid','Region fixed rollback before grid','Region placement missing connector','Region rollback missing connector','Region solver rollback bound'],reason
                errors=[]
                for build,binary in enumerate(builds):
                    prefix=out/f'guard-{index}-{build}';errors.append(check_failure(execute(command(binary,realm,id,seed,prefix),f'guard-{index}-{build}',env=env),prefix,reason))
                failures.append(dict(realm=realm,region=id,seed=seed,kind='native-generation-bound',errors=errors,original_executed=False));print(realm,id,seed,'native generation bound',flush=True);continue
            original=json.loads(generation.stdout)
            if not original['complete']:
                errors=[]
                for build,binary in enumerate(builds):
                    prefix=out/f'exhausted-{index}-{build}';errors.append(check_failure(execute(command(binary,realm,id,seed,prefix),f'exhausted-{index}-{build}',env=env),prefix,f"exhausted {original['attempts']} attempts; next seed {original['next_seed']}"))
                failures.append(dict(realm=realm,region=id,seed=seed,kind='exhausted',original=original,errors=errors,original_executed=True));print(realm,id,seed,'matching exhaustion',flush=True);continue
            directory=root/recipe['sprite_path'].replace('\\','/');ttd=directory/'Terrain.ttd';spr=directory/'Terrain.spr';track(ttd);track(spr)
            raw=scene.assemble(recipe,maps,original['blocks'],ttd.read_bytes());base,final=geometry.geometry(raw,ttd.read_bytes());cell_hash=hashlib.sha256(final[76:]).hexdigest()
            payloads=[out/f'map-{index}-{kind}.bin' for kind in ['raw','base','final']]
            for path,data in zip(payloads,[raw,base,final]):path.write_bytes(data)
            geometry_fixtures.extend([str(ttd),*map(str,payloads)])
            w,h,layers=struct.unpack_from('<3I',final,4);asset=dict(sprite=spr.read_bytes(),frames={})
            expected_blocks=[{**{k:v for k,v in b.items() if k!='source'},'path':paths[b['source']]} for b in original['blocks']]
            projected_objects=sum(bool(c[5]&8) for _,b in maps for c in struct.iter_unpack('<6H',b[76:]));projected_references=sum(ref!=65535 for _,b in maps for c in struct.iter_unpack('<6H',b[76:]) for ref in c[1:4])
            layouts.append(dict(realm=realm,region=id,seed=seed,dimensions=[w,h,layers],source_layers=sorted({v[0][3] for v in maps}),blocks=len(expected_blocks),original=original,initialized_cells_sha256=cell_hash))
            cases=[('northwest',[0,0,min(12,w,h),layers],[256,64],False),('southeast',[w-1,h-1,min(16,w,h),max(1,layers//2)],[-24,192],True)]
            for camera_index,(label,camera,pan,visibility) in enumerate(cases):
                view=(id+realm_index+seed_index+2*camera_index)%4;number=index*2+camera_index;actuals=[];prefixes=[];results=[]
                # Native validation happens before invoking unsafe original traversal.
                for build,binary in enumerate(builds):
                    prefix=out/f'frame-{number}-{build}';prefixes.append(prefix);cmd=command(binary,realm,id,seed,prefix)+['--view',str(view),'--camera',','.join(map(str,camera)),'--pan',','.join(map(str,pan))]
                    if visibility:cmd+=['--visibility']
                    results.append(execute(cmd,f'native-{number}-{build}',env=env))
                if any(result.returncode for result in results):
                    errors=[check_failure(result,prefix,'Rotated traversal exceeds owned grid/coordinate domain') for result,prefix in zip(results,prefixes)];assert errors[0]==errors[1]
                    camera_refusals.append(dict(realm=realm,region=id,seed=seed,camera_label=label,camera=camera,pan=pan,view=view,errors=errors,original_traversal_executed=False));continue
                for prefix in prefixes:actuals.append(json.loads(prefix.with_suffix('.json').read_text()))
                expected=json.loads(run([str(world),str(pe),str(ttd),str(spr),str(payloads[2]),*map(str,camera),*map(str,pan),str(int(visibility)),str(view)],f'world-{number}'))
                pixels=[0x2124]*(512*256)
                for draw in expected['queue']:
                    if draw['kind']==-2:continue
                    ox,oy,values=sprites.frame(asset,draw['frame'])
                    for dx,dy,value in values:
                        x,y=draw['x']-ox+dx,draw['y']-oy+dy
                        if 0<=x<512 and 0<=y<256:pixels[y*512+x]=value
                words=struct.pack('<'+'H'*len(pixels),*pixels);rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in pixels)
                for build,(actual,prefix) in enumerate(zip(actuals,prefixes)):
                    info=actual['map']['recipe'];assert sorted(info['blocks'],key=lambda b:(b['row'],b['column']))==expected_blocks
                    assert info['projected_source_objects']==projected_objects and info['projected_source_references']==projected_references
                    assert actual['map']['cells_sha256']==cell_hash and [actual['map'][k] for k in ['width','height','layers']]==[w,h,layers]
                    assert actual['map']['camera']==camera and actual['map']['pan']==pan
                    gen=info['generation'];assert gen['seed']==seed and gen['next_seed']==original['next_seed'] and gen['complete'] and len(gen['attempts'])==original['attempts']
                    queue=[]
                    for draw in actual['queue']:
                        d={k:v for k,v in draw.items() if k not in ['tile','role']};d['cell']=actual['tiles'][draw['tile']]['cell'];queue.append(d)
                    assert queue==expected['queue'] and actual['remaining_surfaces']==0 and prefix.with_suffix('.565').read_bytes()==words and actual['rgba_sha256']==hashlib.sha256(rgba).hexdigest()
                    assert len(actual['tiles'])==len(actual['owners'])
                    for tile,owner in zip(actual['tiles'],actual['owners']):
                        c=struct.unpack_from('<6H',final,76+12*tile['cell']);assert (tile['definition'],tile['flags8'],tile['flags10'])==(c[0],c[4],c[5]);assert owner==expected['owners'][tile['cell']]
                    images.append(dict(realm=realm,region=id,seed=seed,camera_label=label,camera=camera,pan=pan,view=view,visibility=visibility,build=build,tiles=len(actual['tiles']),draws=len(queue),hidden=sum(d['kind']==-2 for d in queue),sha256=sha(prefix.with_suffix('.565'))))
            print(realm,id,seed,'layout/scene checks complete',flush=True)
        geometry_result=json.loads(run([str(surface),str(pe),*geometry_fixtures],'geometry'))
        assert len(layouts)+len(failures)+len(missing)==len(selections)
        assert images and all(sha(path)==digest for path,digest in inputs.items()),'Inputs changed'
        report=dict(all_match=True,live_validated=False,executable_sha256=HASH,seeds=[0,0xffffffff],selected_recipe_count=sum(map(len,configs.values())),layouts=layouts,images=images,failures=failures,camera_refusals=camera_refusals,unavailable=missing,geometry=geometry_result,artifacts=str(out.relative_to(ROOT)),validated_preview_report=str(prior_path.relative_to(ROOT)),matched_preview_sources=matched_sources,source_and_input_sha256={str(p):v for p,v in inputs.items()},helper_sha256={p.name:sha(p) for p in [helper,world,surface]},scope='All shipped recipes at two seeds; independent ordinary whole-map assembly and original geometry; two bounded corner cameras, selected views/visibility and independent unshaded pixels. Native refusals are explicit and excluded from original execution; missing-file recovery, full scene rendering, entities, lighting, water and live replacement remain separate.')
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(dict(layouts=len(layouts),images=len(images),failures=len(failures),camera_refusals=len(camera_refusals),unavailable=len(missing),geometry=geometry_result),flush=True)
    finally:verify('after')
if __name__=='__main__':main()
