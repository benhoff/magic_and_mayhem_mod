#!/usr/bin/env python3
"""Run the DRAW-02 suite and report independent reference and native parity gates."""
import argparse
import json
from pathlib import Path
import subprocess
from drawing_corpus import ROOT, local, sha, load_tool
from drawing_journeys import CATALOG, admit_suite, coverage, inventory


def run(root,catalog_path,*,surface=None,world=None,scene=None,session=None,recorded_native=None,recorded_presenter=None,headless=False,recorded_output_parent=None):
    catalog_path=local(root,catalog_path);catalog_bytes=catalog_path.read_bytes();catalog=json.loads(catalog_bytes)
    admit_suite(root,catalog)
    register_bytes=(root/'research/runtime/coverage/register.json').read_bytes();register=json.loads(register_bytes)
    observations=[];results=[];errors=[]
    for case in catalog['cases']:
        path=case['catalog']['path'];child=json.loads(local(root,path).read_text())
        admitted_rows=[];reference_admitted=False
        try:
            if case['adapter']=='components':
                from drawing_corpus import admit,check
                admitted=admit(root,child,register)
                for c,identity,_,_ in admitted:
                    admitted_rows.append({'id':case['id']+'-'+c['id'],'scenario':c['scenario'],
                        'boundary':'operation' if c['kind']=='surface' else 'world_canvas',
                        'independent_reference':True,'frames':1,'contiguous':False,'actions_bound':False,
                        'requirements':[],'operations':[],'outcome':'pending'})
                reference_admitted=True
                result=check(root=root,catalog_path=path,surface_executable=surface,world_executable=world,headless=headless) if surface and world else None
                for i,row in enumerate(admitted_rows):
                    row['outcome']='matching' if result and result['cases'][i]['comparison']['matching'] else 'mismatch' if result else 'unsupported'
            elif case['adapter'] in ('scene','producer_sequence'):
                if case['adapter']=='scene':
                    from drawing_scene_corpus import admit,check
                else:
                    from drawing_sequence import admit,check
                _,_,planes=admit(root,child,register)
                admitted_rows.append({'id':case['id'],'scenario':'quick-battle-entry','boundary':'composed_canvas',
                    'independent_reference':True,'frames':len(planes),'contiguous':False,'actions_bound':False,
                    'requirements':[],'operations':[],'outcome':'pending'})
                reference_admitted=True
                result=check(root=root,catalog_path=path,executable=scene,headless=headless) if scene else None
                admitted_rows[0]['outcome']='matching' if result and result['success'] else 'mismatch' if result else 'unsupported'
            elif case['adapter']=='publication':
                from drawing_publication_corpus import admit,check
                _,_,_,planes=admit(root,child,register)
                admitted_rows.append({'id':case['id'],'scenario':'camera-motion','boundary':'observed_primary_blt',
                    'independent_reference':True,'frames':len(planes),'contiguous':True,'actions_bound':False,
                    'requirements':[],'operations':[],'outcome':'pending'})
                reference_admitted=True
                result=check(root=root,catalog_path=path,executable=scene,headless=headless) if scene else None
                admitted_rows[0]['outcome']='matching' if result and result['success'] else 'mismatch' if result else 'unsupported'
            elif case['adapter'] in ('recorded_session','recorded_action_session'):
                from drawing_recorded_session import admit,check
                _,_,frames,_=admit(root,child,register)
                admitted_rows.append({'id':case['id'],'scenario':'save-load' if case['adapter']=='recorded_action_session' else 'quick-battle-entry','boundary':child['boundary'],
                    'independent_reference':True,'frames':len(frames),'contiguous':True,'actions_bound':case['adapter']=='recorded_action_session',
                    'reference_profile':child.get('reference_profile',{'name':'legacy_unmodified_upload'}),
                    'requirements':[],'operations':[],'outcome':'pending'})
                reference_admitted=True
                result=check(root=root,catalog_path=path,native=recorded_native,presenter=recorded_presenter,headless=headless,output_parent=recorded_output_parent) if recorded_native and recorded_presenter else None
                admitted_rows[0]['outcome']=result['outcome'] if result else 'unsupported'
            else:
                from drawing_session import admit,check
                _,frames,_=admit(root,child,register)
                admitted_rows.append({'id':case['id'],'scenario':child['scenario'],'boundary':'presented_frame',
                    'independent_reference':True,'frames':len(frames),'contiguous':True,'actions_bound':True,
                    'requirements':child['requirements'],'operations':child['operations'],'outcome':'pending'})
                reference_admitted=True
                result=check(root=root,catalog_path=path,executable=session,headless=headless) if session else None
                admitted_rows[0]['outcome']=result['outcome'] if result else 'unsupported'
            observations.extend(admitted_rows)
            outcome='matching' if all(row['outcome']=='matching' for row in admitted_rows) else next(row['outcome'] for row in admitted_rows if row['outcome']!='matching')
            results.append({'id':case['id'],'adapter':case['adapter'],'outcome':outcome,
                            'result':result,'reference_admitted':True})
        except (ValueError,OSError,KeyError,TypeError,subprocess.TimeoutExpired) as e:
            # Valid original acquisition survives a native backend failure.
            # Invalid original artifacts never contribute reference coverage.
            for row in admitted_rows:row['outcome']='error'
            observations.extend(admitted_rows if reference_admitted else [])
            errors.append({'id':case['id'],'error':str(e),'reference_admitted':reference_admitted})
            results.append({'id':case['id'],'adapter':case['adapter'],'outcome':'error','reference_admitted':reference_admitted,'error':str(e)})
    if catalog_path.read_bytes()!=catalog_bytes or (root/'research/runtime/coverage/register.json').read_bytes()!=register_bytes:
        raise ValueError('Journey suite/register changed during execution')
    admit_suite(root,catalog)
    state=coverage(observations)
    if errors:state['draw02_complete']=False
    pixels=sum((c.get('result') or {}).get('pixels_compared',0) for c in results)
    mismatches=sum((c.get('result') or {}).get('mismatches',0) for c in results)
    return {'schema':1,'suite_sha256':sha(catalog_bytes),'success':not errors and all(c['outcome']=='matching' for c in results),
            'cases':results,'observations':observations,'errors':errors,'pixels_compared':pixels,'mismatches':mismatches,
            'original_pixels_used_as_native_inputs':False,'original_work_bypassed':False,
            'new_original_execution':False,**state}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--suite',default=CATALOG);p.add_argument('--report',type=Path,required=True)
    p.add_argument('--inventory',action='store_true',help='Report acceptance requirements without claiming comparisons')
    for name in ('surface','world','scene','session'):p.add_argument('--'+name+'-executable',type=Path)
    p.add_argument('--recorded-native-executable',type=Path)
    p.add_argument('--recorded-presenter-executable',type=Path)
    p.add_argument('--recorded-output-parent',type=Path,help='Owned output directory under working for recorded-session artifacts')
    p.add_argument('--claims',type=Path)
    p.add_argument('--run-guards',action='store_true')
    p.add_argument('--headless',action='store_true');p.add_argument('--require-reference-complete',action='store_true')
    p.add_argument('--require-complete',action='store_true')
    a=p.parse_args()
    try:
        output=load_tool('check-drawing-corpus').output_path(a.report)
        # Preserve a source fingerprint for every adapter, backend and fixture
        # used by the suite; inventory is explicitly static accounting.
        fingerprints=load_tool('check-drawing-corpus').sources()
        fingerprints.update(load_tool('check-drawing-scene').sources())
        if (ROOT/'tests/fixtures/drawing-sequence').exists():
            fingerprints.update(load_tool('check-drawing-sequence').sources())
        for directory in ('tests/fixtures/drawing-journeys','tests/fixtures/drawing-publication','tests/fixtures/drawing-recorded-session','tests/fixtures/drawing-storage-session','tests/fixtures/drawing-storage-corrected-session'):
            for path in (ROOT/directory).rglob('*'):
                if path.is_file():fingerprints[str(path.relative_to(ROOT))]=sha(path.read_bytes())
        for name in ('tools/check-drawing-journeys.py','tools/drawing_journeys.py','tools/drawing_session.py','tools/retain-drawing-session.py','tests/test-drawing-journeys.py','tests/test-drawing-session.py','tools/drawing_publication_corpus.py','tools/retain-drawing-publication.py','tools/drawing_publication.py','tests/test-drawing-publication.py','tests/test-drawing-publication-corpus.py'):
            if (ROOT/name).is_file():fingerprints[name]=sha((ROOT/name).read_bytes())
        for name in ('tools/drawing_copy_order.py','tools/drawing_egl_gpu_reference.py','tools/drawing_egl_destination.py','tools/drawing_recorded_session.py','tools/recorded_drawing_plan.py','tools/compare-drawing-quick-battle.py','tools/check-recorded-drawing-session.py','tools/retain-recorded-drawing-session.py','apps/drawing-session/main.cpp','apps/drawing-session/recorded_composition.hpp'):
            fingerprints[name]=sha((ROOT/name).read_bytes())
        claims=json.loads(a.claims.read_text()) if a.claims else None
        if claims:
            if not isinstance(claims['claims'],list) or not claims['claims']:
                raise ValueError('Execution claims must be a nonempty list')
            for name,digest in claims['sources'].items():
                if sha(local(ROOT,name).read_bytes())!=digest:raise ValueError('Prospective journey source changed')
                fingerprints[name]=digest
        tests={}
        if a.run_guards:
            for name in ('drawing-journeys','drawing-session','drawing-publication','drawing-publication-corpus'):
                import sys
                execution=subprocess.run([sys.executable,'-B',str(ROOT/'tests'/('test-'+name+'.py'))],capture_output=True,timeout=120)
                if execution.returncode:raise ValueError('Journey guard failed: '+execution.stderr.decode(errors='replace'))
                tests[name]={'success':True,'log_sha256':sha(execution.stdout+execution.stderr)}
        result=inventory(ROOT,a.suite) if a.inventory else run(ROOT,a.suite,surface=a.surface_executable,
            world=a.world_executable,scene=a.scene_executable,session=a.session_executable,recorded_native=a.recorded_native_executable,recorded_presenter=a.recorded_presenter_executable,headless=a.headless,recorded_output_parent=a.recorded_output_parent)
        if any(sha((ROOT/n).read_bytes())!=h for n,h in fingerprints.items()):
            raise ValueError('Journey sources/fixtures changed during execution')
        result.update(sources=fingerprints,sources_stable=True,execution_performed=not a.inventory,tests=tests)
        if claims:result['claims']=claims['claims']
        output.parent.mkdir(parents=True,exist_ok=True)
        with output.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
        print(json.dumps({k:result[k] for k in ('reference_complete','draw02_complete','execution_performed')}))
        if result.get('errors'):return 2
        return int((not a.inventory and not result['success']) or
                   a.require_reference_complete and not result['reference_complete'] or
                   a.require_complete and not result['draw02_complete'])
    except (ValueError,OSError,KeyError,TypeError,subprocess.TimeoutExpired) as e:p.exit(2,str(e)+'\n')

if __name__=='__main__':raise SystemExit(main())
