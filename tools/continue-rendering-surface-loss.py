#!/usr/bin/env python3
"""Preserve accepted640phases and freeze the corrected800controller, with no new games."""
import argparse,ast,copy,hashlib,io,json,sqlite3,subprocess,tarfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import required_sources
from rendering_transition_assessment import observed_epochs,assess
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/surface-loss-restore-results-20261109/'
ALLOWED={'tools/rendering_surface_loss_live.py','tools/rendering_surface_loss.py',
    'tests/test-rendering-surface-loss.py','tools/close-rendering-surface-loss.py',
    'tools/rendering_transition_assessment.py'}

def syntax(raw):return ast.parse(raw)
def same(a,b):return ast.dump(a,include_attributes=False)==ast.dump(b,include_attributes=False)

def controller_scope(old,new):
    before,after=syntax(old),syntax(new)
    added=[n for n in after.body if isinstance(n,ast.ImportFrom) and n.module=='rendering_transition_live']
    require(len(added)==1 and [(a.name,a.asname) for a in added[0].names]==[('TransitionJourney','Wide')],'Unexpected controller import')
    after.body.remove(added[0]);cls=next(n for n in after.body if isinstance(n,ast.ClassDef) and n.name=='SurfaceLossJourney')
    methods=[n for n in cls.body if isinstance(n,ast.FunctionDef) and n.name=='normal_quit']
    require(len(methods)==1,'Missing extent dispatch')
    expected=syntax('def normal_quit(self):\n if self.size==[640,480]:return super().normal_quit()\n require(self.size==[800,600],"Unsupported surface-loss source extent")\n return Wide.normal_quit(self)\n').body[0]
    require(same(methods[0],expected),'Unreviewed extent dispatch body');cls.body.remove(methods[0])
    require(same(before,after),'Existing640controller behavior changed')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--parent',required=True);p.add_argument('--controls',required=True)
    p.add_argument('--preflight',required=True);p.add_argument('--failed-analysis',required=True)
    p.add_argument('--comparison',action='append',required=True);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh extent continuation required')
    q=dict(schema=1,kind='native_surface_loss_prospective_plan',success=False,original_game_runs=0,newly_passed_requirements=0)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        with Reader(ROOT) as r:
            parent=r.json(a.parent);preflight=r.json(a.preflight);controls=r.json(a.controls);failed=r.json(a.failed_analysis)
            require(parent['success'] and parent['kind']==q['kind'] and preflight['success'] and preflight['original_game_runs']==0 and
                preflight['actual_800_starts']==0 and preflight['parent_plan']==dict(record=a.parent,sha256=r.digest(a.parent)) and
                preflight['refusal']=='Low confirmation controller requires actual640source','Wrong retained zero-game boundary')
            require(controls['success'] and controls['sources_stable'] and controls['original_game_runs']==0 and
                all(r.digest(n)==h for n,h in controls['sources'].items()),'Current extent dispatch/phase and native controls required')
            require(failed['success'] is False and failed['sources_stable'] is True and failed['comparison_processes_stopped'] is True and
                failed['error']=='Wrong provider-loss stopped epoch' and 'epochs' not in failed,'Unexpected initial retained-data refusal')
            preserved=parent['compiler_sources']|parent['runtime_sources']|parent['oracle']['native_sources']
            require(all(r.digest(n)==h for n,h in preserved.items()),'Native/compiler/runtime/oracle changed; this controller-only continuation is inapplicable')
            for binary in [parent['player_binary'],*parent['oracle']['binaries']]:
                require(hashlib.sha256((ROOT/binary['path']).read_bytes()).hexdigest()==binary['sha256']==r.digest(binary['record']),'Exact tested executable changed')
            prior=preserved|parent['sources'];changes={n:dict(before=h,after=r.digest(n)) for n,h in prior.items() if r.digest(n)!=h}
            require(set(changes)==ALLOWED,'Unexpected continuation source delta')
            bundle=tarfile.open(fileobj=io.BytesIO(r.read_bytes(parent['source_phase']['record'])),mode='r:gz')
            old={}
            for name,pair in changes.items():
                body=bundle.extractfile(name).read() if name in parent['source_phase']['members'] else subprocess.check_output(['git','show',parent['source_phase']['source_revision']+':'+name],cwd=ROOT)
                require(hashlib.sha256(body).hexdigest()==pair['before'],'Missing exact phase preimage');old[name]=body
                record=a.report.removesuffix('.json')+'-before-'+hashlib.sha256(name.encode()).hexdigest()[:16]+'.bin'
                require(write_report(ROOT,record,body)==pair['before'],'Preimage custody failure');pair['before_record']=record
            controller_scope(old['tools/rendering_surface_loss_live.py'],(ROOT/'tools/rendering_surface_loss_live.py').read_bytes())
            before,after=syntax(old['tools/rendering_surface_loss.py']),syntax((ROOT/'tools/rendering_surface_loss.py').read_bytes())
            after.body=[n for n in after.body if not(isinstance(n,ast.FunctionDef) and n.name=='source_phase_review')]
            require(same(before,after),'Existing loss receipt/budget decoder changed')
            before,after=syntax(old['tools/rendering_transition_assessment.py']),syntax((ROOT/'tools/rendering_transition_assessment.py').read_bytes())
            after.body=[n for n in after.body if not(isinstance(n,ast.FunctionDef) and n.name=='provider_stopped_header')]
            old_observed=next(n for n in before.body if isinstance(n,ast.FunctionDef) and n.name=='observed_epochs')
            new_observed=next(n for n in after.body if isinstance(n,ast.FunctionDef) and n.name=='observed_epochs')
            old_guard=next(n for n in ast.walk(old_observed) if isinstance(n,ast.Call) and isinstance(n.func,ast.Name) and n.func.id=='require' and
                           len(n.args)==2 and isinstance(n.args[1],ast.Constant) and n.args[1].value=='Wrong provider-loss stopped epoch')
            new_guard=next(n for n in ast.walk(new_observed) if isinstance(n,ast.Call) and isinstance(n.func,ast.Name) and n.func.id=='require' and
                           len(n.args)==2 and isinstance(n.args[1],ast.Constant) and n.args[1].value=='Wrong provider-loss stopped epoch')
            require(same(new_guard.args[0],syntax('provider_stopped_header(h,ring)').body[0].value),'Unreviewed terminal-header branch')
            new_guard.args[0]=old_guard.args[0]
            require(same(before,after),'Other epoch/terminal/input/owner assessment changed')
            before,after=syntax(old['tools/close-rendering-surface-loss.py']),syntax((ROOT/'tools/close-rendering-surface-loss.py').read_bytes())
            for name in ('definition','validate_cases','close_rows'):
                require(same(next(n for n in before.body if isinstance(n,ast.FunctionDef) and n.name==name),
                             next(n for n in after.body if isinstance(n,ast.FunctionDef) and n.name==name)),'Admission conditions changed')
            accepted=[];domains=set()
            for name in a.comparison:
                c=r.json(name);ac=r.json(c['acquisition']['record'])
                require(c['success'] and c['sources_stable'] and c['comparison']['mismatching_pixels']==0 and c['proof']['verified'] and
                    ac['prospective_plan']==dict(record=a.parent,sha256=r.digest(a.parent)) and ac['configuration']['actual_size']==[640,480],
                    'Unaccepted/wrong retained low phase')
                require(all(r.digest(n)==h or n in changes and changes[n]['before']==h and changes[n]['after']==r.digest(n)
                            for n,h in c['sources'].items()),'Unreviewed low execution dependency drift')
                _,epochs,partition=observed_epochs(r,ac,provider_loss=True)
                require(partition==c['partition'] and assess(r,ac,c,epochs,provider_loss=True)==c['proof'],'Raw accepted low scope changed')
                domains.add(ac['configuration']['software_flag']);accepted.append(dict(record=name,sha256=r.digest(name),acquisition=c['acquisition']))
            require(len(accepted)==2 and domains=={False,True},'Both accepted640profiles required')
            require(any(v['acquisition']==failed['acquisition'] for v in accepted),'Failed analysis lost its actual acquisition binding')
            with sqlite3.connect((ROOT/parent['execution_paths']['budget']).as_uri()+'?mode=ro',uri=True) as db:
                rows=list(db.execute('SELECT report,repair_of,charged_games FROM attempts'))
            require(len(rows)==3 and sum(v[2] for v in rows)==6 and sum(bool(v[1]) for v in rows)==1 and not any('800' in v[0] for v in rows),
                    'Cumulative budget changed or second repair attempted')
            require(parent['bounds']['maximum_requests']==6 and parent['bounds']['maximum_original_games']==10 and parent['bounds']['repair_cases']==1,
                    'Original bound changed')
            reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']=='NP.surface-loss-restore-acceptance')
            names=set(prior)|required_sources(b);sources={n:r.digest(n) for n in sorted(names)}
            unique=[];phase=io.BytesIO()
            with tarfile.open(fileobj=phase,mode='w:gz') as out:
                for name,h in sources.items():
                    base=subprocess.run(['git','show','HEAD:'+name],cwd=ROOT,capture_output=True)
                    if not base.returncode and hashlib.sha256(base.stdout).hexdigest()==h:continue
                    body=(ROOT/name).read_bytes();info=tarfile.TarInfo(name);info.size=len(body);info.mode=(ROOT/name).stat().st_mode&0o777
                    out.addfile(info,io.BytesIO(body));unique.append(name)
            record=a.report.removesuffix('.json')+'-unique-sources.tar.gz.bin'
            q=copy.deepcopy(parent)
            q.update(parent_plan=dict(record=a.parent,sha256=r.digest(a.parent)),current_controls=dict(record=a.controls,sha256=r.digest(a.controls)),
                source_phase=dict(record=record,sha256=write_report(ROOT,record,phase.getvalue()),members=unique,
                    source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()),
                sources={n:sources[n] for n in sorted(required_sources(b))},charged_original_starts_before=6,
                controller_continuation=dict(success=True,kind='native_surface_loss_extent_continuation',original_game_runs=0,
                    preflight=dict(record=a.preflight,sha256=r.digest(a.preflight)),retained_low_comparisons=accepted,
                    failed_analysis=dict(record=a.failed_analysis,sha256=r.digest(a.failed_analysis)),
                    source_changes=changes,budget_rows=rows,charges_refunded=0,maximum_requests=6,maximum_original_games=10,
                    unchanged_compiled_player_runtime_oracle=True,existing_low_controller_ast_preserved=True,
                    existing_receipt_budget_and_admission_conditions_ast_preserved=True,raw_low_proofs_reassessed_unchanged=True,
                    existing_assessment_ast_preserved_except_provider_terminal_header=True,
                    header_scope='Accept only active/cancelled/interrupted terminal producer states with cancellation, exact ring geometry, zero reserved words and bounded unacknowledged tail. INTERRUPTED is protocol reason4; close and deadline paths can produce it. No unique emitter is inferred; no normal END, overflow, gap, invalid-stream or unpresented-tail credit.',
                    current_low_cli_execution_claimed=False,new_800_branch_requires_actual_execution=True),
                pilot=PREFIX+'default800-01-acquisition.json',prior_requests=[])
            q['scope']+=' Accepted640captures remain under their exact old controller phase. New800Quit dispatch delegates the existing wide handler. Current controls preserve exact low delegation; no640recollection, source-hash refresh or second game-repair allowance.'
            q['success']=all(r.digest(n)==h for n,h in sources.items());require(q['success'],'Continuation source drift')
    except Exception as error:q.update(success=False,error=str(error))
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    sha=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=sha)),flush=True)
    return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
