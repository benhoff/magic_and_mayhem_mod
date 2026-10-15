#!/usr/bin/env python3
"""Persistent pinned original32 versus native32/native64 encounter and replay."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import struct
import sys
from importlib.machinery import SourceFileLoader
ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--install', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    parser.add_argument('--build-root', type=Path, help='Reuse CMake build directories; configure/build and source checks still run')
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    pe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
    if sha(pe) != PIN:
        raise ValueError('Pinned original SHA-256 differs')
    claim = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    if any(sha(ROOT / k) != v for k, v in claim['sources'].items()):
        raise ValueError('Declared source changed')
    report = dict(success=False, source_executable_sha256=PIN, source_sha256=claim['sources'], claims=claim['claims'], commands=[], wine_required=False, live_replacement=False,
        scope='Independent persistent original PE32 and native encounter: two ordinary type17 actors on sealed12x12 installed Celtic Plains crop; empty World owner, inventory, presentation and effects, fixed retained identities. Original checkpoints replay the prefix; native snapshots restore directly. Full original World/campaign/live replacement excluded.')
    def run(name, command, expected=0, env=None):
        command = [str(x) for x in command]
        report['commands'].append(command)
        p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=900, env=env)
        (out / (name+'.stdout')).write_text(p.stdout)
        (out / (name+'.stderr')).write_text(p.stderr)
        if p.returncode != expected or 'ERROR: AddressSanitizer' in p.stderr or 'runtime error:' in p.stderr:
            raise RuntimeError(name+' failed: '+str(p.returncode)+' '+p.stdout[-1000:]+p.stderr[-2500:])
        return p.stdout
    deps = SourceFileLoader('installed_encounter_dependencies', str(ROOT/'tools/test-installed-encounter.py')).load_module()
    sources = None
    builds = {}
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        image = pe.read_bytes()
        pe_at = struct.unpack_from('<I', image, 0x3c)[0]
        section_at = pe_at + 24 + struct.unpack_from('<H', image, pe_at+20)[0]
        sections = [struct.unpack_from('<8sIIIIIIHHI', image, section_at+40*i) for i in range(struct.unpack_from('<H', image, pe_at+6)[0])]
        report['original_executable_sections_sha256'] = {row[0].rstrip(b'\0').decode(): hashlib.sha256(image[row[4]:row[4]+row[3]]).hexdigest() for row in sections if row[9]&0x20000000}
        def original_bytes(at, size):
            for row in sections:
                offset = at-0x400000-row[2]
                if 0 <= offset and offset+size <= row[3]:
                    return image[row[4]+offset:row[4]+offset+size]
            raise ValueError('Original address outside file-backed sections')
        addresses = [0x516800,0x517580,0x516ee0,0x513370,0x4df6e0,0x4df260,0x4df500,0x509230,0x5093e0,0x509e50,0x52a780,0x52a9f0,0x50acb0,0x51f5e0,0x508b00,0x512800,0x4bf070,0x4bef60]
        report['entry_anchors'] = {hex(at):original_bytes(at,16).hex() for at in addresses}
        inputs = ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani', 'Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd']
        report['installed_inputs'] = {p: sha(args.install/p) for p in inputs}
        for tag, options in [
            ('original32', ['-DCMAKE_CXX_FLAGS=-m32 -O1 -g', '-DCMAKE_EXE_LINKER_FLAGS=-m32 -no-pie', '-DMNM_ENCOUNTER_ORIGINAL_REFERENCE=ON']),
            ('native64', ['-DCMAKE_BUILD_TYPE=', '-DCMAKE_CXX_FLAGS=-O1 -g']),
            ('sanitized', ['-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = (args.build_root.resolve() if args.build_root else out)/tag
            builds[tag] = build
            run(tag+'-configure', ['cmake','-S',ROOT/'apps/native-encounter','-B',build,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',*options])
            closure = deps.dependencies(build,run,tag,extra={'paired-encounter-reference.cpp','paired-encounter-helper-test.cpp','paired-encounter-native.cpp'})
            closure.add('tools/test-paired-encounter.py')
            if sources is None:
                sources = {k: sha(ROOT/k) for k in sorted(closure)}
            else:
                sources.update({k: sha(ROOT/k) for k in sorted(closure-sources.keys())})
            if args.claims and closure-claim['sources'].keys():
                raise ValueError('Unbound compiler/configure inputs: '+str(sorted(closure-claim['sources'].keys())))
            targets = ['paired-encounter-helper-test', 'paired-encounter-reference'] if tag=='original32' else ['paired-encounter-helper-test', 'paired-encounter-native','native-creature-providers-test','native-encounter-test']
            run(tag+'-build',['cmake','--build',build,'--target',*targets,'-j2'])
        report['build_inputs_sha256'] = sources
        env = {**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'}
        helpers = []
        for tag, build in builds.items():
            helpers.append(json.loads(run(tag+'-helpers', [build/'paired-encounter-helper-test',*([pe] if tag=='original32' else []),args.install],env=env)))
        if not all(x==helpers[0] for x in helpers):
            raise ValueError('Helper architecture/sanitizer mismatch')
        report['helper_result'] = helpers[0]
        reference = builds['original32']/'paired-encounter-reference'
        results = []
        for seed in [17,42,91]:
            directory = out/('seed-'+str(seed))
            results.append(json.loads(run('seed-'+str(seed),[reference,pe,'run',args.install,seed,1024,directory,0])))
            for tag in ['native64','sanitized']:
                target = out/(tag+'-'+str(seed)+'.snapshot');trace = out/(tag+'-'+str(seed)+'.trace')
                run(tag+'-'+str(seed),[builds[tag]/'paired-encounter-native',directory/'initial.snapshot',1024,target,trace],env=env)
                if target.read_bytes()!=(directory/'final.snapshot').read_bytes() or trace.read_bytes()!=(directory/'trace.txt').read_bytes():
                    raise ValueError('Native64/sanitized persistent encounter differs')
        report['encounters'] = results
        if not all(sum(r[k] for r in results)>0 for k in ['hits','stuns','knockbacks','deaths','route_completions','stun_recoveries','removals']):
            raise ValueError('Encounter branch coverage absent')
        # Restart both sides at bounded checkpoints. The original rebuilds its
        # own prefix while native32 restores its actual portable continuation.
        previous = out/'seed-17/initial.snapshot';oracle = out/'seed-17/initial.oracle';last = 0
        for end in [1,24,128,512,768,1024]:
            directory = out/('resume-'+str(end))
            run('resume-'+str(end),[reference,pe,'resume',previous,oracle,end-last,directory,0],env={**env,'PATH':'/nonexistent'})
            previous = directory/'final.snapshot';oracle = directory/'final.oracle';last=end
        if previous.read_bytes()!=(out/'seed-17/final.snapshot').read_bytes() or oracle.read_bytes()!=(out/'seed-17/final.oracle').read_bytes():
            raise ValueError('Checkpoint replay/restore differs')
        # An intentional native-only fault must be detected and reproduce at
        # exactly the same field/tick from the saved state before the divergence.
        fault = out/'injected-fault'
        run('injected-fault',[reference,pe,'run',args.install,17,160,fault,137],expected=3)
        replay = out/'reproduced-fault'
        run('reproduced-fault',[reference,pe,'resume',fault/'before.snapshot',fault/'before.oracle',1,replay,137],expected=3)
        for name in ['divergence.txt','after.snapshot','after.oracle']:
            if (fault/name).read_bytes()!=(replay/name).read_bytes():
                raise ValueError('First divergence does not reproduce')
        damaged = out/'damaged.oracle';b=bytearray((fault/'before.oracle').read_bytes());b[-1]^=1;damaged.write_bytes(b)
        run('corrupt-oracle',[reference,pe,'resume',fault/'before.snapshot',damaged,1,out/'rejected-corrupt',0],expected=1)
        if (out/'rejected-corrupt').exists():
            raise ValueError('Corrupt oracle was published')
        for tag in ['native64','sanitized']:
            build = builds[tag]
            run(tag+'-providers',[build/'command-session/native-creature-providers-test'],env=env)
            run(tag+'-installed',[build/'native-encounter-test',args.install],env=env)
        report.update(success=True, native_passed=True, sanitizer_passed=True, original_target_text_unchanged=True, process_resumes=6, process_resume_identical=True, first_divergence_reproduced=True, corrupt_oracle_rejected=True, compared_ticks=3072)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_unchanged']=all(sha(ROOT/k)==v for k,v in claim['sources'].items())
        report['build_inputs_unchanged']=all(sha(ROOT/k)==v for k,v in (sources or {}).items())
        report['installed_inputs_unchanged']=all(sha(args.install/k)==v for k,v in report.get('installed_inputs',{}).items())
        report['artifacts_sha256']={p.relative_to(out).as_posix():sha(p) for p in sorted(out.rglob('*')) if p.is_file() and not any(part in builds for part in p.relative_to(out).parts)}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    if not all(report[k] for k in ['sources_unchanged','build_inputs_unchanged','installed_inputs_unchanged']):
        raise ValueError('Inputs changed during execution')
    print(json.dumps({k:report[k] for k in ['success','compared_ticks','process_resumes','first_divergence_reproduced','sanitizer_passed']}))
if __name__=='__main__':
    main()
