#!/usr/bin/env python3
"""Linked compiler provenance excludes unrelated targets and requires receipts."""
import importlib.util,json,sys,tempfile
from pathlib import Path
repo=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(repo/'tools'))
spec=importlib.util.spec_from_file_location('campaign',repo/'tools/test-native-campaign.py')
campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
with tempfile.TemporaryDirectory(prefix='linked provenance ') as directory:
    root=Path(directory);build=root/'working/build';shell=build/'CMakeFiles/mnm-qt-shell.dir';library=build/'nested/CMakeFiles/native.dir'
    shell.mkdir(parents=True);library.mkdir(parents=True)
    (shell/'link.txt').write_text('c++ CMakeFiles/mnm-qt-shell.dir/main.cpp.o nested/libnative.a -o mnm-qt-shell')
    (library/'link.txt').write_text('ar qc libnative.a CMakeFiles/native.dir/render.cpp.o')
    (shell/'main.cpp.o.d').write_text('main.o: '+repr(str(root/'main.cpp'))+' \\\n '+repr(str(build/'generated.hpp'))+'\n')
    (library/'render.cpp.o.d').write_text('render.o: '+repr(str(root/'renderer.hpp'))+'\n')
    unrelated=build/'CMakeFiles/unrelated.dir';unrelated.mkdir();(unrelated/'bad.o.d').write_text(str(root/'undeclared-test.cpp'))
    actual=campaign.linked_compiler_sources(build,root)
    if actual!={'main.cpp','renderer.hpp'}:raise AssertionError(actual)
    (library/'render.cpp.o.d').unlink()
    try:campaign.linked_compiler_sources(build,root)
    except FileNotFoundError:pass
    else:raise AssertionError('Missing linked compiler receipt admitted')
    (library/'link.txt').unlink()
    try:campaign.linked_compiler_sources(build,root)
    except FileNotFoundError:pass
    else:raise AssertionError('Missing archive link receipt admitted')
print(json.dumps({'success':True,'linked_sources':2,'unrelated_target_excluded':True,'generated_sources_excluded':True,'missing_receipts_refused':2}))
