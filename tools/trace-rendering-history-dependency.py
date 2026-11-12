#!/usr/bin/env python3
"""Trace and perturb a retained dependency in isolated original raster execution."""
import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract
from rendering_operations_history import selected_members, sha, plane_differences

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = ['tools/trace-rendering-history-dependency.py',
           'tools/rendering_operations_history.py', 'tests/world-frame-reference.cpp',
           'tests/world-frame-dependency-reference.cpp', 'tests/sprite-binary-reference.cpp',
           'protocols/include/mnm/world_frame_v1.h','tools/coverage_claims.py']


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    p.add_argument('--executable', type=Path, required=True)
    args = p.parse_args()
    target = ROOT/args.report
    if target.exists() or not target.resolve().is_relative_to(ROOT/'research/runtime'):
        p.error('Require a new immutable research report')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='TOOL.rendering-history-dependency')
    claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{}),scenarios={})]
    result = dict(schema=1, success=False, claims=claims, sources={n:sha((ROOT/n).read_bytes()) for n in SOURCES},
                  inputs={}, cases={}, fresh_game_execution=False,
                  fresh_original_raster_execution=False, fresh_native_raster_execution=False,
                  original_pixels_used_as_native_inputs=False,
                  scope='Isolated original execution on one retained800x600 World. Two pixel watches and original-only counterfactual entry planes locate a dependency; no pre-World writer, live reachability, native initialization or acceptance claim.')
    before_verified = False
    try:
        before = subprocess.run([ROOT/'tools/original-manifest.sh','verify'], capture_output=True, check=True)
        before_verified = True
        if sha(args.executable.read_bytes()) != BUILD:
            raise ValueError('Unsupported original executable')
        result['source_executable_sha256'] = BUILD
        with Reader(ROOT) as reader, tempfile.TemporaryDirectory(prefix='mnm-history-dependency-') as directory:
            directory = Path(directory)
            name = 'research/runtime/rendering-remaining-results-20261026/natural-late-comparison-02.json'
            previous = reader.json(name); result['inputs'][name] = reader.digest(name)
            name = 'research/runtime/rendering-remaining-results-20261026/raw-catalog-01.json'
            catalog = reader.json(name); result['inputs'][name] = reader.digest(name)
            name = 'research/runtime/rendering-remaining-results-20261026/raw-execution-01.tar.gz'
            raw = reader.read_bytes(name); result['inputs'][name] = reader.digest(name)
            if sha(raw) != catalog['archive_sha256']:
                raise ValueError('Raw archive pin differs')
            prefix = 'working/experiments/scene-observer/run-kinxgh80/capture/world-0001.'
            wanted = {prefix+suffix:previous['inputs'][prefix+suffix] for suffix in ('bin','before.565','after.565')}
            files = selected_members(raw,lambda name:name if name in wanted else None)
            if set(files) != set(wanted) or any(sha(files[n]) != h for n,h in wanted.items()):
                raise ValueError('Selected input pin differs')
            for name,data in files.items():
                output = directory/Path(name).name; output.write_bytes(data)
            frame = directory/'world-0001.bin'
            source = (433*800+563)*2; target_pixel = (433*800+548)*2
            # Six input planes stay in this private original-only namespace.
            planes = {'before':files[prefix+'before.565'], 'zero':bytes(960000)}
            for label,at,value in [('source-zero',source,0),('source-sentinel',source,0xffff),
                                   ('target-zero',target_pixel,0),('neighbor-sentinel',source+2,0xffff)]:
                plane = bytearray(planes['before']);struct.pack_into('<H',plane,at,value);planes[label] = bytes(plane)
            flags = ['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie']
            for label,name in [('baseline','world-frame-reference.cpp'),('watch','world-frame-dependency-reference.cpp')]:
                command = flags+[str(ROOT/'tests'/name),'-o',str(directory/label)]
                built = subprocess.run(command, capture_output=True, check=True, timeout=120)
                result.setdefault('binaries',{})[label] = dict(sha256=sha((directory/label).read_bytes()),command=command)
            outputs = {}
            for label,plane in planes.items():
                input_path=directory/(label+'.entry.565');input_path.write_bytes(plane)
                output=directory/(label+'.out.565')
                run=subprocess.run([directory/'watch',args.executable,frame,output,input_path],capture_output=True,check=True,timeout=30)
                rows=[]
                for line in run.stderr.decode().splitlines():
                    if not line.startswith('DEPENDENCY '):continue
                    _,record,op,backend,phase,target_value,source_value=line.split()
                    rows.append(dict(record=int(record),operation=int(op),backend=int(backend),phase=phase,
                                     target=int(target_value),source=int(source_value)))
                outputs[label]=output.read_bytes()
                result['cases'][label]=dict(entry_sha256=sha(plane),output_sha256=sha(outputs[label]),
                                           target=struct.unpack_from('<H',outputs[label],target_pixel)[0],
                                           source=struct.unpack_from('<H',outputs[label],source)[0],trace=rows)
            baseline=directory/'baseline.565'
            subprocess.run([directory/'baseline',args.executable,frame,baseline,directory/'before.entry.565'],capture_output=True,check=True,timeout=30)
            result['watch_preserves_baseline_output']=baseline.read_bytes()==outputs['before']
            result['matches_retained_live']=outputs['before']==files[prefix+'after.565']
            result['matches_retained_zero']=sha(outputs['zero'])==previous['frames'][0]['native_sha256']
            result['source_changes_target']=(result['cases']['source-zero']['target']==0 and result['cases']['source-sentinel']['target']==0xffff)
            result['target_entry_value_irrelevant']=outputs['target-zero']==outputs['before']
            result['neighbor_does_not_change_target']=result['cases']['neighbor-sentinel']['target']==6338
            result['counterfactual_differences']={label:plane_differences(outputs['before'],data,800,600) for label,data in outputs.items() if label not in ('before','zero')}
            result['source_value_changes_before_selected_record']=[row for row in result['cases']['before']['trace'] if row['phase']=='after' and row['record']<1261 and row['source']!=6338]
            result['fresh_original_raster_execution']=True
            for label,data in outputs.items():
                name=str(Path(args.report).with_suffix(''))+'-'+label+'.bin'
                result['cases'][label]['output_record']=dict(record=name,sha256=write_report(ROOT,name,data))
            result['success']=all(result[k] for k in ['watch_preserves_baseline_output','matches_retained_live','matches_retained_zero',
                                                     'source_changes_target','target_entry_value_irrelevant','neighbor_does_not_change_target'])
    except Exception as error:
        result['error']=str(error)
    finally:
        if before_verified:
            verified=subprocess.run([ROOT/'tools/original-manifest.sh','verify'],capture_output=True)
            result['original_manifest_verified_before_after']=verified.returncode==0
            result['success'] &= result['original_manifest_verified_before_after']
        result['executable_stable']=sha(args.executable.read_bytes())==BUILD
        result['success'] &= result['executable_stable']
        result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in result['sources'].items())
        result['success'] &= result['sources_stable']
        write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ['success','error','source_changes_target','target_entry_value_irrelevant','neighbor_does_not_change_target','source_value_changes_before_selected_record']}),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
