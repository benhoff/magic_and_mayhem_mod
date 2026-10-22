#!/usr/bin/env python3
"""Revalidate current original cursor-to-HUD journal metadata, independently of pixels."""
import argparse
import json
from pathlib import Path
from coverage_evidence import read_bytes, read_json, write_report, evidence_digest
from drawing_api_session import BUILD,sha
from drawing_hud_state import decode
from drawing_hud_input_binding import bind
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    raw=read_bytes(a.reference, root=ROOT);r=json.loads(raw)
    if r.get('success') is not True or r.get('source_sha256')!=BUILD or not r.get('normal_original_quit_verified') or not r['staging'].get('hud_input_point'):
        raise ValueError('Complete original cursor point reference required')
    capture=Path(r['experiment'])/'capture';hud_raw=(capture/'hud-state.bin').read_bytes();api_raw=(capture/'session.bin').read_bytes()
    hud=decode(hud_raw)
    if hud!=r['hud_state_observation'] or sha(api_raw)!=r['observation']['journal_sha256']:raise ValueError('Retained journal hashes/state differ')
    if (capture/'session.done').read_bytes()!=api_raw[-128:]:raise ValueError('Terminal API marker differs')
    result=bind(hud,api_raw);names=['tools/analyze-drawing-hud-input.py','tools/drawing_hud_input_binding.py','tools/drawing_hud_state.py','tools/drawing_input_point.py','tools/drawing_api_session.py']
    write_report(ROOT,a.report,(json.dumps(dict(schema=1,success=True,sources={n:evidence_digest(ROOT/n,root=ROOT) for n in names},reference=dict(path=str(a.reference),sha256=sha(raw)),journal_hashes={'hud':sha(hud_raw),'api':sha(api_raw)},analysis=result,original_game_executed=False,scope='Current exact completed-owner cursor record/entry/result/tick joins against immutable original APIprefixes; no click dispatch, selection, tooltip, full physical/native or allocation/lifetime equivalence.'),indent=2)+'\n').encode())
    print(json.dumps({k:v for k,v in result.items() if k!='links'}))


if __name__=='__main__':main()
