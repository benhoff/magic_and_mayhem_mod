#!/usr/bin/env python3
"""Authored region recipes, original block placement, and native scene validation."""
import argparse, configparser, hashlib, importlib.util, json, os, re, struct, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def recipe_rows(data):
    cfg=configparser.ConfigParser(interpolation=None,strict=False,inline_comment_prefixes=(';',));cfg.read_string(data.decode('latin1'))
    rows=[]
    for name in cfg.sections():
        if not re.fullmatch(r'REGION\d+',name,re.I):continue
        f=cfg[name];w,h=map(int,re.fullmatch(r'\(\s*(\d+)\s*,\s*(\d+)\s*\)',f['mapsize']).groups())
        specific=[list(map(int,row)) for row in re.findall(r'\(\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\)',f['specific'])]
        random=[list(map(int,row)) for row in re.findall(r'(\d+)\s*_\s*(\d+)',f.get('random',''))]
        rows.append(dict(id=int(name[6:]),name=f['name'],path=f['path'],sprite_path=f.get('spritepath',f['path']),prefix=f['sectionprefix'],columns=w,rows=h,specific=specific,random=random))
    return sorted(rows,key=lambda row:row['id'])

def assemble(recipe,maps,ttd):
    """Independent anchor-relative rotation and mixed-height ordinary projection."""
    columns,rows=recipe['columns'],recipe['rows'];side=maps[0][0][1]//maps[0][0][6];layers=max(v[0][3] for v in maps)
    width,height=columns*side,rows*side;cells=[[0,65535,65535,65535,0,0] for _ in range(width*height*layers)]
    blocks=[];occupied=set()
    for source,((v,raw),placement) in enumerate(zip(maps,recipe['specific'])):
        section,rotation,col,row=placement
        for by in range(v[7]):
            for bx in range(v[6]):
                dx,dy=[(bx,by),(-by,bx),(-bx,-by),(by,-bx)][rotation];tx,ty=(col+dx)%columns,(row+dy)%rows
                assert (tx,ty) not in occupied;occupied.add((tx,ty))
                path=recipe['path']+'/'+recipe['prefix']+f'{section:02}.map'
                blocks.append(dict(path=path,source_x=bx*side,source_y=by*side,column=tx,row=ty,rotation=rotation))
                for z in range(v[3]):
                    for y in range(side):
                        for x in range(side):
                            c=list(struct.unpack_from('<6H',raw,76+12*((z*v[2]+by*side+y)*v[1]+bx*side+x)))
                            c[1:4]=[65535]*3;c[5]&=0xfff7
                            if rotation:
                                c[0]=struct.unpack_from('<H',ttd,16+356*c[0]+0x94-4*rotation)[0]
                                for _ in range(rotation):c[4]=(c[4]&0xc3ff)|((c[4]&0x1c00)<<1)|((c[4]&0x2000)>>3)
                            px,py=[(x,y),(side-1-y,x),(side-1-x,side-1-y),(y,side-1-x)][rotation]
                            cells[(z*height+ty*side+py)*width+tx*side+px]=c
    assert len(occupied)==columns*rows
    header=bytearray(76);struct.pack_into('<6I',header,0,6,width,height,layers,width*height,len(cells))
    return bytes(header)+b''.join(struct.pack('<6H',*c) for c in cells),blocks

