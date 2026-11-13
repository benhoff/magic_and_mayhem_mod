"""Select a new exact interaction analysis plan without changing legacy guards."""
import ast
import copy

FIELDS = ('compiler_sources', 'runtime_sources', 'player_binary', 'oracle')
SELECTOR = "PREFIX + ('input-plan-08.json' if 'tools/run-rendering-interaction-stable.py' in a['sources'] else 'input-plan-07.json')"


def require(value, message):
    if not value:
        raise ValueError(message)


def bind_plan(legacy, phase, acquired, build, digest, binary_digest):
    """Only replace execution/oracle identities already declared before capture."""
    require(phase['family'] == 'interaction', 'Foreign analysis family')
    require(all(acquired.get(k) is True for k in
                ('success', 'sources_stable', 'private_wine_stopped',
                 'original_manifest_verified_before_after')), 'Incomplete retained acquisition')
    require(build['success'] and build['sources_stable'], 'Incomplete tested build')
    require(phase['compiler_sources'] == build['compiler_sources']
            and phase['runtime_sources'] == build['runtime_sources'], 'Different native build closure')
    expected = phase['compiler_sources'] | phase['runtime_sources']
    require(all(acquired['sources'].get(n) == h == digest(n) for n, h in expected.items()),
            'Acquired native closure differs')
    require(all(digest(n) == h for n, h in acquired['sources'].items()), 'Acquired source changed')
    require(acquired['player_binary']['sha256'] == phase['player_binary']['sha256']
            == build['player']['sha256'] == binary_digest('working/build/qt-shell/mnm-qt-shell'),
            'Different tested Native binary')
    oracle = phase['oracle']
    require(oracle['native_sources'] and all(digest(n) == h for n, h in oracle['native_sources'].items()),
            'Current oracle source changed')
    paths = {'working/build/qt-shell/mnm-acceptance-world-history': 'world-history',
             'working/original-reference': 'original-reference'}
    require(len(oracle['binaries']) == 2
            and {v['path'] for v in oracle['binaries']} == set(paths), 'Different oracle executables')
    for item in oracle['binaries']:
        require(item['sha256'] == build[paths[item['path']]]['sha256']
                == digest(item['record']) == binary_digest(item['path']), 'Oracle executable differs')
    require(set(FIELDS) <= legacy.keys() and set(FIELDS) <= phase.keys(), 'Missing legacy/current bindings')
    value = copy.deepcopy(legacy)
    for key in FIELDS:
        value[key] = copy.deepcopy(phase[key])
    require({k: v for k, v in value.items() if k not in FIELDS}
            == {k: v for k, v in legacy.items() if k not in FIELDS}, 'Changed semantic declaration')
    return value


def select_plan_source(source, selected):
    """Change exactly the legacy main's plan-name expression; retain every guard."""
    require(isinstance(selected, str) and selected.startswith('research/runtime/')
            and selected.endswith('.json') and '..' not in selected.split('/'), 'Unsafe analysis path')
    tree = ast.parse(source)
    functions = [n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name == 'main']
    require(len(functions) == 1, 'Missing unique legacy analyser')
    wanted = ast.dump(ast.parse(SELECTOR, mode='eval').body, include_attributes=False)
    matches = [n for n in ast.walk(functions[0]) if isinstance(n, ast.Assign)
               and len(n.targets) == 1 and isinstance(n.targets[0], ast.Name)
               and n.targets[0].id == 'plan_name']
    require(len(matches) == 1 and ast.dump(matches[0].value, include_attributes=False) == wanted,
            'Legacy analysis selector changed')
    matches[0].value = ast.copy_location(ast.Constant(selected), matches[0].value)
    ast.fix_missing_locations(tree)
    return ast.unparse(tree) + '\n'
