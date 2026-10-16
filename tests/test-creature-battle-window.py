#!/usr/bin/env python3
"""Actual installed-creature campaign window, spells, pursuit and fresh resume."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('window', type=Path)
p.add_argument('cli', type=Path)
p.add_argument('fixture', type=Path)
p.add_argument('--assets', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
a.output = a.output.resolve()
a.output.mkdir(parents=True, exist_ok=False)
a.assets = a.assets.resolve()
commands = []


def run(name, argv, ok=True):
    argv = list(map(str, argv))
    commands.append(argv)
    r = subprocess.run(argv, capture_output=True, text=True, timeout=120,
                       env=dict(os.environ, QT_SCALE_FACTOR='1', QT_AUTO_SCREEN_SCALE_FACTOR='0'))
    (a.output/(name+'.stdout')).write_text(r.stdout)
    (a.output/(name+'.stderr')).write_text(r.stderr)
    assert (r.returncode == 0) == ok, (name, r.stdout[-3000:], r.stderr[-3000:])
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
run('seed', [a.fixture.resolve(), 'seed-campaign', a.assets, seed])
cut = window('attack-cut', [dict(kind='expect', creature_name='Crocodile', actor=2, maximum=360, health=320),
                          dict(kind='cell', wizard=0), dict(kind='button', name='cureButton'),
                          dict(kind='cell', x=3, y=4), dict(kind='button', name='sphereButton'),
                          dict(kind='cell', x=5, y=4), dict(kind='save', name='pending.mnmc'),
                          dict(kind='steps', count=2),
                          dict(kind='expect', actor=2, maximum=360, actor_action='Attacking', cures=1, spheres=1),
                          dict(kind='save', name='attack-flight.mnmc'), dict(kind='pause')], seed)
cut_state = json.loads(run('inspect-attack-cut', [a.cli.resolve(), '--installation', a.assets, 'inspect', cut]))['battle']
assert len(cut_state['creature_rules']) == 2 and all(c['phase'] == 2 and c['target'] for c in cut_state['creature_rules'])
reference = a.output/'reference.mnmc'
run('headless', [a.cli.resolve(), '--installation', a.assets, 'resume', cut, reference, 14])
resumed = window('attack-restart', [dict(kind='expect', actor=2, actor_action='Attacking', creature_name='Crocodile'),
                                    dict(kind='steps', count=14), dict(kind='save', name='continued.mnmc')], cut)
assert resumed.read_bytes() == reference.read_bytes()
counted = a.output/'counted.mnmc'
events = json.loads(run('count-events', [a.fixture.resolve(), 'resume-campaign', a.assets, cut, counted, 14]))
assert counted.read_bytes() == reference.read_bytes()
assert events['melee_hits'] > 0 and events['cure_heals'] == 1 and events['healed'] == 8
assert events['sphere_impacts'] == 1 and events['sphere_damage'] == 20

# Queue an actual player move while both installed creatures have acquired an
# enemy. The fresh window and CLI must preserve pursuit and movement together.
pursuit = a.output/'pursuit.mnmc'
run('pursuit-seed', [a.fixture.resolve(), 'seed-pursuit', a.assets, pursuit])
move_cut = window('move-cut', [dict(kind='cell', actor=2), dict(kind='cell', x=1, y=3, right=True),
                             dict(kind='save', name='pending-move.mnmc'), dict(kind='steps', count=1),
                             dict(kind='save', name='pursuit.mnmc')], pursuit)
move_reference = a.output/'move-reference.mnmc'
run('move-headless', [a.cli.resolve(), '--installation', a.assets, 'resume', move_cut, move_reference, 8])
move_resumed = window('move-restart', [dict(kind='steps', count=8), dict(kind='save', name='pursued.mnmc')], move_cut)
assert move_resumed.read_bytes() == move_reference.read_bytes()
move_counted = a.output/'move-counted.mnmc'
motion = json.loads(run('count-pursuit', [a.fixture.resolve(), 'resume-campaign', a.assets, move_cut, move_counted, 8]))
assert motion['enemy_moved'] and move_counted.read_bytes() == move_reference.read_bytes()

# A matching installation is an explicit campaign-content binding, including
# between battles. Missing binding refuses before publishing an output.
refused = a.output/'unbound.mnmc'
before = cut.read_bytes()
run('reject-missing-profile', [a.cli.resolve(), 'resume', cut, refused, 1], ok=False)
assert not refused.exists() and cut.read_bytes() == before
idle = a.output/'idle.mnmc'
run('create-idle', [a.cli.resolve(), '--installation', a.assets, 'create', idle])
run('inspect-idle', [a.cli.resolve(), '--installation', a.assets, 'inspect', idle])
started = a.output/'started.mnmc'
run('start-idle', [a.cli.resolve(), '--installation', a.assets, 'start', idle, started])

report = dict(success=True, scope='Real Qt installed Crocodile selection/actions and 2x2 footprint presentation; actual Cure/Sphere buttons and campaign save dialogs, attack/flight and player-move/pursuit checkpoints resumed in fresh Qt, CLI and event-counting processes with identical campaign bytes. Explicit installed campaign binding and idle campaign reload.',
              installed_maximum=360, exact_campaign_bytes=True, fresh_process=True,
              attack_events=events, pursuit_events=motion, idle_campaign=True,
              missing_profile_refused=True,
              checkpoint_sha256=hashlib.sha256(cut.read_bytes()).hexdigest(),
              result_sha256=hashlib.sha256(resumed.read_bytes()).hexdigest(), commands=commands)
(a.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report))
