#!/usr/bin/env python3
"""Review exact auxiliary driver drift by replaying retained high-mode decisions."""
import argparse,ast,hashlib,io,json,os,sys,tempfile,types
from pathlib import Path
from unittest.mock import patch
from PIL import Image
from coverage_evidence import Reader,write_report
from rendering_normal_acceptance import PREFIX,DRIVER_SOURCES
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def canonical(row):
    return dict(actions=row['tutorial_actions'],escape=row['escape_sent'],point=row['logical_point'],events=[{k:v for k,v in e.items() if k in ('event','button','down','point')} for e in row['events'] if e['event']!='complete'])
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--before',nargs=2,action='append',required=True);p.add_argument('--execution',nargs=2,action='append',required=True);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'New immutable report required')
    before=dict(a.before);require(set(before)==DRIVER_SOURCES,'Only the two auxiliary driver sources may change')
    with Reader(ROOT) as reader,tempfile.TemporaryDirectory(prefix='mnm-driver-replay-') as directory:
        old={n:reader.read_bytes(r) for n,r in before.items()};changes={n:dict(before=hashlib.sha256(raw).hexdigest(),after=reader.digest(n),before_record=before[n]) for n,raw in old.items()}
        old_ast=ast.parse(old['tools/campaign_storage.py']);new_ast=ast.parse((ROOT/'tools/campaign_storage.py').read_bytes())
        functions=lambda tree:{n.name:ast.dump(n,include_attributes=False) for n in tree.body if isinstance(n,(ast.FunctionDef,ast.AsyncFunctionDef))}
        old_functions,new_functions=functions(old_ast),functions(new_ast)
        require({k:v for k,v in old_functions.items() if k!='validate_mini_inputs'}=={k:v for k,v in new_functions.items() if k!='validate_mini_inputs'},'An unrelated storage function changed')
        for name in ('HERMES_CARD_RECT','HERMES_POWER_TITLE_RECT'):
            values=lambda tree:[ast.literal_eval(n.value) for n in tree.body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id==name for t in n.targets)]
            require(values(old_ast)==values(new_ast),'Existing calibration changed')
        modules=[]
        for label,raw in [('before',old['tools/menu-storage-input.py']),('current',(ROOT/'tools/menu-storage-input.py').read_bytes())]:
            module=types.ModuleType('driver_'+label);module.__file__=str(ROOT/'tools/menu-storage-input.py');exec(compile(raw,module.__file__,'exec'),module.__dict__);modules.append(module)
        class Function:
            def __call__(self,*args):return 1
        library=types.SimpleNamespace(**{n:Function() for n in ['XOpenDisplay','XFlush','XCloseDisplay','XKeysymToKeycode','XTestFakeMotionEvent','XTestFakeButtonEvent','XTestFakeKeyEvent']})
        eligible=[];replays=[];comparisons=[]
        for an,cn in a.execution:
            acquired,comp=reader.json(an),reader.json(cn);ar=dict(record=an,sha256=reader.digest(an))
            require(acquired['success'] is True and comp['success'] is True and comp['acquisition']==ar,'Original execution/comparison incomplete')
            for n,h in (acquired['sources']|acquired['native_admission']['native_only']['sources']).items():
                require((n in DRIVER_SOURCES and h==changes[n]['before']) or reader.digest(n)==h,'Changed engine or capture source: '+n)
            eligible.append(ar);comparisons.append(dict(record=cn,sha256=reader.digest(cn)))
            if acquired['scenario']=='normal-quick':
                replays.append(dict(acquisition=ar,scope='Qt-only Quick driver does not invoke the Python storage-input helper.',decisions=0));continue
            require(acquired['scenario']=='normal-storage' and acquired['configuration']['difficulty']==1 and comp['world_domain']['extents']==[[800,600]],'Only the recorded high-mode storage corpus is eligible')
            artifacts={r['path']:r for r in acquired['artifacts']};base=Path(acquired['temporary_root']).relative_to(ROOT).as_posix();count=0
            for name,record in sorted(artifacts.items()):
                if not name.startswith(base+'/storage-input-') or not name.endswith('.json'):continue
                require(reader.digest(record['record'])==record['sha256'],'Changed retained input record')
                expected=reader.json(record['record']);image=None
                if expected['tutorial_observations']:
                    image_name=base+'/'+expected['tutorial_observations'][0]['image'];original=artifacts[image_name];raw=reader.read_bytes(original['record']);require(hashlib.sha256(raw).hexdigest()==original['sha256'],'Changed retained image');image=Image.open(io.BytesIO(raw)).copy();require(image.size==(800,600),'Wrong normalized image')
                for index,module in enumerate(modules):
                    output=Path(directory)/f'input-{count}-{index}.json';args=['driver','--rect','0','0','800','600','--mode',expected['mode'],'--output',str(output)]
                    if expected['skip_click']:args.append('--skip-click')
                    if expected['mode']=='mini':args.append('--dismiss-dialogue')
                    with patch('sys.argv',args),patch.dict(os.environ,DISPLAY=':replay-no-server'),patch.object(module.c,'CDLL',return_value=library),patch.object(module.time,'sleep'),patch('PIL.ImageGrab.grab',return_value=image):module.main()
                    require(canonical(json.loads(output.read_text()))==canonical(expected),'Current/before driver changes a retained high-mode decision: '+name)
                count+=1
            require(count>0,'No actual retained input decisions replayed');replays.append(dict(acquisition=ar,scope='Every retained high-mode storage-input decision, using original screenshots and the unchanged800x600 viewport branch.',decisions=count,before_and_current_match=True))
        sources={n:reader.digest(n) for n in [*sorted(DRIVER_SOURCES),'tools/review-rendering-driver-branch.py','tools/rendering_normal_acceptance.py']}
        report=dict(schema=1,kind='scoped_rendering_driver_branch_review',success=True,native_binary_changes_allowed=False,sources=sources,changes=changes,eligible_acquisitions=eligible,original_comparisons=comparisons,replays=replays,scope='Exact two auxiliary source pairs only. Existing rule IDs/rectangles retained; the added low640x480 branch is excluded from high800x600 storage inputs and unused by Qt-only Quick. Native/compiler/capture sources must remain identical. Original capture hashes remain in their immutable receipts; current independent comparison must be rerun. This is no general engine/source reuse permission.')
        h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=True,record=a.report,sha256=h,replays=replays)))
if __name__=='__main__':main()
