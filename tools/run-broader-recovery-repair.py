#!/usr/bin/env python3
"""Use the unchanged recovery wrapper with explicit existing repair ancestry."""
import argparse,hashlib,importlib.util,json,os,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from broader_repair_binding import arguments
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('plan','report','repair-of','repair-reason'):p.add_argument('--'+n,required=True)
 args,rest=p.parse_known_args();require(rest==['--kind','transition','--provider','nvidia'],'Only the declared wide NVIDIA transition replacement')
 if os.environ.get('MNM_BROADER_RECOVERY_BUS_OWNED')!='1':
  runtime=ROOT/'working/broader-player/recovery-runtime'/Path(args.report).stem;runtime.mkdir(parents=True,mode=0o700)
  env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_RECOVERY_BUS_OWNED='1',MNM_TRANSITION_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon');env.pop('AT_SPI_BUS_ADDRESS',None);os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
 with Reader(ROOT)as r:
  plan=r.json(args.plan);case=next(v for v in plan['broader_cases']if v['report']==args.report);require(plan['success']and case['repair_of']==args.repair_of and case['repair_reason']==args.repair_reason and case['size']==[800,600]and case['provider']=='nvidia','Unbound explicit repair')
  sources={n:r.digest(n)for n in('tools/run-broader-recovery-repair.py','tools/broader_repair_binding.py')}
 filename=ROOT/'tools/run-broader-recovery.py';original_spec=importlib.util.spec_from_file_location;spec=original_spec('broader_unchanged_recovery_wrapper',filename);base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
 bindings=[]
 def load(name,path,*a,**k):
  spec=original_spec(name,path,*a,**k)
  if Path(path).resolve()==ROOT/'tools/run-rendering-transition.py':
   delegate=spec.loader
   class Loader:
    def create_module(self,s):return delegate.create_module(s)
    def exec_module(self,m):
     delegate.exec_module(m);original_main=m.main
     def main(*positionals,**keywords):
      old=list(sys.argv);sys.argv=arguments(ROOT,old,args.repair_of,args.repair_reason);bindings.append(dict(original_arguments=old,actual_arguments=list(sys.argv),controller_source_sha256=hashlib.sha256(Path(path).read_bytes()).hexdigest()));return original_main(*positionals,**keywords)
     m.main=main
    def __getattr__(self,n):return getattr(delegate,n)
   spec.loader=Loader()
  return spec
 original_write=coverage_evidence.write_report
 def writer(root,path,raw):
  if str(path)==args.report:
   q=json.loads(raw);q['sources'].update(sources);q['repair_ancestry_binding']=dict(repair_of=args.repair_of,repair_reason=args.repair_reason,bindings=bindings,original_controller_actions_and_assertions_unchanged=True)
   q['sources_stable']&=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());q['success']&=q['sources_stable']and len(bindings)==1;raw=(json.dumps(q,indent=2)+'\n').encode()
  return original_write(root,path,raw)
 try:
  importlib.util.spec_from_file_location=load;coverage_evidence.write_report=writer;sys.argv=[str(filename),*rest,'--plan',args.plan,'--report',args.report];return base.main()
 finally:importlib.util.spec_from_file_location=original_spec;coverage_evidence.write_report=original_write
if __name__=='__main__':raise SystemExit(main())
