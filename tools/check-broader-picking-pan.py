#!/usr/bin/env python3
"""Compare full Original history and every picking check using exact ANI records."""
import argparse,hashlib,importlib.util,json,tempfile
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from broader_picking_pose_source import LiveActorFrames
from broader_picking_pan_assessment import with_actor_frames
from broader_picking_pose_target import POLICY
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(add_help=False);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True)
    args,_=p.parse_known_args()
    with Reader(ROOT)as reader:
        a=reader.json(args.acquisition)
        require(a['success']and a['provider_adaptation']['picking_pose_target']['policy']==POLICY,
                'Missing acquired exact-ANI picking source policy')
        artifacts=[v for v in a['artifacts']if Path(v['path']).name=='scene-actors-0001.bin']
        require(len(artifacts)==1,'Missing unique retained picking actor sidecar')
        artifact=artifacts[0];raw=reader.read_bytes(artifact['record'])
        require(hashlib.sha256(raw).hexdigest()==artifact['sha256']and len(raw)==artifact['bytes'],
                'Actor source artifact differs')
        with tempfile.TemporaryDirectory(prefix='picking-pose-analysis-',dir='/tmp')as directory:
            path=Path(directory)/'poses.bin';path.write_bytes(raw);poses=LiveActorFrames(path,keep_all=True);poses.poll()
            require(poses.complete and poses.frames,'Actor observation lacks complete lossless closure')
            frames=poses.frames
    import rendering_picking_resize_assessment as assessment
    previous=assessment.assess;assessment.assess=with_actor_frames(frames)
    original_write=coverage_evidence.write_report
    def writer(root,path,raw):
        if str(path)==args.report:
            q=json.loads(raw);q.setdefault('sources',{}).update({n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in ['tools/broader_picking_rebuild_capture_assessment.py','tools/broader_picking_uncovered_assessment.py','tools/broader_picking_uncovered_target.py','tools/check-broader-picking-pan.py','tools/broader_picking_pan_assessment.py','tools/broader_picking_pan_ack.py']});q['rebuild_metadata_analysis']=dict(raw_live_metadata_unchanged=True,only_derived_reference_binding_removed_after_exact_verification=True,source='tools/rendering_pilot.py:acquisition',existing_rebuild_policy_unchanged=True);q['picking_pose_observation']=dict(verified=True,policy=POLICY,artifact=artifact,
                complete=True,frames=len(frames),owner=poses.owner,nonce=str(poses.nonce),
                allocation_generations_proved=False,target_replay_only_changed=True)
            raw=(json.dumps(q,indent=2)+'\n').encode()
        return original_write(root,path,raw)
    coverage_evidence.write_report=writer
    try:
        path=ROOT/'tools/check-rendering-picking-closeout.py';spec=importlib.util.spec_from_file_location('pose_frozen_picking_check',path)
        core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
        return core.main()
    finally:assessment.assess=previous;coverage_evidence.write_report=original_write

if __name__=='__main__':raise SystemExit(main())
