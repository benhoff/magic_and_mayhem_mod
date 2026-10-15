#!/usr/bin/env python3
"""Compare actual native minimap pixel splices with bounded original callers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/minimap-splice'
    parent.mkdir(parents=True,exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    print(out,flush=True)
    report = dict(schema=1,success=False,sources=claims['sources'],claims=claims['claims'],original_sha256=HASH,
        scope='Bounded mapped original minimap terrain/cell/creature/four camera pixel bodies versus actual production installer/resolver/assembly/kernel; supplied owned-plane, Win32 and Lock/Unlock adapters, full physical words/padding/guards, source/object effects and caller GPR/defined flags/DF/selected full FX. Camera fixtures retain original origin/colour/list preparation with terrain separately stubbed; no combined real driver/game/World integration or live replacement.',
        experiment=str(out.relative_to(ROOT)),live_replacement=False,original_work_bypassed=False)
    commands = []

    def run(label,argv):
        r = subprocess.run([str(a) for a in argv],cwd=ROOT,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'),capture_output=True,text=True,timeout=300)
        log = out/(label+'.log')
        log.write_text(r.stdout+r.stderr)
        commands.append(dict(label=label,returncode=r.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
        r.check_returncode()
        return r.stdout
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        exe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(exe)!=HASH:
            raise ValueError('Unsupported original build')
        objects = []
        names = ['reconstruction/rendering/minimap_pixels.c','reconstruction/rendering/pixel_kernel.c',
                 'runtime/canvas-writers/minimap_route.c','runtime/canvas-writers/minimap_entry.S',
                 'runtime/canvas-writers/minimap_bridge.c','tests/pixel-splice-state-probe.S']
        for name in names:
            obj = out/(Path(name).name+'.o')
            run(Path(name).name,['clang','-m32','-O2','-fno-pie','-fdeclspec','-Wno-ignored-attributes',
                '-DMNM_MINIMAP_WRITER_SELFTEST','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
                '-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj])
            objects.append(obj)
        run('reference-build',['clang++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-fdeclspec',
            '-Wno-ignored-attributes','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',
            ROOT/'tests/minimap-splice-reference.cpp',*objects,'-o',out/'reference'])
        closure = {'tools/test-minimap-splice.py','tools/original-manifest.sh'}
        for dep in out.glob('*.d'):
            for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                p = Path(token).resolve()
                if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    closure.add(p.relative_to(ROOT).as_posix())
        if closure-set(claims['sources']):
            raise ValueError('Undeclared compiler dependencies: '+repr(sorted(closure-set(claims['sources']))))
        report['compiled_project_sources'] = sorted(closure)
        run('production-pe32',['python3',ROOT/'tools/build-minimap-writers.py'])
        run('original-native-minimap',[out/'reference',exe,out/'comparison.json'])
        report['comparison'] = json.loads((out/'comparison.json').read_text())
        report['success'] = report['comparison']['success']
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error:
            report.update(success=False,manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/p)==h for p,h in claims['sources'].items())
        report['inputs_stable'] = sha(ROOT/'original/Arcane_Nocd/Chaos.exe')==HASH
        report['success'] = report['success'] and report['sources_stable'] and report['inputs_stable']
        report['commands'] = commands
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
