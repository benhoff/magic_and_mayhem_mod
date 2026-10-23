#!/usr/bin/env python3
"""Compare generated task6 source and feature storage through real sweep cleanup and retirement."""
from pathlib import Path
import hashlib
import importlib.util
import json
import subprocess
import sys
from owner_planner_comparison import SUITES, main, ROOT
from coverage_evidence import read_json, write_report

BID='AI.owner-task6-generated-source-composition'
SCENARIO='task6-source-composition-20261023'
DIRECTORY='research/runtime/task6-source-composition-results-20261023'
SUITES['task6-composition']=dict(
 ranges=[(0x407c10,0x407de3),(0x40a340,0x40a3a8),(0x413330,0x41393b),(0x40f7f0,0x40f903),(0x419990,0x419a0d),(0x419a10,0x419aa0),(0x407650,0x40777c),(0x423590,0x425c9e),(0x4284e0,0x4289d7),(0x431be0,0x4328cc),(0x426070,0x4261a8),(0x4261b0,0x426310),(0x4289e0,0x428b3d),(0x428b40,0x428e32),(0x418050,0x4181b3),(0x4766b0,0x476b33),(0x432900,0x432fb0),(0x514990,0x514c20)],
 endpoints=[0x416d80,0x597850,0x597870,0x476e30,0x50af10,0x50fd50,0x50fef0,0x416fe0],
 scope='',suite='owner-task6-composition',
 sources=['tests/task6-source-composition-test.cpp','reconstruction/creature-ai/owner_group_task.cpp','reconstruction/creature-ai/owner_group_features.cpp','reconstruction/creature-ai/owner_planner_features.cpp','reconstruction/creature-ai/owner_memory_groups.cpp','reconstruction/creature-ai/owner_task_probe.cpp','reconstruction/creature-ai/regional_navigation.cpp','reconstruction/creature-ai/regional_danger.cpp','reconstruction/pathfinding/route_neighbors.cpp','reconstruction/pathfinding/route_search.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/creature-ai/owner_task_inputs.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/combat/melee.cpp','reconstruction/creature-ai/brain_inference.cpp','assets/dat.cpp'],wrapper='tools/test-task6-source-composition.py')

def run():
 if '--output' not in sys.argv:raise ValueError('Require --output')
 out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):raise ValueError('Require a new owned working/tests output')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(b for b in register['behaviors'] if b['id']==BID)
 SUITES['task6-composition']['scope']=behavior['scope']
 declaration_path=ROOT/DIRECTORY/(out.name+'-inputs.json')
 result_path=ROOT/DIRECTORY/(out.name+'.json')
 if declaration_path.exists() or result_path.exists():raise ValueError('Immutable result identity already exists')
 if '--claims' in sys.argv:
  claims_path=Path(sys.argv[sys.argv.index('--claims')+1]);declaration=json.loads(claims_path.read_text())
 else:
  spec=importlib.util.spec_from_file_location('task_storage_declarations',ROOT/'tools/draft-coverage-claims.py')
  module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
  declaration=module.declaration(ROOT,register,[BID],[SCENARIO])
  # Existing comparison runner consumes a physical declaration. This bounded
  # interchange file has one consumer; the durable declaration is SQLite.
  interchange=ROOT/'working/tests'/('claims-'+out.name+'.json')
  with interchange.open('x') as f:json.dump(declaration,f,indent=2)
  claims_path=interchange;sys.argv.extend(['--claims',str(claims_path)])
 base=subprocess.check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip()
 changes={}
 for name,digest in declaration['sources'].items():
  raw=(ROOT/name).read_bytes();old=subprocess.run(['git','-C',str(ROOT),'show',base+':'+name],capture_output=True)
  if old.returncode or hashlib.sha256(old.stdout).hexdigest()!=digest:changes[name]={'sha256':digest,'text':raw.decode()}
 declaration.update(base_commit=base,changed_inputs=changes)
 bound=write_report(ROOT,declaration_path,(json.dumps(declaration,indent=2)+'\n').encode())
 try:main('task6-composition')
 finally:
  if (out/'report.json').exists():
   report=json.loads((out/'report.json').read_text())
   report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
   report['scope']=behavior['scope']
   report['original_output_used_as_native_input']=False
   report['ownership_domain']='Generated native groups materialize into actual common task source-vector records, alongside populated72 feature buffers. Real original413330 uncached/cache-hit production and40f7f0 clone/factories,423590 planning/replacement/body,426070 creation,40a340 end-of-sweep source cleanup and418050/428b40 retirement are compared. Native logical probe storage and intermediate cursor bytes remain separate; authored World/model and external service boundaries persist.'
   report['source_byte_domain']={'included_ranges':[[0,61],[137,141]],'excluded_ranges':[[61,137]],'tag_offset_normalized':37,'pointer_fields_normalized':[41,45,49,53,57]}
   report['native_representation_policy']='Preserve anonymous allocator-provenance scalar fields; zero anonymous byte25 tag. Check actor tag against borrowed-pointer lowbyte before normalization. Normalize only checked live owned source,borrowed snapshot/live actor and vector identities; roots are invalid after sweep cleanup; root identity is checked only in populated owned vectors, saved-source identity only after a current-sweep save or in an active task; exclude intermediate cursor/probe storage. Baseline suffix retention is unchanged.'
   write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
  if out.exists():(out/'.retain').write_text('Original/native/sanitizer task storage execution, immutable declaration, binaries and diagnostics. Preserve failed results too.\n')
if __name__=='__main__':run()
