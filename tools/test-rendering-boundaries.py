#!/usr/bin/env python3
"""Compare owned palette/count and camera boundaries with pinned original code."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract, required_sources
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());builds={b['id']:b for b in register['builds']}
 behaviors=[b for b in register['behaviors'] if b['id'] in ['RE.palette-effect-builders','RE.terrain-camera-lifecycle']]
 paths={'tools/test-rendering-boundaries.py','tests/rendering-boundary-reference.cpp','tests/sprite-binary-reference.cpp'}
 for b in behaviors:paths.update(required_sources(b))
 sources={n:sha(ROOT/n) for n in sorted(paths)}
 report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds)) for b in behaviors],
             scope='Mode-zero counts 2..256 and clamped camera cell movement only; full effect/camera lifecycle groups remain partial; isolated unchanged PE instructions with a checked padded allocation adapter, no live replacement.')
 def run(name,cmd):
  r=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True,text=True,timeout=180);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r.stdout
 run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  if sha(a.executable)!=BUILD:raise ValueError('Executable hash mismatch')
  binary=out/'reference'
  run('build',['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-I'+str(ROOT/'reconstruction/rendering'),ROOT/'tests/rendering-boundary-reference.cpp',ROOT/'reconstruction/rendering/palette_shading.cpp',ROOT/'reconstruction/rendering/terrain_camera.cpp','-o',binary])
  report['result']=json.loads(run('reference',[binary,a.executable.resolve()]));report['success']=report['result']['success'];report['binary_sha256']=sha(binary);report['original_sha256']=BUILD
 except Exception as e:report['error']=str(e)
 finally:
  run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report.get('result',report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
