#!/usr/bin/env python3
"""In-memory player launch controls; no Wine, original inputs or test payload writes."""
import contextlib
import importlib.util
import io
import os
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import MagicMock, patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('native_player', ROOT / 'tools/run-native-game.py')
player = importlib.util.module_from_spec(spec)
spec.loader.exec_module(player)


class PlayerLaunch(unittest.TestCase):
    @contextlib.contextmanager
    def launch(self, existing=False, environment=None):
        with contextlib.ExitStack() as stack:
            stack.enter_context(patch.dict(os.environ, environment or {}, clear=True))
            stack.enter_context(patch.object(Path, 'exists', return_value=existing))
            stack.enter_context(patch.object(Path, 'is_file', return_value=True))
            stack.enter_context(patch.object(Path, 'is_symlink', return_value=False))
            stack.enter_context(patch.object(Path, 'mkdir'))
            stack.enter_context(patch.object(Path, 'open', return_value=MagicMock()))
            stack.enter_context(patch.object(player.shutil, 'which', return_value='/bin/fixture'))
            stack.enter_context(patch.object(player, 'owned_prefix'))
            prepare = stack.enter_context(patch.object(player, 'prepare_prefix'))
            flock = stack.enter_context(patch.object(player.fcntl, 'flock'))
            run = stack.enter_context(patch.object(player.subprocess, 'run', return_value=subprocess.CompletedProcess([], 0)))
            stack.enter_context(contextlib.redirect_stdout(io.StringIO()))
            stack.enter_context(contextlib.redirect_stderr(io.StringIO()))
            yield prepare, flock, run

    def test_default_requires_native_renderer_and_dedicated_prefix(self):
        with self.launch() as (prepare, _, run):
            self.assertEqual(player.main([]), 0)
            command = run.call_args.args[0]
            self.assertEqual(command[-4:], ['--native-only', '--prefix', str(ROOT / 'working/native-player/wineprefix'), '--'])
            environment = run.call_args.kwargs['env']
            self.assertEqual(environment['MNM_NATIVE_ONLY'], '1')
            self.assertEqual(environment['MNM_NATIVE_PIXEL_NO_FORWARD'], '1')
            self.assertEqual(environment['MNM_PUBLIC_RESIDENT_WORLD'], '1')
            self.assertEqual(environment['MNM_MENU_DIRECTPLAY_DX6'], '1')
            self.assertEqual(environment['MNM_TERRAIN_LIGHTING'], 'native')
            self.assertEqual(player.lighting_selection(environment), ('native', 'none'))
            self.assertEqual(prepare.call_args.args[-1]['MNM_TERRAIN_LIGHTING'], 'native')
            self.assertNotIn('MNM_TERRAIN_LIGHTING', os.environ)
            prepare.assert_called_once()
            self.assertEqual(run.call_count, 1)  # No wineserver wait before fresh initialization.

    def test_explicit_lighting_modes_reach_preparation_and_owner(self):
        for mode in ('original', 'observe', 'native'):
            with self.subTest(mode=mode), self.launch(environment={'MNM_TERRAIN_LIGHTING': mode}) as (prepare, _, run):
                self.assertEqual(player.main([]), 0)
                self.assertEqual(prepare.call_args.args[-1]['MNM_TERRAIN_LIGHTING'], mode)
                self.assertEqual(run.call_args.kwargs['env']['MNM_TERRAIN_LIGHTING'], mode)

    def test_invalid_lighting_policy_refuses_before_prefix_preparation(self):
        for environment in ({'MNM_TERRAIN_LIGHTING': ''},
                            {'MNM_TERRAIN_LIGHTING': 'auto'},
                            {'MNM_TERRAIN_LIGHT_CAPTURE': 'everything'},
                            {'MNM_TERRAIN_LIGHTING': 'original', 'MNM_TERRAIN_LIGHT_CAPTURE': 'first32'}):
            with self.subTest(environment=environment), self.launch(environment=environment) as (prepare, _, run):
                self.assertEqual(player.main([]), 1)
                prepare.assert_not_called()
                run.assert_not_called()

    def test_original_lighting_override_bypasses_worker_staging_and_startup(self):
        from terrain_light_session import Session, stage
        root = ROOT / 'working/lighting-override-control'
        environment = {'MNM_TERRAIN_LIGHTING': 'original'}
        self.assertEqual(stage(root, b'no original artifact', b'unchanged', environment),
                         (b'unchanged', {}))
        with Session(root, {}, environment) as session:
            self.assertIsNone(session.child)

    def test_existing_prefix_waits_without_killing_and_reuses(self):
        with self.launch(existing=True) as (prepare, _, run):
            self.assertEqual(player.main(['--', '--skip-movies', '--no-native-music']), 0)
            self.assertEqual(run.call_args_list[0].args[0], ['wineserver', '-w'])
            self.assertEqual(run.call_args_list[0].kwargs['timeout'], 5)
            self.assertEqual(run.call_args.args[0][-2:], ['--skip-movies', '--no-native-music'])
            prepare.assert_called_once()

    def test_active_prefix_refuses_before_preparation(self):
        with self.launch(existing=True) as (prepare, _, run):
            run.side_effect = subprocess.TimeoutExpired('wineserver', 5)
            self.assertEqual(player.main([]), 1)
            prepare.assert_not_called()
            self.assertEqual(run.call_count, 1)

    def test_concurrent_launch_refuses(self):
        with self.launch() as (prepare, flock, run):
            flock.side_effect = BlockingIOError()
            self.assertEqual(player.main([]), 1)
            prepare.assert_not_called()
            run.assert_not_called()

    def test_foreign_prefix_refuses_before_preparation(self):
        with self.launch(existing=True) as (prepare, _, run):
            with patch.object(player, 'owned_prefix', side_effect=ValueError('unowned')):
                self.assertEqual(player.main([]), 1)
            prepare.assert_not_called()
            run.assert_not_called()

    def test_conflicting_fallback_and_environment_refuse_before_preparation(self):
        for arguments, environment in [(['--', '--frame-readback'], {}),
                                        (['--', '--renderer', 'native'], {}),
                                        ([], {'MNM_NATIVE_DRAW': '0'}),
                                        ([], {'MNM_NATIVE_ONLY': '0'}),
                                        ([], {'MNM_MENU_DIRECTPLAY_DX6': '0'})]:
            with self.subTest(arguments=arguments, environment=environment), self.launch(environment=environment) as (prepare, _, run):
                self.assertEqual(player.main(arguments), 1)
                prepare.assert_not_called()
                run.assert_not_called()

    def test_nonzero_owner_exit_is_preserved(self):
        with self.launch() as (_, _, run):
            run.return_value = subprocess.CompletedProcess([], 124)
            self.assertEqual(player.main([]), 124)

    def test_missing_dependencies_refuse_before_preparation(self):
        with self.launch() as (prepare, _, run):
            with patch.object(player.shutil, 'which', return_value=None):
                self.assertEqual(player.main([]), 1)
            prepare.assert_not_called()
            run.assert_not_called()

    def test_prefix_template_never_overwrites_existing_prefix(self):
        with self.launch(existing=True) as (prepare, _, run):
            self.assertEqual(player.main(['--prefix-template', '/template']), 1)
            prepare.assert_not_called()
            run.assert_not_called()

    def test_prefix_must_be_under_working_and_without_symlinks(self):
        with patch.object(Path, 'is_symlink', return_value=False):
            for path in (Path('/outside'), ROOT / 'working', Path('working/../original/prefix')):
                with self.subTest(path=path), self.assertRaises(ValueError):
                    player.prefix_path(ROOT, path)
        with patch.object(Path, 'is_symlink', return_value=True):
            with self.assertRaises(ValueError):
                player.prefix_path(ROOT, Path('working/player/prefix'))

    def test_bootstrap_checks_manifest_even_on_failure_and_preserves_attempt(self):
        with self.launch() as (_, _, run), patch.object(player.tempfile, 'mkdtemp', return_value=str(ROOT / 'working/bootstrap/run')), patch.object(Path, 'write_text'), patch.object(player, 'stage', side_effect=ValueError('hash mismatch')):
            with self.assertRaisesRegex(ValueError, 'hash mismatch'):
                original_prepare(ROOT, ROOT / 'working/new-prefix', None, {})
            self.assertEqual([c.args[0] for c in run.call_args_list],
                             [[str(ROOT / 'tools/original-manifest.sh'), 'verify']] * 2)

    def test_bootstrap_uses_hash_checked_installer(self):
        prefix = ROOT / 'working/new-prefix'
        with self.launch() as (_, _, run), patch.object(player.tempfile, 'mkdtemp', return_value=str(ROOT / 'working/bootstrap/run')), patch.object(Path, 'write_text'), patch.object(player, 'stage', return_value={'fixture': True}) as stage, patch.object(player, 'admit_prefix') as admit, patch.object(player, 'install_prefix') as install:
            # launch() mocks prepare_prefix; invoke the saved implementation below.
            original_prepare(ROOT, prefix, None, {})
            stage.assert_called_once()
            admit.assert_called_once()
            self.assertEqual(install.call_args.args[2]['WINEPREFIX'], str(prefix))
            self.assertEqual(run.call_count, 2)

    def test_template_clone_uses_original_manifest_bookends(self):
        with self.launch() as (_, _, run), patch.object(player, 'clone') as clone:
            original_prepare(ROOT, ROOT / 'working/new-prefix', Path('/template'), {})
            clone.assert_called_once()
            self.assertEqual(run.call_count, 2)

    def capture_shell(self, script, *arguments):
        # Override exec only inside this Bash control; no child launcher runs.
        result = subprocess.run(['bash', '-c',
            'exec(){ printf "%s\\0" "$@"; exit 0; }; uname(){ echo x86_64; }; script=$1; shift; source "$script" "$@"',
            'launcher-control', str(ROOT / script), *arguments],
            capture_output=True, check=True)
        return result.stdout.decode().rstrip('\0').split('\0')

    def test_x86_default_routes_to_native_and_original_requires_opt_in(self):
        self.assertEqual(self.capture_shell('run-x86.sh'), [str(ROOT / 'run-native.sh')])
        self.assertEqual(self.capture_shell('run-x86.sh', '--original-renderer', 'check'),
                         [str(ROOT / 'tools/run-game.sh'), 'check'])

    def test_public_entrypoint_guards_launch_but_help_requires_no_storage(self):
        command = self.capture_shell('run-native.sh', '--', '--skip-movies')
        self.assertEqual(command[:8], ['python3', '-B', str(ROOT / 'tools/working-retention.py'),
                                      'run', '--reserve-gib', '4', '--', 'python3'])
        self.assertEqual(command[-2:], ['--', '--skip-movies'])
        self.assertEqual(self.capture_shell('run-native.sh', '--help'),
                         ['python3', '-B', str(ROOT / 'tools/run-native-game.py'), '--help'])


