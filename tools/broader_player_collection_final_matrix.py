"""Expose exact-current proofs with an explicit empty source-waiver boundary."""
from broader_player_collection_matrix import P,BINARY,specs,requests,replacements,replacement
from broader_player_collection_matrix import current as complete_current
from rendering_pilot import require

def current(root,reader,spec,row):
    value=complete_current(root,reader,spec,row)
    require(value['phase']['current_native_source_candidate']
            and all(reader.digest(n)==h for n,h in value['current_sources'].items()),
            'Final matrix requires current full Native and analysis source identities')
    require(not value.get('publication_applicability'),'Unexpected retained-Python source waiver')
    value['publication_applicability']=[]
    return value
