#!/usr/bin/env python3
"""Investigate a retained live mismatch using an independent original zero-background replay."""
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


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--experiment',type=Path,required=True)
    args=parser.parse_args();directory=args.experiment.resolve()
    spec=importlib.util.spec_from_file_location('world_check',ROOT/'tools/test-world-frames.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    parent=ROOT/'working/tests/native-world-background';parent.mkdir(parents=True,exist_ok=True);output=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    sources=module.sources();sources['tools/test-world-background.py']=module.sha(Path(__file__).resolve())
    report={'success':False,'sources':sources,'source_executable_sha256':module.BUILD_HASH,'scope':'Independent zero-background replay of a retained live mismatch; does not claim original initial World canvas lifecycle'}
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe'
        if module.sha(pe)!=module.BUILD_HASH:raise ValueError('Original executable hash changed')
        report['entry_anchors']=module.reference_anchor(pe)
        snapshots=sorted(p for p in directory.glob('world-*.bin') if p.stem.removeprefix('world-').isdigit())
        if len(snapshots)!=1:raise ValueError('Expected exactly one retained failed snapshot')
        snapshot=snapshots[0];needed={module.identity(raw,h[9]) for h,raw in module.records(snapshot.read_bytes())}
        bindings=output/'bindings.json';bindings.write_text(json.dumps(module.pinned_bindings(directory/'game',needed),indent=2)+'\n')
        build=ROOT/'working/build/world-frame';reference=output/'reference'
        def run(command):
            with (output/'execution.log').open('a') as log:subprocess.run([str(c) for c in command],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
        run(['cmake','--build',build,'--target','mnm-world-frame-preview','-j4'])
        run(['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/world-frame-reference.cpp','-o',reference])
        run([reference,pe,snapshot,output/'independent.565'])
        run([build/'mnm-world-frame-preview','--root',directory/'game','--snapshot',snapshot,'--bindings',bindings,'--output',output/'native'])
        native=(output/'native.565').read_bytes();independent=(output/'independent.565').read_bytes();live=snapshot.with_suffix('.original.565').read_bytes()
        if native!=independent:raise RuntimeError('Native differs from independent original zero-background replay')
        differences=[i for i,(a,b) in enumerate(zip(struct.unpack('<'+'H'*(len(native)//2),native),struct.unpack('<'+'H'*(len(live)//2),live))) if a!=b]
        if not differences:raise RuntimeError('Retained live gap did not reproduce')
        if module.sources()!={k:v for k,v in sources.items() if k!='tools/test-world-background.py'}:raise RuntimeError('Sources changed')
        report.update(success=True,sources_stable=True,independent_original_pixels_match=True,live_original_pixels_match=False,
            live_mismatches=len(differences),live_mismatch_offsets=differences,original_pixels_used_as_native_inputs=False,
            experiment=str(directory.relative_to(ROOT)),snapshot_sha256=module.sha(snapshot),native_sha256=hashlib.sha256(native).hexdigest(),
            original_manifest_verified_before_after=True,limitation='Live output differs from replay with independent zero initial canvas. Retained initial World pixels or an untraced producer remain unresolved; full live baseline equivalence and bypass blocked for this case.')
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
        (output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(output/'report.json',flush=True)


if __name__=='__main__':main()
