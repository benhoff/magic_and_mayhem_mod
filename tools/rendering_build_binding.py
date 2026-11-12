"""Reject execution records missing a dependency in the linked player graph."""
from rendering_pilot import require
from pathlib import Path


def native_runtime_source_paths(root):
    """Conservative native-DLL input universe frozen before launcher builds."""
    root=Path(root).resolve()
    paths=set()
    for folder in ('runtime','reconstruction','protocols'):
        directory=root/folder
        require(directory.is_dir(),'Missing native runtime source root: '+folder)
        for path in directory.rglob('*'):
            if path.is_file() and path.suffix in ('.c','.h','.cpp','.hpp','.inc','.S','.py','.json'):
                require(path.resolve().is_relative_to(root),'Native runtime source escapes repository')
                paths.add(path.relative_to(root).as_posix())
    require((root/'tools').is_dir(),'Missing native build tools')
    for path in (root/'tools').glob('build-*.py'):
        require(path.resolve().is_relative_to(root),'Native build tool escapes repository')
        paths.add(path.relative_to(root).as_posix())
    return paths


def require_player_source_closure(recorded, linked):
    missing = sorted(set(linked) - set(recorded))
    require(not missing, 'Recorded player source closure omits linked dependencies: ' + ', '.join(missing))
