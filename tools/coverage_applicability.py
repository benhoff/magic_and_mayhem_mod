"""Preservation-only source applicability; immutable execution provenance stays intact.

A reviewed boundary names Python callables, not an arbitrary changed-file waiver.
The projection retains reachable module definitions and globals, imports and all
potential module-initialization effects. It is not a transitive dependency oracle.
"""
import ast
import hashlib
import re
from pathlib import Path

from coverage_claims import behavior_contract, fingerprint, scenario_contract

PREFIX = 'research/runtime/coverage/applicability/'
PHASES = {'controls': 'rerun-controls', 'analysis': 'reanalyze-retained-evidence',
          'comparison': 'recompare-retained-captures', 'execution': 'rerun-affected-scenario'}
STAGES = ('implementation', 'comparison', 'integration', 'replacement')
DYNAMIC_NAMES = {'eval', 'exec', 'globals', 'locals', 'vars', '__import__', 'id', 'getattr', 'setattr', 'delattr'}
DYNAMIC_ATTRS = {'__dict__', '__globals__', '__code__', '__builtins__', 'modules', '_getframe', 'getrefcount', 'getmembers', 'getsource'}


def require(value, message):
    if not value:
        raise ValueError(message)


def safe_relative(path):
    return isinstance(path,str) and not Path(path).is_absolute() and '..' not in Path(path).parts and path == Path(path).as_posix()


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def literal(node):
    try:
        ast.literal_eval(node)
        return True
    except (ValueError, TypeError, SyntaxError, MemoryError, RecursionError):
        return False


def source_dump(node):
    """Stable CPython-3.14-style AST text, independent of ast.dump defaults.

    Python 3.12 prints empty optional fields that later versions omit. Keep the
    existing v1 fingerprints portable without changing any recorded digest.
    """
    if isinstance(node, ast.AST):
        fields=[]
        for key,value in ast.iter_fields(node):
            required_none=key=='value' and isinstance(node,(ast.Constant,ast.MatchSingleton))
            if value==[] or value is None and not required_none:
                continue
            fields.append(key+'='+source_dump(value))
        return type(node).__name__+'('+', '.join(fields)+')'
    if isinstance(node,list):
        return '['+', '.join(source_dump(value) for value in node)+']'
    return repr(node)


def main_guard(node):
    return (isinstance(node, ast.If) and not node.orelse and
            source_dump(node.test) == source_dump(ast.parse("__name__ == '__main__'", mode='eval').body))


def names(node):
    return {n.id for n in ast.walk(node) if isinstance(n, ast.Name) and isinstance(n.ctx, ast.Load)}


def passive_definition(node, trusted_unittest=False, builtin_descriptors=True):
    """Unselected definitions must not introduce import-time effects."""
    if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
        annotations = [a.annotation for a in (*node.args.posonlyargs, *node.args.args,
                       *node.args.kwonlyargs) if a.annotation is not None]
        annotations += [n for n in (node.returns, getattr(node.args.vararg, 'annotation', None),
                                   getattr(node.args.kwarg, 'annotation', None)) if n is not None]
        return (not node.decorator_list and not annotations and
                all(literal(n) for n in node.args.defaults + [x for x in node.args.kw_defaults if x is not None]))
    if isinstance(node, ast.ClassDef):
        safe_base = lambda n: isinstance(n, ast.Attribute) and isinstance(n.value, ast.Name) and n.value.id == 'unittest' and n.attr == 'TestCase' and trusted_unittest
        if node.decorator_list or node.keywords or not all(safe_base(n) for n in node.bases):
            return False
        for child in node.body:
            if isinstance(child, (ast.FunctionDef, ast.AsyncFunctionDef)):
                # Standard descriptor decorators are evaluated while defining a class.
                decorators = child.decorator_list
                if decorators and not builtin_descriptors or any(not isinstance(d, ast.Name) or d.id not in {'classmethod', 'staticmethod', 'property'} for d in decorators):
                    return False
                child = ast.parse(ast.unparse(child)).body[0]
                child.decorator_list = []
                if not passive_definition(child):
                    return False
            elif isinstance(child, ast.Assign) and literal(child.value):
                continue
            elif isinstance(child, (ast.Pass,)) or isinstance(child, ast.Expr) and isinstance(child.value, ast.Constant):
                continue
            else:
                return False
        return True
    return False


