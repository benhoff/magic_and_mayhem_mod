"""Private display adaptation and actual retained preparation refusal controls."""
import importlib.util
import errno
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch,Mock

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_interaction_budget import admit

spec=importlib.util.spec_from_file_location('clipping',ROOT/'tools/run-rendering-clipping.py')
clipping=importlib.util.module_from_spec(spec);spec.loader.exec_module(clipping)


class Display(unittest.TestCase):
    def launcher(self,args):
        class Launcher:
            subprocess=clipping.subprocess
            def start_display(self,path):
                return self.subprocess.Popen(args,stdin=self.subprocess.DEVNULL)
        return Launcher()

    def test_private_depth_change_preserves_all_other_server_arguments(self):
        args=['Xvfb','-displayfd','7','-screen','0','1280x1024x24',
              '-nolisten','tcp','-auth','private-cookie','-extension','GLX']
        launcher=self.launcher(args);original=launcher.subprocess
        with patch.object(clipping.subprocess,'Popen') as launch:
            clipping.desktop_display(launcher,Path('/tmp/unused'))
        self.assertIs(launcher.subprocess,original)
        self.assertEqual(launch.call_args.args[0],args[:5]+['1280x1024x16']+args[6:])

    def test_changed_launcher_shape_refuses_before_process_creation(self):
        for args in (['other','-displayfd','1280x1024x24'],
                     ['Xvfb','-displayfd','1280x1024x16'],
                     ['Xvfb','-displayfd','1280x1024x24','1280x1024x24']):
            launcher=self.launcher(args);original=launcher.subprocess
            with patch.object(clipping.subprocess,'Popen') as launch:
                with self.assertRaises(ValueError):clipping.desktop_display(launcher,Path('/tmp/unused'))
                launch.assert_not_called()
            self.assertIs(launcher.subprocess,original)

    def test_failed_server_restores_launcher_namespace(self):
        launcher=self.launcher(['Xvfb','-displayfd','7','1280x1024x24'])
        original=launcher.subprocess
        with patch.object(clipping.subprocess,'Popen',side_effect=OSError('declared failure')):
            with self.assertRaises(OSError):clipping.desktop_display(launcher,Path('/tmp/unused'))
        self.assertIs(launcher.subprocess,original)


class CapturePaths(unittest.TestCase):
    def test_real_exclusive_source_file_and_directory_alias_both_work(self):
        with tempfile.TemporaryDirectory(dir='/tmp') as tmp:
            root=Path(tmp);out=root/'output';out.mkdir();ram=root/'ram'
            clipping.prepare_capture_paths(out,ram)
            self.assertFalse((out/'worlds.bin').exists())
            self.assertFalse((out/'worlds.bin').is_symlink())
            with (ram/'worlds.bin').open('xb') as f:f.write(b'new exclusive source')
            with (out/'capture/commands.bin').open('xb') as f:f.write(b'new capture')
            self.assertEqual((ram/'capture/commands.bin').read_bytes(),b'new capture')
            self.assertTrue((ram/'.retain').exists())

    def test_precreated_source_alias_refuses_before_output_creation(self):
        with tempfile.TemporaryDirectory(dir='/tmp') as tmp:
            root=Path(tmp);out=root/'output';out.mkdir();ram=root/'ram'
            (out/'worlds.bin').symlink_to(root/'missing.bin')
            with self.assertRaisesRegex(ValueError,'fresh'):clipping.prepare_capture_paths(out,ram)
            self.assertFalse(ram.exists())


