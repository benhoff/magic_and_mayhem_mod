#!/usr/bin/env python3
"""Compare every concrete shared-tick resource release caller, without live hooks."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import shlex
import subprocess
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import evidence_digest, read_json, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RS.screen-resource-release-callers']
RANGES = [(0x473c10,0x473d01),(0x474fe0,0x4750f1),(0x486260,0x486313),
    (0x4a7030,0x4a7107),(0x4a7d60,0x4a7e37),(0x4a84e0,0x4a84fa),
    (0x4a8d00,0x4a8e31),(0x4a9e20,0x4a9f65),(0x4aae70,0x4aafb5),
    (0x4ac510,0x4ac693),(0x4af070,0x4af1b6),(0x4b0060,0x4b0161),
    (0x4b0f50,0x4b10a3),(0x4b2010,0x4b20fe),(0x4b2ae0,0x4b2c0d),
    (0x4b40c0,0x4b42ba),(0x4b70d0,0x4b7236),(0x4b8730,0x4b87c8),
    (0x4b95b0,0x4b96b9),(0x4bb580,0x4bb685),(0x4bc1c0,0x4bc29f),
    (0x4cacb0,0x4cacf1),(0x4a6a60,0x4a6afd),(0x4a6b00,0x4a6b67),
    (0x4a7970,0x4a7a0a),(0x4a7a10,0x4a7a77),(0x4b1d80,0x4b1e2b),
    (0x4b1e30,0x4b1eae),(0x4cab50,0x4cab69),(0x4cab70,0x4cabcc),
    (0x4748a0,0x47493a),(0x473b00,0x473c10),(0x474d70,0x474e62)]


def local_flow(pe, start, end):
    decoder = Decoder()
    todo, rows = [start], {}
    try:
        while todo:
            at = todo.pop()
            if at in rows:
                continue
            if not start <= at < end:
                raise ValueError('Local edge escapes '+hex(at))
            row = decoder.decode(pe.read(at,min(15,end-at)),at)
            rows[at] = row
            mnemonic, operand = row['mnemonic'], row['operand']
            if mnemonic.startswith('ret'):
                continue
            if mnemonic.startswith('j'):
                if not operand.startswith('0x'):
                    raise ValueError('Unresolved local branch')
                todo.append(int(operand,16))
            if mnemonic != 'jmp':
                todo.append(at+row['size'])
        occupied = {a for row in rows.values()
                    for a in range(row['address'],row['address']+row['size'])}
        if occupied != set(range(start,end)) or len(occupied)!=sum(r['size'] for r in rows.values()):
            raise ValueError('Local instruction census mismatch '+hex(start))
        return dict(entry=start,end=end,sha256=hashlib.sha256(pe.read(start,end-start)).hexdigest(),
                    instructions=sorted(rows.values(),key=lambda r:r['address']),closed_local_cfg=True)
    finally:
        decoder.close()


def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',required=True)
    parser.add_argument('--suite',choices=['screens','world'],default='screens')
    args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/args.report).exists():
        parser.error('Require new owned output and immutable report')
    out.mkdir(parents=True);(out/'tmp').mkdir()
    (out/'.retain').write_text('Original screen ownership comparison logs and dependency provenance.\n')
    ids=IDS if args.suite=='screens' else ['RS.world-screen-retirement']
    ranges=RANGES if args.suite=='screens' else [(0x46ab30,0x46ae8c)]
    cpp='tests/screen-resource-ownership-reference.cpp' if args.suite=='screens' else 'tests/world-screen-retirement-reference.cpp'
    reg=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
    behaviors={b['id']:b for b in reg['behaviors']};builds={b['id']:b for b in reg['builds']}
    sources={name:evidence_digest(ROOT/name,root=ROOT)
             for name in sorted(set().union(*(required_sources(behaviors[i]) for i in ids)))}
    report=dict(schema=1,success=False,original_sha256=BUILD,sources=sources,
        claims=[dict(behavior=i,contract_sha256=behavior_contract(behaviors[i],builds)) for i in ids],
        scope=behaviors[ids[0]]['scope'],commands=[],live_replacement=False,
        provider_allocator_and_crt_endpoints_authored=True)
    def run(label,command):
        print(label,flush=True)
        result=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,timeout=240,
            env=os.environ|dict(TMPDIR=os.environ.get('TMPDIR',str(out/'tmp')),
                ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
        payload=result.stdout+b'\n'+result.stderr;(out/(label+'.log')).write_bytes(payload)
        report['commands'].append(dict(label=label,exit=result.returncode,sha256=hashlib.sha256(payload).hexdigest()))
        if result.returncode:raise ValueError(label+': '+payload.decode(errors='replace')[-3500:])
        return result.stdout.decode()
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        image=ROOT/'original/Arcane_Nocd/Chaos.exe';pe=PE32(image.read_bytes())
        if pe.sha256!=BUILD:raise ValueError('Unsupported original executable')
        report['flow']=[local_flow(pe,a,b) for a,b in ranges]
        report['original_entry_anchors']={hex(a):pe.read(a,16).hex() for a,b in ranges}
        if args.suite=='screens':
            report['concrete_release_entries']=[a for a,b in RANGES[:21]]
            tables=[0x5c5ebc,0x5c5ef4,0x5c6120,0x5c63e4,0x5c641c,0x5c6454,0x5c648c,0x5c64c4,0x5c64fc,0x5c6534,0x5c659c,0x5c65d4,0x5c660c,0x5c6644,0x5c667c,0x5c66b4,0x5c66ec,0x5c67f8,0x5c689c,0x5c6940,0x5c6978]
            report['table_release_links']=[dict(table=t,release=a) for t,(a,b) in zip(tables,RANGES[:21])]
            if any(pe.word(t+0x10)!=0x5595d0 or pe.word(t+0x20)!=a for t,(a,b) in zip(tables,RANGES[:21])):raise ValueError('Concrete release table binding changed')
            report['abstract_purecall_table']=0x5c6564
            if pe.word(0x5c6564+0x20)!=0x59b823:raise ValueError('Abstract slot changed')
        for label,flags in [('original',['-m32','-O2','-DMNM_ORIGINAL_REFERENCE']),
            ('sanitized',['-m32','-O1','-DMNM_ORIGINAL_REFERENCE','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
            ('native64',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            run(label+'-build',['g++','-std=c++17','-mstackrealign','-Wall','-Wextra','-Werror',
                '-fno-pie','-no-pie',*flags,'-MMD','-MF',out/(label+'.d'),
                ROOT/cpp,'-o',out/label])
            for token in shlex.split((out/(label+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(token).resolve()
                if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:
                    raise ValueError('Undeclared compiler source '+token)
            report[label]=json.loads(run(label,[out/label,*([image] if label!='native64' else [])]))
        if report['original']!=report['sanitized'] or report['original']!=report['native64']:
            raise ValueError('Cross-configuration mismatch')
        report['success']=True
    except (OSError,ValueError,AssertionError,subprocess.SubprocessError) as exc:
        report['error']=str(exc)
    finally:
        try:
            run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after']=True
        except (OSError,ValueError,subprocess.SubprocessError) as exc:
            report.update(success=False,manifest_error=str(exc))
        report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        digest=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],sha256=digest,error=report.get('error'))),flush=True)
    return int(not report['success'])


if __name__=='__main__':raise SystemExit(main())
