import copy
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from canvas_driver_observation import analyze


class DriverObservation(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.directory=Path(self.tmp.name)
        self.rows=[[0]*32 for _ in range(4)]
        for n,row in enumerate(self.rows,1):
            row[0]=n;row[4]=42;row[27]=1;row[30]=int(n>1)
        created,blt,clear,terminal=self.rows
        created[1:4]=[10,0x6a49e0,0x4e41f1]
        created[5:9]=[100,200,300,300]
        blt[1:4]=[20,0x6a49e0,0x58bc5a];blt[5:7]=[100,200]
        clear[1:4]=[30,0x6a49e0,0x4e41fd];clear[5:7]=[100,200]
        clear[15:22]=[2,2,4,16,0xf800,0x7e0,31]
        fnv=2166136261
        for byte in bytes(8):fnv=((fnv^byte)*16777619)&0xffffffff
        clear[24:27]=[fnv,8,7]
        terminal[1]=99;terminal[25]=1
        (self.directory/'canvas-driver-clear.565').write_bytes(bytes(8))

    def save(self):
        header=bytearray(64);header[:8]=b'MNMCDB01'
        struct.pack_into('<4I',header,8,1,128,0x40209ca7,4096)
        raw=header+b''.join(struct.pack('<32I',*row) for row in self.rows)
        (self.directory/'canvas-driver.bin').write_bytes(raw)

    def test_original_active_observation_rejects_native_substitution(self):
        runner=Path(__file__).resolve().parents[1]/'tools/capture-scene-game.py'
        for option in ['--native-draw','--native-creature-drawing','--native-actor-lifecycle']:
            with self.subTest(option=option):
                result=subprocess.run([sys.executable,str(runner),'--canvas-startup',
                    '--canvas-driver-observe','--samples','1',option],
                    capture_output=True,text=True)
                self.assertEqual(result.returncode,2)
                self.assertIn('Driver clear observation requires original-active',result.stderr)

    def test_valid_real_lease_receipt_is_separate_from_global_ownership(self):
        self.save();result=analyze(self.directory)
        self.assertEqual(result['pixels_compared'],4)
        self.assertEqual(result['initial_blt_hresult'],0)
        self.assertFalse(result['physical_allocation_generation_proved'])
        self.assertFalse(result['driver_loss_restore_observed'])
        self.assertFalse(result['original_pixels_used_as_native_inputs'])

    def test_result_identity_thread_generation_and_lease_failures_refuse(self):
        original=copy.deepcopy(self.rows)
        for at,column,value in [(0,7,301),(0,9,1),(0,3,0),(1,11,1),
                (1,4,43),(1,30,0),(2,12,1),(2,13,1),(2,14,1),(2,26,3),
                (2,15,4096),(2,18,32),(2,23,1),(2,25,7),(2,27,2),
                (3,25,0),(3,29,1),(3,31,1),(3,1,20)]:
            self.rows=copy.deepcopy(original);self.rows[at][column]=value;self.save()
            with self.subTest(row=at,column=column),self.assertRaises(ValueError):
                analyze(self.directory)

    def test_missing_truncated_and_changed_pixels_refuse(self):
        self.save();p=self.directory/'canvas-driver.bin';raw=p.read_bytes()
        p.write_bytes(raw[:-1])
        with self.assertRaises(ValueError):analyze(self.directory)
        p.write_bytes(raw)
        (self.directory/'canvas-driver-clear.565').write_bytes(bytes(7)+b'\x01')
        with self.assertRaises(ValueError):analyze(self.directory)


if __name__=='__main__':unittest.main()
