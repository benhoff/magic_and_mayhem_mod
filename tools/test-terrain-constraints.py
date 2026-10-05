#!/usr/bin/env python3
"""Guarded offline comparisons of selected multi-block region constraint helpers."""
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
def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--source-root',type=Path,default=ROOT)
    args=parser.parse_args();source=args.source_root.resolve()
    parent=ROOT/'working/tests/terrain-constraints';parent.mkdir(parents=True,exist_ok=True)
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
        names=['tests/terrain-constraints-reference.cpp','tests/terrain-constraints-test.cpp',
               'reconstruction/rendering/terrain_constraints.hpp','reconstruction/rendering/terrain_constraints.cpp',
               'reconstruction/rendering/terrain_selection.cpp','reconstruction/rendering/terrain_selection.hpp','assets/map.hpp','tools/decode-cfg.py']
        for path in [pe,Path(__file__),*(source/name for name in names)]:track(path)
        spec=importlib.util.spec_from_file_location('selection_codec',source/'tools/decode-cfg.py')
        codec=importlib.util.module_from_spec(spec);spec.loader.exec_module(codec)
        rows=[];maps=[]
        for path in sorted(f for f in (ROOT/'working/game-clean').rglob('*') if f.suffix.lower()=='.map'):
            track(path);raw=codec.decode(path.read_bytes())[1]
            # Section IDs are caller data, not serialized in the MAP header.
            section=len(rows)%100
            rows.append(struct.pack('<I',section)+raw[:76])
            maps.append(dict(path=str(path.relative_to(ROOT)),fixture_section=section,
                             header_sha256=hashlib.sha256(raw[:76]).hexdigest()))
        fixture=out/'headers.bin';fixture.write_bytes(struct.pack('<I',len(rows))+b''.join(rows));track(fixture)
        print(len(rows),'installed section headers',flush=True)
        include='-I'+str(source/'reconstruction/rendering')
        cpp=str(source/'tests/terrain-constraints-reference.cpp');model=str(source/'reconstruction/rendering/terrain_constraints.cpp');selection=str(source/'reconstruction/rendering/terrain_selection.cpp')
        reference=out/'reference'
        run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie',include,cpp,model,selection,'-o',str(reference)],'compile-reference')
        original=json.loads(run([str(reference),str(pe),str(fixture)],'reference'))
        sanitized=out/'sanitized';env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
        run(['g++','-std=c++17','-O1','-DMNM_NATIVE_ONLY','-fsanitize=address,undefined','-fno-omit-frame-pointer',include,cpp,model,selection,'-o',str(sanitized)],'compile-sanitized')
        native=json.loads(run([str(sanitized),str(pe),str(fixture)],'sanitized',env=env))
        if native!=original:raise RuntimeError('Reference/sanitized fixture counts differ')
        unit=str(source/'tests/terrain-constraints-test.cpp')
        for build,flags in [('normal',['-O2']),('sanitized',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=out/('unit-'+build)
            run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic',*flags,include,unit,model,selection,'-o',str(binary)],'compile-unit-'+build)
            run([str(binary)],'unit-'+build,env=env);track(binary)
        for path,digest in inputs.items():
            if sha(Path(path))!=digest:raise RuntimeError('Input changed: '+path)
        report=dict(all_match=True,live_validated=False,executable_sha256=HASH,original=original,
                    sanitized=native,maps=maps,artifacts=str(out.relative_to(ROOT)),
                    source_and_input_sha256=inputs,helper_sha256={p.name:sha(p) for p in [reference,sanitized]},
                    scope='Selected descriptor expansion/connectors, multi-block admission and carry, candidate/location tables, seeded candidate choices and bounded three-pass pruning. No complete solver/backtracking, retries, recipe generation, scene loading or live replacement.')
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(original,flush=True)
    finally:run([str(ROOT/'tools/original-manifest.sh'),'verify'],'original-after')
    return 0
if __name__=='__main__':raise SystemExit(main())
