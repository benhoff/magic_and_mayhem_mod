#!/usr/bin/env python3
"""Unchanged section copy oracle and explicit assembled terrain image checks."""
import argparse
import configparser
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result

def assembly(raw, ttd, side, rotations):
    w, h, layers = struct.unpack_from('<3I', raw, 4)
    assert side <= min(w, h) and len(rotations) == 4
    out = [[0]*6 for _ in range(side*side*4*layers)]
    for slot, rotation in enumerate(rotations):
        for z in range(layers):
            for y in range(side):
                for x in range(side):
                    c = list(struct.unpack_from('<6H', raw, 76+12*((z*h+y)*w+x)))
                    c[1:4] = [65535]*3
                    c[5] &= 0xfff7
                    if rotation:
                        c[0] = struct.unpack_from('<H', ttd, 16+356*c[0]+0x94-4*rotation)[0]
                        for _ in range(rotation):
                            bits = [(c[4] >> shift) & 1 for shift in [10, 11, 12, 13]]
                            c[4] &= 0xc3ff
                            for bit, shift in zip(bits, [11, 12, 13, 10]):
                                c[4] |= bit << shift
                    tx, ty = [(x,y), (side-1-y,x), (side-1-x,side-1-y), (y,side-1-x)][rotation]
                    out[(z*side*2+slot//2*side+ty)*side*2+slot%2*side+tx] = c
    header = bytearray(76)
    struct.pack_into('<4I', header, 0, 6, side*2, side*2, layers)
    return bytes(header)+b''.join(struct.pack('<6H', *c) for c in out)

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--source-root', type=Path, default=ROOT)
    p.add_argument('--preview', type=Path, required=True)
    p.add_argument('--sanitized', type=Path, required=True)
    args = p.parse_args()
    source = args.source_root.resolve()
    parent = ROOT/'working/tests/terrain-sections'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    def run(command, name, **kwargs):
        r = subprocess.run(command, capture_output=True, text=True, timeout=240, **kwargs)
        (out/(name+'.stdout')).write_text(r.stdout)
        (out/(name+'.stderr')).write_text(r.stderr)
        if r.returncode:
            raise RuntimeError(name+' failed: '+r.stderr[-3000:])
        return r.stdout
    def verify(phase):
        run([str(ROOT/'tools/original-manifest.sh'), 'verify'], 'original-'+phase)
    verify('before')
    try:
        pe = ROOT/'working/game-nocd/Chaos.exe'
        assert sha(pe) == HASH
        root = ROOT/'working/game-clean'
        codec = module('sections_codec', source/'tools/decode-cfg.py')
        geometry = module('sections_geometry', source/'tools/test-terrain-map.py')
        decoder = module('sections_sprites', source/'tools/test-terrain-preview.py')
        configs = [root/f'Realms/{realm}/{realm}.cfg' for realm in ['Celtic','Greek','Medieval']]
        aliases = {}
        for config in configs:
            data = config.read_bytes()
            if not data.startswith(b';'):
                _, data = codec.decode(data)
            parsed = configparser.ConfigParser(interpolation=None, strict=False)
            parsed.read_string(data.decode('latin1'))
            for section in parsed.sections():
                if parsed.has_option(section,'path') and parsed.has_option(section,'spritepath'):
                    directory = root/parsed.get(section,'path').strip().replace('\\','/')
                    aliases[directory] = root/parsed.get(section,'spritepath').strip().replace('\\','/')
        files = sorted(f for f in root.rglob('*') if f.suffix.lower()=='.map')
        assert len(files)==683
        catalogs = {f: (f.parent if (f.parent/'Terrain.ttd').is_file() else aliases[f.parent])/'Terrain.ttd' for f in files}
        builds = [args.preview.resolve(), args.sanitized.resolve()]
        paths = [pe, Path(__file__), *files, *configs, *set(catalogs.values()), *builds]
        for folder in ['apps/terrain-preview','reconstruction/rendering','assets','renderer/sprites']:
            paths += [f for f in (source/folder).glob('*') if f.is_file()]
        paths += [source/f for f in ['tests/terrain-sections-reference.cpp','tests/terrain-sections-test.cpp','tests/world-terrain-reference.cpp','tools/decode-cfg.py','tools/test-terrain-map.py','tools/test-terrain-preview.py']]
        realm = root/'Realms/Celtic/Forest'
        paths += [realm/'Terrain.spr']
        inputs = {f:sha(f) for f in paths}
        fixtures, rows = [], []
        for i, f in enumerate(files):
            _, raw = codec.decode(f.read_bytes())
            path = out/f'map-{i:03}.bin'
            path.write_bytes(raw)
            fixtures += [str(catalogs[f]), str(path)]
            w,h,layers = struct.unpack_from('<3I',raw,4)
            rows.append(dict(path=str(f.relative_to(root)), dimensions=[w,h,layers], selected_cells=4*min(w,h)**2*layers))
        include = '-I'+str(source/'reconstruction/rendering')
        reference = out/'reference'
        model = str(source/'reconstruction/rendering/terrain_sections.cpp')
        src = str(source/'tests/terrain-sections-reference.cpp')
        run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie',include,src,model,'-o',str(reference)],'compile-reference')
        original = json.loads(run([str(reference),str(pe),*fixtures],'reference'))
        native = out/'native-sanitized'
        run(['g++','-std=c++17','-O1','-DMNM_NATIVE_ONLY','-fsanitize=address,undefined','-fno-omit-frame-pointer',include,src,model,'-o',str(native)],'compile-native')
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        sanitized = json.loads(run([str(native),str(pe),*fixtures],'native-sanitized',env=env))
        assert original == sanitized and original['installed_cells']==sum(r['selected_cells'] for r in rows)
        print(original,flush=True)
        helper = out/'world-reference'
        run(['g++','-m32','-std=c++17',include,str(source/'tests/world-terrain-reference.cpp'),'-o',str(helper)],'compile-world')
        sprite = dict(sprite=(realm/'Terrain.spr').read_bytes(),frames={})
        images=[]
        for name in ['CFsec01.map','CFsec02.map','CFsec38.map']:
            _, raw = codec.decode((realm/name).read_bytes())
            ttd = (realm/'Terrain.ttd').read_bytes()
            for side in [10,20]:
                rotations = [0,1,2,3]
                assembled = assembly(raw,ttd,side,rotations)
                _, final = geometry.geometry(assembled,ttd)
                finalpath = out/f'assembled-{name}-{side}.bin'
                finalpath.write_bytes(final)
                w,h,layers = struct.unpack_from('<3I',final,4)
                span=min(20,w,h);camera=[w//2,h//2,span,layers]
                for view in range(4):
                    for visibility in [False,True]:
                        index=len(images)
                        oracle=json.loads(run([str(helper),str(pe),str(realm/'Terrain.ttd'),str(realm/'Terrain.spr'),str(finalpath),*map(str,camera),'256','64',str(int(visibility)),str(view)],f'oracle-{index}'))
                        pixels=[0x2124]*(512*256)
                        for draw in oracle['queue']:
                            if draw['kind']==-2:continue
                            ox,oy,values=decoder.frame(sprite,draw['frame'])
                            for dx,dy,value in values:
                                x,y=draw['x']-ox+dx,draw['y']-oy+dy
                                if 0<=x<512 and 0<=y<256:pixels[y*512+x]=value
                        words=struct.pack('<'+'H'*len(pixels),*pixels)
                        rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in pixels)
                        for build,binary in enumerate(builds):
                            prefix=out/f'frame-{index}-{build}'
                            command=[str(binary),'--root',str(root),'--world','--initialize-terrain','--grid',f'2,2,{side}','--camera',','.join(map(str,camera)),'--view',str(view),'--output',str(prefix)]
                            for slot,r in enumerate(rotations):command += ['--section',f'Realms/Celtic/Forest/{name},0,0,{slot%2},{slot//2},{r}']
                            if visibility:command+=['--visibility']
                            run(command,f'native-{index}-{build}',env=env)
                            actual=json.loads(prefix.with_suffix('.json').read_text())
                            queue=[]
                            for draw in actual['queue']:
                                d={k:v for k,v in draw.items() if k not in ['tile','role']}
                                d['cell']=actual['tiles'][draw['tile']]['cell'];queue.append(d)
                            assert queue==oracle['queue'] and actual['remaining_surfaces']==0
                            assert prefix.with_suffix('.565').read_bytes()==words
                            assert actual['rgba_sha256']==hashlib.sha256(rgba).hexdigest()
                            for tile,owner in zip(actual['tiles'],actual['owners']):
                                c=struct.unpack_from('<6H',final,76+12*tile['cell'])
                                assert (tile['definition'],tile['flags8'],tile['flags10'])==(c[0],c[4],c[5])
                                assert owner==oracle['owners'][tile['cell']]
                            images.append(dict(map=name,side=side,view=view,visibility=visibility,build=build,draws=len(queue),sha256=sha(prefix.with_suffix('.565'))))
        assert all(sha(f)==h for f,h in inputs.items()), 'Inputs changed'
        report=dict(all_match=True,live_validated=False,executable_sha256=HASH,original=original,sanitized=sanitized,maps=rows,images=images,source_and_input_sha256={str(f):h for f,h in inputs.items()},scope='Ordinary-cell section copy: complete destination bytes and source side effects; explicit grid/crops and object/reference projection policy; geometry after assembly; original queues and independent complete SPR pixels; no random region/entity initialization')
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(dict(images=len(images),all_match=True),flush=True)
    finally:
        verify('after')
if __name__=='__main__':main()
