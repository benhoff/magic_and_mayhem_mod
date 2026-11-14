#!/usr/bin/env python3
"""Use the unchanged request state machine with reviewed budget and staged variant joins."""
from functools import partial
import importlib.util
import json
from pathlib import Path
import sys
from broader_secondary_budget import bound, predecessors, P
from broader_secondary_scene_variant import verify_variant
from broader_secondary_staged_scene import verify_staged
from broader_secondary_subprocess import configuration, verify_selection
from coverage_evidence import Reader, write_report
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]


def request_module():
    path = ROOT / 'tools/run-broader-player-case.py'
    spec = importlib.util.spec_from_file_location('secondary_reviewed_request_state_machine', path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    code = module.main.__code__
    module.cumulative_bound = bound
    module.reviewed_predecessors = predecessors
    module.Reader = partial(Reader, memoize=True)
    require(module.main.__code__ is code, 'Original request state machine changed')
    return module


def main():
    # Keep original argument parsing, reservation order, source predicates and
    # operation guards in the original main. These additional checks run before
    # any reservation or admission, and never execute an acquisition themselves.
    require(len(sys.argv) >= 2 and sys.argv[1] in ('acquire', 'admit'), 'Known request operation required')
    require(sys.argv.count('--plan') == sys.argv.count('--case') == 1, 'Unique prospective plan/case required')
    plan_name = sys.argv[sys.argv.index('--plan') + 1]
    cid = sys.argv[sys.argv.index('--case') + 1]
    with Reader(ROOT, memoize=True) as reader:
        phase = dict(record=plan_name, sha256=reader.digest(plan_name))
        plan = reader.json(plan_name)
        require(any(v['id'] == cid for v in plan['broader_cases']), 'Case is not prospectively declared')
        if 'scene_runtime_variant' in plan:
            require(plan['family'] == 'interaction', 'Derived scene scope is interaction only')
            verify_variant(reader, ROOT, plan['scene_runtime_variant'], materialized=True)
            case = next(v for v in plan['broader_cases'] if v['id'] == cid)
            configuration(ROOT,plan,case['report'],reader.digest)
            if sys.argv[1] == 'admit':
                require(sys.argv.count('--validation') == 1, 'One exact completed validation required')
                vn = sys.argv[sys.argv.index('--validation') + 1]
                case = next(v for v in plan['broader_cases'] if v['id'] == cid)
                ar = dict(record=case['report'], sha256=reader.digest(case['report']))
                vr = dict(record=vn, sha256=reader.digest(vn))
                proof = verify_staged(reader, ROOT, phase, ar, vr)
                selection,_ = verify_selection(reader,ROOT,phase,case['report'])
                proof['actual_subprocess_selection'] = selection
                name = P + cid + '-runtime-variant-verification-01.json'
                require(not (ROOT / name).exists(), 'Never overwrite a staged runtime verification')
                proof.update(schema=1, kind='current_secondary_runtime_pre_admission_verification', success=True,
                    sources={n: reader.digest(n) for n in ('tools/run-broader-secondary-source-case.py',
                        'tools/broader_secondary_staged_scene.py', 'tools/broader_secondary_scene_variant.py',
                        'tools/broader_secondary_subprocess.py','patches/secondary-scene-python-startup/sitecustomize.py')},
                    sources_stable=True, actual_ledger_rows_renamed=False)
                digest = write_report(ROOT, name, (json.dumps(proof, indent=2) + '\n').encode())
                print(json.dumps(dict(stage='runtime-variant-pre-admission-verified', report=name, sha256=digest)), flush=True)
    return request_module().main()


if __name__ == '__main__': raise SystemExit(main())
