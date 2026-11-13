"""Select only prospective interaction comparison and retained admission tools."""
import ast
from rendering_pilot import require
OLD_COMPARE='tools/compare-broader-paused-interaction.py'
NEW_COMPARE='tools/compare-broader-observed-paused-interaction.py'
OLD_ADMIT='tools/run-broader-derived-source-case.py'
NEW_ADMIT='tools/run-broader-terminal-staged-source-case.py'
def comparison(command):
    require(isinstance(command,list)and command[:2]==['python3',OLD_COMPARE],'Unexpected paused comparison dispatch')
    result=list(command);result[1]=NEW_COMPARE;return result
def admission(command):
    require(isinstance(command,list)and command[:3]==['python3',OLD_ADMIT,'admit'],'Unexpected derived admission dispatch')
    result=list(command);result[1]=NEW_ADMIT;return result
def selected_source(source):
    tree=ast.parse(source);counts={}
    for node in ast.walk(tree):
        if isinstance(node,ast.Call)and isinstance(node.func,ast.Name)and node.func.id=='execute'and node.args and isinstance(node.args[0],ast.Constant)and node.args[0].value in('compare','admit'):
            label=node.args[0].value;counts[label]=counts.get(label,0)+1
            require(len(node.args)==2,'Changed domain dispatch arguments')
            node.args[1]=ast.Call(func=ast.Name(id='_observed_'+label,ctx=ast.Load()),args=[node.args[1]],keywords=[])
    require(counts=={'compare':1,'admit':1},'Missing unique bounded domain dispatch')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'
