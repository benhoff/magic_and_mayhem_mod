#!/usr/bin/env python3
"""Genuine V22 native campaign Quit report on original graphics/audio."""
import argparse,hashlib,importlib.util,json,os,struct,sys
from pathlib import Path
from types import SimpleNamespace
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BID='HOST.native-campaign-report-quit'
SID='campaign-report-native-quit-20261013'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def fingerprints():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in r['behaviors'] if x['id']==BID);s=next(x for x in r['scenarios'] if x['id']==SID)
    paths=required_sources(b)|set(s['tests']);return dict(sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)},claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={SID:scenario_contract(s)})])
def validate(root,args,before):
 report=json.loads((root/'report.json').read_text())
 if not report['success'] or not report['campaign_handoff'] or not report['original_viewport_presented'] or [(s['screen'],s['ack']) for s in report['states']]!=[(3,0),(18,1),(18,2)]:raise RuntimeError('Incomplete Qt Enter handoff')
 if len({s['thread'] for s in report['states']})!=1:raise RuntimeError('Engine thread changed')
 lines=(root/'shell.log').read_text().splitlines()
 if not any(l.startswith('Menu launcher exited with status 0.') for l in lines) or any(l.startswith('Smoke test passed:') for l in lines):raise RuntimeError('Missing normal original Quit termination')
 experiment=Path(next(l[20:] for l in lines if l.startswith('Evidence directory: ')));metadata=json.loads((experiment/'manifest.json').read_text())
 rows=module('enter_decode','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes());actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
 if actions!=[(3,0),(18,65538),(18,0),(17,3),(17,3),(6,21),(3,4)]:raise RuntimeError('Unexpected callback trace: '+str(actions))
 ticks=[r for r in rows if r['event']==12]
 if len(ticks)!=3 or any(r['menu_id']!=2 or r['initialized']!=1 or r['object']!=0x6cbb78 or r['active_screen']!=0x6cbb78 or r['thread']!=report['states'][0]['thread'] for r in ticks):raise RuntimeError('Original gameplay did not tick on the menu engine thread')
 raw=(experiment/'channel.bin').read_bytes();words=struct.unpack_from('<48I',raw)
 if len(raw)!=524288 or raw[:8]!=b'MNMMCM22' or words[5]!=0 or words[36]!=7 or words[39]!=0:raise RuntimeError('V22 Enter acknowledgement/handoff/retirement mismatch')
 for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
  if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged inputs changed')
 if fingerprints()!=before or json.loads((root/'prospective.json').read_text())!=before:raise RuntimeError('Declared live compiler inputs changed')
 if metadata.get('native_render'):raise RuntimeError('This scope requires original graphics')
 mini=report['mini_states']
 if any(s['actions']!=31 for s in mini[:2]):raise RuntimeError('V22 campaign Mini capabilities differ')
 if report['viewport_returns']!=3 or [(s['screen'],s['ack']) for s in mini]!=[(17,3),(17,4),(6,5)] or any(s['thread']!=report['states'][0]['thread'] for s in mini):raise RuntimeError('Incomplete native campaign Mini round trip')
 answers=[r for r in rows if r['event']==22 and r['depth']==5]
 if [r['argument'] for r in answers]!=[1,0] or any(r['next_screen'] or r['returning']!=1 or r['thread']!=report['states'][0]['thread'] for r in answers):raise RuntimeError('Original No/Yes confirmation mismatch')
 flags=[r['result'] for r in rows if r['event']==25 and r['depth']==5]
 if flags!=[0,1] or not report['main_return']:raise RuntimeError('Original campaign exit flag/Main return mismatch')
 outcome=[r for r in rows if r['event']==28 and r['active_screen']==0x6db967]
 accepted=[r for r in rows if r['event']==3 and r['menu_id']==6 and r['argument']==21]
 main=[r for r in rows if r['event'] in (1,4) and r['menu_id']==3 and r['active_screen']==0x657ce0 and accepted and r['sequence']>accepted[-1]['sequence']]
 if not report['native_defeat'] or len(outcome)!=1 or len(accepted)!=1 or not main:raise RuntimeError('Native report/original OK/fresh Main route incomplete')
 displayed=mini[-1];wire=raw[94208:94208+5648]
 texts=[wire[272+i*256:272+(i+1)*256].split(b'\0')[0].decode('cp1252') for i in range(21)]
 title=wire[16:272].split(b'\0')[0].decode('cp1252')
 if title!=displayed['title'] or texts!=displayed['texts'] or texts[19]!='You quit the battle.':raise RuntimeError('Native labels differ from the original report payload')
 report['campaign_exit']=dict(outcome=outcome[-1],accepted=accepted[-1],main_return=main[0],title=title,texts=texts);report['confirmation_answers']=answers;report['exit_flags']=flags
 resumes=[r for r in rows if r['event']==20 and r['active_screen']==0x6cbb78]
 if len(resumes)<1 or any(r['thread']!=report['states'][0]['thread'] for r in resumes):raise RuntimeError('Original World resume not observed')
 report['world_resumes']=resumes
 report.update(source_sha256=metadata['source_sha256'],sources=before['sources'],execution_claims=before['claims'],source_stable=True,shell_sha256=hashlib.sha256(args.shell.read_bytes()).hexdigest(),raw_directory=str(root),original_graphics_audio=True,original_work_bypassed=False,scope='Fresh Celtic first Region Enter,original World ticks,native Mini Quit twice,original No/resume and Yes,native verbatim campaign Quit defeat,original OK/fresh Main/Main Quit. Experimental campaign V22 with original graphics/audio. No natural victory or defeat,rewards,persistence,loaded Realm/Character or native renderer validation.',experiment=str(experiment),original_callback_trace=actions,world_ticks=ticks,bounded_termination=False,normal_exit=True,channel_retired=True,original_drawing_retained=True,native_report_displayed=True,live_validated=True)
 (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Genuine V22 native campaign Quit report/original OK passed',flush=True)
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-shell/mnm-qt-shell');args=parser.parse_args();before=fingerprints();decl=json.loads(args.claims.read_text())
    if decl['sources']!=before['sources'] or decl['claims']!=before['claims']:raise ValueError('Prospective live declaration mismatch')
    closure=module('campaign_book_closure','tools/test-native-campaign.py').linked_compiler_sources(args.shell.parent,ROOT)
    if not closure<=set(before['sources']):raise ValueError('Undeclared linked compiler input')
    harness=module('campaign_book_harness','tools/test-live-menus.py');harness.validate=lambda root,unused:validate(root,args,before);original_mkdtemp=harness.tempfile.mkdtemp
    def capture(**kwargs):
        path=original_mkdtemp(**kwargs);root=Path(path);(root/'.retain').write_text('Genuine V22 native campaign Quit report validation; preserve raw launch/observer evidence.\n');(root/'prospective.json').write_text(json.dumps(before,indent=2)+'\n');return path
    harness.tempfile=SimpleNamespace(mkdtemp=capture);os.environ.update(MNM_LIVE_MENU_TEST_CAMPAIGN_DEFEAT='1',MNM_EXPERIMENTAL_CAMPAIGN_MENUS='1');sys.argv=[sys.argv[0],'--exit-from','main','--legacy-audio','--shell',str(args.shell)];harness.main()
if __name__=='__main__':main()
