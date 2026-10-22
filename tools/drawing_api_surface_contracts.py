"""Exact reviewed graphics client sites; offset alone never assigns a type."""
from coverage_evidence import read_text as evidence_text
import json
from pathlib import Path

LAYOUT_PATH='research/runtime/drawing-surface-api-layouts-20261011.json'
_LAYOUTS=json.loads(evidence_text(Path(__file__).resolve().parent.parent/LAYOUT_PATH))
if _LAYOUTS['original_sha256']!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':
    raise ValueError('Reviewed surface CALL build mismatch')
SITES={row['site']:row for row in _LAYOUTS['contracts']}
if len(SITES)!=len(_LAYOUTS['contracts']):raise ValueError('Duplicate exact surface layout')


def surface_contract(item):
    row=SITES.get(item['site'])
    if not row:return None
    if item['call']['bytes']!=row['call_bytes'] or item['target_mode']['kind']!='register_memory':
        raise ValueError('Reviewed exact graphics CALL signature/mode mismatch')
    return {'name':row['interface']+'.'+row['method'],'formal_words':row['formal_words'],
            'snapshots':[dict(s) for s in row['snapshots']], 'origin':'exact_original_graphics_site'}
