#!/usr/bin/env python3
"""Bind unchanged public menu/movie controllers to the current player and provider."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys

import coverage_evidence
from broader_player_launch import LaunchBinding
from coverage_evidence import Reader
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = dict(menu='run-rendering-menu-acceptance.py', intros='run-rendering-enabled-intros.py',
                skip='run-rendering-movie-skip.py')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kind', choices=FAMILIES, required=True)
    parser.add_argument('--plan', required=True)
    parser.add_argument('--report', required=True)
    parser.add_argument('--software', action='store_true')
    parser.add_argument('--low', action='store_true')
    parser.add_argument('--skip-index', type=int, choices=(0, 1))
    args = parser.parse_args()
    if os.environ.get('MNM_BROADER_MENU_BUS_OWNED') != '1':
        runtime = ROOT/'working/broader-player/menu-runtime'/Path(args.report).stem
        runtime.mkdir(mode=0o700, parents=True, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_BROADER_MENU_BUS_OWNED='1',
                   ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__, *sys.argv[1:]], env)
    with Reader(ROOT) as reader:
        plan = reader.json(args.plan)
        require(plan['success'] and plan['family'] == args.kind
                and all(reader.digest(n) == h for n, h in (plan['sources']|plan['runtime_sources']|plan['compiler_sources']).items()),
                'Unbound or changed current menu plan')
        matches = [v for v in plan['broader_cases'] if v['report'] == args.report]
        require(len(matches) == 1 and matches[0]['profile'] == ('player-software' if args.software else 'player-default')
                and matches[0]['size'] == ([640, 480] if args.low else [800, 600]), 'Menu case domain differs')
        require((args.kind == 'skip') == (args.skip_index is not None), 'Skip index belongs only to the skip controller')
        case = matches[0]
        phase = dict(record=args.plan, sha256=reader.digest(args.plan))
    binding = LaunchBinding(ROOT, plan, args.report, case['provider'])
    original_write = coverage_evidence.write_report
    def writer(root, path, raw):
        if path == args.report:
            q = json.loads(raw)
            q['delegate_success'] = q['success']
            stable = all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest() == h
                         for n, h in (plan['sources']|plan['runtime_sources']|plan['compiler_sources']).items())
            q.setdefault('sources', {}).update(plan['sources']|plan['runtime_sources']|plan['compiler_sources'])
            q['sources_stable'] = q.get('sources_stable', False) and stable
            try:
                q['provider_adaptation'] = binding.receipt()
                q['provider_adaptation']['phase'] = phase
            except ValueError as error:
                q['provider_admission_error'] = str(error)
                q['success'] = False
            q['success'] = q['success'] and q['sources_stable']
            raw = (json.dumps(q, indent=2)+'\n').encode()
        return original_write(root, path, raw)
    coverage_evidence.write_report = writer
    binding.install()
    try:
        core = binding.load(FAMILIES[args.kind])
        core.PREFIX, core.PLAN = plan['collector_prefix'], args.plan
        sys.argv = [str(ROOT/'tools'/FAMILIES[args.kind]), '--report', args.report]
        if args.kind == 'intros':
            sys.argv += ['--input-plan', args.plan]
        if args.kind == 'skip':
            sys.argv += ['--skip-index', str(args.skip_index)]
        if args.software:
            sys.argv.append('--software')
        if args.low:
            sys.argv.append('--low')
        return core.main()
    finally:
        binding.restore()
        coverage_evidence.write_report = original_write


if __name__ == '__main__':
    raise SystemExit(main())
