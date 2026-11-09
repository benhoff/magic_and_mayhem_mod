"""Hash-pinned ordinary-input policies from retained successful campaign runs."""
import hashlib
from pathlib import Path

POLICIES={
    'retained-v34':('campaign-outcome-input-v34.py','c341df1dd8efcd74414bc3587157885de837827ae43e08101fe149a2956506d3',
        'research/runtime/rendering-progression-results-20261101/target-nominal-victory-full-01.json'),
    'retained-v22':('campaign-outcome-input-v22.py','659842784f08c4e87d0f1f72b57aca13a813cab0910ebbf0165a654b5b4be44d',
        'research/runtime/rendering-progression-results-20261101/completion-progression-travel-diagnostic-20261102-28.json'),
}


ADAPTED_POLICIES={'observed-v34-portal':('campaign-portal-outcome-input.py', '57addd5bcbbd4e1886a0645ac7501275cdb67be7f2f4aabef4fd0c294195c17a', 'retained-v34'), 'observed-current-portal':('campaign-portal-outcome-input.py', '57addd5bcbbd4e1886a0645ac7501275cdb67be7f2f4aabef4fd0c294195c17a', 'current')}


def policy_script(root,policy):
    if policy=='current':return None
    if policy not in POLICIES and policy not in ADAPTED_POLICIES:raise ValueError('Unknown campaign input policy')
    name,expected,_=(POLICIES|ADAPTED_POLICIES)[policy];root=Path(root).resolve();path=root/'tools'/name
    if (path.is_symlink() or not path.is_file() or not path.resolve().is_relative_to(root)
            or hashlib.sha256(path.read_bytes()).hexdigest()!=expected):
        raise ValueError('Retained campaign input policy source differs')
    return path
