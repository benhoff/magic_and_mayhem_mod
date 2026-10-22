#!/usr/bin/env python3
"""Owned raw-journal pipelines and actual SQLite CLI controls; no game execution."""
import contextlib
import copy
import importlib.util
import io
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import identity,read_json,read_bytes,write_report
from drawing_api_session import BUILD,sha
from drawing_engine_windows import analyze
from drawing_diagnostic_cadence import measurements
from drawing_copy_phase import transpose


def module(path):
    path=ROOT/path
    spec=importlib.util.spec_from_file_location('eighth_'+path.stem.replace('-','_'),path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


class Pipelines(unittest.TestCase):
    def test_raw_cadence_windows_to_bounded_measurement(self):
        fixture=module('tests/test-drawing-engine-windows.py')
        raw=fixture.trace([(i,i,0,True) for i in range(1,122)])
        with tempfile.TemporaryDirectory() as temporary:
            path=Path(temporary)/'engine.bin';path.write_bytes(raw)
            observed=analyze(path)
        measured=measurements(observed)
        self.assertEqual(observed['complete_update_windows'],120)
        self.assertEqual(measured['count'],119)
        self.assertEqual(observed['journal_sha256'],sha(raw))
        self.assertFalse(measured['identical_gameplay_work_proved'])

    def test_capture_frontend_refuses_new_control_conflicts_before_launch(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary)
            for flags in (['--primitive-lifetimes'],['--region-track-slot','0'],['--egl-swap-completion']):
                result=subprocess.run([sys.executable,'-B',str(ROOT/'tools/capture-drawing-interactive.py'),
                    '--source-game',str(root/'absent-game'),'--prefix-template',str(root/'absent-prefix'),
                    '--control','--cadence-control','--engine-cadence','--schedule',str(ROOT/'tests/fixtures/drawing-journeys/interactive-schedule.json'),
                    '--report',str(root/'report.json'),*flags],cwd=ROOT,capture_output=True,text=True,timeout=30)
                self.assertEqual(result.returncode,2,result.stdout+result.stderr)
                self.assertFalse((root/'report.json').exists())
                self.assertFalse((root/'absent-game').exists())

    def test_sqlite_phase_planner_preserves_checkpoint_and_wire_denominators(self):
        fixture=module('tests/test-drawing-copy-phase.py')
        records,observation,bindings=fixture.fixture()
        # A payload-free CPU2 checkpoint is allowed before the copy triple;
        # neither checkpoint nor completed World queue marker may be removed.
        checkpoint=[0]*24;checkpoint[:4]=[96,1,10,2]
        records=[(tuple(checkpoint),b'')]+[(tuple([r[0],r[1]+1,*r[2:]]),payload) for r,payload in records]
        for q in observation['returns']:q['producer_return_sequence']+=1
        for b in bindings['bindings']:b['producer_return_sequence']+=1
        for kind in (11,12):
            marker=[0]*24;marker[:3]=[96,len(records)+1,kind];marker[14]=1;records.append((tuple(marker),b''))
        header=[0]*16;header[:2]=struct.unpack('<2I',b'MNMPRO01');header[2:7]=[1,64,0x40209ca7,65536,1]
        wire=struct.pack('<16I',*header)+b''.join(struct.pack('<24I',*r) for r,_ in records)
        with tempfile.TemporaryDirectory(dir=ROOT/'working/tests',prefix='eighth-phase-') as temporary, tempfile.TemporaryDirectory(dir=ROOT/'research/runtime',prefix='.eighth-phase-') as report_directory:
            temporary=Path(temporary);reports=Path(report_directory);capture=temporary/'capture';capture.mkdir()
            (capture/'canvas-producers.bin').write_bytes(wire)
            reference=dict(success=True,normal_original_quit_verified=True,synthetic_protocol_fixture=True,experiment=str(temporary),producer_observation=dict(producer_journal_sha256=sha(wire)),copy_order_observation=observation)
            ref=reports/'reference.json';write_report(ROOT,ref,(json.dumps(reference)+'\n').encode())
            bindings.update(success=True,reference=dict(sha256=sha(read_bytes(ref,root=ROOT))))
            bound=reports/'bindings.json';write_report(ROOT,bound,(json.dumps(bindings)+'\n').encode())
            claims=temporary/'claims.json';claims.write_text(json.dumps(dict(sources={},claims=[])))
            out=reports/'plan.json'
            command=[sys.executable,'-B',str(ROOT/'tools/plan-drawing-copy-phase.py'),'--reference',str(ref),'--bindings',str(bound),'--claims',str(claims),'--report',str(out)]
            execution=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=30)
            self.assertEqual(execution.returncode,0,execution.stdout+execution.stderr)
            self.assertIsNotNone(identity(out.read_bytes()))
            plan=read_json(out,root=ROOT);self.assertEqual(len(plan['selected']),1)
            result=transpose(wire,plan['selected'])
            self.assertEqual(len(result),len(wire));self.assertEqual(result[:160],wire[:160]);self.assertEqual(result[-192:],wire[-192:])
            self.assertFalse(plan['native_replay_performed']);self.assertFalse(plan['original_pixels_used_as_native_inputs'])
            self.assertNotEqual(subprocess.run(command,capture_output=True,timeout=30).returncode,0)

    def test_sqlite_lifetime_cli_with_owned_overlapping_aliases(self):
        fixture=module('tests/test-drawing-surface-lifetimes.py')
        timeline=((1,1),(1,2),(1,3),(2,1),(2,2),(3,1),(3,2),(3,3),(2,3))
        intervals,journal=fixture.lifetime_fixture((0,5,5),(11,11,12),{1:12},timeline=timeline,frame_entry=2)
        cli=module('tools/analyze-drawing-lifetimes.py')
        # A separate temporary registry owns synthetic protocol provenance;
        # the real register never receives invented original-execution evidence.
        with tempfile.TemporaryDirectory(dir=ROOT/'working/tests',prefix='eighth-alias-') as temporary:
            root=Path(temporary);(root/'.evidence-store.json').write_text(json.dumps(dict(schema=1,archives=[])))
            capture=root/'working/capture/capture';capture.mkdir(parents=True)
            (capture/'api-call-intervals.bin').write_bytes(intervals);(capture/'session.bin').write_bytes(journal)
            names=['tools/analyze-drawing-lifetimes.py','tools/drawing_surface_lifetimes.py','tools/drawing_lease_intervals.py','tools/drawing_call_intervals.py','tools/drawing_api_session.py','tests/test-drawing-surface-lifetimes.py','tests/test-drawing-call-intervals.py']
            for name in names:
                target=root/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,target)
            reference=dict(success=True,source_sha256=BUILD,original_drawing_active=True,original_manifest_verified_before_after=True,synthetic_protocol_fixture=True,experiment=str(capture.parent),call_intervals=dict(interval_sha256=sha(intervals)),observation=dict(journal_sha256=sha(journal)))
            ref=root/'research/runtime/reference.json';digest=write_report(root,ref,(json.dumps(reference)+'\n').encode())
            registry=root/'research/runtime/coverage/register.json';registry.parent.mkdir()
            registry.write_text(json.dumps(dict(evidence=[dict(id='synthetic',kind='live_observation',record='research/runtime/reference.json',sha256=digest)])))
            claims=root/'working/claims.json';claims.write_text(json.dumps(dict(sources={},claims=[])))
            out=root/'research/runtime/result.json'
            with patch.object(cli,'ROOT',root),patch.object(sys,'argv',['alias','--evidence','synthetic','--claims',str(claims),'--report',str(out)]),contextlib.redirect_stdout(io.StringIO()):
                cli.main()
            self.assertIsNotNone(identity(out.read_bytes()))
            result=read_json(out,root=root)
            self.assertEqual(result['analysis']['mutator_alias_exposure']['frames_with_known_shared_competitors'],1)
            self.assertFalse(result['original_game_executed']);self.assertFalse(result['analysis']['all_backing_aliases_resolved'])


if __name__=='__main__':unittest.main()
