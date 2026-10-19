#!/usr/bin/env python3
"""Bounded production-facing controls on synthetic inputs; no original files."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
from coverage_claims import behavior_contract

ROOT=Path(__file__).resolve().parents[1]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--reuse-project',type=Path,help='Reuse an identical generated project; CMake rebuilds changed sources')
    a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='AN.world-facing')
    paths={__file__,str(ROOT/'tests/native-world-facing-test.cpp')}
    for base in ('apps/command-session','reconstruction','game','assets'):
        for f in (ROOT/base).rglob('*'):
            if f.is_file() and (f.name=='CMakeLists.txt' or f.suffix in {'.cpp','.c','.hpp','.h','.S','.cmake'}):
                paths.add(str(f))
    paths.update(str(ROOT/f) for f in behavior['documents'])
    sources={Path(f).resolve().relative_to(ROOT).as_posix():sha(Path(f)) for f in sorted(paths)}
    claims=[{'behavior':behavior['id'],'contract_sha256':behavior_contract(behavior,{b['id']:b for b in register['builds']})}]
    # Freeze scope/source declarations before configuring or executing the fixture.
    (out/'declaration.json').write_text(json.dumps({'claims':claims,'sources':sources},indent=2)+'\n')
    project=a.reuse_project.resolve() if a.reuse_project else out/'project'
    cmake=('cmake_minimum_required(VERSION 3.16)\nproject(facing_reconciliation LANGUAGES CXX)\n'
           f'add_subdirectory("{ROOT}/apps/command-session" command-session)\n'
           f'add_executable(native-world-facing "{ROOT}/tests/native-world-facing-test.cpp")\n'
           'target_link_libraries(native-world-facing PRIVATE mnm-native-command-session)\n'
           'target_compile_options(native-world-facing PRIVATE -Wall -Wextra -Werror -pedantic)\n')
    if a.reuse_project:
        if (project/'CMakeLists.txt').read_text()!=cmake:raise ValueError('Different generated build project')
    else:
        project.mkdir();(project/'CMakeLists.txt').write_text(cmake)
    build=project.parent/'build'
    report={'success':False,'scope':'Production cell-coordinate world-facing arithmetic through the native provider orient callback, on synthetic owned profile/terrain. No original comparison, whole targeting lifecycle, configured action-base provenance or live integration.',
            'claims':claims,'source_sha256':sources,'generated_cmake':cmake,'commands':[],
            'original_game_executed':False,'original_artifacts_consumed':False,'original_comparison':False}
    try:
        for name,command in [('configure',['cmake','-S',str(project),'-B',str(build),'-DCMAKE_BUILD_TYPE=Debug']),
                             ('build',['cmake','--build',str(build),'--target','native-world-facing','-j2']),
                             ('execute',[str(build/'native-world-facing')])]:
            report['commands'].append(command)
            run=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=900)
            (out/(name+'.stdout')).write_text(run.stdout);(out/(name+'.stderr')).write_text(run.stderr)
            if run.returncode:raise RuntimeError(name+': '+run.stderr[-2500:])
            if name=='execute':report.update(json.loads(run.stdout.splitlines()[-1]))
        report['sources_unchanged']=all(sha(ROOT/f)==h for f,h in sources.items())
        if not report['sources_unchanged']:raise RuntimeError('Sources changed during native controls')
        dependencies=set()
        for dependency_file in build.rglob('*.o.d'):
            for token in shlex.split(dependency_file.read_text().replace('\\\n',' ')):
                f=Path(token)
                if f.is_absolute() and f.is_file() and f.resolve().is_relative_to(ROOT):
                    name=f.resolve().relative_to(ROOT).as_posix()
                    if not name.startswith('working/'):dependencies.add(name)
        if dependencies-sources.keys():raise RuntimeError('Undeclared compiler input: '+str(sorted(dependencies-sources.keys())))
        report['compiler_dependencies']=sorted(dependencies)
    finally:
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:report[k] for k in ('success','native_facing_cases','sources_unchanged')}))
if __name__=='__main__':main()
