#!/usr/bin/env python3
"""In-memory player launch controls; no Wine, original inputs or test payload writes."""
import contextlib
import importlib.util
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
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
            stack.enter_context(patch.dict(os.environ, dict(MNM_MENU_GAME_SOURCE=str(ROOT/'working/player-launch-fixture-game'), **(environment or {})), clear=True))
            stack.enter_context(patch.object(player.platform, 'machine', return_value='x86_64'))
            stack.enter_context(patch.object(Path, 'exists', return_value=existing))
            stack.enter_context(patch.object(Path, 'is_file', return_value=True))
            stack.enter_context(patch.object(Path, 'is_symlink', return_value=False))
            stack.enter_context(patch.object(Path, 'mkdir'))
            stack.enter_context(patch.object(Path, 'open', return_value=MagicMock()))
            stack.enter_context(patch.object(player.shutil, 'which', return_value='/bin/fixture'))
            stack.enter_context(patch.object(player, 'owned_prefix'))
            stack.enter_context(patch.object(player, 'player_settings', return_value={}))
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
            self.assertEqual(environment['MNM_RTS_CONTROLS'], 'selection')
            self.assertEqual(player.lighting_selection(environment), ('native', 'none'))
            self.assertEqual(prepare.call_args.args[-1]['MNM_TERRAIN_LIGHTING'], 'native')
            self.assertNotIn('MNM_TERRAIN_LIGHTING', os.environ)
            prepare.assert_called_once()
            self.assertEqual(run.call_count, 1)  # No wineserver wait before fresh initialization.

    def test_selection_defaults_preserve_explicit_controls_modes(self):
        for mode in ('0','1','selection'):
            with self.subTest(mode=mode), self.launch(environment={'MNM_RTS_CONTROLS':mode}) as (_, _, run):
                self.assertEqual(player.main([]),0)
                self.assertEqual(run.call_args.kwargs['env']['MNM_RTS_CONTROLS'],mode)
        with self.launch(environment={'MNM_RTS_CONTROLS':'invalid'}) as (prepare, _, run):
            self.assertEqual(player.main([]),1)
            prepare.assert_not_called();run.assert_not_called()

    def test_linux_arm_defaults_to_independent_world(self):
        with self.launch() as (prepare, _, run), patch.object(player.platform, 'machine', return_value='aarch64'), patch.object(player.sys, 'platform', 'linux'):
            self.assertEqual(player.main([]), 0)
            self.assertEqual(run.call_args.args[0][-2:], ['--', '--independent-world'])
            self.assertNotIn('MNM_WORLD_SNAPSHOT', prepare.call_args.args[-1])
            self.assertEqual(prepare.call_args.args[-1]['MNM_WORLD_IMAGE_REUSE'], '1')
            self.assertEqual(run.call_args.kwargs['env']['MNM_WORLD_IMAGE_REUSE'], '1')
            self.assertEqual(run.call_args.kwargs['env']['MNM_WORLD_FIRST_SEAL_BASELINE'], '1')
            self.assertNotIn('MNM_WORLD_IMAGE_REUSE', os.environ)

    def test_local_player_defaults_reuse_configured_prefix_and_saves(self):
        settings = {'schema': 1, 'prefix': 'working/my player/prefix',
                    'save_store': 'working/my player/saves'}
        with self.launch(existing=True) as (prepare, _, run), patch.object(player, 'player_settings', return_value=settings):
            self.assertEqual(player.main([]), 0)
            self.assertEqual(prepare.call_args.args[1], ROOT/'working/my player/prefix')
            self.assertEqual(run.call_args.kwargs['env']['MNM_MENU_SAVE_STORE'], str(ROOT/'working/my player/saves'))
            self.assertNotIn('MNM_MENU_SAVE_STORE', os.environ)

    def test_explicit_prefix_and_save_environment_override_local_defaults(self):
        settings = {'schema': 1, 'prefix': 'working/local/prefix', 'save_store': 'working/local/saves'}
        with self.launch(environment={'MNM_MENU_SAVE_STORE': str(ROOT/'working/explicit/saves')}) as (prepare, _, run), patch.object(player, 'player_settings', return_value=settings):
            self.assertEqual(player.main(['--prefix', 'working/explicit/prefix']), 0)
            self.assertEqual(prepare.call_args.args[1], ROOT/'working/explicit/prefix')
            self.assertEqual(run.call_args.kwargs['env']['MNM_MENU_SAVE_STORE'], str(ROOT/'working/explicit/saves'))

    def test_local_defaults_match_the_previous_manual_arm_launch(self):
        settings={'schema':1,'prefix':'working/existing player/prefix','save_store':'working/existing player/saves'}
        with self.launch(existing=True, environment={'MNM_MENU_SAVE_STORE':str(ROOT/settings['save_store'])}) as (_, _, run), patch.object(player.platform, 'machine', return_value='aarch64'), patch.object(player.sys, 'platform', 'linux'):
            self.assertEqual(player.main(['--prefix',settings['prefix']]), 0)
            manual=run.call_args
        with self.launch(existing=True) as (_, _, run), patch.object(player, 'player_settings', return_value=settings), patch.object(player.platform, 'machine', return_value='aarch64'), patch.object(player.sys, 'platform', 'linux'):
            self.assertEqual(player.main([]), 0)
            self.assertEqual(run.call_args, manual)

    def test_help_does_not_read_local_settings_or_prepare_a_prefix(self):
        with self.launch() as (prepare, _, run), patch.object(player, 'player_settings', side_effect=ValueError('broken settings')) as settings:
            with self.assertRaises(SystemExit) as stopped:player.main(['--help'])
            self.assertEqual(stopped.exception.code, 0)
            settings.assert_not_called(); prepare.assert_not_called(); run.assert_not_called()

    def test_invalid_local_settings_refuse_before_prefix_preparation(self):
        with self.launch() as (prepare, _, run), patch.object(player, 'player_settings', side_effect=ValueError('bad settings')):
            self.assertEqual(player.main([]), 1)
            prepare.assert_not_called(); run.assert_not_called()
        with self.launch() as (prepare, _, run), patch.object(player, 'player_settings', return_value={'schema': 1, 'save_store': '/outside'}):
            self.assertEqual(player.main([]), 1)
            prepare.assert_not_called(); run.assert_not_called()

    def test_arm_image_reuse_explicit_diagnostic_override_is_preserved(self):
        for value in ('0', '1'):
            with self.subTest(value=value), self.launch(environment={'MNM_WORLD_IMAGE_REUSE': value}) as (_, _, run), patch.object(player.platform, 'machine', return_value='aarch64'), patch.object(player.sys, 'platform', 'linux'):
                self.assertEqual(player.main([]), 0)
                self.assertEqual(run.call_args.kwargs['env']['MNM_WORLD_IMAGE_REUSE'], value)

    def test_arm_legacy_independent_flag_also_uses_completed_image(self):
        with self.launch() as (_, _, run), patch.object(player.platform, 'machine', return_value='aarch64'), patch.object(player.sys, 'platform', 'linux'):
            self.assertEqual(player.main(['--', '--independent-world']), 0)
            self.assertEqual(run.call_args.args[0].count('--independent-world'), 1)
            self.assertEqual(run.call_args.kwargs['env']['MNM_WORLD_IMAGE_REUSE'], '1')
            self.assertEqual(run.call_args.kwargs['env']['MNM_WORLD_FIRST_SEAL_BASELINE'], '1')

    def test_arm_inherited_ui_diagnostic_override_is_preserved(self):
        with self.launch(environment={'MNM_WORLD_FIRST_SEAL_BASELINE': '0'}) as (_, _, run), patch.object(player.platform, 'machine', return_value='aarch64'), patch.object(player.sys, 'platform', 'linux'):
            self.assertEqual(player.main([]), 0)
            self.assertEqual(run.call_args.kwargs['env']['MNM_WORLD_FIRST_SEAL_BASELINE'], '0')

    def test_arm_synchronous_override_retains_baseline(self):
        with self.launch() as (_, _, run), patch.object(player.platform, 'machine', return_value='aarch64'):
            self.assertEqual(player.main(['--world-delivery', 'synchronous']), 0)
            self.assertNotIn('--independent-world', run.call_args.args[0])
            self.assertNotIn('MNM_WORLD_IMAGE_REUSE', run.call_args.kwargs['env'])
            self.assertNotIn('MNM_WORLD_FIRST_SEAL_BASELINE', run.call_args.kwargs['env'])

    def test_explicit_independent_mode_preserves_presentation_arguments(self):
        with self.launch() as (_, _, run):
            self.assertEqual(player.main(['--world-delivery', 'independent', '--', '--skip-movies', '--no-native-music']), 0)
            self.assertEqual(run.call_args.args[0][-3:], ['--skip-movies', '--no-native-music', '--independent-world'])
            self.assertNotIn('MNM_WORLD_IMAGE_REUSE', run.call_args.kwargs['env'])

    def test_independent_world_option_is_not_duplicated(self):
        with self.launch() as (_, _, run), patch.object(player.platform, 'machine', return_value='aarch64'):
            self.assertEqual(player.main(['--world-delivery', 'independent', '--', '--independent-world']), 0)
            self.assertEqual(run.call_args.args[0].count('--independent-world'), 1)

    def test_synchronous_conflict_refuses_before_preparation(self):
        with self.launch() as (prepare, _, run):
            self.assertEqual(player.main(['--world-delivery', 'synchronous', '--', '--independent-world']), 1)
            prepare.assert_not_called()
            run.assert_not_called()

    def test_non_linux_arm_keeps_baseline(self):
        with self.launch() as (_, _, run), patch.object(player.platform, 'machine', return_value='aarch64'), patch.object(player.sys, 'platform', 'darwin'):
            self.assertEqual(player.main([]), 0)
            self.assertNotIn('--independent-world', run.call_args.args[0])

    def test_explicit_owner_directory_is_forwarded_before_qt_arguments(self):
        owner=ROOT/'working/pilot/owned-case/owner'
        with self.launch() as (prepare, _, run):
            self.assertEqual(player.main(['--output-owner',str(owner),'--','--skip-movies']),0)
            command=run.call_args.args[0];index=command.index('--output')
            self.assertEqual(command[index:index+4],['--output',str(owner),'--','--skip-movies'])
            prepare.assert_called_once()

    def test_foreign_or_existing_owner_refuses_before_preparation(self):
        for owner,existing in ((Path('/tmp/foreign-owner'),False),(ROOT/'working/pilot/existing-owner',True)):
            with self.subTest(owner=owner),self.launch(existing=existing) as (prepare, _, run):
                self.assertEqual(player.main(['--output-owner',str(owner)]),1)
                prepare.assert_not_called();run.assert_not_called()

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
        with patch.dict(os.environ, {'MNM_WORKING_STORAGE_POLICY': str(ROOT/'tools/working-retention.json')}):
            command = self.capture_shell('run-native.sh', '--', '--skip-movies')
        self.assertEqual(command[:10], ['python3', '-B', str(ROOT / 'tools/working-retention.py'),
                                       '--policy', str(ROOT/'tools/working-retention.json'),
                                       'run', '--reserve-gib', '4', '--', 'python3'])
        with patch.dict(os.environ, {'MNM_WORKING_STORAGE_POLICY': '/owned/alternate-policy.json'}):
            alternate = self.capture_shell('run-native.sh', '--', '--skip-movies')
        self.assertEqual(alternate[3:5], ['--policy', '/owned/alternate-policy.json'])
        self.assertEqual(command[-2:], ['--', '--skip-movies'])
        self.assertEqual(self.capture_shell('run-native.sh', '--help'),
                         ['python3', '-B', str(ROOT / 'tools/run-native-game.py'), '--help'])

    def test_storage_policy_defaults_are_checkout_local_and_keep_explicit_override(self):
        with tempfile.TemporaryDirectory(prefix='native-player-settings-') as name:
            root = Path(name); (root/'working').mkdir()
            script = root/'run-native.sh'; script.write_bytes((ROOT/'run-native.sh').read_bytes())
            def capture(environment):
                result = subprocess.run(['bash', '-c',
                    'exec(){ printenv MNM_WORKING_STORAGE_POLICY > "$LAUNCH_POLICY_CAPTURE"; printf "%s\\0" "$@"; exit 0; }; script=$1; shift; source "$script" "$@"',
                    'policy-control', str(script)], env=environment, capture_output=True, check=True)
                policy = result.stdout.decode().rstrip('\0').split('\0')[4]
                self.assertEqual((root/'inherited-policy').read_text().rstrip('\n'), policy)
                return policy
            environment = dict({k:v for k,v in os.environ.items() if k != 'MNM_WORKING_STORAGE_POLICY'},
                               LAUNCH_POLICY_CAPTURE=str(root/'inherited-policy'))
            self.assertEqual(capture(environment), str(root/'tools/working-retention.json'))
            (root/'working/storage-policy-local.json').write_text('{}')
            self.assertEqual(capture(environment), str(root/'working/storage-policy-local.json'))
            self.assertEqual(capture(dict(environment, MNM_WORKING_STORAGE_POLICY='/explicit policy.json')), '/explicit policy.json')


