#!/usr/bin/env python3
"""Retain read-before-overwrite obligations at every installed World whitelist site."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

from caller_state_liveness import AccessDecoder, inspect
from coverage_evidence import Reader,write_report
from original_x86 import PE32

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', '--git-common-dir'], text=True).strip()).resolve().parent
ORIGINAL = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = ['tools/check-world-caller-state.py', 'tools/caller_state_liveness.py',
    'tools/original_x86.py', 'tests/test-caller-state-liveness.py',
    'runtime/scene/world_raster_callers.h', 'runtime/scene/canvas_producers.c',
    'runtime/scene/canvas_producers.S', 'runtime/scene/kind8_sprite_admission.h',
    'runtime/scene/world_producer_bypass.c', 'runtime/scene/drawing_caller_state.h',
    'protocols/include/mnm/drawing_caller_state_v2.h','protocols/include/mnm/drawing_caller_state_v3.h','protocols/include/mnm/drawing_caller_bindings_v1.h',
    'tools/retain-world-caller-contexts.py','tools/coverage_evidence.py','tools/original-manifest.sh']


def sites(text):
    result = []
    for name in ('raster_callers', 'trail_raster_sites', 'sprite_raster_sites'):
        match = re.search(r'\b' + name + r'\[\]\s*=\s*\{(.*?)\n\};', text, re.S)
        if not match:
            raise ValueError('Missing production caller table: ' + name)
        body = re.sub(r'/\*.*?\*/', '', match[1], flags=re.S)
        rows = re.findall(r'\{(0x[0-9a-f]+),([^{}]*)\{([^{}]+)\}\}', body)
        remainder = re.sub(r'\{0x[0-9a-f]+,[^{}]*\{[^{}]+\}\}\s*,?', '', body)
        if not rows or remainder.strip():
            raise ValueError('Unrecognized caller table row: ' + name)
        for address, metadata, raw in rows:
            values = raw.split(',')
            if len(values) != 10 or any(not re.fullmatch(r'0x[0-9a-f]+', v.strip()) for v in values):
                raise ValueError('Caller signature extent/encoding')
            result.append(dict(table=name, address=int(address, 16), metadata=metadata,
                               signature=bytes(int(v, 16) for v in values)))
    if len({r['address'] for r in result}) != len(result):
        raise ValueError('Duplicate caller site')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    target = ROOT / args.report
    if target.exists() or not target.resolve().is_relative_to(ROOT / 'research/runtime'):
        parser.error('Require a new research/runtime report')
    sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in SOURCES}
    record = dict(schema=1, kind='world_caller_state_requirements', success=False,
        sources=sources, original_game_runs=0, newly_passed_requirements=0,
        complete_caller_state_equivalence=False, original_sha256=ORIGINAL,
        scope='Static explicit production whitelist continuations under normal direct-call returns. Reads, unknown calls/returns and surviving bits remain required proof. Stack return-address mutation, exception unwinding, pixel, floating-state, stack-memory, natural reachability and whole-operation equivalence are not established.')
    decoder = None
    try:
        subprocess.run([str(MAIN / 'tools/original-manifest.sh'), 'verify'], check=True)
        binary = (MAIN / 'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if hashlib.sha256(binary).hexdigest() != ORIGINAL:
            raise ValueError('Unsupported original image')
        image = PE32(binary)
        source_text=(ROOT/'runtime/scene/world_raster_callers.h').read_text()
        # These are the exact enclosing contexts already checked by the runtime
        # guard. Keep static conditional context separate from an observed call.
        for address,signature in ((0x5006b2,'e9910100000fbf4e120f'),
                                  (0x5006e0,'e9670100008b560c8b4e'),
                                  (0x500f51,'e9d00100000fbf46068b')):
            if image.read(address,10).hex()!=signature:
                raise ValueError('Guarded outer continuation differs')
        record['guarded_context_bytes_verified']=True
        opaque_returns=(0x500566,0x500a8c,0x500e94)
        record['opaque_wrapper_callers']=[]
        for address in opaque_returns:
            code=image.read(address-5,5)
            if code[0]!=0xe8 or address+int.from_bytes(code[1:],'little',signed=True)!=0x57e8e0:
                raise ValueError('Opaque outer caller is not the pinned direct call')
            record['opaque_wrapper_callers'].append(dict(return_pc=address,call_bytes=code.hex(),
                return_bytes=image.read(address,10).hex()))
        decoder = AccessDecoder()
        def decode(address):
            return decoder.decode(image.read(address, 15), address)
        unit=subprocess.run(['python3',ROOT/'tests/test-caller-state-liveness.py'],cwd=ROOT,capture_output=True,timeout=120)
        unit_path=target.with_name(target.stem+'-controls.log.bin');write_report(ROOT,unit_path,unit.stdout+unit.stderr)
        if unit.returncode or b'Ran 26 tests'not in unit.stdout+unit.stderr:raise ValueError('Incomplete conditional liveness controls')
        record['controls']=dict(path=str(unit_path.relative_to(ROOT)),passed=26)
        body_path='research/runtime/caller-state-results-20261108/world-entry-bodies-01.json'
        with Reader(ROOT)as reader:
            bodies=reader.json(body_path);record['callee_body_input']=dict(path=body_path,sha256=reader.digest(body_path))
        if bodies['original_sha256']!=ORIGINAL or not bodies['success']:raise ValueError('Invalid retained callee input')
        callee_facts=[]
        for entry in(0x595677,0x5947b2,0x59521a,0x595b47,0x59603e,0x597086,0x596cb8):
            body=next(f for f in bodies['functions']if f['entry']==entry)
            if hashlib.sha256(b''.join(image.read(a,b-a)for a,b in body['ranges'])).hexdigest()!=body['sha256']or image.read(entry,8)!=bytes.fromhex('558bec5657535152'):
                raise ValueError('Callee save block/body changed')
            rets=[i['address']for i in body['instructions']if i['mnemonic']=='ret']
            if not rets or any(image.read(pc-6,7)!=bytes.fromhex('5a595b5f5ec9c3')for pc in rets):
                raise ValueError('Callee volatile restore block changed')
            def without_private_save(address):
                row=decode(address)
                if address in(entry+6,entry+7):
                    row=dict(row,reads=[r for r in row['reads']if r not in('ecx','edx')])
                return row
            ignored=inspect(entry,without_private_save,registers=dict(ecx=0xffffffff,edx=0xffffffff),follow_calls=True)
            if not ignored['closed']or ignored['required']:raise ValueError('Callee consumes incoming ECX/EDX outside private save')
            callee_facts.append(dict(entry=entry,body_sha256=body['sha256'],save_addresses=[entry+6,entry+7],
                restore_returns=rets,input_use_analysis=ignored,
                scope='Normal-return owned disjoint source/destination domain. Ignore only the two private prologue saves when checking incoming value use; literal restored register blocks and separate full original/native GPR/ESP comparisons establish preservation. No exception/stack-alias or other callee facts.'))
        record['callee_facts']=callee_facts
        conditional_calls={0x57e9e6:dict(ecx=0xffffffff,edx=0xffffffff),
            0x57ea23:dict(ecx=0xffffffff,edx=0xffffffff),0x57e9fd:dict(ecx=0xffffffff,edx=0xffffffff)}
        for address,expected in((0x57e9e6,'ff15dc036900'),(0x57ea23,'ff153c4e6c00'),(0x57e9fd,'e8756c0100')):
            if image.read(address,len(bytes.fromhex(expected))).hex()!=expected:raise ValueError('Conditional callee call bytes changed')
        record['conditional_callee_requirements']=dict(word_selector=0x6903dc,word_targets=[0x597086,0x596cb8],
            generic_selector=0x6c4e3c,generic_targets=[0x5947b2,0x59521a,0x595b47,0x59603e],
            direct_black_call=0x57e9fd,black_target=0x595677,
            scope='Only the separate summarized contexts may consume these facts. Match the actual version2 enclosing PC and paired entry-time raw selector observations; unknown targets remain unresolved. Preserve raw analyses unchanged.')
        rows = []
        record['source_commit']=subprocess.check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip()
        record['source_preimages']={}
        for name in SOURCES:
            raw=(ROOT/name).read_bytes()
            previous=subprocess.run(['git','show','HEAD:'+name],cwd=ROOT,capture_output=True)
            if previous.returncode or previous.stdout!=raw:
                preimage=target.with_name(target.stem+'-source-'+hashlib.sha256(name.encode()).hexdigest()[:16]+'.bin')
                write_report(ROOT,preimage,raw)
                record['source_preimages'][name]=str(preimage.relative_to(ROOT))
        for site in sites(source_text):
            if image.read(site['address'], 10) != site['signature']:
                raise ValueError('Production continuation differs from original bytes')
            contexts = [(0x5006b2,)] if site['table'] == 'sprite_raster_sites' else [()]
            context_kind=3 if site['table']=='sprite_raster_sites' else 0
            if site['table'] == 'trail_raster_sites':
                contexts = [(0x5006e0,), (0x500f51,)]
                context_kind=4
            if site['address']in(0x54ab16,0x54657c,0x54aab2,0x54ab72):
                contexts=[(0x5006b2,)];context_kind=1
            if site['address']in(0x57e8fb,0x57e915,0x57e940,0x57e957,0x57e985,0x57e9a8,0x57e9c7):
                contexts=[(p,)for p in opaque_returns];context_kind=2
            analyses = {reg: inspect(site['address'], decode, registers={reg: mask}, follow_calls=True)
                        for reg, mask in [('eax', 0xffffffff), ('ecx', 0xffffffff),
                                          ('edx', 0xffffffff), ('eflags', 0x8d5)]}
            contextual = [dict(returns=list(context),context_kind=context_kind,
                applicability='Conditional normal-return stack context only. A matching actual closed diagnostic context and current pinned stack derivation are required; unknown parents remain required.',
                analyses={reg: inspect(site['address'], decode,
                registers={reg: mask}, follow_calls=True, returns=context)
                for reg, mask in [('eax', 0xffffffff), ('ecx', 0xffffffff),
                                  ('edx', 0xffffffff), ('eflags', 0x8d5)]}) for context in contexts if context]
            summarized=[dict(returns=list(context),context_kind=2,analyses={reg:inspect(site['address'],decode,
                registers={reg:mask},follow_calls=True,returns=context,call_preserved_unused=conditional_calls)
                for reg,mask in [('eax',0xffffffff),('ecx',0xffffffff),('edx',0xffffffff),('eflags',0x8d5)]})
                for context in contexts if context_kind==2]
            rows.append(dict(table=site['table'], address=site['address'],
                             metadata=site['metadata'], signature=site['signature'].hex(),
                             analyses=analyses, contextual=contextual,callee_summary_contextual=summarized))
        record.update(sites=rows, decoder_version=decoder.version, success=True)
        record['counts'] = dict(sites=len(rows),
            **{reg + '_dead_before_read': sum(a['analyses'][reg]['closed'] and not a['analyses'][reg]['required'] for a in rows)
               for reg in ('eax', 'ecx', 'edx', 'eflags')})
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        record['error'] = str(error)
    finally:
        if decoder:
            decoder.close()
        result = subprocess.run([str(MAIN / 'tools/original-manifest.sh'), 'verify'])
        record['original_manifest_after'] = result.returncode == 0
        record['sources_stable'] = sources == {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in SOURCES}
        record['success'] = record['success'] and record['sources_stable'] and record['original_manifest_after']
        write_report(ROOT, target, (json.dumps(record, indent=2) + '\n').encode())
    print(json.dumps({k: record[k] for k in ('success', 'counts', 'error') if k in record}))
    return 0 if record['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
