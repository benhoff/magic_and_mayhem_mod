#!/usr/bin/env python3
"""Compare fixed direct-word World observation with native/private-original replay."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def declaration():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='NP.world-word-entry-observation');s=next(s for s in r['scenarios'] if s['id']=='native-world-word-entry-replay')
    paths=required_sources(b)|set(s['tests'])
    for directory in ('renderer','assets','compat/legacy','protocols','runtime/scene'):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ('.h','.hpp','.c','.cpp','.S','.inc') or p.name=='CMakeLists.txt'))
    return [dict(behavior=b['id'],contract_sha256=behavior_contract(b,{b['id']:b for b in r['builds']}),scenarios={s['id']:scenario_contract(s)})],{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--capture',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    out=args.output.resolve();capture=args.capture.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):parser.error('Require fresh output under working/')
    out.mkdir(parents=True);claims,sources=declaration();metadata=json.loads((capture/'report.json').read_text())
    report=dict(schema=1,success=False,claims=claims,sources=sources,original_sha256=BUILD,live_replacement=False,original_pixels_used_as_native_inputs=False,scope='Fixed original-active capture including clipped direct-word596cb8: owned frame/wire/ABI controls, independent native/original zero-base replay and diagnostic original-before replay versus captured after. Direct-word ablation identifies its missing contribution in this new trace; no original output seeds native drawing or complete native history/live replacement claim.')
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,command,timeout=600):
        with (out/(label+'.log')).open('x') as f:subprocess.run([str(c) for c in command],cwd=ROOT,env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'},stdout=f,stderr=subprocess.STDOUT,check=True,timeout=timeout)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        if not metadata.get('success') or metadata.get('source_executable_sha256')!=BUILD:raise ValueError('Unsupported/incomplete capture')
        # Require source-current observer bodies, independent of the harness that launched them.
        for p in ('runtime/scene/world_trace.c','runtime/scene/world_entry.S','runtime/scene/world_colour.h','reconstruction/rendering/kind8_class.h'):
            if metadata['sources'].get(p)!=sources[p]:raise ValueError('Captured observer body differs: '+p)
        run('replay',['python3',ROOT/'tools/test-kind8-raster.py','--capture',capture,'--build-dir','/tmp/mnm-tint-outline-build-20261020'])
        paths=[Path(l) for l in (out/'replay.log').read_text().splitlines() if l.startswith(str(ROOT/'working/tests/kind8/raster-'))]
        if not paths:raise ValueError('Missing original/native replay report')
        nested=paths[0];result=json.loads((nested/'report.json').read_text())
        if not result.get('success') or not result.get('captured_ordered_queue_match'):raise ValueError('Original/native replay did not close')
        snapshots=sorted((capture/'capture').glob('world-*.bin'));ablations=[]
        for path in snapshots:
            wire=path.read_bytes();at=80;records=[];removed=0
            for i in range(struct.unpack_from('<I',wire,36)[0]):
                size=struct.unpack_from('<I',wire,at)[0];op=struct.unpack_from('<I',wire,at+4)[0];backend=struct.unpack_from('<I',wire,at+52)[0]
                if op==0 and backend==8:removed+=1
                else:records.append(wire[at:at+size])
                at+=size
            if not removed:raise ValueError('Capture did not exercise direct-word drawing')
            stripped=bytearray(wire[:80]+b''.join(records));struct.pack_into('<I',stripped,16,len(stripped));struct.pack_into('<I',stripped,36,len(records))
            ablated=out/(path.stem+'-without-word.bin');ablated.write_bytes(stripped);pixels=ablated.with_suffix('.565')
            run(path.stem+'-ablation',[nested/'reference',ROOT/'original/Arcane_Nocd/Chaos.exe',ablated,pixels,path.with_suffix('.before.565')])
            after=path.with_suffix('.after.565').read_bytes();width,height,stride=struct.unpack_from('<III',wire,24);cropped=b''.join(after[y*stride*2:(y*stride+width)*2] for y in range(height));actual=pixels.read_bytes()
            differences=[i//2 for i in range(0,len(actual),2) if actual[i:i+2]!=cropped[i:i+2]]
            if not differences:raise ValueError('Ablation did not distinguish missing word records')
            ablations.append(dict(sequence=path.stem,removed_records=removed,mismatched_pixels=len(differences),mismatch_indices=differences,all_live_words_cyan=all(cropped[i*2:i*2+2]==b'\xff\x45' for i in differences)))
        if declaration()!=(claims,sources):raise ValueError('Declared sources/contracts changed')
        report.update(success=True,sources_stable=True,active_hook_abi_and_class_gates_passed=result['active_hook_abi_and_class_gates_passed'],captured_ordered_queue_match=True,native_zero_matches_private_original=True,captured_before_replay_matches_live=True,pixels_compared=sum(c['pixels'] for c in result['captured']),captured=result['captured'],capture_inputs=result['capture_inputs'],nested_report_sha256=sha(nested/'report.json'),nested_artifacts=str(nested),ablations=ablations)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