class RetainedPreparations(unittest.TestCase):
    def cases(self):
        with Reader(ROOT) as reader:
            return [dict(report=(name:=clipping.PREFIX+f'default-800-{i:02}-acquisition.json'),
                         result=(a:=reader.json(name)),repair_of=a.get('repair_of'),
                         proved=False,comparison_failed=False,preparation_fixed=False)
                    for i in (1,2,3)]

    def test_actual_preflights_are_zero_games_and_never_positive_clip_proof(self):
        cases=self.cases()
        self.assertTrue(all(c['result']['success'] is False and c['result']['original_game_runs']==0
                            and not c['result'].get('native_admission') for c in cases))
        self.assertEqual(cases[0]['result']['error'],'Launcher input differs from pinned manifest: Chaos.exe')
        self.assertTrue(all(c['result']['error']=='Both environment slots are busy or held; no additional clone was created'
                            for c in cases[1:]))

    def test_failed_preparations_require_the_complete_repair_ancestry(self):
        cases=self.cases()
        with self.assertRaisesRegex(ValueError,'Stop after'):admit(cases,None,7,4)
        result=admit(cases,cases[-1]['report'],7,4)
        self.assertEqual(result['original_games_before'],0)
        self.assertEqual(set(result['repair_ancestors']),{c['report'] for c in cases})

    def test_request_budget_counts_zero_game_failures(self):
        cases=self.cases()
        with self.assertRaisesRegex(ValueError,'request budget exhausted'):
            admit(cases,cases[-1]['report'],3,4)

    def test_unknown_game_count_refuses_even_in_a_repair(self):
        cases=self.cases();cases[0]['result']['original_game_runs']=None
        with self.assertRaisesRegex(ValueError,'unknown'):
            admit(cases,cases[-1]['report'],7,4)


class CountReviews(unittest.TestCase):
    def test_actual_interrupted_starts_are_counted_without_rewriting_inputs(self):
        with Reader(ROOT) as reader:
            for index,recorded in ((9,None),(10,0)):
                name=clipping.PREFIX+f'default-640-{index:02}-acquisition.json'
                review=clipping.PREFIX+f'default-640-{index:02}-game-count-review-01.json'
                original=reader.json(name)
                result=clipping.accounted_game_count(reader,dict(game_count_reviews=[dict(record=review,sha256=reader.digest(review))]),name,original)
                self.assertEqual(result['original_game_runs'],1)
                self.assertFalse(result['success'])
                self.assertEqual(original['original_game_runs'],recorded)
                self.assertEqual(reader.json(name)['original_game_runs'],recorded)

    def test_review_digest_and_duplicate_binding_refuse(self):
        with Reader(ROOT) as reader:
            name=clipping.PREFIX+'default-640-09-acquisition.json'
            review=clipping.PREFIX+'default-640-09-game-count-review-01.json'
            ref=dict(record=review,sha256=reader.digest(review))
            for refs in ([dict(ref,sha256='0'*64)],[ref,ref]):
                with self.assertRaises(ValueError):
                    clipping.accounted_game_count(reader,dict(game_count_reviews=refs),name,reader.json(name))

    def test_unknown_count_has_no_implicit_correction(self):
        with Reader(ROOT) as reader:
            name=clipping.PREFIX+'default-640-09-acquisition.json'
            with self.assertRaisesRegex(ValueError,'exact review required'):
                clipping.accounted_game_count(reader,{},name,reader.json(name))


class PrefixWait(unittest.TestCase):
    def test_transient_pool_lock_retries_only_before_target_exists(self):
        with tempfile.TemporaryDirectory(dir='/tmp') as tmp:
            target=Path(tmp)/'absent'
            borrow=Mock(side_effect=[BlockingIOError(errno.EAGAIN,'busy'),('lease','proof')])
            with patch.object(clipping.time,'sleep'):
                self.assertEqual(clipping.borrow_with_deadline(borrow,None,target,{}),('lease','proof'))
            self.assertEqual(borrow.call_count,2)

    def test_partial_target_never_retries(self):
        with tempfile.TemporaryDirectory(dir='/tmp') as tmp:
            for target in (Path(tmp),Path(tmp)/'broken'):
                if target.name=='broken':target.symlink_to(Path(tmp)/'missing')
                borrow=Mock(side_effect=BlockingIOError(errno.EAGAIN,'busy'))
                with self.assertRaises(BlockingIOError):clipping.borrow_with_deadline(borrow,None,target,{})
                self.assertEqual(borrow.call_count,1)

    def test_expired_lock_and_other_failures_refuse(self):
        with tempfile.TemporaryDirectory(dir='/tmp') as tmp:
            for error in (BlockingIOError(errno.EAGAIN,'busy'),BlockingIOError(errno.EACCES,'denied'),ValueError('other')):
                borrow=Mock(side_effect=error)
                with patch.object(clipping.time,'monotonic',side_effect=[0,121]),self.assertRaises(type(error)):
                    clipping.borrow_with_deadline(borrow,None,Path(tmp)/'absent',{})
                self.assertEqual(borrow.call_count,1)


if __name__=='__main__':unittest.main()
