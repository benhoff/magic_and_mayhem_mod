#!/usr/bin/env python3
"""Exercise actual copy admission/diagnostic headers with controlled Win32 stubs."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from copy_timing import decode
from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    r = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    b = next(x for x in r['behaviors'] if x['id'] == 'NP.copy-contention-timing-observation')
    s = next(x for x in r['scenarios'] if x['id'] == 'copy-timing-private-20261012')
    claims = [dict(behavior=b['id'], contract_sha256=behavior_contract(b, {x['id']: x for x in r['builds']}),
                   scenarios={s['id']: scenario_contract(s)})]
    paths = required_sources(b) | set(s['tests'])

    def fingerprints():
        return {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}

    sources = fingerprints()
    declaration = json.loads(args.claims.read_text())
    if declaration['claims'] != claims or declaration['sources'] != sources:
        raise ValueError('Scope/source declaration mismatch')
    parent = ROOT/'working/tests/copy-timing';parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent));print(out, flush=True)
    (out/'.retain').write_text('Controlled copy timing diagnostics evidence; not live gameplay.\n')
    (out/'claims.json').write_bytes(args.claims.read_bytes())

    def run(command, name, timeout=180):
        result = subprocess.run(list(map(str, command)), capture_output=True, text=True, timeout=timeout)
        (out/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
        return result.stdout

    run(['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-pthread',
         ROOT/'tests/copy-timing-test.cpp', '-o', out/'probe'], 'compile')
    probe = json.loads(run([out/'probe', out/'copy-timing.bin'], 'probe', 30))
    trace = decode((out/'copy-timing.bin').read_bytes());wait = trace['waits'][0]
    if not (probe['success'] and wait['complete'] and wait['elapsed_tick_ms'] == 65
            and wait['elapsed_qpc_ms'] == 65 and wait['intervening_published_acquisitions'] == 3
            and wait['owner_publication_age_qpc_ms'] == 16):
        raise ValueError('Incomplete contention observation')
    negative = 0
    data = (out/'copy-timing.bin').read_bytes()
    for bad in [b'', data[:-1], b'badmagic'+data[8:], data[:32]+data[32:132]+data[32:132]]:
        try:
            decode(bad)
        except ValueError:
            negative += 1
        else:
            raise ValueError('Malformed diagnostic accepted')
    run(['python3', ROOT/'tools/build-render-bridge.py'], 'build-production', 300)
    run(['python3', ROOT/'tools/build-render-bridge.py', '--selftest'], 'build-selftest', 300)
    if fingerprints() != sources:
        raise ValueError('Sources changed during execution')
    report = dict(success=True, probe=probe, decoded=trace, decoder_refusals=negative,
                  pe32_builds=2, source_stable=True, sources=sources, execution_claims=claims,
                  scope=b['scope'], live_validated=False, original_work_bypassed=False,
                  artifacts={p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print('Controlled contention/recovery diagnostics and2 PE32 builds passed', flush=True)


if __name__ == '__main__':
    main()
