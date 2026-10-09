#!/usr/bin/env python3
"""Synthetic completion/refusal checks; no engine or original artifacts consumed."""
import hashlib
import io
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from native_world_startup import complete, run_startup


class StartupCompletion(unittest.TestCase):
    def test_capture_dispatch(self):
        for returncode in (0, 1):
            with self.subTest(returncode=returncode), tempfile.TemporaryDirectory() as directory:
                repository=Path(directory)
                root=repository/'working/experiments/scene-observer/run-fixture'
                root.mkdir(parents=True);self.fixture(root)
                class Process:
                    def __init__(self):self.stdout=io.StringIO(str(root)+'\n');self.returncode=returncode
                    def poll(self):return self.returncode
                    def wait(self):return self.returncode
                with patch('native_world_startup.subprocess.Popen',return_value=Process()) as launch:
                    if returncode:
                        with self.assertRaises(RuntimeError):run_startup(repository,repository/'build',repository/'prefix')
                        self.assertFalse(json.loads((root/'native-world.json').read_text())['success'])
                    else:run_startup(repository,repository/'build',repository/'prefix')
                    command=launch.call_args.args[0]
                    self.assertIn('--canvas-producers',command)
                    self.assertIn('--world-producer-handoff',command)
                    self.assertIn('--skip-window-screenshot',command)
                    self.assertEqual(command[command.index('--producer-oracle-mib')+1],'3072')
                    self.assertNotIn('--world-raster-prefix',command)
                    self.assertEqual(command[command.index('--samples')+1],'16')
                    self.assertTrue(launch.call_args.kwargs['start_new_session'])
                    self.assertFalse(any(k.startswith('MNM_') for k in launch.call_args.kwargs['env']))

    def fixture(self, root):
        capture = root/'capture'; capture.mkdir()
        output = root/'native-producers'; output.mkdir()
        h = bytearray(64); h[:8] = b'MNMPRO01'
        struct.pack_into('<5I', h, 8, 1, 64, 0x40209ca7, 32768, 16)
        records = []; checks = []
        for q in range(1, 17):
            for kind in (11, 10, 12):
                r = [0]*24; r[0]=96; r[1]=len(records)+1; r[2]=kind
                r[3]=1; r[5]=r[6]=1; r[14]=q
                records.append(struct.pack('<24I', *r))
                if kind==10:
                    pixels = struct.pack('<H', q)
                    (capture/f'producer-oracle-{q:04d}.565').write_bytes(pixels)
                    path = f'native-{q:04d}.565'; (output/path).write_bytes(pixels)
                    checks.append(dict(sequence=r[1], oracle=q, canvas=1, path=path,
                                       sha256=hashlib.sha256(pixels).hexdigest(), gpu_equal=True))
            raw = b'MNMSTQ01'+struct.pack('<14I', 1,64,100,q,q,0,0,1,1,0,0,36,1,0)+struct.pack('<9I',0,0,0,0,0,0,8,0,0)
            (capture/f'startup-queue-{q:04d}.bin').write_bytes(raw)
        stream = bytes(h)+b''.join(records)
        (capture/'canvas-producers.bin').write_bytes(stream)
        (capture/'canvas-producers.done').write_bytes(b'MNMPDONE'+struct.pack('<6I',1,16,48,0,16,len(stream)))
        native = dict(success=True, queues=16, records=48, remaining_surfaces=0,
                      viewport_image_uploads=0, original_oracles_read=False,
                      original_pixels_used_as_native_inputs=False, native_bypass_count=0,
                      live_replacement=False, input_sha256=hashlib.sha256(stream).hexdigest(),
                      world_handoff=True, world_queues=16,
                      world_frames=[dict(queue=q,gpu_world_equal=True) for q in range(1,17)], checkpoints=checks)
        (output/'live-report.json').write_text(json.dumps(native))
        return native

    def test_completion_and_refusals(self):
        cases = ['verify','normal','pixel-mismatch','normal-different-pixels','missing-marker','missing-queue',
                 'missing-native-report','missing-native-canvas','identity','hash','gpu','world-count',
                 'record-count','bypass','original-input','worker-error','source-hash','world-order','writeback']
        for case in cases:
            with self.subTest(case=case), tempfile.TemporaryDirectory() as directory:
                root=Path(directory); native=self.fixture(root)
                if case in ('pixel-mismatch','normal-different-pixels'):
                    (root/'capture/producer-oracle-0001.565').write_bytes(b'\xff\xff')
                if case=='missing-marker': (root/'capture/canvas-producers.done').unlink()
                if case=='missing-queue': (root/'capture/startup-queue-0002.bin').unlink()
                if case=='missing-native-canvas': (root/'native-producers/native-0001.565').unlink()
                if case=='identity': native['checkpoints'][0]['canvas']=2
                if case=='hash': native['checkpoints'][0]['sha256']='bad'
                if case=='gpu': native['checkpoints'][0]['gpu_equal']=False
                if case=='world-count': native['world_frames'].pop()
                if case=='record-count': native['records']=47
                if case=='source-hash': native['input_sha256']='bad'
                if case=='world-order': native['world_frames'].reverse()
                if case=='writeback': native['native_canvas_writebacks']=1
                if case=='bypass': native['native_bypass_count']=1
                if case=='original-input': native['original_pixels_used_as_native_inputs']=True
                if case=='worker-error': native.update(success=False,error='unsupported draw')
                (root/'native-producers/live-report.json').write_text(json.dumps(native))
                if case=='missing-native-report': (root/'native-producers/live-report.json').unlink()
                good=case in ('verify','normal','normal-different-pixels')
                if good:
                    result=complete(root,case=='verify')
                    self.assertTrue(result['success'])
                    self.assertEqual(len(result['checkpoints']),16)
                    self.assertEqual(result['startup_queues']['unsupported_kind_counts'],{8:16})
                else:
                    with self.assertRaises(RuntimeError): complete(root,True)
                    result=json.loads((root/'native-world.json').read_text())
                    self.assertFalse(result['success'])
                    self.assertTrue(result['error'])
                    if case=='pixel-mismatch': self.assertEqual(result['checkpoints'][0]['mismatches'],1)


if __name__=='__main__': unittest.main()