class PlayerSettings(unittest.TestCase):
    def test_absent_settings_preserve_default(self):
        with tempfile.TemporaryDirectory() as name:
            self.assertEqual(player.player_settings(Path(name)), {})

    def test_versioned_settings_accept_paths_with_spaces(self):
        with tempfile.TemporaryDirectory() as name:
            root=Path(name); path=root/'working/native-player/launch.json'; path.parent.mkdir(parents=True)
            settings={'schema':1,'prefix':'working/player one/prefix','save_store':'working/player one/saves'}
            path.write_text(json.dumps(settings))
            self.assertEqual(player.player_settings(root), settings)

    def test_malformed_unsupported_and_oversized_settings_refuse(self):
        cases=['{', '[]', '{"schema":true}', '{"schema":2}', '{"schema":1,"unknown":1}',
               '{"schema":1,"prefix":null}', '{"schema":1,"save_store":" "}', ' '*8193]
        with tempfile.TemporaryDirectory() as name:
            root=Path(name); path=root/'working/native-player/launch.json'; path.parent.mkdir(parents=True)
            for content in cases:
                with self.subTest(content=content[:60]):
                    path.write_text(content)
                    with self.assertRaises(ValueError):player.player_settings(root)

    def test_symlinked_settings_refuse(self):
        with tempfile.TemporaryDirectory() as name:
            root=Path(name); path=root/'working/native-player/launch.json'; path.parent.mkdir(parents=True)
            target=root/'settings.json'; target.write_text('{"schema":1}'); path.symlink_to(target)
            with self.assertRaises(ValueError):player.player_settings(root)


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
