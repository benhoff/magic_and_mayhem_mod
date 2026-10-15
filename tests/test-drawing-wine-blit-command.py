#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('blit',ROOT/'tools/inspect-drawing-wine-blit-command.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class BlitDiscovery(unittest.TestCase):
    def test_pinned_selected_dispatch(self):
        data=Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes();x=m.inspect(data)
        self.assertEqual(x['dispatch']['handler_rva'],0x1ce280);self.assertEqual(x['emitter']['command_opcode'],42);self.assertFalse(x['worker_identity_observed'])
    def test_module_drift_and_truncation_refuse(self):
        data=Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes()
        for bad in (data[:-1],bytes([data[0]^1])+data[1:]):
            with self.assertRaises(ValueError):m.inspect(bad)


if __name__=='__main__':unittest.main()
