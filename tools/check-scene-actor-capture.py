#!/usr/bin/env python3
"""Inspect an immutable actual actor sidecar without granting rendering rows."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

from coverage_evidence import Reader, write_report
from scene_actor_observation import decode

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition', required=True)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    target = Path(a.report)
    if not a.report.startswith('research/runtime/attachment-combat-results-20261107/') or '..' in target.parts or (ROOT/target).exists():
        p.error('New owned immutable report required')
    q = dict(schema=1, kind='actual_scene_actor_observation_inspection', success=False,
             newly_passed_requirements=0, attachment_placement_proved=False,
             combat_output_proved=False, original_game_runs=0)
    with Reader(ROOT) as reader:
        q['analysis_sources'] = {n: reader.digest(n) for n in
            ('tools/check-scene-actor-capture.py', 'tools/scene_actor_observation.py')}
        try:
            acquired = reader.json(a.acquisition)
            q['acquisition'] = dict(record=a.acquisition, sha256=reader.digest(a.acquisition))
            q['execution_success'] = acquired.get('success')
            q['execution_error'] = acquired.get('error')
            choices = [v for v in acquired['artifacts'] if v['path'].endswith('/scene-actors-0001.bin')]
            if len(choices) != 1:
                raise ValueError('Expected one actual actor sidecar; found '+str(len(choices)))
            artifact = choices[0]
            raw = reader.read_bytes(artifact['record'])
            if hashlib.sha256(raw).hexdigest() != artifact['sha256']:
                raise ValueError('Actor capture hash differs')
            q['sidecar'] = artifact
            observed = decode(raw)
            frames = observed['frames']
            q.update(owner=observed['owner'], nonce=observed['nonce'],
                     frames=len(frames), complete=observed['complete'],
                     extents=sorted({tuple(f['extent']) for f in frames}),
                     views=sorted({f['view'] for f in frames}),
                     gameplay_points=sum(bool(f['validity'] & 2) for f in frames),
                     roles=dict(Counter(str(d[1]) for f in frames for d in f['draws'])),
                     modes=dict(Counter(str(actor[14]) for f in frames for actor in f['actors'])),
                     first_generation=frames[0]['generation'], last_generation=frames[-1]['generation'])
            q['domains'] = [dict(extent=list(size), view=view,
                frames=sum(f['extent']==list(size) and f['view']==view for f in frames),
                attachment_candidate_draws=sum(d[1]>0 for f in frames if f['extent']==list(size) and f['view']==view for d in f['draws']))
                for size in sorted({tuple(f['extent']) for f in frames}) for view in range(4)]
            q['samples'] = [dict(generation=f['generation'], view=f['view'], extent=f['extent'],
                gameplay_sequence=f['gameplay_sequence'], actors=[list(v) for v in f['actors']],
                draws=[list(v) for v in f['draws']]) for f in frames[:2]]
            journals = [v for v in acquired['artifacts'] if v['path'].endswith('/gameplay-events.bin')]
            if len(journals)==1:
                raw = reader.read_bytes(journals[0]['record'])
                if len(raw)<16 or raw[:8]!=b'MNMGP001' or (len(raw)-16)%64:
                    raise ValueError('Gameplay journal envelope')
                rows=[struct.unpack_from('<16I',raw,at) for at in range(16,len(raw),64)]
                q['gameplay'] = dict(artifact=journals[0], rows=len(rows),
                    melee=[list(r) for r in rows if r[1]==2][:100],
                    melee_count=sum(r[1]==2 for r in rows))
            q['success'] = bool(frames and q['gameplay_points'])
        except Exception as error:
            q['error'] = str(error)
        q['analysis_sources_stable'] = all(reader.digest(n)==h for n,h in q['analysis_sources'].items())
        q['success'] &= q['analysis_sources_stable']
    sha = write_report(ROOT, a.report, (json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(record=a.report,sha256=sha,**{k:q.get(k) for k in
        ('success','error','execution_success','frames','extents','views','roles','gameplay_points')})))
    return int(not q['success'])


if __name__ == '__main__':
    raise SystemExit(main())
