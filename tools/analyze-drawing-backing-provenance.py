#!/usr/bin/env python3
"""Classify retained unknown cache entries without revising historical evidence."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_api_session import BUILD,sha
from drawing_source_write_trace import analyze
from drawing_storage_transitions import decode,bind_producer
from drawing_backing_provenance import classify
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    stable();raw=a.reference.read_bytes();ref=json.loads(raw)
    if ref.get('success') is not True or ref.get('source_sha256')!=BUILD or ref.get('original_manifest_verified_before_after') is not True:raise ValueError('Unsupported reference provenance')
    capture=(Path(ref['experiment'])/'capture').resolve()
    if not capture.is_relative_to(ROOT/'working'):raise ValueError('Capture outside worktree')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        t=analyze(capture)
        if t!=ref['source_write_trace_observation']:raise ValueError('Retained trace artifacts or joins changed')
        transition=(capture/'canvas-storage-transitions-0000.bin').read_bytes();producer=(capture/'canvas-producers.bin').read_bytes()
        if sha(transition)!=ref['canvas_storage_transition_observation']['journal_sha256'] or sha(producer)!=ref['producer_observation']['producer_journal_sha256']:raise ValueError('Retained lifecycle input changed')
        spec=importlib.util.spec_from_file_location('producer',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
        _,records=m.decode(producer,complete=True);observed=decode(transition)
        if not observed['closed']:raise ValueError('Unclosed transition cohort')
        binding=bind_producer(observed,records);result=classify(t['leases'],observed)
        tests=subprocess.run(['python3',str(ROOT/'tests/test-drawing-backing-provenance.py')],capture_output=True,text=True,check=True)
        stable();result.update(sources=sources,sources_stable=True,claims=claims['claims'],reference=dict(path=str(a.reference.resolve().relative_to(ROOT)),sha256=sha(raw)),transition_journal_sha256=sha(transition),producer_journal_sha256=sha(producer),producer_binding=binding,decoder_tests=tests.stderr,original_game_executed=False,scope='Fresh offline replay of retained original02 trace/cache lifecycle. No new original execution, physical allocation ownership or GPU completion claim.')
        with a.report.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
        print(json.dumps({k:result[k] for k in ('unknown_entries','unknown_canvas_identities','classifications')}))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
