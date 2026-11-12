#!/usr/bin/env python3
"""Bounded PE32 minimap marker row-table read-set comparison; no game execution."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_evidence import write_report
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
EXPECTED='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):
    with path.open('rb') as source:return hashlib.file_digest(source,'sha256').hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--pe',type=Path,default=ROOT/'working/game-nocd/Chaos.exe')
    args=parser.parse_args();out=args.output.resolve()
    if not out.is_relative_to(ROOT/'working') or out.exists() or args.report.exists():parser.error('Fresh owned output and report required')
    out.mkdir(parents=True);(out/'.retain').write_text('Pinned original minimap read-set evidence; preserve.\n')
    paths=['tools/test-minimap-row-readset.py','tests/minimap-row-readset-original.cpp','tests/sprite-binary-reference.cpp','runtime/scene/minimap_marker_row.h','tests/minimap-marker-row-test.c']
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='RS.minimap-creature-markers')
    sources={name:sha(ROOT/name) for name in set(paths)|required_sources(behavior)}
    claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in register['builds']}),scenarios={})]
    report=dict(schema=1,success=False,scope=__doc__,sources=sources,claims=claims,commands=[],original_game_executed=False,source_executable_sha256=sha(args.pe))
    def run(command):
        result=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True,timeout=120)
        record=dict(command=[str(x) for x in command],exit_code=result.returncode,stdout=result.stdout,stderr=result.stderr)
        report['commands'].append(record)
        if result.returncode:raise ValueError(result.stderr or result.stdout or 'Command failed')
        return result.stdout
    try:
        run([ROOT/'tools/original-manifest.sh','verify'])
        if report['source_executable_sha256']!=EXPECTED:raise ValueError('Pinned PE changed')
        binary=out/'original'
        run(['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',ROOT/'tests/minimap-row-readset-original.cpp','-o',binary])
        native=out/'native-contract'
        run(['cc','-std=c11','-O1','-g','-Wall','-Wextra','-Werror','-fsanitize=undefined,address',ROOT/'tests/minimap-marker-row-test.c','-o',native])
        run([native]);report['native_contract_sanitizers_passed']=True
        report['binary_sha256']=sha(binary)
        report['comparison']=json.loads(run([binary,args.pe]))
        report['success']=report['comparison']['success'] is True and report['comparison']['cases']==24
    except Exception as error:report['error']=str(error)
    finally:
        try:run([ROOT/'tools/original-manifest.sh','verify']);report['original_verified_after']=True
        except Exception as error:report.update(success=False,original_manifest_error=str(error))
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in sources.items())
        report['success'] &= report['sources_stable']
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({key:report.get(key) for key in ['success','error','comparison','sources_stable']}))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
