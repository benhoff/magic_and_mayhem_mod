#!/usr/bin/env python3
"""Original public interaction schedule with an observed display readiness barrier."""
import argparse
import importlib.util
import os
from pathlib import Path
import sys
import time
from rendering_interaction_stability import ready_generation

ROOT = Path(__file__).resolve().parents[1]
PLAN = 'research/runtime/rendering-interaction-results-20261107/input-plan-08.json'


def main():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('--report', required=True)
    args, _ = parser.parse_known_args()
    if os.environ.get('MNM_INTERACTION_BUS_OWNED') != '1':
        runtime = ROOT / 'working/interaction-runtime' / Path(args.report).stem
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        isolated = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_INTERACTION_BUS_OWNED='1')
        isolated.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__, *sys.argv[1:]], isolated)
    spec = importlib.util.spec_from_file_location('interaction_base', ROOT / 'tools/run-rendering-interaction.py')
    base = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(base)
    base.PLAN = PLAN
    original_sources = base.native_runtime_source_paths
    # Extend the existing conservative acquisition provenance with this control
    # adapter. No native DLL source or original method is replaced.
    base.native_runtime_source_paths = lambda root: original_sources(root) | {
        'tools/run-rendering-interaction-stable.py', 'tools/rendering_interaction_stability.py',
        'tools/prepare-rendering-interaction-stable.py'}

    class StableJourney(base.InteractionJourney):
        def settle(self, action, released, delay=.55):
            point = next(v['logical'] for v in reversed(self.actions) if v['action'] == 'pointer')
            started = time.monotonic()
            seen = set()
            while time.monotonic() - started <= 5:
                base.require(self.child.poll() is None, 'Player exited during pointer readiness')
                raw = (self.session / 'frame-input.bin').read_bytes()
                raw = raw[:16 + (len(raw) - 16) // 384 * 384]
                rows = base.decode(raw, allow_pending=True)['rows']
                generation = ready_generation(rows, self.viewport.poll(), point, '-hover' in action,
                                              released['frame_generation'])
                if generation is not None:
                    seen.add(generation)
                if len(seen) >= 3 and time.monotonic() - started >= max(delay, 1.2):
                    self.actions.append(dict(action='pointer-readiness', name=action, point=point,
                                             presented_source_generations=sorted(seen),
                                             host_ns=time.monotonic_ns()))
                    self.mark(action, 'settled')
                    return
                time.sleep(.04)
            raise ValueError('Three admitted displayed source generations not observed within pointer readiness bound')

    base.InteractionJourney = StableJourney
    return base.main()


if __name__ == '__main__':
    raise SystemExit(main())
