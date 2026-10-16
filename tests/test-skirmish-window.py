#!/usr/bin/env python3
"""Real Qt window actions/dialogs/timer versus headless and fresh-window restores."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import importlib.util
import os
spec = importlib.util.spec_from_file_location('skirmish_test', Path(__file__).with_name('test-skirmish.py'))
test = importlib.util.module_from_spec(spec)
spec.loader.exec_module(test)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('window', type=Path)
    p.add_argument('cli', type=Path)
    p.add_argument('--output', type=Path)
    p.add_argument('--claims', type=Path)
    args = p.parse_args()
    claims = test.declaration(args.claims)
    temporary = tempfile.TemporaryDirectory(prefix='mnm-skirmish-window-') if not args.output else None
    out = Path(temporary.name) if temporary else args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    assert not any(out.iterdir()), 'Refusing occupied output'

    def cli(*arguments):
        r = subprocess.run([str(args.cli.resolve()), *map(str, arguments)], capture_output=True, text=True, timeout=30)
        assert not r.returncode, (arguments, r.stdout, r.stderr)
        return [json.loads(row) for row in r.stdout.splitlines()]

    def window(name, config, load=None):
        script = out / (name + '.json')
        script.write_text(json.dumps(config, indent=2) + '\n')
        directory = out / name
        command = ['xvfb-run', '-a', '-s', '-screen 0 1280x1024x24', str(args.window.resolve()), '--test-script', str(script), '--test-output', str(directory)]
        if load:
            command += ['--load', str(load)]
        env = dict(os.environ, QT_SCALE_FACTOR='1', QT_AUTO_SCREEN_SCALE_FACTOR='0')
        r = subprocess.run(command, capture_output=True, text=True, timeout=70, env=env)
        (out / (name + '.stdout')).write_text(r.stdout)
        (out / (name + '.stderr')).write_text(r.stderr)
        assert not r.returncode and 'ERROR: AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr, (name, r.returncode, r.stderr, (directory / 'failure.txt').read_text() if (directory / 'failure.txt').exists() else '')
        report = json.loads((directory / 'report.json').read_text())
        assert report['success']
        return directory, report

    def button(t, name):
        return dict(tick=t, kind='button', name=name + 'Button')

    def cell(t, actor=None, x=None, y=None, right=False):
        return dict(tick=t, kind='cell', **(dict(actor=actor) if actor is not None else dict(x=x, y=y)), right=right)

    initial = out / 'initial.mnms'
    cli('create', initial)
    raw = initial.read_bytes()
    _, start, size = test.campaign_offsets(raw)
    bad = bytearray(raw[start:start + size])
    bad[8] ^= 1
    corrupt = out / 'corrupt.mnms'
    corrupt.write_bytes(test.replace_campaign(raw, bad))
    actions = [button(0, 'summon'), cell(0, x=2, y=5), dict(tick=0, kind='save', name='pending.mnms'),
               dict(tick=1, kind='save', name='casting.mnms'), dict(tick=1, kind='pause'),
               dict(tick=2, kind='load', path=str(corrupt), reject=True),
               cell(3, actor=2), cell(3, x=1, y=3, right=True), cell(3, actor=0),
               dict(tick=5, kind='save', name='movement.mnms'), cell(5, actor=2), button(5, 'stop'),
               cell(6, actor=2), cell(6, actor=1, right=True), cell(6, actor=0)]
    for t in [6, 9, 12, 15, 24]:
        actions += [button(t, 'bolt'), cell(t, actor=1)]
    actions += [dict(tick=10, kind='save', name='flight.mnms'), dict(tick=13, kind='save', name='combat.mnms')]
    actions.sort(key=lambda a: a['tick'])
    config = dict(end=100, timer=True, actions=actions)
    directory, result = window('orders', config)
    command_log = directory / 'orders.txt'
    headless = out / 'headless.mnms'
    headless_trace = cli('resume', initial, headless, 200, command_log)
    visible_trace = [json.loads(row) for row in (directory / 'trace.jsonl').read_text().splitlines()]
    assert visible_trace == headless_trace and (directory / 'final.mnms').read_bytes() == headless.read_bytes()
    assert result['outcome'] == 1, result
    continuations = 0
    images = []
    for saved in result['saves']:
        checkpoint = directory / saved['name']
        phase = cli('inspect', checkpoint)[0]
        if saved['name'] == 'pending.mnms':
            assert phase['pending'] == 1
        if saved['name'] == 'casting.mnms':
            assert phase['wizards'][0]['spell'] == 1 and phase['wizards'][0]['remaining'] == 2
        if saved['name'] == 'movement.mnms':
            assert phase['units'][2]['control'] == 2 and phase['units'][2]['position'] != [2, 5, 1]
        if saved['name'] == 'flight.mnms':
            assert phase['projectiles'] and any(0 < b['remaining'] < 4 for b in phase['projectiles'])
        if saved['name'] == 'combat.mnms':
            assert any(u['action'] == 2 and u['remaining'] > 0 for u in phase['units'])
        remaining = dict(config, timer=False, actions=actions[saved['next_action']:])
        resumed_dir, resumed_result = window('resume-' + checkpoint.stem, remaining, checkpoint)
        assert (resumed_dir / 'final.mnms').read_bytes() == headless.read_bytes(), saved
        assert (resumed_dir / 'final.png').read_bytes() == (directory / 'final.png').read_bytes(), saved
        resume_log = out / (checkpoint.stem + '-remaining.txt')
        resume_log.write_text('\n'.join(command_log.read_text().splitlines()[saved['orders']:]) + '\n')
        resumed_headless = out / (checkpoint.stem + '-headless.mnms')
        cli('resume', checkpoint, resumed_headless, 200, resume_log)
        assert resumed_headless.read_bytes() == headless.read_bytes(), saved
        images.append(dict(checkpoint=saved['name'], final_canvas_sha256=hashlib.sha256((resumed_dir / 'final.png').read_bytes()).hexdigest()))
        continuations += 1
    defeated, defeat_result = window('defeat', dict(end=100, timer=True, actions=[]))
    defeat_headless = out / 'defeat-headless.mnms'
    cli('resume', initial, defeat_headless, 200)
    assert defeat_result['outcome'] == 2 and (defeated / 'final.mnms').read_bytes() == defeat_headless.read_bytes()
    # Exercise the actual load dialog as well as startup loading.
    loaded, loaded_result = window('load-dialog', dict(end=100, timer=False, actions=[dict(tick=0, kind='load', path=str(directory / 'casting.mnms'))] + actions[4:]))
    assert (loaded / 'final.mnms').read_bytes() == headless.read_bytes()
    assert claims['sources'] == {p: hashlib.sha256((test.ROOT / p).read_bytes()).hexdigest() for p in claims['sources']}
    report = dict(all_match=True, timer_victory=True, timer_defeat=True, actual_save_load_dialogs=True, failed_load_rollback=True, pause_frozen=True, window_continuations=continuations, headless_continuations=continuations, final_canvas_matches=images, victory_tick=result['tick'], defeat_tick=defeat_result['tick'], claims=claims['claims'], source_sha256=claims['sources'], binaries={str(p.relative_to(test.ROOT) if p.is_relative_to(test.ROOT) else p): hashlib.sha256(p.read_bytes()).hexdigest() for p in [args.window.resolve(), args.cli.resolve()]}, wine_required=False, original_comparison=False)
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()