def python_projection(raw, symbols):
    """Fingerprint reviewed callable roots plus conservative local dependencies.

    Canonical __main__ dispatch is excluded: this describes named callable/test
    semantics, never current CLI dispatch or discovery of newly added test cases.
    External dependencies remain in the original complete evidence source map.
    """
    require(isinstance(symbols, list) and symbols and all(isinstance(s, str) and s.isidentifier() for s in symbols)
            and len(symbols) == len(set(symbols)), 'Callable roots must be unique top-level identifiers')
    require(len(raw) <= 4*1024**2, 'Python source projection exceeds bounded input')
    try:
        tree = ast.parse(raw)
    except (SyntaxError, UnicodeDecodeError) as error:
        raise ValueError('Invalid Python baseline/current source') from error
    bound = set()
    unittest_imports = 0
    unittest_other = False
    for n in tree.body:
        if isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            bound.add(n.name); unittest_other |= n.name == 'unittest'
        elif isinstance(n, (ast.Assign, ast.AnnAssign)):
            targets = n.targets if isinstance(n, ast.Assign) else [n.target]
            for t in targets:
                for name in (x.id for x in ast.walk(t) if isinstance(x, ast.Name)):
                    bound.add(name); unittest_other |= name == 'unittest'
        elif isinstance(n, (ast.Import, ast.ImportFrom)):
            for a in n.names:
                name = a.asname or (a.name.split('.')[0] if isinstance(n, ast.Import) else a.name)
                bound.add(name)
                if name == 'unittest':
                    if isinstance(n, ast.Import) and a.name == 'unittest':unittest_imports += 1
                    else:unittest_other = True
    trusted_unittest = unittest_imports == 1 and not unittest_other
    builtin_descriptors = not bound & {'classmethod', 'staticmethod', 'property'}
    definitions, spine = {}, []
    for node in tree.body:
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            require(node.name not in definitions, 'Duplicate top-level binding cannot be scoped')
            definitions[node.name] = node
            if not passive_definition(node, trusted_unittest, builtin_descriptors):
                spine.append(node)
        elif isinstance(node, ast.Assign) and all(isinstance(t, ast.Name) for t in node.targets):
            for target in node.targets:
                require(target.id not in definitions, 'Rebound top-level name cannot be scoped')
                definitions[target.id] = node
            if not literal(node.value):
                spine.append(node)
        elif main_guard(node):
            continue
        else:
            require(not any(isinstance(n, ast.ImportFrom) and any(a.name == '*' for a in n.names) for n in ast.walk(node)),
                    'Wildcard import cannot be scoped')
            spine.append(node)
    require(all(isinstance(definitions.get(s), (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)) for s in symbols),
            'Missing callable root')
    selected, todo = set(), set(symbols) | set().union(*(names(n) for n in spine))
    while todo:
        name = todo.pop()
        if name in selected or name not in definitions:
            continue
        selected.add(name)
        todo.update(names(definitions[name]) - selected)
    nodes = spine + [definitions[n] for n in sorted(selected)]
    for node in nodes:
        require(not names(node) & DYNAMIC_NAMES and not any(isinstance(n, ast.Attribute) and n.attr in DYNAMIC_ATTRS for n in ast.walk(node)),
                'Dynamic namespace/code inspection cannot be scoped')
    return fingerprint(dict(schema='python-callable-closure-v1', symbols=sorted(symbols),
                            spine=[source_dump(n) for n in spine],
                            definitions={n: source_dump(definitions[n]) for n in sorted(selected)}))


