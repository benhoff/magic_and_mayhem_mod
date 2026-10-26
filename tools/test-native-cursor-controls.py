#!/usr/bin/env python3
"""Check production bridge/provider admission, strict journals and PE32 builds."""
import argparse,hashlib,json,subprocess,sys
from pathlib import Path
from coverage_evidence import write_report
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/a.report).exists():p.error('Fresh owned paths required')
    out.mkdir(parents=True);(out/'.retain').write_text('Provider/copy lock order and native production regression binaries and diagnostics.\n')
    reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in reg['behaviors'] if x['id']=='NP.cursor-provider-lifetime')
    paths=required_sources(b)|{'tools/test-native-cursor-controls.py','tests/native-copy-lifetime-test.cpp','tests/copy-timing-test.cpp','tests/test-native-cursor-journal.py','tools/native_cursor_journal.py','tools/native_cursor_launch.py','tools/build-render-bridge.py','tools/build-shadow-bridge.py','tools/build-native-draw.py','protocols/generate.py'}
    for directory in ['runtime/render','runtime/native-draw','protocols/include','protocols/schemas']:
        paths.update(f.relative_to(ROOT).as_posix() for f in (ROOT/directory).rglob('*') if f.is_file() and f.suffix in ('.c','.h','.json','.inc'))
    sources={n:sha(ROOT/n) for n in sorted(paths)}
    report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in reg['builds']}),scenarios={})],commands=[],original_game_executed=False,live_replacement=False)
    def run(name,argv):
        with (out/(name+'.log')).open('xb') as log:r=subprocess.run(list(map(str,argv)),cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=240)
        report['commands'].append(dict(name=name,argv=list(map(str,argv)),exit=r.returncode,log_sha256=sha(out/(name+'.log'))))
        if r.returncode:raise ValueError(name+' failed: '+(out/(name+'.log')).read_text()[-3000:])
        return (out/(name+'.log')).read_text()
    try:
        run('compile',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-pthread',ROOT/'tests/native-copy-lifetime-test.cpp','-o',out/'probe'])
        report['copy_admission']=[json.loads(line) for line in run('admission',[out/'probe']).splitlines()]
        run('strict-readers',[sys.executable,'-B',ROOT/'tests/test-native-cursor-journal.py','-v'])
        run('production',[sys.executable,ROOT/'tools/build-render-bridge.py','--output',out/'production'])
        run('selftest-build',[sys.executable,ROOT/'tools/build-render-bridge.py','--selftest','--output',out/'selftest'])
        report['selftest_execution']=False
        report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','copy_admission']}),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
