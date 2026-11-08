#!/usr/bin/env python3
"""Compare two bounded original caller contracts; do not promote player rows."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BIDS = ('RS.screen-font-installation', 'RS.dc-image-wrapper-callers')
OUT = 'research/runtime/rendering-contract-recovery-results-20261108/'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name', required=True)
    a = parser.parse_args()
    if not a.name or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-' for c in a.name):
        parser.error('Use a fresh lowercase acquisition name')
    out = ROOT/'working/contract-recovery'/a.name
    record = OUT+a.name+'.json'
    if out.exists() or (ROOT/record).exists():
        parser.error('Immutable acquisition already exists')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Exact sources, original input identity, binaries, logs and case transcripts.\n')
    report = dict(schema=1, kind='isolated_render_contract_comparison', success=False,
                  original_game_runs=0, newly_passed_requirements=0,
                  player_acceptance='pending', original_recovery='pending',
                  original_sha256=PIN, commands=[], artifacts={})
    original = ROOT/'original/Arcane_Nocd/Chaos.exe'
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        builds = {b['id']: b for b in register['builds']}
        behaviors = [next(b for b in register['behaviors'] if b['id'] == bid) for bid in BIDS]
        names = sorted(set().union(*(required_sources(b) for b in behaviors)))
        sources = {n: digest(ROOT/n) for n in names}
        claims = [dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds),
                       scenarios={s['id']: scenario_contract(s) for s in register['scenarios']
                                  if b['id'] in s['behaviors']}) for b in behaviors]
        inventory_name = builds['nocd-40209ca7']['inventory']
        inventory = reader.json(inventory_name)
        entries = {0x5575b0, 0x58d1a0, 0x58d240, 0x58d280, 0x58d370}
        bodies = [f for f in inventory['functions'] if f['entry'] in entries]
        if {f['entry'] for f in bodies} != entries:
            raise ValueError('Missing original function boundaries')
        report.update(sources=sources, claims=claims,
                      inventory=dict(record=inventory_name, sha256=reader.digest(inventory_name)),
                      bodies=bodies, scope=[b['scope'] for b in behaviors])
    # Git revision plus unique source bytes, before compilation or execution.
    revision = subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
    unique = {}
    for name in names:
        data = (ROOT/name).read_bytes()
        old = subprocess.run(['git','show',revision+':'+name],cwd=ROOT,capture_output=True)
        if old.returncode or hashlib.sha256(old.stdout).hexdigest() != sources[name]:
            pointer = OUT+a.name+'-source-'+hashlib.sha256(name.encode()).hexdigest()[:16]+'.bin'
            h = write_report(ROOT, pointer, data)
            unique[name] = dict(record=pointer, sha256=h)
    declaration = dict(schema=1, kind='prospective_isolated_render_contract_inputs',
                       base_commit=revision, sources=sources, claims=claims, unique_sources=unique,
                       original_sha256=PIN, original_game_runs=0, newly_passed_requirements=0)
    declaration_path = OUT+a.name+'-inputs.json'
    report['prospective_inputs'] = dict(record=declaration_path,
        sha256=write_report(ROOT, declaration_path, (json.dumps(declaration,indent=2)+'\n').encode()))
    temporary = tempfile.TemporaryDirectory(prefix='mnm-render-contract-', dir='/tmp')
    env = dict(os.environ, TMPDIR=temporary.name, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(label, argv, timeout=180):
        log = out/(label+'.log')
        print(label, flush=True)
        with log.open('xb') as stream:
            result = subprocess.run([str(x) for x in argv], cwd=ROOT, env=env,
                                    stdout=stream, stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(label=label, argv=[str(x) for x in argv],
                                        returncode=result.returncode, log=log.relative_to(ROOT).as_posix()))
        result.check_returncode()
        return log.read_bytes()

    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh','verify'])
        if digest(original) != PIN:
            raise ValueError('Wrong original executable')
        report['compiler'] = run('compiler-version', ['g++','--version']).decode()
        report['protected_disassembly'] = run('original-functions', [
            'objdump','-d','-Mintel','--start-address=0x58d1a0','--stop-address=0x58d3ad',original]).decode()
        code = ROOT/'tests/render-contract-recovery.cpp'
        for mode, flags in (
            ('original', ['-m32','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE']),
            ('native', []),
            ('sanitized', ['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
        ):
            target = out/mode
            run(mode+'-compile', ['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',
                *flags,'-MMD','-MF',out/(mode+'.d'),code,
                *([ROOT/'tests/render-contract-call.S'] if mode=='original' else []),'-o',target])
            dependencies = (out/(mode+'.d')).read_text().replace('\\\n',' ').split(':',1)[1].split()
            closure = {Path(p).resolve().relative_to(ROOT).as_posix() for p in dependencies
                       if Path(p).resolve().is_relative_to(ROOT)}
            if not closure <= set(sources):
                raise ValueError('Undeclared compiler dependencies: '+repr(sorted(closure-set(sources))))
            run(mode+'-execute', [target, original])
        outputs = [(out/(mode+'-execute.log')).read_bytes() for mode in ('original','native','sanitized')]
        # Diagnostics are on the same output stream, including the independently
        # checked original ABI and the five byte-identical compared bodies.
        if not outputs[0] == outputs[1] == outputs[2]:
            raise ValueError('Original/native/sanitized case transcripts differ')
        rows = [line.split() for line in outputs[0].decode().splitlines() if line.startswith(('font ','dc '))]
        if len(rows) != 1536 or sum(r[0]=='font' for r in rows) != 512 or any(len(r)!=7 for r in rows):
            raise ValueError('Incomplete bounded comparison matrix')
        report.update(success=True, cases=1536, font_cases=512, dc_cases=1024,
                      original_native_sanitized_match=True, compared_bodies_unchanged=True,
                      original_outputs_used_as_native_inputs=False,
                      original_callee_registers_stack_seh_checked=True,
                      dc_eax_ecx_defined_flags_and_argument_slot_checked=True,
                      supplier_pixels_gdi_or_win32_equivalence=False,
                      installed_font_transition_observed=False)
    except Exception as exc:
        report.update(success=False,error=str(exc))
    finally:
        try:
            run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
            if digest(original) != PIN:
                raise ValueError('Original executable changed')
            report['original_manifest_verified_before_after'] = any(
                c['label']=='manifest-before' and c['returncode']==0 for c in report['commands'])
        except Exception as exc:
            report.update(success=False,manifest_error=str(exc))
        report['changed_sources'] = [n for n,h in sources.items() if digest(ROOT/n)!=h]
        report['sources_stable'] = not report['changed_sources']
        report['success'] = report['success'] and report['sources_stable']
        for path in sorted(out.iterdir()):
            if path.is_file() and path.name != '.retain':
                name = OUT+a.name+'-artifact-'+path.name+'.bin'
                report['artifacts'][path.name] = dict(record=name, sha256=write_report(ROOT,name,path.read_bytes()))
        h = write_report(ROOT,record,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'], error=report.get('error'), record=record, sha256=h)),flush=True)
        temporary.cleanup()
    return int(not report['success'])


if __name__ == '__main__':
    sys.exit(main())
