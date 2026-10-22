#!/usr/bin/env python3
"""Compare an owned actor concrete provider frontier against independently seeded original PE32 execution."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import subprocess

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import read_json, evidence_digest, write_report
from original_x86 import PE32, Decoder

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BID = 'RS.actor-concrete-provider-frontier'
SCENARIO = 'actor-concrete-provider-callers-20261022'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--variants', type=int, default=64, choices=range(1, 65))
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require a new retained output below working/tests')
    record = ROOT/'research/runtime/actor-concrete-provider-results-20261022'/(out.name+'.json')
    input_record = record.with_name(out.name+'-inputs.json')
    if record.exists() or input_record.exists(): parser.error('Require a new immutable report identity')
    (out/'tmp').mkdir(parents=True)
    (out/'.retain').write_text('Actor source session raw executions, inputs, binaries and failures.\n')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == BID)
    scenario = next(s for s in register['scenarios'] if s['id'] == SCENARIO)
    builds = {b['id']: b for b in register['builds']}
    inputs = required_sources(behavior) | {'tools/test-actor-services.py'}
    sources = {p: sha(ROOT/p) for p in sorted(inputs)}
    report = dict(schema=1, success=False, sources=sources,
                  claims=[dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds),
                               scenarios={SCENARIO: scenario_contract(scenario)}) for b in [behavior]],
                  scope=behavior['scope'], original_sha256=PIN, variants=args.variants,
                  original_output_used_as_native_input=False, live_replacement=False,
                  physical_driver_equivalence=False, commands=[],
                  representation_policy={'temporaries':'Stack/scratch addresses normalized; initialized payload bytes retained. Void EAX excluded. All owned allocation bytes compared before and after explicit caller teardown.'})
    base = subprocess.check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip()
    changed_inputs = {}
    for path,digest in sources.items():
        data=(ROOT/path).read_bytes()
        old=subprocess.run(['git','-C',str(ROOT),'show',base+':'+path],capture_output=True)
        if old.returncode or hashlib.sha256(old.stdout).hexdigest()!=digest:
            changed_inputs[path]=dict(sha256=digest,text=data.decode('utf-8'))
    declaration=dict(schema=1,scope='Prospective declaration and exact unique source changes against the named Git base; no execution result.',
                     base_commit=base,sources=sources,claims=report['claims'],changed_inputs=changed_inputs)
    declared_sha=write_report(ROOT,input_record,(json.dumps(declaration,indent=2)+'\n').encode())
    report['prospective_inputs']=dict(record=input_record.relative_to(ROOT).as_posix(),sha256=declared_sha)
    env = dict(os.environ, TMPDIR=str(out/'tmp'), MNM_FRONTIER_VARIANTS=str(args.variants),
               ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    verified = False

    def run(label, command, stream=False):
        command = [str(v) for v in command]
        report['commands'].append(dict(label=label, argv=command))
        print(label, flush=True)
        with (out/(label+'.log')).open('x') as log:
            if not stream:
                subprocess.run(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT,
                               check=True, timeout=300)
                return
            with (out/(label+'.bin.zst')).open('xb') as target:
                encoder = subprocess.Popen(['zstd','-q','-3','-c'], stdin=subprocess.PIPE, stdout=target, stderr=log)
                child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=encoder.stdin, stderr=log)
                encoder.stdin.close()
                try:
                    if child.wait(timeout=600) or encoder.wait(timeout=30):
                        raise ValueError(label+' failed; inspect retained log')
                finally:
                    for process in [child, encoder]:
                        if process.poll() is None: process.kill()
                        process.wait()
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify']); verified = True
        image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if image.sha256 != PIN: raise ValueError('Original build differs')
        inventory_path = ROOT/builds['nocd-40209ca7']['inventory']
        inventory = read_json(inventory_path, root=ROOT)
        functions = {f['entry']: f for f in inventory['functions']}
        report['inputs'] = {str(inventory_path.relative_to(ROOT)): evidence_digest(inventory_path, root=ROOT)}
        module_spec = importlib.util.spec_from_file_location('actor_session_prior_runner', ROOT/'tools/test-actor-services.py')
        prior = importlib.util.module_from_spec(module_spec); module_spec.loader.exec_module(prior)
        headers = dict(prior.HEADERS)
        headers['source'] = 'reconstruction/rendering/actor_source_services.hpp'
        headers['session'] = 'reconstruction/rendering/actor_session.hpp'
        headers['transitions'] = 'reconstruction/rendering/actor_transition_services.hpp'
        headers['frontier'] = 'reconstruction/rendering/actor_frontier_providers.hpp'
        roots = {key: sorted({int(v,16) for v in re.findall(r'case (0x[0-9a-f]+):', (ROOT/path).read_text())})
                 for key,path in headers.items()}
        composed = set().union(*roots.values())
        endpoints = [spec for spec in prior.PROVIDERS if spec[0] not in composed]
        # Original transition control is composed; concrete task bodies remain
        # named downstream boundaries rather than silently receiving recovery.
        callbacks = [0x50acb0,0x510010,0x50afa0,0x50ccd0,0x50d580,0x50d840,0x50d860,
                     0x50d850,0x50d870,0x50d8f0,0x51c510,0x51c230,0x521750,0x521770,0x521780]
        endpoints += [(entry,0,0,True,False) for entry in callbacks]
        endpoints += [(0x4a1120,0,0,True,True),(0x59c359,4,0,False,False),
            (0x5b74d8,1,0,False,False),(0x59d9eb,1,0,False,False),(0x59cf25,3,0,False,False),
            (0x5c5208,1,4,False,False),(0x5c51fc,2,8,False,False),
            (0x504cc0,0,0,True,True),(0x504ec0,0,0,True,True),(0x505160,0,0,True,False),
            (0x4f3320,4,16,True,False),(0x4ed440,1,4,True,False),(0x522720,1,4,True,False),
            (0x48a270,0,0,True,False),(0x546c20,3,12,True,False),(0x513ff0,0,0,True,False),
            (0x5120c0,0,0,True,False),(0x4970b0,1,4,True,False),(0x57d560,0,0,True,False),
            (0x484c00,1,4,True,False),(0x4a2f90,0,0,False,False),
            (0x1001f500,2,8,False,False),(0x1001f540,6,24,False,False),
            (0x1001f580,6,24,False,False),(0x1001f5c0,1,4,False,False),
            (0x1001f600,3,12,False,False),(0x1001f640,1,4,True,False)]
        endpoints += [(e,1,4,True,True) for e in [0x582440,0x582a80,0x583330,0x583bb0,0x586660,0x5870d0,0x585d80,0x587e70,0x588730,0x588bf0]]
        if len({p[0] for p in endpoints})!=len(endpoints): raise ValueError('Duplicate supplied endpoint')
        report['concrete_task_handlers_supplied'] = callbacks
        report['supplied_providers'] = [dict(address=e, stack_arguments=n, callee_pop_bytes=p, ecx_receiver=c, edx_argument=d)
                                        for e,n,p,c,d in endpoints]
        decoder = Decoder(); abi = {}
        def decode(entry):
            result = []
            for first,end in functions[entry]['ranges']:
                while first < end:
                    instruction = decoder.decode(image.read(first, min(15,end-first)), first)
                    result.append(instruction); first += instruction['size']
            return result
        def pop(entry, visiting=frozenset()):
            if entry in abi: return abi[entry]
            if entry in visiting: raise ValueError('Cyclic tail ABI')
            code = decode(entry)
            values = {int(i['operand'],0) if i['operand'] else 0 for i in code if i['mnemonic']=='ret'}
            if not values:
                for i in code:
                    if i['mnemonic']=='jmp' and i['operand'].startswith('0x'):
                        target=int(i['operand'],16)
                        if not any(lo<=target<hi for lo,hi in functions[entry]['ranges']): values.add(pop(target, visiting|{entry}))
            if len(values)!=1: raise ValueError('Ambiguous original ABI '+hex(entry)+' '+str(values))
            abi[entry] = values.pop(); return abi[entry]
        try:
            for entry in sorted(composed): pop(entry)
        finally: decoder.close()
        generated = ['#pragma once', '#include <cstdint>', 'struct ServiceAbi{unsigned id,pop;};']
        generated += ['static constexpr unsigned ServiceRoots[]={0x506290};',
                      'static constexpr ServiceAbi ServiceAbis[]={{0x506290,12}};',
                      'static constexpr ServiceAbi SessionAbis[]={'+','.join('{'+hex(e)+','+str(p)+'}' for e,p in sorted(abi.items()))+'};']
        names = dict(values='Value',controllers='Controller',world='World',ui='Ui',dependencies='Dependency',popups='Popup',collections='Collection',effects='Effect',construction='Construction',providers='Provider',composition='Composition')
        for key,name in names.items(): generated.append('static constexpr unsigned '+name+'Roots[]={'+','.join(hex(v) for v in roots[key])+'};')
        for name in ['Composition','Providers','WorldServices','UiServices','Dependencies','Popups','Collections','Effects','Construction']:
            generated.append('static constexpr bool Use'+name+'='+('true' if name=='Composition' else 'false')+';')
        generated += ['struct ServiceProvider{unsigned id,arguments,pop;bool receiver,second;unsigned char bytes[8];};',
                      'static constexpr ServiceProvider ServiceProviders[]={']
        for entry,count,cleanup,receiver,second in endpoints:
            data = image.read(entry,8) if entry < 0x600000 else bytes(8)
            generated.append('{'+','.join([hex(entry),str(count),str(cleanup),str(receiver).lower(),str(second).lower(),'{'+','.join(hex(v) for v in data)+'}'])+'},')
        generated.append('};')
        protected = composed | {0x5065e0,0x50da50,0x594490,0x40e290,0x40e4e0,0x40e650,0x40e8a0,0x40eb70,0x4132e0,0x4eac10,0x4eaca0,0x4eade0,0x59bee0,0x40e2c0,0x40e8d0,0x40eec0,0x509790,
            0x42d830,0x430140,0x42f220,0x42a590,0x429aa0,0x429a40,0x431840,0x430220,0x431940,0x4319c0,0x42af40,0x42f480,0x42f290,0x4297b0,0x42fa50,0x42f9d0,0x431b30,0x430e80,0x42f950,0x52cbe0,0x58f6f0,0x40a6b0,0x40eeb0,0x50a930}
        protected -= {p[0] for p in endpoints}
        ranges = []
        for entry in sorted(protected):
            for first,end in functions[entry]['ranges']:
                data = image.read(first,end-first); index=len(ranges)
                generated.append('static constexpr unsigned char Body'+str(index)+'[]={'+','.join(hex(v) for v in data)+'};')
                ranges.append((first,len(data),index))
        generated += ['struct ProtectedBody{unsigned address,length;const unsigned char* expected;};',
                      'static constexpr ProtectedBody ProtectedBodies[]={'+','.join('{'+hex(a)+','+str(n)+',Body'+str(i)+'}' for a,n,i in ranges)+'};']
        header = out/'actor-services-generated.hpp'; header.write_text('\n'.join(generated)+'\n')
        report['generated_inputs'] = {header.name: sha(header)}
        report['protected_original_entries'] = sorted(protected)
        modes=[('original',['-m32','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE']),
               ('native',[]),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]
        for name,flags in modes:
            run(name+'-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,'-I',out,'-MD','-MF',out/(name+'.d'),ROOT/'tests/actor-frontier-reference.cpp',*([ROOT/'tests/actor-services-call.S'] if name=='original' else []),'-o',out/name])
            for token in shlex.split((out/(name+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
                path = Path(token).resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(out):
                    relative=path.relative_to(ROOT).as_posix()
                    if relative not in sources or sha(path)!=sources[relative]: raise ValueError('Undeclared compiler input '+relative)
            command = ['bwrap','--die-with-parent','--unshare-net','--ro-bind','/','/','--bind',out,out,'--dev','/dev','--proc','/proc','--',out/name]
            if name=='original': command.append(ROOT/'original/Arcane_Nocd/Chaos.exe')
            run(name+'-execute',command,True)
        children=[subprocess.Popen(['zstd','-qdc',out/(name+'-execute.bin.zst')],stdout=subprocess.PIPE) for name in ['native','original','sanitized']]
        count=0; digest=hashlib.sha256()
        try:
            while True:
                left=children[0].stdout.read(65536)
                for mode,child in zip(['original','sanitized'],children[1:]):
                    right=child.stdout.read(65536)
                    if left!=right:
                        at=next((i for i,(l,r) in enumerate(zip(left,right)) if l!=r),min(len(left),len(right)))
                        report['mismatch']=dict(mode=mode,byte=count+at,native=left[max(0,at-16):at+32].hex(),other=right[max(0,at-16):at+32].hex())
                        raise ValueError('Session state/callback streams differ')
                if not left: break
                digest.update(left);count+=len(left)
            if any(p.wait(timeout=10) for p in children): raise ValueError('Decompression failed')
        finally:
            for p in children:
                if p.poll() is None: p.kill()
                p.wait();p.stdout.close()
        for name,_ in modes:
            text=(out/(name+'-execute.log')).read_text()
            if 'ERROR: AddressSanitizer' in text or 'runtime error:' in text: raise ValueError('Sanitizer execution failed')
            completed=re.findall(r'^frontier_returned ([0-9a-f]+) (\d+) live 0$',text,re.M)
            expected={(format(root,'x'),str(seed)) for root in roots['frontier']+[0x414890,0x4637c0,0x5055a0] for seed in range(args.variants)}
            if len(completed)!=len(expected) or set(completed)!=expected:
                raise ValueError('Incomplete normal frontier matrix '+name)
        report.update(success=True,cases=15*args.variants,checkpoints=30*args.variants,
                      bytes_compared=count,comparison_sha256=digest.hexdigest(),
                      composed_routes=len(composed), roots=roots['frontier'],caller_roots=[0x414890,0x4637c0,0x5055a0],
                      selected_original_bodies_unchanged=True,sanitizer_passed=True,
                      binaries_sha256={name:sha(out/name) for name,_ in modes})
    except (OSError,ValueError,KeyError,AssertionError,subprocess.SubprocessError) as error:
        report['success']=False
        report['error']=type(error).__name__+': '+str(error)
    finally:
        if verified:
            run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/p)==value for p,value in sources.items())
        report['success'] &= report['sources_stable']
        write_report(ROOT,record,(json.dumps(report,indent=2)+'\n').encode())
    print(record,flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
