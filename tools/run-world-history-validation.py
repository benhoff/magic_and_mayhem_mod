#!/usr/bin/env python3
"""Freeze shared sources and run isolated native startup history validation."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=['live','synthetic'], default='live')
    args = parser.parse_args()
    parent = ROOT/'working/tests/world-history-snapshot'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    frozen = output/'source'
    print(output, flush=True)
    names = subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'], cwd=ROOT).decode().split('\0')
    snapshot = {}
    for name in sorted(set(names)):
        path = ROOT/name
        if not name or name.split('/')[0] in {'original','working','.git'} or name.startswith('.') or not path.is_file():
            continue
        if not name.startswith('tests/fixtures/') and path.suffix not in {'.inc','.c','.h','.S','.cpp','.hpp','.py','.sh','.md','.json','.tsv','.cmake','.mk'} and path.name != 'CMakeLists.txt':
            continue
        target = frozen/name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)
        snapshot[name] = hashlib.sha256(target.read_bytes()).hexdigest()
    work = frozen/'working'
    work.mkdir()
    # Staging uses cp -a: its source must be a directory, never a symlink.
    subprocess.run(['cp','-a','--reflink=auto',str(ROOT/'working/game-nocd'),str(work/'game-nocd')],check=True)
    for name in ['game-clean','manifests','wineprefix-x86_64']:
        (work/name).symlink_to(ROOT/'working'/name, target_is_directory=True)
    (frozen/'original').symlink_to(ROOT/'original', target_is_directory=True)
    # The manifest scanner deliberately does not follow directory symlinks.
    # Delegate verification to the real repository; never rewrite the manifest.
    verifier = ROOT/'tools/original-manifest.sh'
    verifier_hash = hashlib.sha256(verifier.read_bytes()).hexdigest()
    proxy = frozen/'tools/original-manifest.sh'
    proxy.write_text('#!/bin/sh\nexec '+shlex.quote(str(verifier))+' verify\n')
    snapshot['tools/original-manifest.sh'] = hashlib.sha256(proxy.read_bytes()).hexdigest()
    (output/'source-snapshot.json').write_text(json.dumps(snapshot, indent=2)+'\n')

    def run(name, command):
        with (output/(name+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=frozen, stdout=log,
                           stderr=subprocess.STDOUT, timeout=900, check=True)

    report = {'success':False, 'source_root':str(frozen), 'snapshot_manifest':str(output/'source-snapshot.json'), 'delegated_original_verifier':{'path':str(verifier),'sha256':verifier_hash},
              'scope':'Frozen source execution with current-workspace freshness reported separately; no silent evidence hash refresh.'}
    try:
        if args.mode == 'synthetic':
            run('synthetic', ['python3',frozen/'tools/test-world-live-history.py'])
            reports = list((work/'tests/world-live-history').glob('run-*/report.json'))
            report['synthetic_report'] = str(reports[-1])
        else:
            build = work/'build/world-frame'
            run('configure', ['cmake','-S',frozen/'compat/legacy','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
            run('build', ['cmake','--build',build,'--target','mnm-world-live','-j4'])
            claims = output/'claims.json'
            run('claims', ['python3',frozen/'tools/draft-coverage-claims.py','--behavior','NR.world-history-channel',
                           '--behavior','NR.world-live-history','--scenario','native-world-live-history-installed','--output',claims])
            live_reports = []
            for mode in ['history-normal','history-verify']:
                # A native-zero first canvas is intentionally strict in verify
                # mode. Retain failed fixtures; never treat a retry as all-seed equivalence.
                for attempt in range(3 if mode.endswith('verify') else 1):
                    before = set((work/'experiments/scene-observer').glob('run-*'))
                    command = ['python3',frozen/'tools/capture-scene-game.py','--world-live',mode,
                               '--live-frames','16','--claims',claims,'--prefix-template',ROOT/'working/wineprefix-x86_64']
                    name = f'{mode}-{attempt}'
                    with (output/(name+'.log')).open('x') as log:
                        result = subprocess.run([str(c) for c in command],cwd=frozen,stdout=log,stderr=subprocess.STDOUT,timeout=900)
                    added = set((work/'experiments/scene-observer').glob('run-*')) - before
                    if len(added) != 1:
                        raise RuntimeError('Missing isolated experiment directory: '+name)
                    experiment = added.pop()
                    path = experiment/'report.json'
                    if result.returncode == 0:
                        live_reports.append(str(path));break
                    native_path = experiment/'native-world.json'
                    refused = json.loads(native_path.read_text()) if native_path.exists() else {}
                    if mode != 'history-verify' or refused.get('error') != 'Native World differs from original; frame refused':
                        raise RuntimeError('Frozen live history failed: '+name)
                    report.setdefault('refused_fixtures',[]).append({'experiment':str(experiment),'native_report':str(native_path)})
                else:
                    raise RuntimeError('All bounded verify fixtures refused; startup producer gaps remain')
            report['live_reports'] = live_reports
        report['success'] = True
    finally:
        report['workspace_changed_since_snapshot'] = [p for p,h in snapshot.items()
                if p!='tools/original-manifest.sh' and (not (ROOT/p).is_file() or hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=h)]
        (output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(output/'report.json', flush=True)


if __name__ == '__main__':
    main()
