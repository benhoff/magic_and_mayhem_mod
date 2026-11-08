"""Narrow retained-reader applicability; native/runtime hashes never get exceptions."""
from coverage_evidence import Reader
from rendering_pilot import require

PARSER='tools/palette_observation.py'
ANALYSIS_ONLY='tools/check-palette-cycle.py'

def execution_sources(reader,acquired,reference=None,acquisition_reference=None):
 recorded=acquired['sources'];current={n:reader.digest(n)for n in recorded};drift={n for n in recorded if current[n]!=recorded[n]}
 if not drift:return dict(recorded),None
 require(reference is not None and drift<={PARSER,ANALYSIS_ONLY},'Changed native/runtime or unreviewed palette acquisition source')
 q=reader.json(reference['record']);require(reader.digest(reference['record'])==reference['sha256']and q['success']and q['kind']=='palette_retained_reader_source_applicability','Missing exact palette reader review')
 require(q['acquisition']==acquisition_reference and q['old_sources']==recorded,'Palette reader review uses another source phase')
 require(all(reader.digest(n)==h for n,h in q['sources'].items()),'Palette reader review source drift')
 require(q['old_full_refusal']=='Palette queue regression'and q['old_new_world_prefix_equal']and q['current_complete_custody']and q['native_runtime_sources_unchanged'],'Reader correction exceeds reviewed boundary')
 require(all(q['effective_sources'][n]==current[n]for n in recorded),'Palette review does not bind current effective sources')
 return dict(current),reference
