#!/usr/bin/env python3
"""Revalidate retained original selection toggles and selected pause counters."""
import argparse
import json
from pathlib import Path
import struct
from drawing_api_session import BUILD, sha
from drawing_documented_semantics import observe_toggles, observe_pauses
from drawing_region_input import decode as region_decode
from drawing_semantic_state import decode as semantic_decode

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    raw = a.reference.read_bytes()
    r = json.loads(raw)
    if not r['success'] or r['source_sha256'] != BUILD or not r['normal_original_quit_verified']:
        raise ValueError('Successful pinned original and actual target exit required')
    capture = Path(r['experiment']) / 'capture'
    region_bytes = (capture / 'region-input.bin').read_bytes()
    semantic_bytes = (capture / 'semantic-state.bin').read_bytes()
    journal = (capture / 'session.bin').read_bytes()
    region, semantic = region_decode(region_bytes), semantic_decode(semantic_bytes)
    if region != r['region_input_observation'] or semantic != r['semantic_state_observation']:
        raise ValueError('Retained original observation differs')
    rows = list(struct.iter_unpack('<32I', journal[64:]))
    actions = r['input_schedule']['actions']
    toggles, pauses = observe_toggles(region, actions), observe_pauses(semantic, rows, actions)
    names = ['tools/drawing_documented_semantics.py', 'tools/analyze-drawing-documented-input.py',
             'tools/drawing_region_selection.py', 'tools/drawing_region_input.py',
             'tools/drawing_semantic_state.py', 'tools/drawing_api_session.py',
             'tests/test-drawing-documented-semantics.py']
    witnesses = []
    for t in toggles['transitions']:
        action = actions[t['action_ordinal'] - 1]
        image = capture / action['presentation_file']
        witnesses.append(dict(action_ordinal=t['action_ordinal'], path=str(image),
                              sha256=sha(image.read_bytes()), synchronized_original_frame=False))
    result = dict(schema=1, success=True, review_only=True,
                  sources={n: sha((ROOT / n).read_bytes()) for n in names},
                  reference=dict(path=str(a.reference), sha256=sha(raw)),
                  journal_sha256=dict(region=sha(region_bytes), semantic=sha(semantic_bytes), input=sha(journal)),
                  toggles=toggles, pauses=pauses, visual_witnesses=witnesses,
                  original_game_reexecuted=False, complete_presented_journey=False,
                  scope='Static immutable original callback/mapped-key/selected-scalar analysis. Explicit bounded actor select-clear-reselect and stable World counter while render updates continue, followed by advancement. No allocation generation, keyboard message phase, all animation/simulation pause, exact action/frame/source ownership or native parity.')
    with a.report.open('x') as out:
        json.dump(result, out, indent=2)
        out.write('\n')
    print(json.dumps(dict(success=True, toggles=len(toggles['transitions']), pauses=len(pauses['pause_intervals']))))


if __name__ == '__main__':
    main()
