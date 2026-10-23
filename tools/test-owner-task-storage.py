#!/usr/bin/env python3
"""Compare fallback task storage, planner caller and normal retirement."""
from pathlib import Path
import hashlib
import importlib.util
import json
import subprocess
import sys
from owner_planner_comparison import SUITES, main, ROOT
from coverage_evidence import read_json, write_report

BID='AI.owner-fallback-owned-lifecycle'
SCENARIO='owner-task-storage-20261023'
DIRECTORY='research/runtime/owner-task-storage-results-20261023'
SUITES['task-storage']=dict(
 ranges=[(0x419e40,0x419f88),(0x418050,0x4181b3),(0x415c40,0x416523),(0x423590,0x425c9e),(0x4289e0,0x428b3d),(0x428b40,0x428e32)],
 endpoints=[0x597850,0x597870,0x417580,0x428e40,0x428f80,0x4181d0,0x416fe0,0x476e30],
 scope='',suite='owner-task-storage',
 sources=['tests/owner-task-storage-test.cpp','reconstruction/creature-ai/owner_task_scheduler.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/creature-ai/owner_task_inputs.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/combat/melee.cpp'],wrapper='tools/test-owner-task-storage.py')

def run():
 if '--output' not in sys.argv:raise ValueError('Require --output')
 out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):raise ValueError('Require a new owned working/tests output')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(b for b in register['behaviors'] if b['id']==BID)
 SUITES['task-storage']['scope']=behavior['scope']
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
 try:main('task-storage')
 finally:
  if (out/'report.json').exists():
   report=json.loads((out/'report.json').read_text())
   report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
   report['scope']=behavior['scope']
   report['original_output_used_as_native_input']=False
   report['task_storage_domain']='Four shared sentinel families, valid scalar/POD tree destruction, trivial vectors; owned source vector empty. Caller task-map/work-vector buffers explicitly released by fixture after real task cleanup.'
   report['allocator_tag_offsets']=[0x37,0xa4,0xa5,0xb4,0xb5,0xc4,0xd4,0x101,0x102,0x111,0x112,0x121,0x131]
   report['native_representation_policy']='Zero constructor stack-derived allocator tag bytes; exactly thirteen original offsets excluded. No original output seeds native state.'
   write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
  if out.exists():(out/'.retain').write_text('Original/native/sanitizer task storage execution, immutable declaration, binaries and diagnostics. Preserve failed results too.\n')
if __name__=='__main__':run()
