#!/usr/bin/env python3
"""Compare cursor clients, deferred borrows and disposal with unchanged NoCD code."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import shlex
import subprocess
from coverage_evidence import evidence_digest, read_json, write_report
from original_x86 import PE32
ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--executable', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--build-directory', type=Path, required=True)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    if a.output.exists() or a.build_directory.exists() or (ROOT/a.report).exists():
        p.error('Require new owned output/build/report paths')
    if not a.output.resolve().is_relative_to(ROOT/'working/tests'):
        p.error('Durable raw output must be under this worktree working/tests')
    a.output.mkdir(parents=True)
    a.build_directory.mkdir(parents=True)
    (a.output/'.retain').write_text('Cursor ownership execution, logs and exact fixture binaries.\n')
    declaration = read_json(a.claims)
    sources = declaration['sources']
    stable = lambda: all(evidence_digest(ROOT/n, root=ROOT) == h for n, h in sources.items())
    report = dict(schema=1, success=False, sources=sources, claims=declaration['claims'],
                  commands=[], original_game_executed=False, live_replacement=False,
                  all_source_alias_ownership_proved=False, original_sha256=BUILD,
                  scope='Serial valid-memory cursor client, deferred queue and disposal caller control/state comparison. Scripted OS/COM endpoints; unchanged original copy/destructor bodies are shared external dependencies. No physical pixels, real OS concurrency, allocation alias ownership or installed native cursor replacement.')
    def run(label, command, timeout=120):
        log = a.output/(label+'.log')
        with log.open('x') as f:
            result = subprocess.run(list(map(str,command)), cwd=ROOT, stdout=f,
                                    stderr=subprocess.STDOUT, timeout=timeout,
                                    env=os.environ | {'TMPDIR':str(a.build_directory.resolve()),
                                                      'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                                                      'UBSAN_OPTIONS':'halt_on_error=1'})
        report['commands'].append(dict(label=label, argv=list(map(str,command)), exit=result.returncode,
                                        log=log.relative_to(ROOT).as_posix()))
        if result.returncode:
            raise ValueError(label+': '+log.read_text(errors='replace')[-4000:])
        return log.read_text()
    try:
        if not stable():
            raise ValueError('Prospective sources changed before execution')
        run('original-before', [ROOT/'tools/original-manifest.sh','verify'])
        raw = a.executable.read_bytes()
        if hashlib.sha256(raw).hexdigest() != BUILD:
            raise ValueError('Unsupported original image')
        pe = PE32(raw)
        report['original_functions'] = {hex(at):dict(bytes=n,sha256=hashlib.sha256(pe.read(at,n)).hexdigest()) for at,n in [(0x4a45b0,849),(0x4a4910,1234),(0x4a37d0,46),(0x58d5b0,138),(0x58d660,227)]}
        for name, flags in [('optimized',['-O2']),('sanitized',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary = a.build_directory/name
            dep = a.build_directory/(name+'.d')
            run(name+'-build',['g++','-m32','-std=c++17','-fno-pie','-no-pie','-mstackrealign','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*flags,'-MMD','-MF',dep,ROOT/'tests/cursor-ownership-reference.cpp','-o',binary])
            for item in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path = Path(item).resolve()
                if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:
                    raise ValueError('Undeclared compiled dependency '+str(path))
            report[name] = json.loads(run(name,[binary,a.executable]))
            expected = dict(success=True,client_cases=10368,queue_cases=1280,dispose_cases=12,retry_cases=6,original_instructions_unchanged=True)
            if report[name] != expected:
                raise ValueError('Execution denominator differs')
            # Retain the exact executed fixture on durable storage; RAM output is disposable.
            target = a.output/name
            target.write_bytes(binary.read_bytes())
        report['success'] = True
    except (OSError,ValueError,subprocess.SubprocessError) as e:
        report['error'] = str(e)
    finally:
        try:
            run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after'] = any(c['label']=='original-before' and c['exit']==0 for c in report['commands'])
        except (OSError,ValueError,subprocess.SubprocessError) as e:
            report.update(success=False,manifest_error=str(e))
        report['sources_stable'] = stable()
        report['success'] &= report['sources_stable']
        report['artifacts'] = {f.relative_to(ROOT).as_posix():dict(bytes=f.stat().st_size,sha256=hashlib.sha256(f.read_bytes()).hexdigest()) for f in a.output.iterdir() if f.is_file()}
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],error=report.get('error'),results=report.get('optimized'))))
    return int(not report['success'])


if __name__ == '__main__':
    raise SystemExit(main())
