#!/usr/bin/env python3
"""Genuine fresh Celtic V22 native book round trip on original graphics/audio."""
import argparse,hashlib,importlib.util,json,os,struct,sys
from pathlib import Path
from types import SimpleNamespace
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BID='HOST.native-campaign-grimoire-first'
SID='campaign-grimoire-native-first-20261013'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def fingerprints():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in r['behaviors'] if x['id']==BID);s=next(x for x in r['scenarios'] if x['id']==SID)
    paths=required_sources(b)|set(s['tests']);return dict(sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)},claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={SID:scenario_contract(s)})])
def validate(root,args,before):
    report=json.loads((root/'report.json').read_text());expected=[(3,0),(18,1),(27,2),(27,3),(27,4),(27,5),(27,6),(18,7),(3,8)]
    if not report['success'] or [(s['screen'],s['ack']) for s in report['states']]!=expected:raise RuntimeError('Incomplete genuine native campaign book transitions')
    if len({s['thread'] for s in report['states']})!=1:raise RuntimeError('Original engine thread changed')
    lines=(root/'shell.log').read_text().splitlines();experiments=[l[20:] for l in lines if l.startswith('Evidence directory: ')]
    if len(experiments)!=1 or not any(l.startswith('Menu launcher exited with status 0.') for l in lines):raise RuntimeError('Missing normal launch/Quit provenance')
    exp=Path(experiments[0]);meta=json.loads((exp/'manifest.json').read_text())
    if meta.get('native_render'):raise RuntimeError('This scope requires original graphics')
    trace=module('campaign_book_decode','tools/test-menu-observer.py').decode((exp/'events.bin').read_bytes());actions=[(r['menu_id'],r['argument']) for r in trace if r['event']==3]
    if actions!=[(3,0),(18,2),(0,0),(0,2),(0,124),(0,125),(0,3999),(18,1),(3,4)]:raise RuntimeError('Unexpected original campaign book callback trace: '+str(actions))
    raw=(exp/'channel.bin').read_bytes()
    if len(raw)!=524288 or raw[:8]!=b'MNMMCM22' or struct.unpack_from('<I',raw,20)[0] or struct.unpack_from('<I',raw,144)[0]!=9:raise RuntimeError('V22 channel did not retire after acknowledged Quit')
    for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
        if hashlib.sha256((exp/'game'/name).read_bytes()).hexdigest()!=meta[key]:raise RuntimeError('Staged executable/adapter changed')
    if fingerprints()!=before or json.loads((root/'prospective.json').read_text())!=before:raise RuntimeError('Declared live inputs changed')
    report.update(shell_sha256=hashlib.sha256(args.shell.read_bytes()).hexdigest(),sources=before['sources'],execution_claims=before['claims'],source_sha256=meta['source_sha256'],experiment=str(exp),raw_directory=str(root),source_stable=True,original_work_bypassed=False,normal_quit=True,live_validated=True,original_graphics_audio=True,original_callback_trace=actions)
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Genuine first-region V22 native Grimoire round trip passed',flush=True)
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-shell/mnm-qt-shell');args=parser.parse_args();before=fingerprints();decl=json.loads(args.claims.read_text())
    if decl['sources']!=before['sources'] or decl['claims']!=before['claims']:raise ValueError('Prospective live declaration mismatch')
    closure=module('campaign_book_closure','tools/test-native-campaign.py').linked_compiler_sources(args.shell.parent,ROOT)
    if not closure<=set(before['sources']):raise ValueError('Undeclared linked compiler input')
    harness=module('campaign_book_harness','tools/test-live-menus.py');harness.validate=lambda root,unused:validate(root,args,before);original_mkdtemp=harness.tempfile.mkdtemp
    def capture(**kwargs):
        path=original_mkdtemp(**kwargs);root=Path(path);(root/'.retain').write_text('Genuine V22 fresh campaign native book validation; preserve raw launch/observer evidence.\n');(root/'prospective.json').write_text(json.dumps(before,indent=2)+'\n');return path
    harness.tempfile=SimpleNamespace(mkdtemp=capture);os.environ.update(MNM_LIVE_MENU_TEST_CAMPAIGN_GRIMOIRE='1',MNM_EXPERIMENTAL_CAMPAIGN_MENUS='1',MNM_GRIMOIRE_TEST_OBSERVE='1');sys.argv=[sys.argv[0],'--exit-from','main','--legacy-audio','--shell',str(args.shell)];harness.main()
if __name__=='__main__':main()
