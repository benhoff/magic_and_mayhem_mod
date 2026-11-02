#!/usr/bin/env python3
"""Exact V4 source-prefix transport, independent Python decoding and legacy guards."""
import argparse,hashlib,json,os,shlex,subprocess,sys
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--report',required=True);a=parser.parse_args()
    out=a.output.resolve()
    if not out.is_relative_to(ROOT/'working') or out.exists() or (ROOT/a.report).exists():parser.error('Fresh owned output and immutable report required')
    out.mkdir(parents=True);(out/'.retain').write_text('Exact source transport, malformed controls and legacy equality fixtures.\n')
    names=['tools/test-campaign-source-capture.py','tools/resident_capture.py','apps/qt-shell/profile_capture_codec.hpp',
           'apps/qt-shell/resident_profile_capture.cpp','apps/qt-shell/resident_profile_capture.hpp',
           'tests/campaign-source-capture-test.cpp','tests/test-campaign-source-capture.py',
           'tests/resident-profile-capture-test.cpp','tests/resident-profile-capture-test.py','tools/profile_native_resident_helpers.py']
    report=dict(schema=1,kind='campaign_source_transport_controls',success=False,original_game_executed=False,
                sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names},checks=[])
    def run(command):
        result=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,timeout=180,
                              env=dict(os.environ,TMPDIR='/tmp/mnm-campaign-validation-20261101/tmp',ASAN_OPTIONS='detect_leaks=1'))
        report['checks'].append(dict(command=list(map(str,command)),returncode=result.returncode,
                                    stdout=result.stdout.decode(errors='replace'),stderr=result.stderr.decode(errors='replace')))
        if result.returncode:raise ValueError('Source capture control failed: '+str(command))
        if result.stdout:
            try:report['checks'][-1]['result']=json.loads(result.stdout)
            except ValueError:pass
    try:
        flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True))
        flags+=shlex.split(subprocess.check_output(['pkg-config','--cflags','libzstd'],text=True))
        report['zstd_headers_version']=subprocess.check_output(['pkg-config','--modversion','libzstd'],text=True).strip()
        for mode,options in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            for family,source,python in [('campaign','tests/campaign-source-capture-test.cpp','tests/test-campaign-source-capture.py'),
                                         ('legacy','tests/resident-profile-capture-test.cpp','tests/resident-profile-capture-test.py')]:
                binary=out/(mode+'-'+family);wire=out/(mode+'-'+family+'-wire')
                run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-fPIC','-pthread',*options,ROOT/source,
                     ROOT/'apps/qt-shell/resident_profile_capture.cpp',*flags,'-o',binary])
                run([binary,wire]);run([sys.executable,'-B',ROOT/python,wire])
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in report['sources'].items());report['success'] &= report['sources_stable']
        report['artifacts']=[]
        for file in sorted(out.rglob('*')):
            if file.is_file():
                name=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(file.relative_to(out)).encode()).hexdigest()[:20]+'.bin'
                raw=file.read_bytes();h=write_report(ROOT,name,raw);report['artifacts'].append(dict(path=str(file.relative_to(out)),record=name,sha256=h,bytes=len(raw)))
        h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],sources_stable=report['sources_stable'],error=report.get('error'),record=a.report,sha256=h)),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
