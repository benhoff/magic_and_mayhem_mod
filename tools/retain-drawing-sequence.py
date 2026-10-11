#!/usr/bin/env python3
"""Retain every independent checkpoint in a pinned sixteen-queue source history."""
import argparse
from collections import Counter
import gzip
import json
from pathlib import Path
import struct
from drawing_corpus import ROOT, BUILD, local, sha, load_tool

EVIDENCE = 'NR.world-producer-handoff.original-gpu-current-20261008'
CHUNK = 24 * 1024 * 1024


def retain(source_root, destination):
    from rendering_coverage import evidence_record
    source_root = source_root.resolve()
    destination = destination.resolve()
    if destination.exists() or not destination.is_relative_to((ROOT / 'tests/fixtures').resolve()):
        raise ValueError('Choose a new directory beneath tests/fixtures')
    register = json.loads((source_root / 'research/runtime/coverage/register.json').read_text())
    ev = next(e for e in register['evidence'] if e['id'] == EVIDENCE)
    report = evidence_record(source_root, ev, {b['id']: b for b in register['builds']})
    if report['original_pixels_used_as_native_inputs'] or report['original_oracles_read_by_native']:
        raise ValueError('Oracle-dependent historical source history')
    inputs = report['inputs']

    def pinned(name):
        data = local(source_root, name).read_bytes()
        if inputs.get(name) != sha(data):
            raise ValueError('Historical artifact changed: ' + name)
        return data

    stream_path = next(p for p in inputs if p.endswith('/capture/canvas-producers.bin'))
    capture = Path(stream_path).parent
    game = capture.parent / 'game'
    raw = pinned(stream_path)
    inspector = load_tool('inspect-canvas-producers')
    _, records = inspector.decode(raw)
    if len(raw) > 128 * 1024 * 1024 or records[-1][0][2] != 12 or records[-1][0][14] != 16:
        raise ValueError('Expected complete sixteen-queue source history')
    prepared, packed_by_hash = {}, {}

    def pack(data, historical=None):
        digest = sha(data)
        if digest not in packed_by_hash:
            packed = gzip.compress(data, mtime=0)
            path = destination / (digest + '.gz')
            prepared[path] = packed
            packed_by_hash[digest] = {'path': path.relative_to(ROOT).as_posix(), 'encoding': 'gzip',
                                      'sha256': sha(packed), 'raw_sha256': digest}
        entry = dict(packed_by_hash[digest])
        if historical:
            entry['historical_path'] = historical
        return entry

    catalog = {'schema': 1, 'build': ev['build'], 'source_sha256': BUILD, 'evidence': EVIDENCE,
               'historical_report_sha256': ev['sha256'], 'original_inputs_verified': True,
               'scope': 'All1062 original checkpoints from46647contiguous startup records and16World queues, including menu/loading and ordered World/HUD/minimap/cursor canvases. Presentation and action identities were not observed; one historically admitted raster bypass was independently compared with original execution.',
               'historical_stream': {'path': stream_path, 'sha256': sha(raw), 'bytes': len(raw)},
               'input_chunks': [pack(raw[i:i+CHUNK]) for i in range(0, len(raw), CHUNK)],
               'producer_counts': dict(sorted(Counter(str(r[2]) for r, _ in records).items())),
               'boundary': {'sequence': len(records), 'queues': 16, 'canvas': records[-1][0][3],
                            'width': records[-1][0][5], 'height': records[-1][0][6]},
               'assets': [], 'checkpoints': [],
               'observations': {'scenario': 'quick-battle-entry', 'boundary': 'composed_canvas',
                                'contiguous_producer_records': True, 'contiguous_presented_frames': False,
                                'actions_bound': False, 'original_work_bypassed': True,
                                'historical_bypass_scope': report['scope']},
               'remaining': ['All14 full presented journeys and action/frame identities',
                             'Physical display copy/Flip/driver failures and screen placement',
                             'Caller-visible state, return values, readbacks and recovery',
                             'Camera/combat/effects/tooltip/save-load/transitions/results/shutdown sequences'],
               'retention_sources': {p: sha((ROOT / p).read_bytes()) for p in
                   ('tools/retain-drawing-sequence.py', 'tools/drawing_corpus.py', 'tools/inspect-canvas-producers.py',
                    'tools/retain-drawing-scene.py', 'tools/rendering_coverage.py')}}
    # Asset-backed sprite bindings use independent encoded sources, never
    # frames reconstructed from destination pixels.
    frame_key = load_tool('retain-drawing-scene').frame_key
    needed, active = set(), False
    selected = {}
    names = {p[:-1].decode('ascii') for r,p in records if r[2] in (7,20)}
    candidates = {Path(p).relative_to(game).as_posix().casefold(): p for p in inputs if Path(p).is_relative_to(game)}
    for name in sorted(names):
        selected[name] = candidates[name.replace('\\','/').casefold()]
    for r,p in records:
        if r[2] == 11: active = True
        elif r[2] == 12: active = False
        elif active and r[2] == 9: needed.add(frame_key(p[:r[19]], r[17]))
    for asset in sorted(report['asset_sources'], key=lambda a:a['path']):
        if not asset['path'].lower().endswith('.spr'): continue
        historical = (game / asset['path']).as_posix()
        data = pinned(historical)
        if len(data)<24 or struct.unpack_from('<I',data,8)[0]!=4: continue
        count,palettes=struct.unpack_from('<II',data,12)
        table=24+palettes*768; base=table+count*4
        keys=set()
        for i in range(count):
            at=base+struct.unpack_from('<I',data,table+i*4)[0]
            size=struct.unpack_from('<I',data,at)[0]
            keys.add(frame_key(data[at:at+size],palettes!=0))
        if keys & needed:
            selected[asset['path']]=historical; needed-=keys
        if not needed: break
    if needed: raise ValueError('Unmapped World source frames')
    for name,historical in sorted(selected.items()):
        entry=pack(pinned(historical),historical);entry['name']=name;catalog['assets'].append(entry)
    previous={}
    native={c['oracle']:c for c in report['native']['checkpoints']}
    comparisons={c['oracle']:c for c in report['checkpoints']}
    for r,_ in records:
        if r[2]!=10: continue
        ordinal=r[14]; historical=(capture / f'producer-oracle-{ordinal:04}.565').as_posix()
        original=pinned(historical); n=native[ordinal]
        if not comparisons[ordinal]['equal'] or n['sha256']!=sha(original) or (n['sequence'],n['canvas'])!=(r[1],r[3]):
            raise ValueError('Independent checkpoint identity mismatch')
        if len(original)!=r[5]*r[6]*2: raise ValueError('Partial original checkpoint')
        prior=previous.get((r[3],r[5],r[6]))
        if prior:
            base_ordinal,base=prior
            delta=(int.from_bytes(original,'little')^int.from_bytes(base,'little')).to_bytes(len(original),'little')
            packed=gzip.compress(delta,mtime=0)
            path=destination/(sha(delta)+'.xor.gz');prepared[path]=packed
            oracle={'path':path.relative_to(ROOT).as_posix(),'encoding':'xor-gzip',
                    'sha256':sha(packed),'delta_sha256':sha(delta),'raw_sha256':sha(original),
                    'base_checkpoint':base_ordinal,'historical_path':historical}
        else:
            oracle=pack(original,historical)
        previous[(r[3],r[5],r[6])]=(ordinal,original)
        catalog['checkpoints'].append({'ordinal':ordinal,'sequence':r[1],'canvas':r[3],'width':r[5],'height':r[6],
                                       'queue':r[16],'reason':r[15],'oracle':oracle})
    if len(catalog['checkpoints'])!=1062 or sum(r[0][2]==11 for r in records)!=16:
        raise ValueError('Incomplete original sequence denominator')
    catalog['comparison_pixels']=sum(c['width']*c['height'] for c in catalog['checkpoints'])
    destination.mkdir(parents=True)
    for path,data in prepared.items():
        with path.open('xb') as out:out.write(data)
    with (destination/'sequence.json').open('x') as out:
        json.dump(catalog,out,indent=2);out.write('\n')
    return catalog


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-root',type=Path,default=ROOT)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();c=retain(a.source_root,a.output)
    print(json.dumps({'checkpoints':len(c['checkpoints']),'assets':len(c['assets']),'boundary':c['boundary']}))

if __name__=='__main__':main()
