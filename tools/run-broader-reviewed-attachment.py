#!/usr/bin/env python3
"""Run a prospectively reviewed attachment controller with one exact setup guard fix."""
import argparse,hashlib,importlib.util,json,os,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader,write_report
from broader_command_history_launch import WorldBinding as BaseBinding
from broader_attachment_plan_path import adapt
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'

class Binding(BaseBinding):
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs);self.guard=None;self.adapted=set()
    def patch(self,module):
        if Path(getattr(module,'__file__','/')).name=='run-scene-attachment.py'and id(module)not in self.adapted:
            self.adapted.add(id(module));path=ROOT/'tools/run-scene-attachment.py'
            code,raw,self.guard=adapt(path.read_bytes(),str(path),self.phase_name)
            self.guard['source_record']=dict(record=P+Path(self.report).stem+'-controller-original.bin',sha256=write_report(ROOT,P+Path(self.report).stem+'-controller-original.bin',path.read_bytes()))
            n=P+Path(self.report).stem+'-adapted-controller.bin'
            self.guard['adapted_source']=dict(record=n,sha256=write_report(ROOT,n,raw))
            exec(code,module.__dict__)
        return super().patch(module)
    def receipt(self):
        q=super().receipt();require(self.guard is not None,'Missing exact attachment plan guard adaptation');q['controller_plan_path_adaptation']=self.guard;return q

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--plan',required=True);parser.add_argument('--report',required=True);a=parser.parse_args()
    with Reader(ROOT)as r:
        phase=r.json(a.plan);require(phase['success']and phase['family']=='attachment'and phase['attachment_plan_guard_review'],'Unreviewed attachment setup adaptation')
        review=phase['attachment_plan_guard_review'];require(r.digest(review['record'])==review['sha256'],'Attachment setup review changed')
        v=r.json(review['record']);require(v['success']and v['original_game_runs']==0 and v['Native_sources_unchanged']and all(r.digest(n)==h for n,h in v['sources'].items()),'Stale attachment setup controls')
        case=next(c for c in phase['broader_cases']if c['report']==a.report);require(a.report.startswith(phase['collector_prefix'])and a.plan.startswith(P),'Unbound report/phase')
        sources={n:r.digest(n)for n in['tools/run-broader-reviewed-attachment.py','tools/broader_attachment_plan_path.py']}
    if os.environ.get('MNM_BROADER_WORLD_BUS_OWNED')!='1':
        runtime=ROOT/'working/broader-player/world-runtime'/Path(a.report).stem;runtime.mkdir(mode=0o700,parents=True,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_WORLD_BUS_OWNED='1',MNM_INTERACTION_BUS_OWNED='1',MNM_PAUSE_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon');env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    path=ROOT/'tools/run-broader-history-world.py';spec=importlib.util.spec_from_file_location('reviewed_attachment_history',path);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core);core.WorldBinding=Binding
    original=coverage_evidence.write_report
    def writer(root,path,raw):
        if str(path)==a.report:
            q=json.loads(raw);q['sources'].update(sources);q['sources_stable']&=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());q['success']&=q['sources_stable'];q['attachment_setup_review']=review;raw=(json.dumps(q,indent=2)+'\n').encode()
        return original(root,path,raw)
    coverage_evidence.write_report=writer
    try:return core.main()
    finally:coverage_evidence.write_report=original
if __name__=='__main__':raise SystemExit(main())
