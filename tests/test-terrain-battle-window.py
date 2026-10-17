#!/usr/bin/env python3
"""Actual Qt installed-profile terrain battle, mixed spells and fresh resume."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('window', type=Path)
p.add_argument('map_cli', type=Path)
p.add_argument('arena_cli', type=Path)
p.add_argument('fixture', type=Path)
p.add_argument('--actor-profiles', action='store_true')
p.add_argument('--assets', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
a.output = a.output.resolve()
a.output.mkdir(parents=True, exist_ok=False)
a.assets = a.assets.resolve()
commands = []
env = dict(os.environ, QT_SCALE_FACTOR='1', QT_AUTO_SCREEN_SCALE_FACTOR='0',
           XDG_CONFIG_HOME=str(a.output/'qt-config'), QT_STYLE_OVERRIDE='Fusion')


def run(name, argv, ok=True):
    argv = list(map(str, argv))
    commands.append(argv)
    r = subprocess.run(argv, capture_output=True, text=True, timeout=180, env=env)
    (a.output/(name+'.stdout')).write_text(r.stdout)
    (a.output/(name+'.stderr')).write_text(r.stderr)
    assert (r.returncode == 0) == ok, (name, r.stdout[-2000:], r.stderr[-2000:])
    return r.stdout


def window(name, actions, load):
    script = a.output/(name+'.json')
    script.write_text(json.dumps(actions, indent=2)+'\n')
    out = a.output/name
    run(name, ['xvfb-run', '-a', '-s', '-screen 0 1280x1200x24', a.window.resolve(),
               '--installation', a.assets, '--test-script', script,
               '--test-output', out, '--load-campaign', load])
    assert json.loads((out/'report.json').read_text())['success']
    return out/'final.mnmc'


seed = a.output/'seed.mnmc'

run('seed', [a.fixture.resolve(), 'seed', a.assets, seed])
cut = window('attack-cut', ([dict(kind='expect', actor=0, actor_width=1, actor_height=4, actor_type=0),
                             dict(kind='expect', actor=2, actor_width=2, actor_height=2, actor_type=17)] if a.actor_profiles else []) + [dict(kind='expect', terrain=True, cure_enabled=True, sphere_enabled=True, creature_name='Crocodile', actor=2, maximum=360, health=320),
                          dict(kind='cell', actor=2),
                          # This covered wizard cell cannot itself be a2x2
                          # standing origin. It exercises actual footprint picking.
                          dict(kind='cell', wizard=0) if a.actor_profiles else dict(kind='cell', x=6, y=2, z=7), dict(kind='button', name='cureButton'),
                          dict(kind='cell', x=9, y=3, z=7), dict(kind='button', name='sphereButton'),
                          dict(kind='cell', x=9, y=5, z=7), dict(kind='save', name='pending.mnmc'),
                          dict(kind='steps', count=2),
                          dict(kind='expect', actor=2, maximum=360, actor_action='Attacking', cures=1, spheres=1),
                          dict(kind='save', name='attack-flight.mnmc'), dict(kind='pause')], seed)
reference = a.output/'reference.mnmc'
run('headless', [a.map_cli.resolve(), a.assets, 'resume', cut, reference, 14])
resumed = window('attack-restart', [dict(kind='expect', actor=2, actor_action='Attacking', creature_name='Crocodile'),
                                    dict(kind='steps', count=14), dict(kind='save', name='continued.mnmc')], cut)
assert resumed.read_bytes() == reference.read_bytes()
counted = a.output/'counted.mnmc'
events = json.loads(run('count-events', [a.fixture.resolve(), 'resume', a.assets, cut, counted, 14]))
assert counted.read_bytes() == reference.read_bytes()
assert events['melee_hits'] > 0 and events['cure_heals'] == 1 and events['healed'] == 8
assert events['sphere_impacts'] == 1 and events['sphere_damage'] == 20

pursuit = a.output/'pursuit.mnmc'
run('pursuit-seed', [a.fixture.resolve(), 'seed-pursuit', a.assets, pursuit])
move_cut = window('move-cut', [dict(kind='cell', actor=2),
                             # Visible part of the ground diamond outside the
                             # nearby creature glyph's presentation rectangle.
                             dict(kind='cell', x=7, y=1, z=7, pixel_dx=20, right=True),
                             dict(kind='save', name='pending-move.mnmc'), dict(kind='steps', count=1),
                             dict(kind='save', name='pursuit.mnmc')], pursuit)
move_reference = a.output/'move-reference.mnmc'
run('move-headless', [a.map_cli.resolve(), a.assets, 'resume', move_cut, move_reference, 8])
move_resumed = window('move-restart', [dict(kind='steps', count=8), dict(kind='save', name='pursued.mnmc')], move_cut)
assert move_resumed.read_bytes() == move_reference.read_bytes()
move_counted = a.output/'move-counted.mnmc'
motion = json.loads(run('count-pursuit', [a.fixture.resolve(), 'resume', a.assets, move_cut, move_counted, 8]))
assert motion['enemy_moved'] and motion['enemy_pursued'] and motion['player_moved']
assert move_counted.read_bytes() == move_reference.read_bytes()

refused = a.output/'unbound.mnmc'
before = cut.read_bytes()
run('reject-arena-profile', [a.arena_cli.resolve(), '--installation', a.assets, 'resume', cut, refused, 1], ok=False)
assert not refused.exists() and cut.read_bytes() == before
run('reject-arena-create-creatures', [a.arena_cli.resolve(), 'create-creatures', refused], ok=False)
assert not refused.exists()
idle = a.output/'idle.mnmc'
run('create-idle', [a.map_cli.resolve(), a.assets, 'create-species' if a.actor_profiles else 'create-creatures', idle])
run('inspect-idle', [a.map_cli.resolve(), a.assets, 'inspect', idle])
started = a.output/'started.mnmc'
run('start-idle', [a.map_cli.resolve(), a.assets, 'start', idle, started])
summoned = window('fresh-summon', [dict(kind='expect', terrain=True, creature_name='Crocodile'),
                                   dict(kind='cell', wizard=0), dict(kind='button', name='summonButton'),
                                   dict(kind='cell', x=7, y=1, z=7, pixel_dx=20),
                                   dict(kind='save', name='pending.mnmc'), dict(kind='steps', count=3),
                                   dict(kind='expect', actor=2, maximum=360),
                                   dict(kind='save', name='summoned.mnmc')], started)
fresh = json.loads(run('inspect-summoned', [a.map_cli.resolve(), a.assets, 'inspect', summoned]))['battle']
assert any(u['role'] == 1 and u['owner'] == 0 and u['health'] == 360 for u in fresh['units'])
assert any(u['role'] == 1 and u['owner'] == 1 and u['health'] == 360 for u in fresh['units'])
summon_reference = a.output/'summon-reference.mnmc'
run('summon-headless', [a.map_cli.resolve(), a.assets, 'resume', a.output/'fresh-summon/pending.mnmc', summon_reference, 3])
assert summoned.read_bytes() == summon_reference.read_bytes()


report = dict(success=True, scope='Actual Qt installed Plains encounter with declared legacy or per-actor geometry and secondary creature picks, real Cure/Sphere buttons/save dialogs; attack/flight and actual retreat/enemy pursuit resume through fresh window, CLI and event-counting processes with identical full campaign bytes. Idle installed campaign binding and missing arena-context refusal. Native authored scheduling/placements, no original full AI/map/campaign/live replacement claim.',
              exact_campaign_bytes=True, fresh_process=True, installed_maximum=360,
              attack_events=events, pursuit_events=motion, idle_campaign=True, fresh_player_and_ai_summons=True,
              mismatched_profile_refused=True, nonstanding_secondary_pick=not a.actor_profiles, per_actor_profiles=a.actor_profiles,
              checkpoint_sha256=hashlib.sha256(cut.read_bytes()).hexdigest(),
              result_sha256=hashlib.sha256(resumed.read_bytes()).hexdigest(), commands=commands)
(a.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report))
