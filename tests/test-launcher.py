#!/usr/bin/env python3
"""Check launcher command composition without starting Wine or a compositor.

Requires the prepared working installation, like test-tooling.sh.
"""

import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest


REPO = Path(__file__).resolve().parents[1]


class LauncherTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="mnm-launcher-")
        self.addCleanup(self.temp.cleanup)
        self.scope = Path(self.temp.name) / "fake gamescope"
        self.scope.write_text('#!/bin/bash\nwhile [[ $1 != -- ]]; do shift; done\nshift\nexec "$@"\n')
        self.scope.chmod(0o755)
        self.env = dict(os.environ, MNM_GAMESCOPE_BIN=str(self.scope),
                        MNM_DISPLAY_SIZE="1920x1080", DISPLAY=":99",
                        WINEPREFIX=str(REPO / "working/wineprefix-test"))
        self.env.pop("MNM_RUNNER", None)

    def check_command(self, *args):
        result = subprocess.run(
            [str(REPO / "tools/run-game.sh"), "check", "--wine",
             str(REPO / "tests/fixtures/fake-wine.sh"), *args],
            env=self.env, capture_output=True, text=True, check=True)
        command = next(line.removeprefix("Command: ")
                       for line in result.stdout.splitlines() if line.startswith("Command: "))
        return shlex.split(command)

    def test_default_and_game_arguments(self):
        command = self.check_command("--", "argument with spaces", "--game-option")
        self.assertEqual(command[:14], [str(self.scope), "-w", "800", "-h", "600",
                         "-W", "1280", "-H", "960", "-S", "fit", "-F", "nearest", "-f"])
        self.assertEqual(command[14], "--")
        self.assertEqual(command[-2:], ["argument with spaces", "--game-option"])
        self.assertNotIn("explorer", command)

    def test_sizes_and_extra_options(self):
        command = self.check_command("-w", "--size", "1600x1200", "--game-size", "640x480",
                                     "--gamescope-arg", "-F", "--gamescope-arg", "linear")
        self.assertEqual(command[1:9], ["-w", "640", "-h", "480", "-W", "1600", "-H", "1200"])
        self.assertEqual(command[13:16], ["-F", "linear", "--"])
        self.assertNotIn("-f", command)
        self.assertNotIn("explorer", command)
        self.assertEqual(self.check_command("--size", "1600x1200", "-w")[5:9],
                         ["-W", "1600", "-H", "1200"])

    def test_scale_and_window_size(self):
        command = self.check_command("-s")
        self.assertEqual(command[5:9], ["-W", "1920", "-H", "1080"])
        command = self.check_command("--window-size", "1600x1200")
        self.assertEqual(command[5:9], ["-W", "1600", "-H", "1200"])
        self.assertNotIn("-f", command)

    def test_direct_wine(self):
        command = self.check_command("--no-gamescope", "-w")
        self.assertEqual(command[1:3], ["explorer", "/desktop=MagicMayhem,1280x960"])
        self.assertNotIn(str(self.scope), command)
        for args in [("--size", "1600x1200", "--no-gamescope"),
                     ("--no-gamescope", "--size", "1600x1200")]:
            self.assertIn("/desktop=MagicMayhem,1600x1200", self.check_command(*args))

    def test_runner_composition(self):
        runner = str(REPO / "tests/fixtures/fake-game-runner.sh")
        command = self.check_command("--runner", runner, "-w")
        self.assertEqual(command[14], runner)
        self.assertTrue(command[15].endswith("/Chaos.exe"))

    def test_invalid_arguments_and_missing_compositor(self):
        for args in [("--size", "0x600"), ("--game-size", "800"),
                     ("--gamescope-arg",), ("--gamescope-arg", "--"),
                     ("--no-gamescope", "--gamescope-arg", "-r")]:
            result = subprocess.run([str(REPO / "tools/run-game.sh"), "check", *args],
                                    env=self.env, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0, args)
        self.env["MNM_GAMESCOPE_BIN"] = str(Path(self.temp.name) / "missing")
        with self.assertRaises(subprocess.CalledProcessError) as caught:
            self.check_command()
        self.assertIn("install gamescope or use --no-gamescope", caught.exception.stderr)
        self.check_command("--no-gamescope")


if __name__ == "__main__":
    unittest.main()
