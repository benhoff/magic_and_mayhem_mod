#!/usr/bin/env python3
"""Guarded offline comparisons of the selected catalog construction and connector threshold."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import shlex

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--source-root',type=Path,default=ROOT)
    args=parser.parse_args();source=args.source_root.resolve()
    parent=ROOT/'working/tests/terrain-catalog';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,label,**kwargs):
        result=subprocess.run(command,capture_output=True,text=True,timeout=240,**kwargs)
        (out/(label+'.stdout')).write_text(result.stdout)
        (out/(label+'.stderr')).write_text(result.stderr)
        if result.returncode:raise RuntimeError(label+' failed: '+result.stdout[-500:]+result.stderr[-1500:])
        return result.stdout
    sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
    inputs={}
    def track(path):inputs[str(path)]=sha(path)
    run([str(ROOT/'tools/original-manifest.sh'),'verify'],'original-before')
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe'
        if sha(pe)!=HASH:raise RuntimeError('Unsupported executable')
        names=['tests/terrain-catalog-reference.cpp','tests/terrain-catalog-build-test.cpp',
               'reconstruction/rendering/terrain_catalog.hpp','reconstruction/rendering/terrain_catalog.cpp',
               'reconstruction/rendering/terrain_specific.cpp','reconstruction/rendering/terrain_specific.hpp',
               'tests/terrain-catalog-build-test.cpp','assets/region_recipe.cpp','assets/region_recipe.hpp','assets/persistence_internal.hpp','assets/persistence.hpp','tools/test-terrain-region.py','assets/asset_file.cpp','assets/asset_file.hpp','assets/path_resolver.cpp','assets/path_resolver.hpp','assets/packed_container.cpp',
               'reconstruction/rendering/terrain_solver.cpp','reconstruction/rendering/terrain_solver.hpp',
               'reconstruction/rendering/terrain_constraints.cpp','reconstruction/rendering/terrain_constraints.hpp','reconstruction/rendering/terrain_selection.cpp','reconstruction/rendering/terrain_selection.hpp','assets/map.hpp','tools/decode-cfg.py','reconstruction/rendering/CMakeLists.txt']
        for path in [pe,Path(__file__),*(source/name for name in names)]:track(path)
        if (source/'source-baseline.json').exists():track(source/'source-baseline.json')
        exports=[ROOT/'working/region-plan-decompiled/0052d530.c',
                 ROOT/'working/region-admission-decompiled/0052edf0.c',
                 ROOT/'working/region-plan-decompiled/0052d440.c',
                 ROOT/'working/region-plan-decompiled/0052f3f0.c',
                 *(ROOT/'working/region-candidates-export/pseudo'/name for name in ['00530c60.c','005310f0.c','00531710.c'])]
        for path in exports:track(path)
        spec=importlib.util.spec_from_file_location('selection_codec',source/'tools/decode-cfg.py')
        codec=importlib.util.module_from_spec(spec);spec.loader.exec_module(codec)
        spec=importlib.util.spec_from_file_location('catalog_recipes',source/'tools/test-terrain-region.py')
        parser_module=importlib.util.module_from_spec(spec);spec.loader.exec_module(parser_module)
        rows=[];maps=[];cfgs=[];header_cache={};missing=[]
        for realm in ['Celtic','Greek','Medieval']:
            config=ROOT/('working/game-clean/Realms/'+realm+'/'+realm+'.cfg');track(config)
            raw=config.read_bytes();raw=raw if raw.startswith(b';') else codec.decode(raw)[1]
            cfg=out/(realm+'.cfg');cfg.write_bytes(raw);track(cfg);cfgs.append(str(cfg))
            for recipe in parser_module.recipe_rows(raw):
                entries=[list(q)+[1] for q in recipe['specific']]+[[q[0],-1,-1,-1,q[1]] for q in recipe['random']]
                row=struct.pack('<5I',recipe['id'],recipe['columns'],recipe['rows'],len(recipe['specific']),len(recipe['random']))
                for entry in entries:
                    path=ROOT/'working/game-clean'/recipe['path'].replace('\\','/')/(recipe['prefix']+f'{entry[0]:02}.map')
                    if not path.exists():
                        missing.append(dict(realm=realm,region=recipe['id'],section=entry[0],path=str(path.relative_to(ROOT))))
                        row+=struct.pack('<5i',*entry)+bytes(76);continue
                    if path not in header_cache:
                        track(path);header_cache[path]=codec.decode(path.read_bytes())[1][:76]
                        maps.append(dict(path=str(path.relative_to(ROOT)),header_sha256=hashlib.sha256(header_cache[path]).hexdigest()))
                    row+=struct.pack('<5i',*entry)+header_cache[path]
                rows.append(row)
        fixture=out/'catalogs.bin';fixture.write_bytes(struct.pack('<I',len(rows))+b''.join(rows));track(fixture)
        print(len(rows),'installed recipes',len(maps),'referenced headers',flush=True)
        include='-I'+str(source/'reconstruction/rendering')
        cpp=str(source/'tests/terrain-catalog-reference.cpp');model=str(source/'reconstruction/rendering/terrain_catalog.cpp');selection=str(source/'reconstruction/rendering/terrain_selection.cpp');constraints=str(source/'reconstruction/rendering/terrain_constraints.cpp');solver=str(source/'reconstruction/rendering/terrain_solver.cpp');specific=str(source/'reconstruction/rendering/terrain_specific.cpp');recipe=str(source/'assets/region_recipe.cpp')
        reference=out/'reference'
        run(['g++','-m32','-std=c++17','-ffunction-sections','-Wl,--gc-sections','-O2','-fno-pie','-no-pie',include,cpp,recipe,model,specific,solver,selection,constraints,'-o',str(reference)],'compile-reference')
        original=json.loads(run([str(reference),str(pe),str(fixture),*cfgs],'reference'))
        sanitized=out/'sanitized';env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
        qt=shlex.split(run(['pkg-config','--cflags','--libs','Qt6Core'],'qt-flags'))
        dependencies=[str(source/'assets'/name) for name in ['asset_file.cpp','path_resolver.cpp','packed_container.cpp']]
        run(['g++','-std=c++17','-ffunction-sections','-Wl,--gc-sections','-O1','-DMNM_NATIVE_ONLY','-fsanitize=address,undefined','-fno-omit-frame-pointer',include,cpp,recipe,model,specific,solver,selection,constraints,*dependencies,*qt,'-o',str(sanitized)],'compile-sanitized')
        native=json.loads(run([str(sanitized),str(pe),str(fixture),*cfgs],'sanitized',env=env))
        if native!=original:raise RuntimeError('Reference/sanitized fixture counts differ')
        unit=str(source/'tests/terrain-catalog-build-test.cpp')
        for build,flags in [('normal',['-O2']),('sanitized',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=out/('unit-'+build)
            run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic',*flags,include,unit,model,specific,solver,selection,constraints,'-o',str(binary)],'compile-unit-'+build)
            run([str(binary)],'unit-'+build,env=env);track(binary)
        for path,digest in inputs.items():
            if sha(Path(path))!=digest:raise RuntimeError('Input changed: '+path)
        report=dict(all_match=True,live_validated=False,executable_sha256=HASH,original=original,
                    sanitized=native,maps=maps,missing_sections=missing,artifacts=str(out.relative_to(ROOT)),
                    source_and_input_sha256=inputs,helper_sha256={p.name:sha(p) for p in [reference,sanitized]},
                    scope='Owned recipe-to-catalog construction, original connector threshold window and descriptor builder. Native CFG fields independently compared. No original CFG/file/dialog execution, generated scenes or live replacement.')
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(original,flush=True)
    finally:run([str(ROOT/'tools/original-manifest.sh'),'verify'],'original-after')
    return 0
if __name__=='__main__':raise SystemExit(main())
