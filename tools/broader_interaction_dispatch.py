"""Bound only interaction comparison dispatch; retain all original acquisition guards."""
import ast


def require(value, message):
    if not value:
        raise ValueError(message)


def comparison_route(value):
    require(value['sources_stable'] and value['comparison_processes_stopped']
            and value['original_manifest_verified_before_after'], 'Incomplete current comparison')
    if value['success']:
        require(value['comparison']['success'] and value['comparison']['mismatching_pixels'] == 0
                and value['proof']['verified'], 'Incomplete successful comparison')
        return 'complete'
    require('comparison' in value, 'Comparison never reached pixels')
    if value['comparison']['success'] and value['comparison']['mismatching_pixels'] == 0:
        require(value.get('error') == 'Camera packet crosses a source chunk', 'Unreviewed semantic refusal')
        return 'semantics'
    require(value['comparison']['mismatching_pixels'] > 0, 'Incomplete pixel disagreement')
    return 'negative'


def dispatch_source(source):
    tree = ast.parse(source)
    functions = [v for v in tree.body if isinstance(v, ast.FunctionDef) and v.name == 'main']
    require(len(functions) == 1, 'Missing original domain coordinator')
    calls = [v for v in ast.walk(functions[0]) if isinstance(v, ast.Call)
             and isinstance(v.func, ast.Name) and v.func.id == 'execute'
             and v.args and isinstance(v.args[0], ast.Constant) and v.args[0].value == 'compare']
    wanted = ast.dump(ast.parse("case['comparison_command']", mode='eval').body, include_attributes=False)
    require(len(calls) == 1 and len(calls[0].args) == 2
            and ast.dump(calls[0].args[1], include_attributes=False) == wanted, 'Original comparison dispatch changed')
    calls[0].args[1] = ast.copy_location(ast.parse('_bound_comparison_command()', mode='eval').body, calls[0].args[1])
    ast.fix_missing_locations(tree)
    return ast.unparse(tree) + '\n'