original_prepare = player.prepare_prefix

def run_report(path):
    import hashlib
    import json
    from coverage_claims import behavior_contract, required_sources, scenario_contract
    from coverage_evidence import write_report
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behaviors = [b for b in register['behaviors'] if b['id'] in
                 ('NP.native-player-launch', 'TL.player-lighting-default')]
    builds = {b['id']: b for b in register['builds']}
    paths = set().union(*(required_sources(b) for b in behaviors))
    sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sorted(paths)}
    claims = [{'behavior': b['id'], 'contract_sha256': behavior_contract(b, builds)} for b in behaviors]
    scenario = next(s for s in register['scenarios'] if s['id'] == 'TL.player-lighting-default-controls')
    next(c for c in claims if c['behavior'] == 'TL.player-lighting-default')['scenarios'] = {
        scenario['id']: scenario_contract(scenario)}
    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(PlayerLaunch))
    stable = all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == sha for p, sha in sources.items())
    report = dict(schema=1, success=result.wasSuccessful() and stable, sources_stable=stable,
                  sources=sources, claims=claims, tests=result.testsRun,
                  failures=[text for _, text in result.failures], errors=[text for _, text in result.errors],
                  scope='In-memory launch orchestration and actual Bash entrypoint routing. Filesystem initialization, Wine and native-owner execution are mocked.',
                  original_game_executed=False, original_artifacts_consumed=False,
                  actual_prefix_initialized=False, qt_built=False, complete_replacement=False,
                  command='python3 -B tests/test-native-player-launcher.py --report ' + str(path))
    write_report(ROOT, path, (json.dumps(report, indent=2) + '\n').encode())
    return 0 if report['success'] else 1


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--report':
        raise SystemExit(run_report(Path(sys.argv[2])))
    unittest.main()
