#!/usr/bin/env python3
"""Source-only native prediction of bounded API physical witnesses; no journey credit."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_api_session import BUILD,sha
from drawing_producer_points import inspect_points
from drawing_scene_corpus import asset_name,raster_inputs
from drawing_presented_session import rgba565
ROOT=Path(__file__).resolve().parents[1]


def load_producers(raw):
    spec=importlib.util.spec_from_file_location('producer_inspector',ROOT/'tools/inspect-canvas-producers.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    return module.decode(raw)[1]

def wire_prefix(raw,last):
    # V3 decode expands owned references; canonical record size is not wire size.
    at=64
    for _ in range(last):
        if at+96>len(raw):raise ValueError('Incomplete native source prefix')
        size=struct.unpack_from('<I',raw,at)[0]
        if size<96 or at+size>len(raw):raise ValueError('Invalid native source wire record')
        at+=size
    return raw[:at]


def candidates(observation,api,records):
    state={0:(False,0)};active=False;completed=0
    for r,_ in records:
        if r[2]==11:active=True
        elif r[2]==12:active=False;completed=r[14]
        state[r[1]]=(active,completed)
    selected=[];excluded=[];prior=0
    for frame in observation['frames']:
        points=frame['points'];source=points[0]['source'];call=api[frame['entry']];reason=None
        if not frame['producer_live_at_all_points']:reason='producer-stopped-or-failed'
        elif not frame['both_generations_known']:reason='unknown-generation'
        elif frame['competing_mutation_count']:reason='observed-competing-mutation'
        elif any(p['source'][3:14]!=source[3:14] for p in points[1:]):reason='source-point-state-changed'
        elif call[2]!=7 or call[21]!=16 or call[22:28]!=(0,0,0,0,source[7],source[8]):reason='unsupported-transfer-geometry'
        elif state[source[3]][0]:reason='world-queue-active'
        elif source[3]<=prior:reason='repeated-or-reordered-prefix'
        if reason:excluded.append(dict(ordinal=frame['ordinal'],entry=frame['entry'],reason=reason));continue
        prior=source[3];selected.append(dict(ordinal=frame['ordinal'],entry=frame['entry'],boundary=dict(sequence=source[3],queues=state[source[3]][1],canvas=source[6],width=source[7],height=source[8])))
    return selected,excluded


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--input-manifest',type=Path,required=True);p.add_argument('--executable',type=Path,required=True);p.add_argument('--display-conversion',choices=('normalized-nearest','bit-replicated'),default='normalized-nearest');p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    original_bytes=a.reference.read_bytes();reference=json.loads(original_bytes)
    pin_bytes=a.input_manifest.read_bytes();pin=json.loads(pin_bytes)
    if pin.get('executable_sha256')!=BUILD or pin.get('original_manifest_verified_before_after') is not True:raise ValueError('Pinned reference input manifest required')
    if reference.get('success') is not True or reference.get('source_sha256')!=BUILD or reference.get('original_drawing_active') is not True or reference.get('original_manifest_verified_before_after') is not True:raise ValueError('Independent original-active reference required')
    capture=Path(reference['experiment'])/'capture';raw=(capture/'canvas-producers.bin').read_bytes();records=load_producers(raw);observed=inspect_points(capture)
    if observed!=reference['producer_observation']:raise ValueError('Source-point observation/reference changed')
    api={r[29]:r for r in struct.iter_unpack('<32I',(capture/'session.bin').read_bytes()[64:-128]) if r[1]==1}
    selected,excluded=candidates(observed,api,records)
    if not selected:raise ValueError('No bounded source prediction candidates')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    for name in ('tools/compare-drawing-source-points.py','tools/drawing_producer_points.py','tools/drawing_scene_corpus.py','tools/drawing_presented_session.py','tools/inspect-canvas-producers.py'):sources[name]=sha((ROOT/name).read_bytes())
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-source-prediction';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));inputs=run/'inputs';assets=inputs/'assets';assets.mkdir(parents=True)
    def verify(label):
        result=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (run/(label+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
    verify('original-before')
    last=selected[-1]['boundary']['sequence'];prefix=raster_inputs(wire_prefix(raw,last));(inputs/'producers.bin').write_bytes(prefix)
    asset_hashes={};game=(Path(reference['experiment'])/'game').resolve()
    for r,payload in records[:last]:
        if r[2] not in (7,20):continue
        name=payload[:-1].decode('ascii');normalized=asset_name(name);path=game
        for part in normalized.split('/'):
            matches=[x for x in path.iterdir() if x.name.casefold()==part.casefold()]
            if len(matches)!=1:raise ValueError('Ambiguous/missing encoded source asset')
            path=matches[0]
        if not path.resolve().is_relative_to(game):raise ValueError('Source asset escapes staged installation')
        data=path.read_bytes();digest=sha(data)
        if pin['staged_files'].get(str(path.relative_to(game)))!=digest:raise ValueError('Source asset differs from prospective installed-input pin')
        if name in asset_hashes and asset_hashes[name]!=digest:raise ValueError('Encoded source asset changed')
        asset_hashes[name]=digest;target=assets/normalized.casefold();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    # World resources verify captured frame identities against independently
    # decoded SPR inputs. Supply the pinned encoded catalogue, never synthesize
    # source files from captured destination or frame output pixels.
    sprites=sorted(path for path in game.rglob('*') if path.is_file() and path.suffix.casefold()=='.spr')
    if len(sprites)>256 or sum(path.stat().st_size for path in sprites)>512*1024*1024:raise ValueError('Encoded SPR catalogue exceeds native limits')
    for path in sprites:
        relative=str(path.relative_to(game));data=path.read_bytes();digest=sha(data)
        if pin['staged_files'].get(relative)!=digest:raise ValueError('SPR source differs from installed-input pin')
        asset_hashes[relative]=digest;target=assets/relative.casefold();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    declaration=dict(schema=2,boundary=selected[-1]['boundary'],frames=[f['boundary'] for f in selected],assets=asset_hashes,retain_checkpoints=[])
    (inputs/'scene.json').write_text(json.dumps(declaration,indent=2)+'\n')
    environment={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};environment.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    command=['xvfb-run','-a',str(a.executable.resolve()),str(inputs/'producers.bin'),str(assets),str(inputs/'scene.json'),str(run/'native')]
    try:execution=subprocess.run(command,capture_output=True,timeout=600,env=environment)
    finally:verify('original-after')
    (run/'native.log').write_bytes(execution.stdout+execution.stderr)
    native=json.loads((run/'native/scene-report.json').read_text()) if (run/'native/scene-report.json').exists() else None
    comparisons=[]
    if execution.returncode==0:
        if native.get('success') is not True or native.get('frames_published')!=len(selected) or any(native.get(k) is not False for k in ('original_pixels_used_as_native_inputs','original_oracles_read','original_raster_results_used_as_native_inputs','live_replacement')):raise ValueError('Native prediction provenance/closure failed')
        for i,frame in enumerate(selected,1):
            actual=rgba565((run/f'native/frame-{i:04d}.565').read_bytes(),a.display_conversion);expected=(capture/f"api-physical-oracle-{frame['ordinal']:06d}.rgba").read_bytes()
            metadata=reference['physical_api_observation']['frames'][frame['ordinal']-1]
            if sha(expected)!=metadata['sha256'] or len(actual)!=len(expected):raise ValueError('Original/native full plane identity changed')
            differences=sum(actual[at:at+4]!=expected[at:at+4] for at in range(0,len(expected),4))
            comparisons.append(dict(**frame,native_sha256=sha(actual),original_sha256=sha(expected),pixels=len(actual)//4,different_pixels=differences))
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Prediction sources changed during execution')
    report=dict(schema=1,success=execution.returncode==0 and all(not f['different_pixels'] for f in comparisons),sources=sources,sources_stable=True,claims=claims['claims'],reference=dict(path=str(a.reference),sha256=sha(original_bytes)),source_input_manifest=dict(path=str(a.input_manifest),sha256=sha(pin_bytes)),artifacts=str(run.relative_to(ROOT)),executable_sha256=sha(a.executable.read_bytes()),native_exit=execution.returncode,native=native,selected=selected,excluded=excluded,comparisons=comparisons,source_assets=asset_hashes,native_input_sha256=sha(prefix),original_game_executed=False,original_manifest_verified_before_after=True,original_pixels_used_as_native_inputs=False,observed_raster_results_removed=True,display_conversion=a.display_conversion,coherent_presented_frames=False,complete_journey_coverage=False,scope='Source-only native composed-canvas prediction of an explicitly restricted API physical cohort. Every excluded witness remains listed; stable metadata/zero differences do not prove all original writers, immutable storage, complete presented journeys, driver recovery or replacement.')
    a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],selected=len(selected),excluded=len(excluded),compared=len(comparisons),different_pixels=sum(f['different_pixels'] for f in comparisons),native_exit=execution.returncode)))

if __name__=='__main__':main()
