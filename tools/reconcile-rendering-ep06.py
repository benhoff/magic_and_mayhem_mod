#!/usr/bin/env python3
"""Retain the exact 36-row EP06 evidence frontier without promoting final claims."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_ep06 import candidates, failed_facts, nominal_facts, reconcile

ROOT = Path(__file__).resolve().parents[1]
BEHAVIORS = ['TOOL.rendering-ep06-reconciliation', 'NP.rendering-operation-census']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pair', nargs=3, action='append', default=[], metavar=('CONFIG', 'ACQUISITION', 'COMPARISON'))
    parser.add_argument('--negative', action='append', default=[])
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    name = Path(args.report)
    if name.is_absolute() or '..' in name.parts or not args.report.startswith('research/runtime/rendering-ep06-results-20261101/') or (ROOT/name).exists():
        parser.error('New immutable owned report required')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors = [b for b in register['behaviors'] if b['id'] in BEHAVIORS]
    builds = {b['id']:b for b in register['builds']}
    scenario = next(s for s in register['scenarios'] if s['id']=='SC.rendering-ep06-frontier')
    report = dict(schema=1, kind='rendering_ep06_evidence_reconciliation', success=False, sources={}, inputs={},
                  claims=[dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds),
                               scenarios={scenario['id']:scenario_contract(scenario)}) for b in behaviors])
    try:
        with Reader(ROOT) as reader:
            plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
            required = set().union(*(required_sources(b) for b in behaviors))
            required.update(['tools/rendering_ep06.py', 'tools/reconcile-rendering-ep06.py', 'tests/test-rendering-ep06.py',
                             'tools/inspect-canvas-producers.py', 'tools/resident_capture.py', 'tools/rendering_acceptance.py',
                             'tools/rendering_normal_acceptance.py', 'tools/native_gpu_copy_diagnostics.py'])
            controls = ['tests/test-rendering-ep06.py', 'tests/test-ep06-census-policy.py',
                        'tests/test-rendering-normal-acceptance.py', 'tests/test-native-copy-routes.py',
                        'tests/test-native-copy-output.py']
            required.update(controls)
            report['sources'] = {n:reader.digest(n) for n in sorted(required)}
            evidence = candidates(ROOT, reader, register, plan)
            facts = []
            for configuration, acquisition, comparison in args.pair:
                pair = {k:dict(record=n, sha256=reader.digest(n)) for k,n in [('acquisition',acquisition),('comparison',comparison)]}
                facts.append(nominal_facts(ROOT, reader, configuration, pair))
            negatives = []
            for n in args.negative:
                negatives.append(failed_facts(ROOT, reader, dict(record=n,sha256=reader.digest(n))))
            report.update(reconcile(plan, evidence, facts, negatives))
            report['inputs'] = {r['record']:r['sha256'] for r in evidence}
            for f in facts:
                report['inputs'].update({r['record']:r['sha256'] for r in f['admission']['pair'].values()})
                report['sources'].update(f['admission']['sources'])
            report['inputs'].update({r['execution']['record']:r['execution']['sha256'] for r in negatives})
            for negative in negatives:
                for n,h in negative['sources'].items():
                    if n in report['sources'] and report['sources'][n]!=h:
                        raise ValueError('Different failed/current source phases: '+n)
                    report['sources'][n]=h
            report['controls'] = []
            for control in controls:
                run = subprocess.run([sys.executable, '-B', str(ROOT/control)], cwd=ROOT,
                                     capture_output=True, text=True, timeout=60)
                report['controls'].append(dict(path=control, returncode=run.returncode, output=run.stdout+run.stderr))
                if run.returncode:
                    raise ValueError('EP06 control failed: '+control)
            report['sources_stable'] = all(reader.digest(n)==h for n,h in report['sources'].items())
            report['success'] = report['sources_stable']
    except Exception as error:
        report.update(success=False, error=str(error))
    digest = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('success','error','ep06_obligations','whole_operations_accepted','rows_with_current_scoped_proof')} | dict(record=args.report,sha256=digest)), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
