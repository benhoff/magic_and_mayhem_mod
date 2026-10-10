#!/usr/bin/env python3
"""Bounded batch launcher dispatch, cancellation and failure semantics without game media."""
import io
import json
from pathlib import Path
import sys
import struct
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from native_world_batch import command, producer_refusal, run_batch


class BatchLauncher(unittest.TestCase):
    def test_original_refusal_survives_window_close(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);capture=root/'capture';capture.mkdir()
            self.assertIsNone(producer_refusal(root))
            row=[0]*24;row[0]=96;row[1]=1;row[2]=13;row[14]=45;row[15]=8
            path=capture/'canvas-producers.bin';path.write_bytes(bytes(64)+struct.pack('<24I',*row))
            self.assertIn('reason 45',producer_refusal(root))
            row[2]=9;path.write_bytes(bytes(64)+struct.pack('<24I',*row)[:50])
            self.assertIsNone(producer_refusal(root))
            (capture/'world-batch-fault-0001.bin').write_bytes(b'fault')
            self.assertIn('guard refused',producer_refusal(root))
    def test_commands(self):
        for manual in (False, True):
            args=command(Path('/repo'), Path('/build'), Path('/prefix'), manual, Path('/claims'))
            self.assertEqual('--manual-input' in args, manual)
            for flag in ('--world-raster-prefix','--samples'):
                self.assertEqual(args[args.index(flag)+1], '16')
            self.assertIn('--world-raster-batch', args)
            self.assertIn('--skip-window-screenshot', args)
            self.assertEqual(args[args.index('--claims')+1], '/claims')

    def test_extended_command_requires_exact_bound(self):
        args=command(Path('/repo'),Path('/build'),Path('/prefix'),False,queues=32)
        for flag in ('--world-raster-prefix','--samples'):
            self.assertEqual(args[args.index(flag)+1],'32')
        for queues in (0,17,31,33,64):
            with self.assertRaises(ValueError):command(Path('/repo'),Path('/build'),Path('/prefix'),False,queues=queues)

    def test_completion_cancellation_refusal(self):
        for case in ('complete','cancel','refuse','incomplete'):
            with self.subTest(case=case), tempfile.TemporaryDirectory() as directory:
                repository=Path(directory);root=repository/'working/experiments/scene-observer/run-fixture';root.mkdir(parents=True)
                result=dict(success=True,native_canvas_writebacks=16,native_producers=dict(profile=dict(cpu_composition_ms=1,checkpoint_diagnostics_ms=2,stream_ingest_ms=3)))
                if case=='cancel':result=dict(success=False,cancelled=True)
                if case=='incomplete':result['native_canvas_writebacks']=15
                (root/'report.json').write_text(json.dumps(result))
                class Process:
                    stdout=io.StringIO(str(root)+'\n')
                    returncode=1 if case=='refuse' else 0
                    def poll(self):return self.returncode
                    def wait(self):return self.returncode
                with patch('native_world_batch.subprocess.Popen', return_value=Process()) as launch:
                    if case in ('refuse','incomplete'):
                        with self.assertRaises(RuntimeError):run_batch(repository,repository/'build',repository/'prefix')
                    else:
                        run_batch(repository,repository/'build',repository/'prefix')
                        self.assertEqual(json.loads((root/'native-world.json').read_text()),result)
                    self.assertTrue(launch.call_args.kwargs['start_new_session'])
                    self.assertFalse(any(k.startswith('MNM_') for k in launch.call_args.kwargs['env']))


if __name__=='__main__':unittest.main()
