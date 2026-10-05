#!/usr/bin/env python3
"""Configured Redcap: original CFG/sample/action comparisons and installed MAP continuation."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    if '--inner' not in sys.argv:
        subprocess.run(['xvfb-run','-a',sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=700);return
    args=[a for a in sys.argv[1:] if a!='--inner']
    if len(args)!=1:raise ValueError('Supply world-scene build directory (normal or sanitized)')
    build=Path(args[0]).resolve();export=build/'mnm-map-navigation-export';scene=build/'mnm-world-scene-preview';sandbox=build/'world/mnm-world-sandbox'
    parent=ROOT/'working/tests/creature-navigation';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    os.environ.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    def run(command,name,good=True):
        p=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=120)
        (out/f'{name}.stdout').write_text(p.stdout);(out/f'{name}.stderr').write_text(p.stderr)
        if p.returncode!=(0 if good else 1) or 'runtime error:' in p.stderr or 'AddressSanitizer' in p.stderr:
            raise RuntimeError((command,p.returncode,p.stdout,p.stderr))
        return p.stdout
    def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
    def verify(phase):run([ROOT/'tools/original-manifest.sh','verify'],'original-'+phase)
    verify('before')
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe';assert sha(pe)==HASH
        models=ROOT/'reconstruction/pathfinding';files=sorted(models.glob('*.cpp'));files=[p for p in files if p.name not in ('route_replay.cpp','route_neighbor_replay.cpp','replay_trace.cpp')]
        oracle=out/'original-reference'
        run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I',models,'-I',ROOT/'tests',ROOT/'tests/map-navigation-reference.cpp',*files,'-o',oracle],'compile-reference')
        codec=module('map_codec',ROOT/'tools/decode-cfg.py');geometry=module('map_geometry',ROOT/'tools/test-terrain-map.py');pixels=module('map_pixels',ROOT/'tools/test-terrain-preview.py');ani=module('map_ani',ROOT/'tests/test-native-ani-motion.py')
        animation=ROOT/'working/game-clean/Creatures/redcap.ani'
        assets=ROOT/'working/game-clean';creaturePath=assets/'Creatures/RedCap.spr';creature={'sprite':creaturePath.read_bytes(),'frames':{}}
        cfgPath=assets/'CFG/Encrypted/creature.cfg'
        decodedCfg=codec.decode(cfgPath.read_bytes())[1].decode('latin1')
        import configparser
        cfg=configparser.ConfigParser();cfg.read_string('\n'.join(line for line in decodedCfg.splitlines() if '=' in line or line.strip().startswith('[')))
        keys=['TileHeight','TileSizeXY','Acceleration','SwimmingAbility','CanFly','GroundSpeed','FlyingSpeed']
        installed=[cfg['CREATURE_10'][key] for key in keys]
        fixtures=[installed,['-5','-1','-9','1','FALSE','-5','-6'],['9','9','0','1','FALSE','2000','2000'],['2','2','1','1','false','1','2'],['5','1','64','1','fAlSe','99','0'],['3','1','10','1','TRUE','4','0']]
        fields=out/'selected-fields.txt';fields.write_text('\n'.join(v for row in fixtures for v in row)+'\n')
        profileSources=['tests/creature-profile-reference.cpp','assets/animation_decode.cpp','assets/creature_movement.cpp','assets/config_loader.cpp','reconstruction/motion/creature_profile.cpp','reconstruction/motion/creature_motion.cpp','reconstruction/animation/no_cd.cpp','reconstruction/pathfinding/route_scalar.cpp','reconstruction/pathfinding/route_request.cpp','reconstruction/pathfinding/route_search.cpp']
        profileOracle=out/'profile-reference'
        run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-Wno-unused-function','-ffunction-sections','-Wl,--gc-sections','-DORIGINAL_REFERENCE',*[f'-I{ROOT/p}' for p in ['assets','reconstruction/animation','reconstruction/motion','reconstruction/pathfinding']],*[ROOT/p for p in profileSources],'-o',profileOracle],'compile-profile-reference')
        profileOriginal=json.loads(run([profileOracle,pe,fields,animation],'original-profile'))
        assert profileOriginal['all_match'] and profileOriginal['maximum']==235
        pixelChecks=0;continuations=0;rows=[];tracked=[pe,creaturePath,animation,cfgPath,export,scene,sandbox,oracle,profileOracle]
        for realm,stem in [('Plains','CP'),('Forest','CF'),('Village','CV')]:
            request=f'Realms/Celtic/{realm}/{stem}sec01.map';ttdPath=assets/f'Realms/Celtic/{realm}/Terrain.ttd';sprPath=ttdPath.with_suffix('.spr')
            raw=codec.decode((assets/request).read_bytes())[1];ttd=ttdPath.read_bytes();sourceW,sourceH,layers=struct.unpack_from('<3I',raw,4)
            _,prepared=geometry.geometry(raw,ttd);terrain={'sprite':sprPath.read_bytes(),'frames':{}};tracked.extend([assets/request,ttdPath,sprPath])
            for cx,cy in [(0,0),(4,4)]:
                prefix=out/f'{realm}-{cx}-{cy}'
                report=json.loads(run([export,assets,request,f'Realms/Celtic/{realm}',cx,cy,8,8,prefix,10],f'import-{realm}-{cx}'))
                assert report['creature_profile']['samples']==profileOriginal['samples']
                assert report['creature_profile']['height']==3 and report['creature_profile']['acceleration']==10
                header=struct.pack('<6I',6,8,8,layers,64,64*layers)+bytes(52);expected=[]
                for z in range(layers):
                    for y in range(8):
                        for x in range(8):
                            at=76+12*((z*sourceH+cy+y)*sourceW+cx+x);c=list(struct.unpack_from('<6H',prepared,at))
                            if x in (0,7) or y in (0,7):c[5]|=0x4000
                            expected.append(c)
                payload=header+b''.join(struct.pack('<6H',*c) for c in expected)
                assert Path(str(prefix)+'.geometry').read_bytes()==payload
                frozen=Path(str(prefix)+'.frozen');binary=frozen.read_bytes();sizes=struct.unpack_from('<7I',binary,64)
                cellAt=92+sizes[0]+sizes[1];assert binary[cellAt:cellAt+sizes[2]]==payload[76:]
                assert binary[cellAt+sizes[2]:cellAt+sizes[2]+sizes[3]]==ttd[16:]
                original=json.loads(run([oracle,pe,frozen],f'original-{realm}-{cx}'));assert original['all_match'] and original['sealed_moves_accepted']==0
                assert report['standing'];start=report['standing'][0]
                initial=out/f'{realm}-{cx}-start.mnms';chosen=None
                for index,goal in enumerate(reversed(report['standing'])):
                    if goal[2]!=start[2] or index>=64:continue
                    candidate=out/f'{realm}-{cx}-candidate-{index}.mnms'
                    planned=json.loads(run([sandbox,'move-terrain-ani',frozen,animation,0,candidate,*start,*goal,1],f'plan-{realm}-{cx}-{index}'))
                    entity=planned['entities'][0]
                    if entity['action'] in (2,3) and entity['route'] and all(p[5] in (0,4) for p in entity['route']):chosen=goal;break
                assert chosen is not None,(realm,cx,'No bounded ordinary route')
                creation=out/f'{realm}-{cx}-configured'
                run([export,assets,request,f'Realms/Celtic/{realm}',cx,cy,8,8,creation,10,*start,*chosen,0],f'initial-{realm}-{cx}')
                initial=Path(str(creation)+'.mnms')
                assert Path(str(creation)+'.frozen').read_bytes()==binary
                trace=[json.loads(line) for line in run([sandbox,'trace',initial,400],f'trace-{realm}-{cx}').splitlines()]
                assert trace[-1]['entities'][0]['action']==3,(realm,cx,'Route did not arrive')
                for row in trace:
                    e=row['entities'][0];assert 0<e['position'][0]<7 and 0<e['position'][1]<7
                for view in range(4):
                    options=['--root',assets,'--realm',f'Realms/Celtic/{realm}','--terrain-map',Path(str(prefix)+'.geometry'),'--view',view]
                    movie=out/f'{realm}-{cx}-v{view}'
                    run([scene,'--checkpoint',initial,*options,'--output',movie,'--frames',28],f'frames-{realm}-{cx}-{view}')
                    frames=json.loads(Path(str(movie)+'.json').read_text())['frames']
                    for i,frame in enumerate(frames):
                        assert frame['tick']==trace[i]['tick'] and frame['actors'][0]['fine']==trace[i]['entities'][0]['fine']
                        assert [d['key'] for d in frame['queue']]==sorted(d['key'] for d in frame['queue'])
                        image=[0x2124]*(512*256)
                        for draw in frame['queue']:
                            ox,oy,opaque=pixels.frame(creature if draw['creature'] else terrain,draw['frame'])
                            for x,y,value in opaque:
                                x+=draw['x']-ox;y+=draw['y']-oy
                                if 0<=x<512 and 0<=y<256:image[y*512+x]=value
                        assert Path(f'{movie}-{i:03}.565').read_bytes()==struct.pack('<'+'H'*len(image),*image);pixelChecks+=1
                    split=out/f'{realm}-{cx}-v{view}-split';run([scene,'--checkpoint',initial,*options,'--output',split,'--frames',14],f'split-{realm}-{cx}-{view}')
                    resumed=out/f'{realm}-{cx}-v{view}-resume';run([scene,'--checkpoint',Path(str(split)+'.mnms'),*options,'--ticks',1,'--output',resumed,'--frames',14],f'resume-{realm}-{cx}-{view}')
                    assert json.loads(Path(str(resumed)+'.json').read_text())['frames']==frames[14:]
                    assert Path(str(resumed)+'.mnms').read_bytes()==Path(str(movie)+'.mnms').read_bytes()
                    for i in range(14):
                        for extension in ['565','png']:assert Path(f'{resumed}-{i:03}.{extension}').read_bytes()==Path(f'{movie}-{i+14:03}.{extension}').read_bytes()
                    continuations+=1
                # A visual map with identical dimensions but changed terrain bytes must refuse before output.
                wrong=out/f'{realm}-{cx}-wrong.geometry';bad=bytearray(payload);bad[76+12*(9+64)+8]^=1;wrong.write_bytes(bad)
                run([scene,'--checkpoint',initial,'--root',assets,'--realm',f'Realms/Celtic/{realm}','--terrain-map',wrong,'--output',out/f'{realm}-{cx}-refused'],f'refuse-{realm}-{cx}',good=False)
                assert not list(out.glob(f'{realm}-{cx}-refused*'))
                rows.append({'map':request,'crop':[cx,cy,8,8],'layers':layers,'standing':len(report['standing']),'start':start,'goal':chosen,'original':original,'trace':str(out/f'trace-{realm}-{cx}.stdout'),'geometry_sha256':sha(Path(str(prefix)+'.geometry'))})
        # Unsupported type and unavailable fields refuse before output publication.
        refusedPrefix=out/'unsupported-type'
        run([export,assets,'Realms/Celtic/Plains/CPsec01.map','Realms/Celtic/Plains',0,0,8,8,refusedPrefix,12],'refuse-type',good=False)
        assert not list(out.glob('unsupported-type*'))
        assert sha(pe)==HASH
        sources=[Path(__file__),*[ROOT/p for p in profileSources],ROOT/'assets/creature_movement.hpp',ROOT/'reconstruction/motion/creature_profile.hpp',ROOT/'reconstruction/motion/creature_motion.hpp',ROOT/'reconstruction/animation/no_cd.hpp',ROOT/'tests/creature-profile-test.cpp',ROOT/'tests/creature-motion-test.cpp',ROOT/'assets/CMakeLists.txt',ROOT/'apps/world-sandbox/frozen_navigation.cpp',ROOT/'apps/world-sandbox/frozen_navigation.hpp',ROOT/'tests/map-navigation-test.cpp',ROOT/'tests/map-navigation-reference.cpp',ROOT/'tests/clearance-native-reference.hpp',ROOT/'tests/cell-validity-native-reference.hpp',ROOT/'tests/cell-support-native-reference.hpp',ROOT/'tests/movement-native-reference.hpp',*files,*models.glob('*.hpp'),*list((ROOT/'apps/world-scene').glob('*.cpp')),*list((ROOT/'apps/world-scene').glob('*.hpp')),ROOT/'apps/world-scene/CMakeLists.txt']
        result={'all_match':True,'live_validated':False,'original_profile':profileOriginal,'original_manifest_verified_before_after':False,'input_unchanged':True,'original_sha256':HASH,'original_scope':'Selected CFG fields via controlled profile callbacks; unmodified ANI sample builder; installed movement action with private completion adapter; unmodified support/validity and complete movement-helper predicates on projected installed terrain; no original whole-search or map/entity load equivalence','pixel_comparisons':pixelChecks,'fresh_process_continuations':continuations,'maps':rows,'source_sha256':{str(p.relative_to(ROOT)):sha(p) for p in sources},'input_artifact_sha256':{str(p.relative_to(ROOT)):sha(p) for p in tracked}}
    finally:verify('after')
    result['original_manifest_verified_before_after']=True
    (out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'all_match':True,'crops':len(rows),'pixels':pixelChecks,'continuations':continuations}),flush=True)
if __name__=='__main__':main()
