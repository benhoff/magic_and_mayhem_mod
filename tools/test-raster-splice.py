#!/usr/bin/env python3
"""Compare actual native raster pixel splices with bounded original callers."""
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
    parser.add_argument('--frame-fixture',type=Path,help='Retained source-only 40x40 SPELL frame from raster refusal')
    args = parser.parse_args()
    fixture=args.frame_fixture.resolve() if args.frame_fixture else None
    fixture_hash='c606edaedb6fa88fdef428809a980c0858046d03722573eb12b8c1e24b3106a0'
    if fixture and (not fixture.is_relative_to(ROOT/'working') or sha(fixture)!=fixture_hash):
        raise ValueError('Unsupported retained raster fixture')
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/raster-splice'
    parent.mkdir(parents=True,exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    print(out,flush=True)
    report = dict(schema=1,success=False,sources=claims['sources'],claims=claims['claims'],original_sha256=HASH,
        scope='Bounded canonical RLE pixels across generic indexed/WORD, black silhouette, opaque/three blends, displacement, projected shadow and grayscale production interior splices versus pinned original bytes, padded RGB565/555, ABI nonvolatile registers/WORD result/defined flags/DF/selected full FX; original preparation retained. Private fast body volatile ECX/EDX/upper EAX and cdecl scratch argument mutation excluded; generic workspace separately checked. No real game/driver/World integration or live replacement',
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
        run('atomic-build',['clang','-O1','-g','-std=c11','-Wall','-Wextra','-Werror',
            '-fsanitize=address,undefined','-fno-omit-frame-pointer',ROOT/'tests/raster-kernel-test.c',
            ROOT/'reconstruction/rendering/raster_pixels.c',ROOT/'reconstruction/rendering/colour_rect.c',
            ROOT/'reconstruction/rendering/pixel_kernel.c','-o',out/'atomic'])
        report['atomic_result']=run('atomic-sanitized',[out/'atomic']).strip()
        objects = []
        names = ['reconstruction/rendering/raster_pixels.c','reconstruction/rendering/pixel_kernel.c',
                 'runtime/canvas-writers/raster_entry.S','runtime/canvas-writers/raster_bridge.c',
                 'tests/raster-splice-state-probe.S']
        for name in names:
            obj = out/(Path(name).name+'.o')
            run(Path(name).name,['clang','-m32','-O2','-fno-pie','-fdeclspec','-Wno-ignored-attributes',
                '-DMNM_RASTER_SELFTEST','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
                '-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj])
            objects.append(obj)
        run('reference-build',['clang++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-fdeclspec',
            '-Wno-ignored-attributes','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',
            ROOT/'tests/raster-splice-reference.cpp',*objects,'-o',out/'reference'])
        closure = {'tools/test-raster-splice.py','tools/original-manifest.sh','tests/raster-kernel-test.c',
                   'reconstruction/rendering/colour_rect.c','reconstruction/rendering/colour_rect.h'}
        for dep in out.glob('*.d'):
            for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                p = Path(token).resolve()
                if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    closure.add(p.relative_to(ROOT).as_posix())
        if closure-set(claims['sources']):
            raise ValueError('Undeclared compiler dependencies: '+repr(sorted(closure-set(claims['sources']))))
        report['compiled_project_sources'] = sorted(closure)
        run('production-pe32',['python3',ROOT/'tools/build-raster-writers.py'])
        run('original-native-raster',[out/'reference',exe,out/'comparison.json'])
        report['comparison'] = json.loads((out/'comparison.json').read_text())
        report['success'] = report['comparison']['success']
        if fixture:
            run('captured-original-native-raster',[out/'reference',exe,out/'captured-comparison.json',fixture])
            report['captured_comparison']=json.loads((out/'captured-comparison.json').read_text())
            report['frame_fixture']=dict(path=str(fixture.relative_to(ROOT)),sha256=sha(fixture))
            report['success']=report['success'] and report['captured_comparison']['success'] and report['captured_comparison']['captured_native_cases']>0 and sha(fixture)==fixture_hash
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
