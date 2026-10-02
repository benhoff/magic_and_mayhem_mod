#!/usr/bin/env python3
"""Synthetic trace/replay tests; these are not observations of a running game."""
import copy
import importlib.util
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
        with self.assertRaises(RuntimeError):
            trace.memory_values("00400104: 00000001\n", 0x400100, 1, 4)

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
