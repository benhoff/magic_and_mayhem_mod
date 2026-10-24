#!/usr/bin/env python3
"""Compare whole-brain normal destruction, shared sentinel lifetimes, source aliases and all eight owner rows."""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import subprocess
import sys
from owner_planner_comparison import SUITES, main, ROOT
from coverage_evidence import read_json, write_report

BID='AI.owner-brain-teardown'
SCENARIO='whole-brain-teardown-20261023'
DIRECTORY='research/runtime/whole-brain-teardown-results-20261023'
SUITES['brain-teardown']=dict(
 ranges=[(0x415ac0,0x415c39),(0x4c0920,0x4c0b62),(0x4c0b70,0x4c0bca),(0x4c0e30,0x4c0ec5),(0x514810,0x51485f),(0x418050,0x4181b3),(0x407650,0x407707),(0x505f70,0x50607c)],
 endpoints=[0x597850,0x597870,0x5979b0],
 scope='',suite='owner-whole-brain-teardown',
 sources=['tests/whole-brain-teardown-test.cpp'],wrapper='tools/test-whole-brain-teardown.py')

def run():
 if '--output' not in sys.argv:raise ValueError('Require --output')
 out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
 scratch=ROOT/'working/compiler-tmp';scratch.mkdir(parents=True,exist_ok=True)
 os.environ['TMPDIR']=str(scratch)
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):raise ValueError('Require a new owned working/tests output')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(b for b in register['behaviors'] if b['id']==BID)
 SUITES['brain-teardown']['scope']=behavior['scope']
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
 try:main('brain-teardown')
 finally:
  if (out/'report.json').exists():
   report=json.loads((out/'report.json').read_text())
   report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
   report['scope']=behavior['scope']
   report['original_output_used_as_native_input']=False
   report['ownership_domain']='Independent checked value-address fixture with two ordinary brains0/1,125 actor slots,all eight owner rows,balanced0/1/3-node observation trees,cache forests,task-owned forests,overlapping primary/secondary source vectors,pending task map and borrowed alias map,scalar trees,inline sources and0/1/1024/1025-element deques. Actual whole4c0920 and415ac0 execute with only allocation/free/buffer-free service redirects. Compare complete final brain bytes,exact allocation release order,remaining shared sentinel counters,other-brain preservation,actor assignments,canaries and zero surviving owned allocations.'
   report['native_representation_policy']='Checked PE32 value-address teardown is build-specific reconstruction,not a portable RB insertion/iterator ABI or native session integration. Independently authored valid balanced trees and source/task constructors are fixture inputs; original constructor/reset/scheduling,other task families,arbitrary malformed/cyclic ownership,allocation failure/SEH and live replacement excluded. Native owner0/1 admission refusal occurs before releases. Final destructor leaves enclosing brain and actor pool borrowed.'

   write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
  if out.exists():(out/'.retain').write_text('Original/native/sanitizer task storage execution, immutable declaration, binaries and diagnostics. Preserve failed results too.\n')
if __name__=='__main__':run()
