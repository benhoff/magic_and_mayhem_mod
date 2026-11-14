"""Execute unchanged final-reader bodies with explicit current selector bindings."""
import ast
import json
from pathlib import Path
import sys
import coverage_evidence
from coverage_evidence import Reader
from rendering_pilot import require
from broader_player_current_interaction_final_matrix import current
from broader_current_interaction_final_gate import supplement, CONJUNCTION
ROOT = Path(__file__).resolve().parents[1]
READERS = {'acceptance': 'check-broader-player-acceptance-discovery.py',
           'failure': 'check-broader-discovery-failure-relevance.py',
           'boundary': 'check-broader-discovery-boundary-impact.py'}


def selected_source(source):
    tree = ast.parse(source)
    matrices = gates = 0
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom):
            if node.module == 'broader_player_discovery_final_matrix':
                node.module = 'broader_player_current_interaction_final_matrix'
                matrices += 1
            elif node.module == 'broader_discovery_final_gate' and any(n.name == 'supplement' for n in node.names):
                require(len(node.names) == 1, 'Changed final supplement import')
                node.module = 'broader_current_interaction_final_gate'
                gates += 1
    require(matrices == 1 and gates in (0, 1), 'Changed final reader imports')
    return ast.unparse(ast.fix_missing_locations(tree)) + '\n'


def run(kind, caller):
    require(kind in READERS and sys.argv.count('--report') == 1, 'Known final reader/report required')
    report = sys.argv[sys.argv.index('--report') + 1]
    path = ROOT / 'tools' / READERS[kind]
    with Reader(ROOT) as reader:
        sources = {n: reader.digest(n) for n in (
            'tools/broader_current_interaction_final_runner.py',
            'tools/broader_player_current_interaction_final_matrix.py',
            'tools/broader_current_interaction_final_gate.py', caller)}
        original = dict(path=path.relative_to(ROOT).as_posix(), sha256=reader.digest(path))
        conjunction = dict(record=CONJUNCTION, sha256=reader.digest(CONJUNCTION))
    original_write = coverage_evidence.write_report

    def writer(root, target, body):
        if str(target) == report:
            result = json.loads(body)
            result['sources'] = result.get('sources', {}) | sources
            result['retained_interaction_final_binding'] = dict(original_reader=original,
                original_predicate_nodes_changed=False, import_modules_changed_only=True, current_selector='broader_player_current_interaction_final_matrix.current',
                supplement='broader_current_interaction_final_gate.supplement', conjunction=conjunction,
                Native_runtime_compiler_player_sources_changed=False, original_game_runs=0)
            with Reader(ROOT) as reader:
                stable = all(reader.digest(n) == h for n, h in result['sources'].items())
            if 'sources_stable' in result: result['sources_stable'] &= stable
            result['success'] &= stable
            body = (json.dumps(result, indent=2) + '\n').encode()
        return original_write(root, target, body)

    coverage_evidence.write_report = writer
    try:
        namespace = dict(__name__='current_interaction_final_reader', __file__=str(path))
        selected = selected_source(path.read_text())
        exec(compile(selected, str(path), 'exec'), namespace)
        return namespace['main']()
    finally:
        coverage_evidence.write_report = original_write
