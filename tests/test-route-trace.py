#!/usr/bin/env python3
"""Synthetic trace/replay tests; these are not observations of a running game."""
import copy
import importlib.util
import os
from pathlib import Path
import struct
import tempfile
import unittest

REPO = Path(__file__).resolve().parent.parent


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, REPO / "tools" / filename)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


trace = load("trace", "trace-route-experiment.py")
validator = load("validator", "validate-route-trace.py")


def fixture(count):
    snapshot = bytearray(0x20c)
    struct.pack_into("<I", snapshot, 0x10, count)
    snapshot[-1] = 0x77
    return {"image_base": 0x400000, "object": 0x123000, "entry_sp": 0x456000,
        "arguments": [0xffffffff, 131, 0xfffffff9, 42], "dimensions": [64, 64],
        "configured_budget": 100, "snapshot_from_search": snapshot.hex(),
        "flag_after_search": 7, "budget_after": 80,
        "search": {"coordinates": [63, 3, 0xfffffff9], "unknown_argument": 42,
            "object": 0x123000, "context": 0x690148, "budget_pointer": 0x455ffc,
            "budget_before": 100, "flag_before": 1},
        "return": {"snapshot": snapshot.hex(), "eax": int(count != 0),
            "route_present": int(count != 0), "unknown_d03": 0,
            "configured_budget": 100, "flag": 7}}


