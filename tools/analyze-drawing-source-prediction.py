#!/usr/bin/env python3
"""Separate retained CPU-source equality from physical display equality."""
import argparse
import importlib.util
import json
from pathlib import Path
from drawing_api_session import sha
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prediction',type=Path,required=True);p.add_argument('--reference',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 prediction=json.loads(a.prediction.read_text());reference=json.loads(a.reference.read_text());claims=json.loads(a.claims.read_text())
 if prediction['reference']['sha256']!=sha(a.reference.read_bytes()) or reference['success'] is not True or prediction['native_exit']!=0:raise ValueError('Independent retained comparison identities required')
 sources=dict(claims['sources']);sources.update({n:sha((ROOT/n).read_bytes()) for n in ('tools/analyze-drawing-source-prediction.py','tools/inspect-canvas-producers.py')})
 if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
 spec=importlib.util.spec_from_file_location('producers',ROOT/'tools/inspect-canvas-producers.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
 capture=Path(reference['experiment'])/'capture';raw=(capture/'canvas-producers.bin').read_bytes()
 if sha(raw)!=reference['producer_observation']['producer_journal_sha256']:raise ValueError('Retained original producer changed')
 _,records=module.decode(raw);comparisons=[];excluded=[]
 for i,frame in enumerate(prediction['comparisons'],1):
  boundary=frame['boundary'];record=records[boundary['sequence']-1][0]
  if record[2]!=10 or record[3]!=boundary['canvas'] or record[5:7]!=(boundary['width'],boundary['height']):
   excluded.append(dict(ordinal=frame['ordinal'],reason='prefix-does-not-end-at-exact-source-checkpoint'));continue
  native_path=ROOT/prediction['artifacts']/f'native/frame-{i:04d}.565';native=native_path.read_bytes();original_path=capture/f'producer-oracle-{record[14]:04d}.565';original=original_path.read_bytes()
  if sha(native)!=prediction['native']['frames'][i-1]['sha256'] or len(native)!=len(original) or len(original)!=boundary['width']*boundary['height']*2:raise ValueError('Retained native/original full CPU plane identity changed')
  physical=(capture/f"api-physical-oracle-{frame['ordinal']:06d}.rgba").read_bytes()
  if sha(physical)!=frame['original_sha256']:raise ValueError('Retained physical plane changed')
  differences=sum(original[x:x+2]!=native[x:x+2] for x in range(0,len(original),2))
  comparisons.append(dict(ordinal=frame['ordinal'],sequence=boundary['sequence'],oracle=record[14],pixels=len(original)//2,cpu_different_pixels=differences,physical_different_pixels=frame['different_pixels'],original_cpu_sha256=sha(original),native_cpu_sha256=sha(native),physical_uniform_rgb=all(physical[x:x+3]==physical[:3] for x in range(0,len(physical),4))))
 stable=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items())
 report=dict(schema=1,success=stable,sources=sources,sources_stable=stable,claims=claims['claims'],prediction=dict(path=str(a.prediction),sha256=sha(a.prediction.read_bytes())),reference=dict(path=str(a.reference),sha256=sha(a.reference.read_bytes())),comparisons=comparisons,excluded=excluded,cpu_pixels_compared=sum(c['pixels'] for c in comparisons),cpu_different_pixels=sum(c['cpu_different_pixels'] for c in comparisons),physical_different_pixels=sum(c['physical_different_pixels'] for c in comparisons),original_game_executed=False,native_reexecuted=False,original_pixels_used_as_native_inputs=False,coherent_presented_frames=False,complete_journey_coverage=False,scope='Offline hash-bound comparison of retained native output and independent original CPU checkpoints at identical source prefixes; physical comparison remains independently negative where recorded. No fresh engine execution, all-writer ownership, actual display presentation, journey or replacement claim.')
 a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ('success','cpu_pixels_compared','cpu_different_pixels','physical_different_pixels')}))
if __name__=='__main__':main()
