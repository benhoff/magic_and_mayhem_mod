"""Recheck all pause guards with exact stopped terminal identities when necessary."""
from broader_interaction_pause import verify_pause as original
from broader_terminal_ui_identity import from_acquisition,bind_pause,MESSAGE
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
CONTROLS=P+'terminal-ui-identity-controls-01.json'
from broader_pause_observation import bind as bind_observed_pause
OBSERVED=P+'public-pause-observation-controls-01.json'
def verify_pause(reader,acquired):
    for name,tests in ((CONTROLS,20),(OBSERVED,30)):
        control=reader.json(name)
        require(control['success']and control['tests']==tests and all(reader.digest(n)==h for n,h in control['sources'].items()),'Current stopped pause observation controls required')
    terminal=from_acquisition(reader,acquired)
    return bind_observed_pause(terminal)(reader,acquired)
