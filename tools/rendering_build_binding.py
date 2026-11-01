"""Reject execution records missing a dependency in the linked player graph."""
from rendering_pilot import require


def require_player_source_closure(recorded, linked):
    missing = sorted(set(linked) - set(recorded))
    require(not missing, 'Recorded player source closure omits linked dependencies: ' + ', '.join(missing))