def evaluate(root, register, builds, evidence, sources, claims, scenarios, read_json, read_bytes, digest):
    """Return scoped compatible paths and diagnostics, without refreshing hashes."""
    compatible, diagnostics, seen, active = {}, [], set(), {}
    behaviors = {b['id']: b for b in register['behaviors']}
    for ref in register.get('evidence_applicability', []):
        rid = ref['id']
        require(isinstance(rid, str) and rid and rid not in seen, 'Duplicate/empty applicability review ID')
        seen.add(rid)
        require(safe_relative(ref['record']) and ref['record'].startswith(PREFIX) and digest(ref['record']) == ref['sha256'], rid+': applicability report identity differs')
        report = read_json(ref['record'])
        require(report['schema'] == 1 and report['kind'] == 'scoped_source_applicability' and report['id'] == rid
                and report['preservation_only'] is True and isinstance(report['reason'], str) and report['reason'].strip(),
                rid+': invalid preservation review')
        eid, bid = report['evidence']['id'], report['behavior']
        previous=active.get((bid,eid))
        require(report.get('supersedes') == previous, rid+': repeated scope needs its exact preceding review ID')
        active[(bid,eid)]=rid
        compatible.get(bid,{}).pop(eid,None)
        ev, b = evidence.get(eid), behaviors.get(bid)
        claim = claims.get(eid, {}).get(bid)
        expired = []
        if ev is None or b is None or not claim:
            expired.append('missing_original_bound_claim')
        elif (fingerprint(ev) != report['evidence']['definition_sha256'] or
              ev['record'] != report['evidence']['record'] or ev['sha256'] != report['evidence']['sha256']):
            expired.append('evidence_definition_changed')
        elif (report['contract_sha256'] != claim['contract_sha256'] or
              report['contract_sha256'] != behavior_contract(b, builds)):
            expired.append('behavior_contract_changed')
        elif (report['scenarios'] != claim.get('scenarios', {}) or any(
              sid not in scenarios or scenario_contract(scenarios[sid]) != h or bid not in scenarios[sid]['behaviors']
              or eid not in scenarios[sid]['evidence'] for sid, h in report['scenarios'].items())):
            expired.append('scenario_contract_changed')
        anchor_ref = report['baseline_anchor']
        require(safe_relative(anchor_ref['record']) and anchor_ref['record'].startswith(PREFIX) and digest(anchor_ref['record']) == anchor_ref['sha256'],
                rid+': baseline status anchor differs')
        anchor = read_json(anchor_ref['record'])
        require(anchor['schema'] == 1 and anchor['behavior']['id'] == bid
                and fingerprint(anchor['evidence']) == report['evidence']['definition_sha256']
                and behavior_contract(anchor['behavior'], builds) == report['contract_sha256'],
                rid+': original accepted contract anchor differs')
        status = report['preserve_status']
        require(isinstance(status, dict) and status and set(status) <= set(STAGES)
                and all(v != 'none' for v in status.values()), rid+': invalid preserved stages')
        require(all(anchor['behavior']['status'][k] == v for k,v in status.items()), rid+': status exceeds accepted baseline')
        entries = report['source_contracts']
        require(isinstance(entries, list) and entries, rid+': no reviewed source boundaries')
        paths, matched, phases = set(), set(), {}
        for entry in entries:
            path = entry['path']
            require(safe_relative(path) and path not in paths and entry['phase'] in PHASES, rid+': duplicate path/unknown phase')
            paths.add(path); phases[path] = entry['phase']
            require(safe_relative(entry['baseline_record']) and entry['baseline_record'].startswith(PREFIX), rid+': baseline must be pinned evidence')
            baseline = read_bytes(entry['baseline_record'])
            require(sha(baseline) == entry['recorded_sha256'], rid+': original source bytes differ')
            if ev is not None:
                require(sources[eid].get(path) == entry['recorded_sha256'], rid+': boundary does not match recorded execution provenance')
            if path.startswith(('runtime/','renderer/','apps/','assets/','audio/','reconstruction/','protocols/')):
                require(entry['phase'] == 'execution' and entry['kind'] == 'full-source-v1',
                        rid+': native/runtime sources require full execution provenance')
            if entry['kind'] == 'python-callables-v1':
                require(Path(path).suffix == '.py' and path.startswith(('tools/', 'tests/'))
                        and entry['phase'] in {'analysis', 'controls'}, rid+': callable boundaries cannot waive runtime/native code')
                expected = python_projection(baseline, entry['symbols'])
                require(expected == entry['projection_sha256'], rid+': baseline projection differs')
                try:
                    actual = python_projection(read_bytes(path), entry['symbols'])
                except (ValueError, SyntaxError):
                    actual = None
                if actual == expected:
                    matched.add(path)
            elif entry['kind'] == 'full-source-v1':
                require(entry['projection_sha256'] == entry['recorded_sha256'], rid+': full-source boundary differs')
                if digest(path) == entry['recorded_sha256']:
                    matched.add(path)
            else:
                raise ValueError(rid+': unsupported source boundary')
        if not expired:
            existing = compatible.setdefault(bid, {})
            existing[eid] = dict(id=rid, compatible_sources=matched, phases=phases, preserve_status=status,
                new_stage_validation=[k for k in STAGES if b['status'][k] != 'none' and b['status'][k] != status.get(k)])
        diagnostics.append(dict(id=rid, evidence=eid, behavior=bid, state='historical' if expired else 'reviewed',
                                reasons=expired, compatible_sources=sorted(matched), phases=phases))
    for row in diagnostics:
        if active[(row['behavior'],row['evidence'])] != row['id']:
            row['state']='superseded'
    return compatible, diagnostics


def plan(changed, review=None):
    """A missing or changed runtime dependency never becomes an analysis waiver."""
    review = review or {}
    retained = review.get('compatible_sources', set())
    actions = {}
    if review.get('new_stage_validation'):
        actions['validate-expanded-milestones'] = list(review['new_stage_validation'])
    for path in sorted(set(changed) - set(retained)):
        phase = review.get('phases', {}).get(path, 'execution')
        actions.setdefault(PHASES[phase], []).append(path)
    return [dict(action=action, sources=paths) for action, paths in sorted(actions.items())]
