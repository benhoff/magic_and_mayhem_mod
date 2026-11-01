#!/usr/bin/env python3
"""Reproduce a late include entering the actual linked compiler dependency graph."""
import hashlib
import importlib.util
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from rendering_build_binding import require_player_source_closure
spec=importlib.util.spec_from_file_location('binding_campaign',ROOT/'tools/test-native-campaign.py')
campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)


class BuildBindingTests(unittest.TestCase):
    def test_header_added_by_rebuild_cannot_use_prebuild_source_receipt(self):
        with tempfile.TemporaryDirectory(prefix='mnm-binding-') as directory:
            root=Path(directory);apps=root/'apps';apps.mkdir()
            build=root/'working/build/qt-shell';objects=build/'CMakeFiles/mnm-qt-shell.dir';objects.mkdir(parents=True)
            source=apps/'main.cpp';source.write_text('int main(){return 0;}\n')
            obj=objects/'main.cpp.o'
            (objects/'link.txt').write_text('g++ CMakeFiles/mnm-qt-shell.dir/main.cpp.o -o mnm-qt-shell\n')
            env=dict(os.environ,TMPDIR=directory)
            def compile_source():
                subprocess.run(['g++','-c','-MMD','-MF',str(obj)+'.d',str(source),'-o',str(obj)],check=True,env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            compile_source()
            before=campaign.linked_compiler_sources(build,root)
            self.assertIn('apps/main.cpp',before)
            header=apps/'late.hpp';header.write_text('constexpr int answer=3;\n')
            source.write_text('#include "late.hpp"\nint main(){return answer;}\n')
            compile_source()
            after=campaign.linked_compiler_sources(build,root)
            self.assertIn('apps/late.hpp',after)
            with self.assertRaisesRegex(ValueError,'late.hpp'):
                require_player_source_closure({name:'a'*64 for name in before},after)
            current={name:hashlib.sha256((root/name).read_bytes()).hexdigest() for name in after}
            require_player_source_closure(current,after)


if __name__=='__main__':unittest.main()
