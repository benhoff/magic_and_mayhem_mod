#!/usr/bin/env python3
"""Tests independent of Ghidra; no live game or original media consumed."""

import contextlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

REPO = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("decompile_game", REPO / "tools/decompile-game.py")
tool = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tool)


class ExportTests(unittest.TestCase):
    def test_annotated_export_requires_successful_markup(self):
        executable = REPO / "working/game-nocd/Chaos.exe"
        real_run = tool.subprocess.run
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for annotations_ok in (False, True):
                def fake_run(command, **kwargs):
                    if "ExportGameDecompilation.java" not in command:
                        return real_run(command, **kwargs)
                    self.assertIn("AnnotateRouteMilestone.java", command)
                    output = Path(command[command.index("ExportGameDecompilation.java") + 1])
                    (output / "export-summary.json").write_text('{"succeeded":7,"failed":0}')
                    kwargs["stdout"].write(
                        "Applied hash-checked route milestone names, signatures and partial types.\n"
                        if annotations_ok else "ERROR: annotation script failed\n")
                    return SimpleNamespace(returncode=0)

                with patch.object(tool, "REPO", root), \
                     patch.object(tool, "find_headless", return_value=Path("/fixture/analyzeHeadless")), \
                     patch.object(tool, "prepare_native", side_effect=lambda p: p), \
                     patch.object(tool.subprocess, "run", side_effect=fake_run):
                    self.assertEqual(self.run_tool(str(executable), "--annotated"),
                                     0 if annotations_ok else 1)

    def test_arm_native_build_preserves_user_install(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            installation = root / "user-install"
            cpp = installation / "Ghidra/Features/Decompiler/src/decompile/cpp"
            cpp.mkdir(parents=True)
            (cpp / "Makefile").write_text("# fixture\n")
            (installation / "support").mkdir()
            headless = installation / "support/analyzeHeadless"
            headless.write_text("#!/bin/sh\n")

            def build(command, **kwargs):
                self.assertIn("ARCH_TYPE=", command)
                (kwargs["cwd"] / "ghidra_opt").write_bytes(b"fixture native decompiler")
                return SimpleNamespace(returncode=0)

            with patch.object(tool, "REPO", root / "repo"), \
                 patch.object(tool.platform, "system", return_value="Linux"), \
                 patch.object(tool.platform, "machine", return_value="aarch64"), \
                 patch.object(tool.shutil, "which", return_value="/fixture/tool"), \
                 patch.object(tool.subprocess, "run", side_effect=build), \
                 contextlib.redirect_stdout(io.StringIO()):
                prepared = tool.prepare_native(headless)
                self.assertNotEqual(prepared, headless)
                native = prepared.parent.parent / "Ghidra/Features/Decompiler/os/linux_arm_64/decompile"
                self.assertTrue(native.exists())
                self.assertEqual(tool.prepare_native(prepared), prepared)
            self.assertFalse((installation / "Ghidra/Features/Decompiler/os").exists())
            self.assertFalse((cpp / "ghidra_opt").exists())

    def test_gui_launcher_symlink_discovery(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            installation = root / "ghidra_install"
            (installation / "support").mkdir(parents=True)
            headless = installation / "support/analyzeHeadless"
            headless.write_text("#!/bin/sh\nexit 0\n")
            headless.chmod(0o755)
            launcher = installation / "ghidraRun"
            launcher.touch()
            link = root / "ghidra"
            link.symlink_to(launcher)
            with patch.object(tool, "REPO", root), \
                 patch.object(tool.shutil, "which", side_effect=lambda name: str(link) if name == "ghidra" else None), \
                 patch.dict(tool.os.environ, {"GHIDRA_HOME": ""}):
                self.assertEqual(tool.find_headless(None), headless)

    def run_tool(self, *args):
        with patch.object(sys, "argv", ["decompile-game.py", *args]), \
             contextlib.redirect_stdout(io.StringIO()), \
             contextlib.redirect_stderr(io.StringIO()):
            return tool.main()

    def test_hash_rejection_before_output_creation(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            executable = root / "unknown.exe"
            executable.write_bytes(b"not the supported executable")
            with patch.object(tool, "REPO", root):
                with self.assertRaisesRegex(ValueError, "Unknown executable"):
                    self.run_tool(str(executable))
            self.assertFalse((root / "working").exists())

    def test_missing_ghidra_and_non_overwriting_disassembly(self):
        executable = REPO / "working/game-nocd/Chaos.exe"
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with patch.object(tool, "REPO", root), \
                 patch.object(tool, "find_headless", return_value=None):
                self.assertEqual(self.run_tool(str(executable)), 1)
                self.assertEqual(self.run_tool(str(executable), "--disassembly-only"), 0)
            runs = list((root / "working/decompiled").iterdir())
            self.assertEqual(len(runs), 2)
            for run in runs:
                manifest = json.loads((run / "manifest.json").read_text())
                self.assertEqual(manifest["status"], "disassembly_only")
                self.assertEqual(manifest["sha256"], tool.EXPECTED_HASH)
                self.assertEqual(len(list(run.glob("*.asm"))), 7)
                self.assertIn("512800:", (run / "route_request.asm").read_text())
                self.assertIn("ret", (run / "route_search.asm").read_text())
                self.assertFalse((run / "c").exists())


if __name__ == "__main__":
    unittest.main()
