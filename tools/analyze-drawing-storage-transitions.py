#!/usr/bin/env python3
"""Revalidate retained transition and producer journals at exact source prefixes."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_api_session import BUILD,sha
from drawing_storage_transitions import decode,bind_producer,bind_queries
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        raw=a.reference.read_bytes();reference=json.loads(raw)
        if reference.get('success') is not True or reference.get('source_sha256')!=BUILD or reference.get('original_drawing_active') is not True or reference.get('original_manifest_verified_before_after') is not True:
            raise ValueError('Unsupported original reference provenance')
        capture=(Path(reference['experiment'])/'capture').resolve()
        if not capture.is_relative_to(ROOT/'working'):raise ValueError('Capture outside worktree storage')
        transition=(capture/'canvas-storage-transitions-0000.bin').read_bytes();producer=(capture/'canvas-producers.bin').read_bytes()
        if sha(transition)!=reference['canvas_storage_transition_observation']['journal_sha256'] or sha(producer)!=reference['producer_observation']['producer_journal_sha256']:
            raise ValueError('Retained source artifact changed')
        spec=importlib.util.spec_from_file_location('producer',ROOT/'tools/inspect-canvas-producers.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        _,records=module.decode(producer,complete=True);observed=decode(transition)
        if not observed['closed']:raise ValueError('Unclosed observed transitions')
        binding=bind_producer(observed,records)
        query_binding=bind_queries(observed,reference.get('wine_map_storage_observation',{}),reference.get('wine_client_memory_observation',{}),reference.get('wine_retained_map_source_observation',{}))
        guards=subprocess.run(['python3',str(ROOT/'tests/test-drawing-storage-transitions.py')],capture_output=True,text=True,check=True)
        names=['tools/analyze-drawing-storage-transitions.py','tools/drawing_storage_transitions.py','tools/drawing_storage_point.py','tools/inspect-canvas-producers.py','tests/test-drawing-storage-transitions.py']
        report=dict(schema=1,success=True,sources={n:sha((ROOT/n).read_bytes()) for n in names},reference=dict(path=str(a.reference),sha256=sha(raw)),transition_journal_sha256=sha(transition),producer_journal_sha256=sha(producer),binding=binding,decoder_guards=guards.stderr,unknown_callers=sum(not e['caller_rva'] for e in observed['events']),original_game_executed=False,storage_ownership_proved=False,scope='Offline binding of observed cache generations to exact retained producer prefixes; source/transition artifacts match the original report. Every observed Lock return gets an epoch, including checkpoints skipped by source sampling. No all-call/alias/allocation ownership or full journey claim.')
        report['query_binding']=query_binding
        a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(binding))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
