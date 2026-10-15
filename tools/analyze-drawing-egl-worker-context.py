#!/usr/bin/env python3
"""Crosscheck activeEGLcontexts against exactpacket/worker/submission IDs."""
import argparse
import json
from pathlib import Path
from drawing_api_session import sha
from drawing_worker_context import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    raw=a.reference.read_bytes();x=json.loads(raw)
    if x.get('success') is not True or x.get('original_drawing_active') is not True:raise ValueError('Admittedoriginalreference required')
    capture=Path(x['experiment'])/'capture';contexts=decode((capture/'egl-worker-context.bin').read_bytes())
    if contexts!=x['egl_worker_context_observation']:raise ValueError('Context journal/reference changed')
    bindings={b['worker']:b for b in x['wine_blit_packet_observation']['bindings']};workers={c['call']:c for c in x['wine_blit_worker_observation']['calls']};submissions={c['call']:c for c in x['wine_submission_observation']['calls']};physical={f['entry']:f['ordinal'] for f in x['egl_physical_observation']['frames']}
    links=[];excluded=[]
    for call in contexts['calls']:
        if not call['stable_active_context']:
            excluded.append(dict(egl_call=call['call'],reason='missing-unstable-inactive-or-changed-context'));continue
        value=call['points'][0]['context'];binding=bindings.get(value['worker']);worker=workers.get(value['worker']);submission=submissions.get(value['submission'])
        if binding is None or worker is None or submission is None or (binding['command'],binding['submission'],binding['worker_thread'])!=(value['command'],value['submission'],value['win_thread']) or worker['thread']!=value['win_thread']:raise ValueError('Activecontext differsfrom admittedpacket/worker/submission')
        links.append(dict(egl_call=call['call'],physical_ordinal=physical.get(call['call']),worker=value['worker'],command=value['command'],submission=value['submission'],win_thread=value['win_thread'],unix_thread=call['points'][0]['unix_thread'],source_generation=worker['source_generation'],destination_generation=worker['destination_generation'],source_storage_owned=False,thread_equivalence_proved=False,scanout_proved=False))
    names=['tools/analyze-drawing-egl-worker-context.py','tools/drawing_worker_context.py'];report=dict(schema=1,success=True,sources={n:sha((ROOT/n).read_bytes()) for n in names},reference=dict(path=str(a.reference),sha256=sha(raw)),links=links,excluded=excluded,stable_active_links=len(links),links_with_physical_witness=sum(l['physical_ordinal'] is not None for l in links),win_unix_thread_relation_validated=False,source_storage_owned=False,complete_presented_journey=False,scope='Offline exactportableworker/command/submission identitycrosscheck forstableactivebefore/afterEGLintervals;retainsallunassignedcalls.Noimmutabletexturebytes/thread-equivalence/scanout/fullroutes orcompletejourneyclaim.')
    a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ('stable_active_links','links_with_physical_witness')}))


if __name__=='__main__':main()
