#!/usr/bin/env python3
"""Qt launcher configuration reuse with fake CMake and a generated shell."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class QtLauncherStartup(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='mnm launch ')
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        (self.root/'tools').mkdir(); (self.root/'bin').mkdir()
        shutil.copy2(ROOT/'tools/run-qt-shell.sh', self.root/'tools/run-qt-shell.sh')
        self.build = self.root/'working/build/qt-shell'; self.build.mkdir(parents=True)
        self.log = self.root/'cmake-calls.jsonl'
        cmake = self.root/'bin/cmake'
        cmake.write_text('''#!/usr/bin/env python3
import json,os,sys
from pathlib import Path
args=sys.argv[1:]
with open(os.environ['FIXTURE_CMAKE_LOG'],'a') as out:out.write(json.dumps(args)+'\\n')
if '--build' in args:sys.exit(int(os.environ.get('FIXTURE_BUILD_EXIT','0')))
build=Path(args[args.index('-B')+1]);build.mkdir(parents=True,exist_ok=True)
source=args[args.index('-S')+1]
settings={arg.split('=',1)[0][2:]:arg.split('=',1)[1] for arg in args if arg.startswith('-D')}
(build/'CMakeCache.txt').write_text('CMAKE_HOME_DIRECTORY:INTERNAL='+source+'\\n'+''.join(key+':STRING='+value+'\\n' for key,value in settings.items()))
(build/'Makefile').touch()
''')
        cmake.chmod(0o755)
        shell = self.build/'mnm-qt-shell'
        shell.write_text('''#!/usr/bin/env python3
import json,os,sys
print(json.dumps({'arguments':sys.argv[1:],'assets':os.environ['MNM_VERIFY_ASSETS'],'cache':os.environ['MNM_CACHE_ADAPTERS']}))
'''); shell.chmod(0o755)
        self.env = dict(os.environ, PATH=str(self.root/'bin')+':'+os.environ['PATH'],
                        FIXTURE_CMAKE_LOG=str(self.log))
        for name in ('MNM_QT_BUILD_TESTS','MNM_QT_BUILD_TYPE','MNM_QT_RECONFIGURE',
                     'MNM_VERIFY_ASSETS','MNM_CACHE_ADAPTERS'):
            self.env.pop(name,None)

    def launch(self, *args):
        return subprocess.run(['bash',str(self.root/'tools/run-qt-shell.sh'),*args],
                              env=self.env, capture_output=True,text=True)

    def calls(self):
        return [json.loads(line) for line in self.log.read_text().splitlines()]

    def test_reuse_keeps_incremental_build_and_arguments(self):
        self.assertEqual(self.launch().returncode,0)
        self.assertEqual(self.launch('--repo','path with spaces').returncode,0)
        calls=self.calls()
        self.assertEqual(len(calls),3)
        self.assertEqual(calls[-1][0],'--build')
        self.assertIn('mnm-qt-shell',calls[-1])
        observed=json.loads(self.launch('--skip-movies').stdout)
        self.assertEqual(observed['arguments'][-1],'--skip-movies')
        self.assertEqual((observed['assets'],observed['cache']),('0','1'))

    def test_changed_settings_reconfigure(self):
        self.launch()
        for key,value in [('MNM_QT_BUILD_TESTS','ON'),('MNM_QT_BUILD_TYPE','Debug'),
                          ('MNM_QT_RECONFIGURE','1')]:
            self.env[key]=value
            previous=len(self.calls());self.assertEqual(self.launch().returncode,0)
            self.assertEqual(len(self.calls())-previous,2)
        self.assertNotIn('--target',self.calls()[-1])

    def test_missing_build_system_and_wrong_source_reconfigure(self):
        self.launch();(self.build/'Makefile').unlink();self.launch()
        self.assertEqual(len(self.calls()),4)
        (self.build/'CMakeCache.txt').write_text('CMAKE_HOME_DIRECTORY:INTERNAL=/other\n')
        self.launch();self.assertEqual(len(self.calls()),6)

    def test_build_failure_does_not_launch_and_overrides_are_kept(self):
        self.env.update(MNM_VERIFY_ASSETS='1',MNM_CACHE_ADAPTERS='0')
        observed=json.loads(self.launch().stdout)
        self.assertEqual((observed['assets'],observed['cache']),('1','0'))
        self.env['FIXTURE_BUILD_EXIT']='7'
        result=self.launch();self.assertEqual(result.returncode,7);self.assertEqual(result.stdout,'')


if __name__ == '__main__':
    unittest.main()
