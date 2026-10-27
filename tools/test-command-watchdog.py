#!/usr/bin/env python3
"""Actual queue deadline interleavings: late ACK, idle, cancellation and invalid peers."""
import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import subprocess
import tarfile
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--claims',type=Path,help='Prospective scope/source declaration')
    parser.add_argument('--baseline',action='store_true',help='Retain an expected old-code regression failure without acceptance credit')
    args=parser.parse_args();out=args.output.resolve()
    if not out.is_relative_to(ROOT/'working') or out.exists() or args.report.exists():parser.error('Fresh owned output/report required')
    out.mkdir(parents=True);(out/'.retain').write_text('Queue watchdog controls and exact preimages; preserve.\n')
    paths=['tools/test-command-watchdog.py','tests/render-command-writer-test.c','tests/render-command-queue-test.c','tests/render-command-pressure-test.c','runtime/render/command_queue.h','runtime/render/command_channel.h','runtime/render/copy_order.h','runtime/render/command_pressure.h','protocols/include/mnm/render_command_ring.h','protocols/include/mnm/render_commands_v1.h','protocols/include/mnm/render_commands_v2.h','protocols/include/mnm/render_commands_v3.h']
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    report=dict(schema=1,success=False,sources=sources,scope=__doc__,baseline=args.baseline,original_game_executed=False,commands=[])
    if args.claims:
        declaration=json.loads(args.claims.read_text())
        for p,h in declaration['sources'].items():
            if hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=h:raise ValueError('Prospective source changed: '+p)
        sources.update(declaration['sources']);report['claims']=declaration['claims']
    def run(command):
        r=subprocess.run([str(v) for v in command],cwd=ROOT,capture_output=True,text=True,timeout=90,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'))
        report['commands'].append(dict(command=[str(v) for v in command],exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr));return r
    try:
        for label,flags in [('native',['-O2']),*([] if args.baseline else [('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])])]:
            for fixture in ['render-command-queue-test.c',*([] if args.baseline else ['render-command-pressure-test.c'])]:
                binary=out/(label+'-'+fixture.removesuffix('.c'))
                r=run(['cc','-std=c11','-Wall','-Wextra','-Werror',*flags,ROOT/'tests'/fixture,'-o',binary])
                if r.returncode:raise ValueError('Control build failed')
                r=run([binary])
                if args.baseline:
                    observed=[json.loads(line) for line in r.stdout.splitlines()]
                    report['old_false_stall_observed']=r.returncode==1 and any(v.get('watchdog_case')==0 and v.get('reason')==4 and v.get('expected_reason')==0 for v in observed)
                    if not report['old_false_stall_observed']:raise ValueError('Expected old-code false stall was not observed')
                elif r.returncode:raise ValueError('Queue/admission control failed')
        report['success']=not args.baseline
    except Exception as error:report['error']=str(error)
    finally:
        report['sources_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items());report['success'] &= report['sources_stable']
        raw=io.BytesIO()
        with tarfile.open(fileobj=raw,mode='w:gz') as archive:
            for p in sorted(sources):
                payload=(ROOT/p).read_bytes();entry=tarfile.TarInfo('sources/'+p);entry.size=len(payload);archive.addfile(entry,io.BytesIO(payload))
            for p in out.iterdir():
                if p.is_file():archive.add(p,arcname='outputs/'+p.name,recursive=False)
        rawpath=args.report.with_name(args.report.stem+'-raw.tar.gz');write_report(ROOT,rawpath,raw.getvalue());report['raw_record']=str(rawpath)
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ['success','error','old_false_stall_observed','sources_stable']}))
    return 0 if report['success'] or (args.baseline and report.get('old_false_stall_observed') and report['sources_stable']) else 1
if __name__=='__main__':raise SystemExit(main())