class TraceTests(unittest.TestCase):
    def test_prefix_selection(self):
        self.assertEqual(trace.resolve_prefix(None, {}, "x86_64"),
                         REPO / "working/wineprefix-x86_64")
        self.assertEqual(trace.resolve_prefix(None, {}, "i686"),
                         REPO / "working/wineprefix-i686")
        self.assertEqual(trace.resolve_prefix(None, {}, "aarch64"),
                         REPO / "working/wineprefix")
        self.assertEqual(trace.resolve_prefix(None, {"WINEPREFIX": "/tmp/custom-wine"}, "x86_64"),
                         Path("/tmp/custom-wine"))
        self.assertEqual(trace.resolve_prefix("/tmp/explicit-wine", {"WINEPREFIX": "/tmp/custom-wine"}, "x86_64"),
                         Path("/tmp/explicit-wine"))

    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory()
        cls.replay = validator.build_replay(Path(cls.directory.name))

    @classmethod
    def tearDownClass(cls):
        cls.directory.cleanup()

    def test_module_base(self):
        self.assertEqual(trace.module_base(" PE 00400000-    006d0000 Deferred chaos\n"), 0x400000)
        self.assertEqual(trace.module_base("PE 01400000-016d0000 Export chaos.exe\n"), 0x1400000)
        for text in ("PE 00400000-006d0000 chaos_helper", "",
                     "PE 00400000-006d0000 chaos\nPE 01400000-016d0000 chaos\n"):
            with self.assertRaises(RuntimeError):
                trace.module_base(text)

    def test_memory_parser(self):
        self.assertEqual(trace.memory_values("00400100: 00000001 ffffffff\n"
            "00400108: 00000003\n", 0x400100, 3, 4), [1, 0xffffffff, 3])
        self.assertEqual(trace.memory_values("\x1b[?25l00400100: 51 8b 54\n",
            0x400100, 3, 1), [0x51, 0x8b, 0x54])
        self.assertEqual(trace.memory_values(
            "0x00000000512800 chaos+0x112800:  51 8b 54 24 10\n",
            0x512800, 5, 1), [0x51, 0x8b, 0x54, 0x24, 0x10])
        self.assertEqual(trace.memory_values(
            "0x00000000400100 chaos+0x100:  00000001 ffffffff\n"
            "0x00000000400108:  00000003\n", 0x400100, 3, 4),
            [1, 0xffffffff, 3])
        with self.assertRaises(RuntimeError):
            trace.memory_values("00400104: 00000001\n", 0x400100, 1, 4)

    def test_single_value_memory_output(self):
        # Actual failing x86 WineDbg response to x /1x 0x6c5494.
        self.assertEqual(trace.memory_values(
            "\x1b[?25lx /1x 0x6c5494\r\n\x1b[?25h 00000050\r\n",
            0x6c5494, 1, 4), [80])
        self.assertEqual(trace.memory_values(" 01\r\n", 0x690354, 1, 1), [1])
        for output in ("Cannot read memory 00000050\n", "00000050\n00000060\n",
                       "00000050\n", "ff\n"):
            count, width = (2, 4) if output == "00000050\n" else (1, 4)
            with self.subTest(output=output), self.assertRaises(RuntimeError):
                trace.memory_values(output, 0x6c5494, count, width)

    def test_recoverable_cleanup_detaches_after_removing_breakpoints(self):
        class Process:
            def poll(self):
                return None
            def wait(self, timeout):
                return 0

        debugger = object.__new__(trace.Debugger)
        debugger.process = Process()
        debugger.stopped = True
        debugger.target_exited = False
        debugger.master, slave = trace.pty.openpty()
        debugger.log = tempfile.TemporaryFile()
        debugger.commands = tempfile.TemporaryFile()
        commands = []
        debugger.command = lambda command: commands.append(command)
        try:
            self.assertEqual(debugger.cleanup([1, 2, 3, 4], keep_game=True),
                             "breakpoints_removed_and_detached")
            self.assertEqual(commands, ["delete 1", "delete 2", "delete 3", "delete 4", "detach"])
        finally:
            os.close(slave)

    def test_target_exit_is_reported_before_register_read(self):
        debugger = object.__new__(trace.Debugger)
        debugger.buffer = (
            b"Invalid address (0x00000000512800 chaos+0x112800) for breakpoint 1, disabling it\r\n"
            b"\x1b[?25lProcess of pid=0134 has terminated\r\nWine-dbg>")
        debugger.target_exited = False
        with self.assertRaisesRegex(trace.TargetExited, "Game process exited"):
            debugger.prompt(1)
        self.assertTrue(debugger.target_exited)

    def test_cleanup_does_not_detach_from_exited_target(self):
        class Process:
            def poll(self):
                return None
            def wait(self, timeout):
                return 0

        debugger = object.__new__(trace.Debugger)
        debugger.process = Process()
        debugger.target_exited = True
        debugger.master, slave = trace.pty.openpty()
        debugger.log = tempfile.TemporaryFile()
        debugger.commands = tempfile.TemporaryFile()
        def unexpected_command(command):
            self.fail(f"Exited target must not receive {command}")
        debugger.command = unexpected_command
        try:
            self.assertEqual(debugger.cleanup([1, 2, 3, 4], keep_game=True),
                             "target_already_exited")
        finally:
            os.close(slave)

    def test_non_x86_fails_before_breakpoints(self):
        class WrongArchitecture:
            def register(self, name):
                raise RuntimeError("no x86 context")
            def command(self, command):
                raise AssertionError("must not install breakpoints")
        with self.assertRaisesRegex(RuntimeError, "no x86"):
            trace.capture(WrongArchitecture(), 0x400000, 1, 1,
                          Path(self.directory.name), [])

    def test_replay_counts(self):
        for count in (0, 1, 16, 17, 0xffffffff):
            with self.subTest(count=count):
                self.assertEqual(validator.validate_call(fixture(count), self.replay), [])

    def test_wrong_bytes_fail_before_breakpoints(self):
        class WrongBytes:
            def register(self, name):
                return 0x400000
            def memory(self, address, count, width):
                return [0] * count
            def command(self, command):
                raise AssertionError("must not install breakpoints")
        with self.assertRaisesRegex(RuntimeError, "instruction bytes"):
            trace.capture(WrongBytes(), 0x400000, 1, 1,
                          Path(self.directory.name), [])

    def test_detects_mismatches(self):
        changes = [("search", "coordinates", [62, 3, 0xfffffff9]),
            ("search", "object", 0), ("search", "context", 0),
            ("search", "budget_pointer", 0), ("search", "flag_before", 0),
            ("return", "eax", 0), ("return", "unknown_d03", 1),
            ("return", "flag", 0), ("return", "snapshot", "00" * 0x20c)]
        for section, field, value in changes:
            record = copy.deepcopy(fixture(1))
            record[section][field] = value
            with self.subTest(field=field):
                self.assertTrue(validator.validate_call(record, self.replay))

    def test_rejects_invalid_domain_and_snapshot(self):
        for field, value in (("dimensions", [0, 64]),
                             ("snapshot_from_search", "00")):
            record = fixture(1)
            record[field] = value
            with self.assertRaises(ValueError):
                validator.validate_call(record, self.replay)


if __name__ == "__main__":
    unittest.main()
