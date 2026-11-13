"""Conservative local Python source closure for one actual controller family."""
import ast
from pathlib import Path
from rendering_pilot import require


def python_sources(root, seeds):
    root=Path(root).resolve();pending=[root/n for n in seeds];seen=set()
    bases=(root/'tools',root/'protocols/python')
    while pending:
        path=pending.pop().resolve()
        require(path.is_relative_to(root)and path.is_file()and path.suffix=='.py',
                'Missing or escaping declared Python dependency')
        name=path.relative_to(root).as_posix()
        if name in seen:continue
        seen.add(name)
        for node in ast.walk(ast.parse(path.read_bytes(),filename=str(path))):
            imports=[v.name for v in node.names]if isinstance(node,ast.Import)else([node.module]if isinstance(node,ast.ImportFrom)and node.module else[])
            for imported in imports:
                for base in bases:
                    candidate=base/Path(*imported.split('.')).with_suffix('.py')
                    if candidate.is_file():pending.append(candidate)
            # Include constant dynamic filenames even when the call happens
            # through a loader object or a conditional filename assignment.
            if isinstance(node,ast.Constant)and isinstance(node.value,str):
                value=node.value
                if '\n'in value or not value or len(value)>240:continue
                candidate=Path(value)
                if candidate.is_absolute()or'..'in candidate.parts:continue
                for base in bases:
                    for target in (base/candidate,base/(value+'.py'),root/candidate):
                        if target.suffix=='.py'and target.is_file():pending.append(target)
    return seen
