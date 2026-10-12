#!/usr/bin/env python3
"""Check private DirectPlay file/ownership guards; never execute the DLLs."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import coverage_claims
import directplay_dx6_launch as provider

ROOT = Path(__file__).resolve().parents[1]
BID = 'NP.directplay-prefix-installation-guards'


def fingerprints():
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == BID)
    plan = json.loads((ROOT / behavior['validation_dependencies'][0]).read_text())
    builds = {b['id']: b for b in register['builds']}
    return dict(sources={p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest()
                         for p in sorted(coverage_claims.required_sources(behavior))},
                claims=[dict(behavior=BID, contract_sha256=coverage_claims.behavior_contract(behavior, builds),
                             scenarios={plan['id']: coverage_claims.scenario_contract(plan)})])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--builtin-prefix', type=Path, required=True,
                        help='Read-only Wine11.16 prefix containing the pinned builtin inputs')
    args = parser.parse_args()
    source = args.builtin_prefix.resolve() / 'drive_c/windows/syswow64'
    for name, digest in provider.BUILTIN.items():
        if hashlib.sha256((source / name.lower()).read_bytes()).hexdigest() != digest:
            raise ValueError('Unsupported builtin fixture input: ' + name)
    before = fingerprints()
    parent = ROOT / 'working/tests/directplay-prefix-guards'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    (root / 'prospective.json').write_text(json.dumps(before, indent=2) + '\n')
    print(root, flush=True)
    os.environ['MNM_MENU_DIRECTPLAY_DX6'] = '1'
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        game = root / 'game'
        game.mkdir()
        profile = provider.stage(root)
        metadata = dict(directplay_dx6=profile)
        prefix = root / 'prefix'
        provider.admit_prefix(root, metadata, prefix)

        def seed(target):
            directory = target / 'drive_c/windows/syswow64'
            directory.mkdir(parents=True)
            for name in provider.BUILTIN:
                shutil.copy2(source / name.lower(), directory / name.lower())

        def hashes(target):
            return {p.relative_to(target).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                    for p in target.rglob('*') if p.is_file()}

        def reject_unchanged(call, target):
            prior = hashes(target)
            try:
                call()
            except ValueError:
                if hashes(target) != prior:
                    raise AssertionError('Rejected installation modified its prefix')
            else:
                raise AssertionError('Unsafe prefix was admitted')

        seed(prefix)
        provider.install_prefix(root, metadata, dict(WINEPREFIX=str(prefix)))
        provider.check_prefix(prefix)
        for name, digest in provider.BUILTIN.items():
            if hashlib.sha256((root / 'directplay-prefix-before' / name).read_bytes()).hexdigest() != digest:
                raise AssertionError('Builtin backup changed')
        reuse = root / 'reuse'
        reuse.mkdir()
        reused = dict(directplay_dx6=profile)
        prior = hashes(prefix)
        provider.admit_prefix(reuse, reused, prefix)
        provider.install_prefix(reuse, reused, dict(WINEPREFIX=str(prefix)))
        if reused['directplay_prefix']['initialize'] or hashes(prefix) != prior:
            raise AssertionError('Idempotent reuse modified its prefix')
        rejected = 0
        # Mutating the last input catches partial writes to earlier inputs.
        for index, name in enumerate(provider.BUILTIN):
            case = root / ('bad-' + str(index)); case.mkdir()
            target = case / 'prefix'
            data = dict(directplay_dx6=profile)
            provider.admit_prefix(case, data, target)
            seed(target)
            path = target / 'drive_c/windows/syswow64' / name.lower()
            value = bytearray(path.read_bytes())
            if hashlib.sha256(value).hexdigest() != provider.BUILTIN[name]:
                raise AssertionError('Fixture patch original hash changed')
            value[-1] ^= 1
            path.write_bytes(value)
            reject_unchanged(lambda: provider.install_prefix(case, data, dict(WINEPREFIX=str(target))), target)
            rejected += 1
        foreign = root / 'foreign'; seed(foreign)
        reject_unchanged(lambda: provider.admit_prefix(reuse, {}, foreign), foreign); rejected += 1
        reject_unchanged(lambda: provider.install_prefix(reuse, reused, dict(WINEPREFIX=str(foreign))), foreign); rejected += 1
        # Valid prefix ownership still cannot authorize a second active session.
        os.environ['WINEPREFIX'] = str(prefix)
        with (prefix / '.mnm-directplay-session.lock').open('a') as lease:
            fcntl.flock(lease, fcntl.LOCK_EX | fcntl.LOCK_NB)
            reject_unchanged(lambda: provider.launch(reuse, reused, []), prefix); rejected += 1
        if (reuse / 'original-process-wait.cmd').exists():
            raise AssertionError('Concurrent refusal launched a process')
        if fingerprints() != before:
            raise AssertionError('Sources/claims changed during guard validation')
        report = dict(before, success=True, cases=2, guard_rejections=rejected,
                      supplied_archive_sha256=provider.SOURCE_SHA256,
                      scope='Private file/ownership fixtures; no game/DLL code/network execution.',
                      game_executed=False, networking_executed=False, builtin_inputs=provider.BUILTIN)
        (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print('DirectPlay prefix guards passed:2 cases/' + str(rejected) + ' unchanged rejections', flush=True)
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
