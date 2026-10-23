#!/usr/bin/env python3
"""Bind a closed source-only replay's first World canvas to same-run Lock epochs."""
from coverage_evidence import read_json, evidence_digest, write_report
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_storage_transitions import decode, bind_producer
from canvas_initial_history import unique

ROOT=Path(__file__).resolve().parents[1]
def sha(p): return evidence_digest(p)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--capture-report',type=Path,required=True)
    p.add_argument('--replay-report',type=Path,required=True)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    if a.output.exists(): p.error('Refusing evidence overwrite')
    declaration=json.loads(a.claims.read_text());sources=declaration['sources']
    if any(sha(ROOT/n)!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
        physical_allocation_ownership_proved=False,all_writers_observed=False,
        original_pixels_used_as_native_inputs=False,live_replacement=False)
    inputs={}
    def pin(path): inputs[str(path.resolve())]=sha(path)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        for path in (a.capture_report,a.replay_report):pin(path)
        capture=read_json(a.capture_report);replay=read_json(a.replay_report)
        if not capture['success'] or not replay['success'] or replay['capture_report_sha256']!=sha(a.capture_report):raise ValueError('Requires successful matching capture/replay')
        if replay['original_pixels_used_as_native_inputs'] or replay['mismatches'] or not replay['completed']:raise ValueError('Incomplete source-only replay')
        experiment=Path(capture['experiment']);stream=experiment/'capture/canvas-producers.bin';pin(stream)
        spec=importlib.util.spec_from_file_location('producer_inspector',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
        _,records=m.decode(stream.read_bytes())
        first=next(r for r,b in records if r[2]==11)
        before=next(r for r,b in reversed(records[:first[1]-1]) if r[2]==10 and r[3]==first[3])
        if before[5:8]!=first[5:8]:raise ValueError('Initial checkpoint/World extent or stride changed')
        checkpoints=unique(replay['checkpoints'],'sequence');completed=checkpoints[before[1]]
        if replay['completed']!=len(checkpoints):raise ValueError('Incomplete or duplicate replay cohort')
        if not completed['native_completed'] or completed['mismatches']:raise ValueError('First-World before-state not reconstructed')
        intervening=[r for r,b in records if before[1]<r[1]<first[1]]
        # Creates/releases and every recorded writer to this destination break
        # continuity. A presentation copy reads this canvas as its source.
        if any(r[3]==first[3] and r[2] not in (3,10) for r in intervening):raise ValueError('Writer/lifecycle transition after compared checkpoint')
        original=experiment/'capture'/completed['path'];pin(original)
        nativeReport=a.replay_report.parent/'native/native-report.json';pin(nativeReport)
        native=read_json(nativeReport)
        cp=unique(native['checkpoints'],'oracle')[before[14]];owned=a.replay_report.parent/'native'/cp['path'];pin(owned)
        if original.read_bytes()!=owned.read_bytes() or len(owned.read_bytes())!=first[5]*first[6]*2:raise ValueError('Full initial canvas differs')
        journals=list((experiment/'capture').glob('canvas-storage-transitions-*.bin'))
        if len(journals)!=1:raise ValueError('Requires one closed same-run storage journal')
        pin(journals[0]);storage=decode(journals[0].read_bytes());binding=bind_producer(storage,records)
        if not storage['closed'] or storage['calls']!=storage['returns']:raise ValueError('Unclosed storage calls')
        events=[e for e in storage['events'] if e['canvas']==first[3] and e['producer_prefix']<first[1]]
        generations=[r for r,b in records if r[1]<first[1] and r[3]==first[3] and r[2] in (1,2)]
        if [r[2] for r in generations]!=[1]:raise ValueError('First canvas generation not continuously live')
        report.update(success=True,first_world_queue=first[14],first_world_sequence=first[1],logical_canvas=first[3],
            initial_checkpoint_sequence=before[1],initial_oracle=before[14],initial_pixels_compared=first[5]*first[6],
            initial_original_sha256=sha(original),initial_native_sha256=sha(owned),
            native_initial_canvas_fully_defined=True,intervening_records=[dict(sequence=r[1],op=r[2],destination=r[3],source=r[4]) for r in intervening],
            first_canvas_creation_sequence=generations[0][1],preworld_storage_events=events,
            storage_summary={k:storage[k] for k in ('calls','returns','closed','observed_lock_return_epochs','address_identities')},
            storage_producer_binding=binding,all_replay_checkpoints=replay['completed'],
            limits=['Logical recorder generations and observed Lock epochs only; physical allocations, COM aliases, unobserved writer intervals and general driver leases remain unresolved.',
                    'Full before-state equality applies to this captured scenario; it does not identify the cause of historical physical-frame residuals.'])
    except Exception as e:report['error']=str(e)
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
        report.update(original_manifest_verified_before_after=True,inputs=inputs,
            inputs_stable=all(sha(Path(n))==h for n,h in inputs.items()),sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()))
        report['success'] &= report['inputs_stable'] and report['sources_stable']
        write_report(ROOT,a.output,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps({k:report.get(k) for k in ('success','error','initial_pixels_compared','logical_canvas')}))
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
