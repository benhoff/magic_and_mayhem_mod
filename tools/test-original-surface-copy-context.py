#!/usr/bin/env python3
"""Compare dynamic retry arguments from unchanged original copy wrappers."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import read_bytes, write_report

ROOT=Path(__file__).resolve().parents[1]
BEHAVIOR='RE.surface-copy.retry-context'
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    p=Path(path).resolve()
    return hashlib.sha256(read_bytes(p,root=ROOT) if p.is_relative_to(ROOT) else p.read_bytes()).hexdigest()


def cases():
    for values in itertools.product([0x58c4a0,0x58c8a0],range(2),range(2),range(3),range(2),
                                    range(3),range(4),[0,0x80004005],range(8),range(2)):
        shape=values[-1]
        yield [*values[:-1],*([1,1,5,4,2,2] if not shape else [-2147483648,-3,2147483647,7,2147483647,-2147483648])]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--main-root',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve();target=a.report.resolve()
    if out.exists() or target.exists() or not out.is_relative_to(ROOT/'working/tests') or not target.is_relative_to(ROOT/'research/runtime'):
        p.error('Require new owned output and research report paths')
    out.mkdir(parents=True);(out/'.retain').write_text('Retain original copy context inputs, binaries, full event logs and failed hypotheses.\n')
    (out/'tmp').mkdir()
    env=dict(os.environ,TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    b=next(b for b in register['behaviors'] if b['id']==BEHAVIOR)
    sources={name:sha(ROOT/name) for name in sorted(required_sources(b))}
    report=dict(schema=1,success=False,original_sha256=BUILD,sources=sources,
                claims=[dict(behavior=BEHAVIOR,contract_sha256=behavior_contract(b,{v['id']:v for v in register['builds']}))],
                scope=b['scope'],commands=[],models={},original_instruction_bytes_patched=False,
                real_driver_executed=False,live_replacement=False,artifacts=out.relative_to(ROOT).as_posix())

    def run(label,argv):
        print(label,flush=True)
        result=subprocess.run([str(v) for v in argv],cwd=ROOT,env=env,capture_output=True,timeout=120)
        data=result.stdout+b'\n'+result.stderr;(out/(label+'.log')).write_bytes(data)
        report['commands'].append(dict(label=label,exit_code=result.returncode,sha256=hashlib.sha256(data).hexdigest(),tail=data[-1500:].decode(errors='replace')))
        result.check_returncode();return result.stdout

    manifest=a.main_root.resolve()/'tools/original-manifest.sh';original=a.main_root.resolve()/'original/Arcane_Nocd/Chaos.exe'
    verified_before=False
    try:
        run('manifest-before',[manifest,'verify']);verified_before=True
        if sha(original)!=BUILD:raise ValueError('Original executable hash differs')
        rows=list(cases());inputs=out/'inputs.txt'
        inputs.write_text(''.join(' '.join(str(v) for v in [n,*row])+'\n' for n,row in enumerate(rows)))
        report['cases']=len(rows);report['inputs_sha256']=sha(inputs)
        # GCC's PE32 thiscall pointer extension is required by the original ABI;
        # keep strict ISO diagnostics for the separately compiled native models.
        common=['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror']
        harness=ROOT/'tests/surface-copy-retry-context.cpp'
        reference=out/'original'
        run('original-build',[*common,'-m32','-fno-pie','-no-pie',harness,'-o',reference])
        original_rows=json.loads(run('original-events',[reference,original,inputs]))
        if len(original_rows)!=len(rows):raise ValueError('Incomplete original cases')
        report['original_binary_sha256']=sha(reference)
        for mode,flags in [('native64',[]),('native32',['-m32']),('sanitized64',[
            '-O1','-g1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer','-fno-pie','-no-pie']),
            ('sanitized32',['-m32','-O1','-g1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
            binary=out/mode
            run(mode+'-build',[*common,'-pedantic',*flags,'-DMNM_NATIVE_MODEL',harness,
                ROOT/'reconstruction/rendering/surface_wrappers.cpp','-o',binary])
            actual=json.loads(run(mode+'-events',[binary,original,inputs]))
            if actual!=original_rows:
                mismatch=next((n for n,(old,new) in enumerate(zip(original_rows,actual)) if old!=new),min(len(actual),len(original_rows)))
                report['first_mismatch']=dict(index=mismatch,input=rows[mismatch],original=original_rows[mismatch],model=actual[mismatch] if mismatch<len(actual) else None)
                raise ValueError('Original/model retry context differs in '+mode)
            report['models'][mode]=dict(matched=len(actual),binary_sha256=sha(binary))
        report['success']=True
    except subprocess.CalledProcessError as error:
        report['error']=report['commands'][-1]['label']+' exited '+str(error.returncode)
    except Exception as error:report['error']=repr(error)
    finally:
        try:
            run('manifest-after',[manifest,'verify']);report['original_manifest_verified_before_after']=verified_before
        except Exception as error:
            report['original_manifest_verified_before_after']=False;report['manifest_error']=repr(error)
        report['sources_stable']=all(sha(ROOT/name)==value for name,value in sources.items())
        report['success'] &= report['sources_stable'] and report['original_manifest_verified_before_after']
        report['source_variants']={name:(ROOT/name).read_text() for name in sources if name in {
            'reconstruction/rendering/surface_copy_retry_geometry.hpp','tests/surface-copy-retry-context.cpp','tools/test-original-surface-copy-context.py'}}
        write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps({k:report.get(k) for k in ['success','error','cases','models','first_mismatch']}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
