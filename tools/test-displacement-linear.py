#!/usr/bin/env python3
"""Execute pinned original distortion leaves with captured right-edge input."""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--input',type=Path,required=True);parser.add_argument('--native',action='store_true');args=parser.parse_args()
    if not args.input.resolve().is_relative_to(ROOT/'working'):raise ValueError('Require retained owned input')
    parent=ROOT/'working/tests/displacement-linear';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));report={'success':False}
    env=dict(os.environ,TMPDIR=str(ROOT/'working/compiler-tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='8');Path(env['TMPDIR']).mkdir(exist_ok=True)
    def run(command):
        result=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,text=True,timeout=180)
        with (out/'execution.log').open('a') as f:f.write(result.stdout+result.stderr)
        if result.returncode:raise RuntimeError('Command failed: '+repr(command))
        return result.stdout
    try:
        run([str(ROOT/'tools/original-manifest.sh'),'verify'])
        run(['python3','tools/draft-coverage-claims.py','--behavior','RS.displacement-linear-stride','--scenario','RS.displacement-linear-stride.owned-comparison','--output',str(out/'claims.json')]);claims=json.loads((out/'claims.json').read_text());report.update(sources=claims['sources'],claims=claims['claims'])
        for directory in ('apps','assets','audio','compat','reconstruction','renderer','runtime','protocols'):
            for source in (ROOT/directory).rglob('*'):
                if source.is_file() and (source.suffix in ('.cpp','.hpp','.h','.c','.S','.py') or source.name=='CMakeLists.txt'):report['sources'][source.relative_to(ROOT).as_posix()]=sha(source)
        exe=ROOT/'working/game-nocd/Chaos.exe';expected='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
        if sha(exe)!=expected:raise ValueError('Original executable hash')
        report['inputs']={str(exe.resolve()):expected,str(args.input.resolve()):sha(args.input)}
        binary=out/'reference';run(['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie','tests/displacement-linear-reference.cpp','-o',str(binary)])
        run([str(binary),str(exe),str(args.input.resolve()),str(out/'original.json')]);report['original']=json.loads((out/'original.json').read_text());report['reference_sha256']=sha(binary);report['success']=True
        if args.native:
            run(['cmake','-S','apps/qt-shell','-B','working/build/qt-shell','-DCMAKE_BUILD_TYPE=RelWithDebInfo'])
            run(['cmake','--build','working/build/qt-shell','--target','displacement-linear-test','other-writer-atomic-test','canvas-sequence-test','-j','8'])
            build=ROOT/'working/build/qt-shell/world-rolling';native=build/'displacement-linear-test'
            report['native']=json.loads(run(['xvfb-run','-a',str(native),str(args.input.resolve()),str(out/'native.565')]))
            if (out/'native.565').read_bytes()!=(out/'original.json.565').read_bytes():raise ValueError('Native differs from isolated original canvas')
            report['original_native_pixels_match']=True;report['native_sha256']=sha(native)
            report['atomic_regression']=run([str(build/'canvas-producers/other-writer-atomic-test')]).strip()
            report['canvas_regression']=run([str(build/'canvas-producers/canvas-sequence-test')]).strip()
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as e:report.update(success=False,error=str(e))
    finally:
        if 'sources' in report:
            report['sources_unchanged']=all(sha(ROOT/p)==v for p,v in report['sources'].items());report['inputs_unchanged']=all(sha(Path(p))==v for p,v in report.get('inputs',{}).items())
            if not report['sources_unchanged'] or not report['inputs_unchanged']:report.update(success=False,error='Sources/inputs changed')
        try:run([str(ROOT/'tools/original-manifest.sh'),'verify']);report['original_manifest_verified_after']=True
        except (OSError,RuntimeError,subprocess.SubprocessError) as e:report.update(success=False,cleanup_error=str(e))
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