def main():
    p=argparse.ArgumentParser();p.add_argument('--source-root',type=Path,default=ROOT)
    for name in ['preview','sanitized','inspect','inspect-sanitized']:p.add_argument('--'+name,type=Path,required=True)
    args=p.parse_args();source=args.source_root.resolve();root=ROOT/'working/game-clean'
    parent=ROOT/'working/tests/terrain-region';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
    def run(cmd,name,**kw):
        r=subprocess.run(cmd,capture_output=True,text=True,timeout=240,**kw);(out/(name+'.stdout')).write_text(r.stdout);(out/(name+'.stderr')).write_text(r.stderr)
        if r.returncode:raise RuntimeError(name+' failed: '+r.stderr[-2000:])
        return r.stdout
    def verify(phase):run([str(ROOT/'tools/original-manifest.sh'),'verify'],'original-'+phase)
    verify('before')
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe';assert sha(pe)==HASH
        codec=module('region_codec',source/'tools/decode-cfg.py');geometry=module('region_geometry',source/'tools/test-terrain-map.py');sprites=module('region_sprites',source/'tools/test-terrain-preview.py')
        configs=[root/f'Realms/{r}/{r}.cfg' for r in ['Celtic','Greek','Medieval']]
        recipes={};inputs={};track=lambda f:inputs.setdefault(f,sha(f))
        builds=[args.preview.resolve(),args.sanitized.resolve()];inspectors=[args.inspect.resolve(),args.inspect_sanitized.resolve()]
        for f in [pe,Path(__file__),*configs,*builds,*inspectors]:track(f)
        for folder in ['assets','renderer/sprites','reconstruction/rendering','apps/terrain-preview']:
            for f in (source/folder).glob('*'):
                if f.is_file():track(f)
        for name in ['tests/terrain-region-reference.cpp','tests/terrain-region-test.cpp','tests/region-recipe-test.cpp','tests/world-terrain-reference.cpp','tools/decode-cfg.py','tools/test-terrain-map.py','tools/test-terrain-preview.py']:track(source/name)
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0')
        for config in configs:
            b=config.read_bytes();raw=b if b.startswith(b';') else codec.decode(b)[1];expected=recipe_rows(raw);recipes[config.parent.name]=expected
            for i,binary in enumerate(inspectors):assert json.loads(run([str(binary),str(root),str(config.relative_to(root))],f'recipes-{config.parent.name}-{i}',env=env))==expected
        fixtures=[];selected=[]
        for region_id in [6,9]:
            recipe=next(r for r in recipes['Celtic'] if r['id']==region_id);maps=[];fixture=struct.pack('<3I',recipe['columns'],recipe['rows'],len(recipe['specific']))
            for placement in recipe['specific']:
                f=root/recipe['path'].replace('\\','/')/f"{recipe['prefix']}{placement[0]:02}.map";track(f);_,raw=codec.decode(f.read_bytes());v=struct.unpack_from('<19I',raw);maps.append((v,raw));fixture+=struct.pack('<4i',*placement)+raw[:76]
            fp=out/f'region-{region_id}.bin';fp.write_bytes(fixture);fixtures.append(str(fp));selected.append((recipe,maps))
        include='-I'+str(source/'reconstruction/rendering');models=[str(source/'reconstruction/rendering'/f) for f in ['terrain_region.cpp','terrain_sections.cpp']];cpp=str(source/'tests/terrain-region-reference.cpp')
        oracle=out/'reference';run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie',include,cpp,*models,'-o',str(oracle)],'compile-reference')
        original=json.loads(run([str(oracle),str(pe),*fixtures],'reference'))
        native=out/'native-sanitized';run(['g++','-std=c++17','-O1','-DMNM_NATIVE_ONLY','-fsanitize=address,undefined','-fno-omit-frame-pointer',include,cpp,*models,'-o',str(native)],'compile-sanitized')
        sanitized=json.loads(run([str(native),str(pe),*fixtures],'native-sanitized',env=env));assert original==sanitized;print(original,flush=True)
        helper=out/'world-reference';run(['g++','-m32','-std=c++17',include,str(source/'tests/world-terrain-reference.cpp'),'-o',str(helper)],'compile-world')
        images=[]
        for recipe,maps in selected:
            realm=root/recipe['sprite_path'].replace('\\','/');ttd=realm/'Terrain.ttd';spr=realm/'Terrain.spr';track(ttd);track(spr)
            assembled,blocks=assemble(recipe,maps,ttd.read_bytes());_,final=geometry.geometry(assembled,ttd.read_bytes());fp=out/f'final-{recipe["id"]}.bin';fp.write_bytes(final)
            w,h,layers=struct.unpack_from('<3I',final,4);camera=[w//2,h//2,min(20,w,h),layers];asset=dict(sprite=spr.read_bytes(),frames={})
            for view in range(4):
                for visibility in [False,True]:
                    index=len(images);expected=json.loads(run([str(helper),str(pe),str(ttd),str(spr),str(fp),*map(str,camera),'256','64',str(int(visibility)),str(view)],f'world-{index}'))
                    pixels=[0x2124]*(512*256)
                    for d in expected['queue']:
                        if d['kind']==-2:continue
                        ox,oy,values=sprites.frame(asset,d['frame'])
                        for dx,dy,value in values:
                            x,y=d['x']-ox+dx,d['y']-oy+dy
                            if 0<=x<512 and 0<=y<256:pixels[y*512+x]=value
                    words=struct.pack('<'+'H'*len(pixels),*pixels);rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in pixels)
                    for build,binary in enumerate(builds):
                        prefix=out/f'frame-{index}-{build}';cmd=[str(binary),'--root',str(root),'--region-config','realms\\CELTIC\\celtic.CFG','--region-id',str(recipe['id']),'--world','--initialize-terrain','--view',str(view),'--output',str(prefix)]
                        if visibility:cmd+=['--visibility']
                        run(cmd,f'native-{index}-{build}',env=env);actual=json.loads(prefix.with_suffix('.json').read_text());queue=[]
                        for draw in actual['queue']:
                            d={k:v for k,v in draw.items() if k not in ['tile','role']};d['cell']=actual['tiles'][draw['tile']]['cell'];queue.append(d)
                        assert queue==expected['queue'] and actual['map']['recipe']['blocks']==blocks and actual['map']['layers']==layers
                        assert actual['remaining_surfaces']==0 and prefix.with_suffix('.565').read_bytes()==words and actual['rgba_sha256']==hashlib.sha256(rgba).hexdigest()
                        for tile,owner in zip(actual['tiles'],actual['owners']):
                            c=struct.unpack_from('<6H',final,76+12*tile['cell']);assert (tile['definition'],tile['flags8'],tile['flags10'])==(c[0],c[4],c[5]);assert owner==expected['owners'][tile['cell']]
                        images.append(dict(region=recipe['id'],dimensions=[w,h,layers],view=view,visibility=visibility,build=build,draws=len(queue),sha256=sha(prefix.with_suffix('.565'))))
        rejections=[]
        cases=[('random',['--region-config','Realms/Celtic/Celtic.cfg','--region-id','1']),('wildcard',['--region-config','Realms/Medieval/Medieval.cfg','--region-id','16']),('unknown',['--region-config','Realms/Celtic/Celtic.cfg','--region-id','99']),('missing-id',['--region-config','Realms/Celtic/Celtic.cfg']),('id-without-cfg',['--region-id','6']),('conflicting-map',['--region-config','Realms/Celtic/Celtic.cfg','--region-id','6','--map','dummy'])]
        for build,binary in enumerate(builds):
            for name,flags in cases:
                r=subprocess.run([str(binary),'--root',str(root),'--world','--initialize-terrain',*flags],capture_output=True,text=True,timeout=30,env=env);assert r.returncode==1 and 'Sanitizer' not in r.stderr;rejections.append(dict(build=build,case=name,error=r.stderr.splitlines()[-1]))
        assert all(sha(f)==digest for f,digest in inputs.items()),'Inputs changed'
        report=dict(all_match=True,live_validated=False,executable_sha256=HASH,original=original,sanitized=sanitized,recipes=recipes,images=images,rejections=rejections,source_and_input_sha256={str(f):h for f,h in inputs.items()},helper_sha256={name:sha(out/name) for name in ['reference','native-sanitized','world-reference']},scope='Fixed authored region placement only; selected original descriptor/admission/placement calls for two installed regions; native owned recipes checked independently; ordinary entity/reference projection, initialized mixed-height scenes, original queues and independent unshaded SPR pixels; no random constraint solver or live replacement')
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(dict(recipes=sum(map(len,recipes.values())),images=len(images),rejections=len(rejections)),flush=True)
    finally:verify('after')
if __name__=='__main__':main()
