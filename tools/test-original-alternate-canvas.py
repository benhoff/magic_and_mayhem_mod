#!/usr/bin/env python3
"""Bounded original alternate-canvas provenance, initializer and x87 copy recovery."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import itertools
import json
import os
import re
from pathlib import Path
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RS.alternate-canvas-initializer', 'RS.alternate-canvas-x87-copy',
       'RS.alternate-canvas-provenance']
GLOBALS = (0x6e1f68, 0x6e1f6c, 0x6e1f70, 0x6e1f74, 0x6e1f78)


def sha(path):
    return hashlib.sha256(evidence_bytes(path)).hexdigest()


def provenance(image):
    retained = ROOT/'research/runtime/all-original-recovery-results-20261020/flow-review-final.json'
    old = json.loads(evidence_text(retained))
    raw = ROOT/old['raw_flow']['path']
    if sha(raw) != old['raw_flow']['sha256']:
        raise ValueError('Retained instruction frontier changed; restore archived input first')
    flow = json.loads(evidence_text(raw))
    if not flow['success'] or flow['original_sha256'] != BUILD:
        raise ValueError('Unadmitted original frontier')
    rows = {r['address']: r for r in flow['rows']}
    decoder = Decoder()
    focused = []
    try:
        for start, end in ((0x59455c, 0x5945ec), (0x4e40d0, 0x4e4263)):
            instructions = []; at = start
            while at < end:
                row = decoder.decode(image.read(at, min(15, end-at)), at)
                rows[at] = row; instructions.append(row); at += row['size']
            if at != end or instructions[-1]['mnemonic'] != 'ret':
                raise ValueError('Focused function boundary differs')
            focused.append(dict(entry=start, end=end,
                sha256=hashlib.sha256(image.read(start,end-start)).hexdigest(),
                instructions=instructions))
    finally:
        decoder.close()
    references = []
    for address in GLOBALS:
        needle = struct.pack('<I',address); at = 0; sites = []
        while True:
            at = image.raw.find(needle,at)
            if at < 0: break
            section = next((s for s in image.sections if s['offset'] <= at < s['offset']+s['stored']),None)
            va = section['address']+at-section['offset'] if section else None
            owners = [r for delta in range(15) if (r := rows.get(va-delta if va else -1))
                      and r['address'] <= va and va+4 <= r['address']+r['size']
                      and hex(address) in r['operand']]
            for row in owners:
                if image.read(row['address'],row['size']).hex() != row['bytes']:
                    raise ValueError('Retained instruction bytes changed')
            sites.append(dict(file_offset=at,address=va,instructions=owners,
                classification='decoded_operand_candidate' if owners else 'unassigned_literal'))
            at += 1
        sec = image.section(address)
        references.append(dict(address=address,initial_bytes=image.read(address,4,True).hex(),
            zero_filled=address >= sec['address']+sec['stored'],section=sec['name'],sites=sites))
    constructions = []
    for row in rows.values():
        if row['mnemonic'] not in ('push','mov','lea','add','sub'): continue
        if any(0x6e1f00 <= int(a,16) < 0x6e2000 for a in re.findall(r'0x([0-9a-f]+)',row['operand'])):
            constructions.append(row)
    # Keep the full unresolved frontier. Direct operands never close computed,
    # bulk or external writes; no pointer provenance is manufactured here.
    return dict(schema=1,success=True,static_only=True,original_sha256=BUILD,
        prior_frontier=dict(path=old['raw_flow']['path'],sha256=sha(raw)),
        globals=references,nearby_address_constructions=sorted(constructions,key=lambda r:r['address']),
        focused_functions=focused,unresolved_indirect_edges=len(flow['unresolved_indirect_edges']),
        live_alternate_route_proved=False,allocation_or_initial_clear_proved=False,
        all_writer_ownership_proved=False,
        limitations=['PE loader zero-fill is global initialization, not pixel allocation or clear.',
            'Literal/nearby address review does not exclude register-relative, bulk or imported writers.',
            '59455c is discovered code; no original caller or enable/source publication is established.'])


def compare(native, original):
    procs = [subprocess.Popen(['zstd','-qdc',p],stdout=subprocess.PIPE) for p in (native,original)]
    digest = hashlib.sha256(); flags = {}; count = 0
    try:
        for case, values in enumerate(itertools.product((32,64,96),(1,2,5),(-32,-4,-1,0,1,4,32),
                                     (-33,-16,-4,-1,0,1,4,16,31,32,33,256),range(14))):
            records = [p.stdout.read(4124) for p in procs]
            if any(len(r)!=4124 for r in records): raise ValueError(f'Truncated x87 case {case}')
            if struct.unpack('<6I',records[0][:24]) != (case,*[v & 0xffffffff for v in values]):
                raise ValueError('x87 identity differs')
            if records[0] != records[1]:
                raise ValueError(f'x87 mismatch case {case}: {values}; flags {[struct.unpack_from("<I",r,24)[0] for r in records]}')
            status = str(struct.unpack_from('<I',records[0],24)[0]); flags[status]=flags.get(status,0)+1
            digest.update(records[0]); count += 1
        if count != 10584 or any(p.stdout.read(1) for p in procs): raise ValueError('x87 denominator/trailing bytes')
        if any(p.wait(timeout=10) for p in procs): raise ValueError('Decompression failed')
        return dict(cases=count,physical_words_compared=count*2048,bytes_compared=count*4124,
            x87_exception_flag_histogram=flags,native_atomic_refusals=8,sha256=digest.hexdigest(),
            original_body_unchanged=True,control_word=0x037f,original_stack_top_preserved=True)
    finally:
        for p in procs:
            if p.poll() is None: p.kill(); p.wait()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    a=parser.parse_args(); out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require a new retained output under working/tests')
    out.mkdir(parents=True); (out/'tmp').mkdir(); (out/'.retain').write_text('Alternate canvas original recovery evidence; preserve raw outcomes.\n')
    env=dict(os.environ,TMPDIR=str(out/'tmp'))
    register=json.loads(evidence_text(ROOT/'research/runtime/coverage/register.json'))
    behaviors={b['id']:b for b in register['behaviors']}; builds={b['id']:b for b in register['builds']}
    paths=set().union(*(required_sources(behaviors[i]) for i in IDS))|{'tools/coverage_claims.py','tools/original_x86.py'}
    sources={p:sha(ROOT/p) for p in sorted(paths)}
    claims=[dict(behavior=i,contract_sha256=behavior_contract(behaviors[i],builds)) for i in IDS]
    report=dict(schema=1,success=False,sources=sources,claims=claims,original_sha256=BUILD,
        live_replacement=False,original_pixels_used_as_native_inputs=False,
        actual_reachability_proved=False,alternate_allocation_proved=False,
        scope='Complete original initializer with owned mode/create/COM boundaries and separate complete original x87 copy comparison. No actual alternate enablement/allocation/writer or original caller proof.')
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label, command, compressed=False):
        print(label,flush=True)
        with (out/(label+'.log')).open('x') as log:
            if not compressed:
                result=subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=subprocess.PIPE,
                    stderr=log,check=True,timeout=180)
                log.write(result.stdout.decode()); return result.stdout.decode()
            with (out/(label+'.bin.zst')).open('xb') as dest:
                encoder=subprocess.Popen(['zstd','-q','-3','-c'],stdin=subprocess.PIPE,stdout=dest,stderr=log)
                process=subprocess.Popen([str(x) for x in command],cwd=ROOT,env=env,stdout=encoder.stdin,stderr=log)
                encoder.stdin.close()
                try:
                    if process.wait(timeout=180): raise ValueError(label+' failed')
                    if encoder.wait(timeout=30): raise ValueError('Compression failed')
                finally:
                    for p in (process,encoder):
                        if p.poll() is None: p.kill(); p.wait()
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']); before=True
        image_path=ROOT/'original/Arcane_Nocd/Chaos.exe'; image=PE32(evidence_bytes(image_path))
        if image.sha256!=BUILD: raise ValueError('Original hash changed')
        static=provenance(image); static.update(sources=sources)
        (out/'provenance.json').write_text(json.dumps(static,indent=2)+'\n')
        flags=['-std=c++17','-O2','-Wall','-Wextra','-Werror']
        original_flags=['-m32','-mstackrealign','-fno-pie','-no-pie']
        for label,extra in [('native',[]),('original',original_flags+['-DMNM_ORIGINAL_REFERENCE'])]:
            run(label+'-build',['g++',*flags,*extra,ROOT/'tests/alternate-canvas-x87.cpp','-o',out/label])
            run(label,[out/label,*([image_path] if label=='original' else [])],True)
        report['x87']=compare(out/'native.bin.zst',out/'original.bin.zst')
        run('initializer-build',['g++',*flags,*original_flags,ROOT/'tests/alternate-canvas-initializer.cpp','-o',out/'initializer'])
        report['initializer']=json.loads(run('initializer',[out/'initializer',image_path]))
        report.update(success=True,outputs_sha256={p.name:sha(p) for p in sorted(out.glob('*.bin.zst'))},
            binaries_sha256={n:sha(out/n) for n in ('native','original','initializer')})
    except (OSError,ValueError,subprocess.SubprocessError) as error:
        report['error']=str(error)
    finally:
        if before:
            try: run('original-after',[ROOT/'tools/original-manifest.sh','verify']); report['original_manifest_verified_before_after']=True
            except (OSError,subprocess.SubprocessError) as error: report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items()); report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n'); print(out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__': raise SystemExit(main())
