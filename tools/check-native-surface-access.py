#!/usr/bin/env python3
"""Replay native owned access state against independent real Surface2 outputs."""
import argparse,importlib.util,json,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('a',ROOT/'tools/check-surface-access.py');a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);args=p.parse_args()
 if args.report.exists():p.error('Refusing retained report overwrite')
 cap,data=a.load();a.compare(data);names=['renderer/surface_access.hpp','tests/render-surface-access-test.cpp','tools/check-native-surface-access.py','tools/check-surface-access.py','tools/capture-surface-access.py',str(a.DEFAULT.relative_to(ROOT)),cap['corpus']['path']];sources={n:a.sha(ROOT/n) for n in names};parent=ROOT/'working/tests/native-surface-access';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));manifest=run/'inputs.txt';manifest.write_text(str(len(data['rows']))+'\n'+''.join(' '.join(str(v) for v in [c['id'],c['bits'],c['caps'],len(c['operations']),*c['operations']])+'\n' for c in (r['input'] for r in data['rows'])));binary=run/'native';output=run/'outputs.bin'
 subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),str(ROOT/'tests/render-surface-access-test.cpp'),'-o',str(binary)],check=True);proc=subprocess.run([str(binary),str(manifest),str(output)],capture_output=True,text=True,timeout=30);(run/'native.log').write_text(proc.stdout+proc.stderr);a.require(proc.returncode==0,proc.stdout+proc.stderr);native=json.loads(proc.stdout);raw=output.read_bytes();at=0;fields=0;pixels=0;excluded=0
 for row in data['rows']:
  for op,expected in zip(row['input']['operations'],row['steps']):
   values=list(struct.unpack_from('<29I',raw,at));at+=116;a.require(values[:2]==[expected['result'],expected['dc_out_state']],'Native admission/output result differs');n=26 if op==0 and values[0] else 27;a.require(values[2:2+n]==expected['descriptor'][:n],'Native lock descriptor differs');fields+=n if op==0 else 0;excluded+=int(n==26)
  values=list(struct.unpack_from('<28I',raw,at));at+=112;expected=row['final_lock'];a.require(values[0]==expected['result'],'Native terminal admission differs');n=26 if values[0] else 27;a.require(values[1:1+n]==expected['descriptor'][:n],'Native terminal descriptor differs');fields+=n;excluded+=int(n==26)
  if values[0]==0:
   words=list(struct.unpack_from('<48I',raw,at));at+=192;a.require(words==row['pixels'],'Native final pixels differ');pixels+=48
 a.require(at==len(raw),'Native output extent differs');a.require(all(a.sha(ROOT/n)==h for n,h in sources.items()),'Sources changed during comparison');out=dict(schema=1,success=True,sources=sources,native=native,driver_descriptor_field_comparisons=fields,unstable_failure_caps_excluded=excluded,driver_pixel_comparisons=pixels,inputs_sha256=a.sha(manifest),binary_sha256=a.sha(binary),outputs_sha256=a.sha(output),artifacts=str(run.relative_to(ROOT)),scope='Owned single-thread SurfaceAccessState replay compares all independent driver HRESULTs/output mutations and stable descriptor fields. Native tokens/packed storage/layout/budget policies; signed terminal mapping debt explicitly represented. Final native bytes remain untouched. No actual COM/HDC/GPU raster/drawing admission/alias/cross-thread/game/loss/Windows/live equivalence.')
 with args.report.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
 print(json.dumps(dict(native=native,descriptor_fields=fields,pixels=pixels,unstable_caps_excluded=excluded)))
if __name__=='__main__':main()
