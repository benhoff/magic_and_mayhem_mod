#!/usr/bin/env python3
"""Cross-tool owned journals and SQLite CLI controls; no original execution."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import identity, read_json, read_bytes, write_report
from drawing_api_session import BUILD, sha
from drawing_copy_bindings import associate
from drawing_worker_producer_points import decode


def module(name):
    path=ROOT/'tests'/('test-drawing-'+name+'.py')
    spec=importlib.util.spec_from_file_location('seventh_'+name.replace('-','_'),path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result


def copy_pipeline():
    args=module('copy-bindings').fixture()
    raw,legacy=module('worker-producer-points').fixture()
    observed=decode(raw,legacy)
    for index,packet in enumerate(args[4]):
        packet.update(worker_thread=index+11,worker_entry_record=2*index+1,worker_return_record=2*index+2)
    bindings=associate(*args)
    records=[]
    for sequence in range(1,14):
        row=[0]*24;row[:4]=[96,sequence,5,99]
        returned=next((r for r in args[0]['returns'] if r['producer_return_sequence']==sequence),None)
        if returned:
            row[2]=6;row[3:15]=[returned['destination'],returned['source'],*returned['destination_extent_stride'],*returned['destination_position'],*returned['source_rectangle'],returned['raw_key']]
        records.append(struct.pack('<24I',*row))
    wire=b'X'*64+b''.join(records)
    reference=dict(success=True,normal_original_quit_verified=True,synthetic_protocol_fixture=True,
        copy_order_observation=args[0],wine_worker_producer_observation=observed,
        producer_observation=dict(producer_journal_sha256=sha(wire)))
    return reference,dict(bindings,success=True),observed,wire,raw,legacy


class Pipeline(unittest.TestCase):
    def test_exact_copy_worker_and_delayed_return_pipeline(self):
        spec=importlib.util.spec_from_file_location('worker_prefix',ROOT/'tools/analyze-drawing-worker-prefixes.py')
        classifier=importlib.util.module_from_spec(spec);spec.loader.exec_module(classifier)
        reference,joined,observed,wire,*_=copy_pipeline()
        result,counts,outside=classifier.classify(reference,joined,observed,wire)
        self.assertEqual(counts,{'touches-copy-canvas':2});self.assertEqual(outside,[])
        relations=[r['copy_return_execution_associations'][0]['relation'] for r in result]
        self.assertEqual(relations,['commands-started-after-worker-return','same-request-worker'])
        self.assertTrue(all(not r['source_storage_owned'] for r in result))
        observed['calls'].append(dict(copy.deepcopy(observed['calls'][0]),worker=3))
        self.assertEqual(classifier.classify(reference,joined,observed,wire)[2],[3])
        joined['bindings'][0]['commands'][0]['worker_entry_record']=2
        with self.assertRaises(ValueError):classifier.classify(reference,joined,observed,wire)

    def test_sqlite_input_output_and_no_overwrite_for_real_clis(self):
        # These protocol fixtures deliberately supply acquisition-admission fields
        # to exercise offline CLI wiring. They are owned synthetic inputs, never
        # an assertion that the original game ran.
        with tempfile.TemporaryDirectory(dir=ROOT/'working/tests',prefix='seventh-cli-') as work, tempfile.TemporaryDirectory(dir=ROOT/'research/runtime',prefix='.seventh-cli-') as reports:
            work=Path(work);reports=Path(reports);capture=work/'capture';capture.mkdir()
            reference,joined,observed,wire,points,legacy=copy_pipeline()
            reference['experiment']=str(work)
            for name,raw in [('canvas-producers.bin',wire),('wine-worker-producer-points.bin',points),('wine-blit-worker.bin',legacy)]:
                (capture/name).write_bytes(raw)
            reference_path=reports/'reference.json'
            write_report(ROOT,reference_path,(json.dumps(reference)+'\n').encode())
            joined['reference']=dict(sha256=sha(read_bytes(reference_path,root=ROOT)))
            joined_path=reports/'bindings.json';write_report(ROOT,joined_path,(json.dumps(joined)+'\n').encode())
            self.run_cli('worker-prefixes',['--reference',reference_path,'--bindings',joined_path],reports/'workers.json')
            value=read_json(reports/'workers.json',root=ROOT)
            self.assertEqual(value['counts'],{'touches-copy-canvas':2})
            self.assertEqual(value['reference']['sha256'],sha(read_bytes(reference_path,root=ROOT)))
            egl,marker=module('egl-source-domains').fixture()
            egl.update(success=True,source_sha256=BUILD,normal_original_quit_verified=True,synthetic_protocol_fixture=True,experiment=str(work))
            (capture/'egl-physical.done').write_bytes(marker)
            egl_path=reports/'egl-reference.json';write_report(ROOT,egl_path,(json.dumps(egl)+'\n').encode())
            self.run_cli('egl-source-domains',['--reference',egl_path],reports/'domains.json')
            self.assertEqual(read_json(reports/'domains.json',root=ROOT)['analysis']['closed_physical_calls'],3)
            mutex,api=module('cursor-mutex-binding').fixture()
            for name,raw in [('cursor-mutex.bin',mutex),('session.bin',api),('session.done',api[-128:])]:
                (capture/name).write_bytes(raw)
            ref=dict(success=True,normal_original_quit_verified=True,synthetic_protocol_fixture=True,experiment=str(work),cursor_mutex_sha256=sha(mutex),observation=dict(journal_sha256=sha(api)))
            ref_path=reports/'mutex-reference.json';write_report(ROOT,ref_path,(json.dumps(ref)+'\n').encode())
            # Authored declaration interchange is consumed by this one CLI.
            claims=work/'claims.json';claims.write_text(json.dumps(dict(sources={},claims=[])))
            self.run_cli('cursor-mutex',['--original-reference',ref_path,'--claims',claims],reports/'mutex.json')
            self.assertEqual(read_json(reports/'mutex.json',root=ROOT)['analysis']['inactive_points'],1)

    def run_cli(self,name,args,output):
        command=[sys.executable,'-B',str(ROOT/('tools/analyze-drawing-'+name+'.py')),*map(str,args),'--report',str(output)]
        result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=30)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertIsNotNone(identity(output.read_bytes()))
        before=output.read_bytes()
        refused=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=30)
        self.assertNotEqual(refused.returncode,0)
        self.assertEqual(output.read_bytes(),before)


if __name__=='__main__':unittest.main()
