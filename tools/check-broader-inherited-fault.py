#!/usr/bin/env python3
"""Run unchanged fault assertions with explicit pinned-parent input resolution."""
import argparse,hashlib,importlib.util,json,sys
from pathlib import Path
from types import SimpleNamespace
from coverage_evidence import write_report
from broader_input_plan_inheritance import input_binding
from broader_player_sources import python_sources
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--provider',choices=['nvidia','mesa'],required=True);known,remaining=p.parse_known_args();sys.argv=[sys.argv[0],*remaining]
 target='check-broader-worker-hardware-case.py'if known.provider=='nvidia'else'check-broader-worker-player-case.py';spec=importlib.util.spec_from_file_location('inherited_fault_checker',ROOT/'tools'/target);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
 if known.provider=='mesa':core.input_binding=input_binding
 else:
  original_spec=importlib.util.spec_from_file_location
  def selecting_spec(name,path,*a,**kw):
   result=original_spec(name,path,*a,**kw)
   if Path(path).name=='check-broader-worker-player-case.py':
    original_loader=result.loader
    class Loader:
     def create_module(self,spec):return original_loader.create_module(spec)
     def exec_module(self,module):original_loader.exec_module(module);module.input_binding=input_binding
    result.loader=Loader()
   return result
  core.importlib=SimpleNamespace(util=SimpleNamespace(spec_from_file_location=selecting_spec,module_from_spec=importlib.util.module_from_spec))
 names=python_sources(ROOT,['tools/check-broader-inherited-fault.py','tools/broader_input_plan_inheritance.py'])
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in names}
 def writer(root,path,raw):
  value=json.loads(raw);value.setdefault('sources',{}).update(sources);stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in value['sources'].items());value['sources_stable']=value.get('sources_stable',True)and stable;value['success']=value['success']and stable;value['input_reference_metadata_reanalysis']=dict(original_game_runs=0,new_native_executions=0,changed_original_report=False,all_other_fault_assertions_delegated_unchanged=True);return write_report(root,path,(json.dumps(value,indent=2)+'\n').encode())
 core.write_report=writer
 return core.main()
if __name__=='__main__':raise SystemExit(main())
