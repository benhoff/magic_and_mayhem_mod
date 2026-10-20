#!/usr/bin/env python3
"""Compare owned queue routing and kind8 methods with pinned PE32 execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = ['reconstruction/rendering/world_dispatch.hpp', 'tests/world-dispatch-test.cpp',
           'reconstruction/rendering/kind8_method.hpp', 'tests/kind8-method-test.cpp',
           'reconstruction/rendering/world_queue.hpp', 'tests/world-queue-state-test.cpp',
           'tests/drawing-queue-state-reference.cpp', 'tools/coverage_claims.py',
           'tools/test-world-dispatch.py', 'tests/drawing-dispatch-reference.cpp',
           'tests/sprite-binary-reference.cpp']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def normalize(call):
    tag, args = call['tag'], call['args']
    if tag == 0x57dc60:
        return None  # Original clip setup is explicitly outside this row dispatcher.
    stack = {0x595677: ('copy', 3), 0x6c4e3c: ('copy', 5),
             0x689b7c: ('terrain', 5), 0x57de00: ('clippedCopy', 5),
             0x6903dc: ('wordCopy', 3), 0x689b78: ('displacement', 3)}
    if tag in stack:
        name, count = stack[tag]
        return dict(backend=name, values=[42, *args[1:count]])
    if tag == 0x8000000c:
        if not call['ecx_object']:
            raise ValueError('Virtual call receiver is not the fixture object')
        return dict(backend='virtualObject', values=[42, *args[:2]])
    if tag == 0x4df840:
        # Receiver is the original object's first word (a launch-local vtable).
        # The old fixture logs only its object equality, so its numeric value is
        # deliberately excluded. All six other logical inputs are compared.
        return dict(backend='objectHelper', values=[77, call['edx'], *args[:5]])
    fast = {0x57e540: ('shadow', 1), 0x57e8e0: ('opaque', 3),
            0x57f5f0: ('quarter', 3), 0x57ec90: ('half', 3),
            0x57f0f0: ('threeQuarter', 3), 0x57fe10: ('alternateShadow', 3),
            0x580a90: ('distortion', 3), 0x580260: ('alternateBlend', 3),
            0x581200: ('tint', 5), 0x5806f0: ('wave', 2)}
    if tag not in fast or not call['ecx_object']:
        raise ValueError('Unknown call or wrong original source receiver: ' + hex(tag))
    name, count = fast[tag]
    return dict(backend=name, values=[42, call['edx'], *args[:count]])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--executable', type=Path, default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    parser.add_argument('--wine-prefix', type=Path, help='Reuse an isolated synthetic-test prefix under working/tests')
    args = parser.parse_args()
    out = args.output.resolve()
    if not out.is_relative_to(ROOT/'working') or out.exists():
        parser.error('Require a new output directory under working/')
    prefix = args.wine_prefix.resolve() if args.wine_prefix else out/'wine'
    if not prefix.is_relative_to(ROOT/'working/tests'):
        parser.error('Synthetic hook prefix must be isolated under working/tests')
    out.mkdir(parents=True)
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors = [b for b in register['behaviors'] if b['id'] in
                 ('RS.world-dispatch-routing', 'RS.world-kind8-methods', 'RS.world-queue-state',
                  'RS.world-dispatch-targets', 'RS.world-kind8-virtual','RS.world-kind8-empty-methods')]
    if len(behaviors) != 6:
        raise ValueError('Register routing and method contracts before execution')
    builds = {b['id']: b for b in register['builds']}
    paths = set(SOURCES) | set().union(*(required_sources(b) for b in behaviors))
    sources = {p: sha(ROOT/p) for p in sorted(paths)}
    scenario=next(s for s in register['scenarios'] if s['id']=='native-kind8-empty-methods')
    claims = [dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds), **({'scenarios':{scenario['id']:scenario_contract(scenario)}} if b['id']=='RS.world-kind8-empty-methods' else {})) for b in behaviors]
    report = dict(schema=1, success=False, claims=claims, sources=sources,
                  source_executable_sha256=BUILD, scope=__doc__,
                  live_replacement=False, raster_pixels_compared=False,
                  helper_receiver_identity_compared=False, clip_setup_compared=True,
                  clip_setup_domain="Bounded queue-state sample, fixed clip/dimensions; no live graphics helper effects")
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')

    def run(label, command, output=None, env=None):
        with (out/(label+'.log')).open('x') as log:
            if output:
                with (out/output).open('x') as result:
                    subprocess.run([str(v) for v in command], cwd=ROOT, check=True,
                                   stdout=result, stderr=log, timeout=180, env=env)
            else:
                subprocess.run([str(v) for v in command], cwd=ROOT, check=True,
                               stdout=log, stderr=subprocess.STDOUT, timeout=180, env=env)

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        if sha(args.executable) != BUILD:
            raise ValueError('Unsupported original executable')
        raw = args.executable.read_bytes()
        pe = struct.unpack_from('<I', raw, 60)[0]
        sections, optional = struct.unpack_from('<H', raw, pe+6)[0], struct.unpack_from('<H', raw, pe+20)[0]
        address, expected = 0x54aaa0, '8b4424088b542404505283c134e85eb1ffffc20800'
        for index in range(sections):
            _, va, size, offset = struct.unpack_from('<4I', raw, pe+24+optional+index*40+8)
            delta = address-0x400000-va
            if 0 <= delta and delta+len(expected)//2 <= size:
                actual = raw[offset+delta:offset+delta+len(expected)//2].hex()
                if actual != expected:
                    raise ValueError('Inherited additive wrapper bytes changed')
                report['anchors'] = {hex(address): actual}
                break
        else:
            raise ValueError('Inherited additive wrapper not file backed')
        run('original-build', ['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie',
            '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
            ROOT/'tests/drawing-dispatch-reference.cpp', '-o', out/'original'])
        run('native-build', ['g++', '-O2', '-std=c++17', '-Wall', '-Wextra', '-Werror',
            '-pedantic', ROOT/'tests/world-dispatch-test.cpp', '-o', out/'native'])
        run('original', [out/'original', args.executable], 'original.jsonl')
        run('native', [out/'native'], 'native.jsonl')
        original = [json.loads(line) for line in (out/'original.jsonl').read_text().splitlines()]
        native = [json.loads(line) for line in (out/'native.jsonl').read_text().splitlines()]
        if len(original) != 2432 or len(native) != len(original):
            raise ValueError('Incomplete comparison matrix')
        calls = 0
        for actual, expected in zip(native, original):
            expected['calls'] = [row for c in expected['calls'] if (row := normalize(c)) is not None]
            if actual != expected:
                (out/'mismatch.json').write_text(json.dumps(dict(native=actual, original=expected), indent=2)+'\n')
                raise ValueError('Native routing differs from original; see mismatch.json')
            calls += len(actual['calls'])
        run('state-original-build', ['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie',
            '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
            ROOT/'tests/drawing-queue-state-reference.cpp', '-o', out/'state-original'])
        run('state-native-build', ['g++', '-O2', '-std=c++17', '-Wall', '-Wextra', '-Werror',
            ROOT/'tests/world-queue-state-test.cpp', '-o', out/'state-native'])
        run('state-original', [out/'state-original', args.executable, '--sample'], 'state-original.jsonl')
        run('state-native', [out/'state-native'], 'state-native.jsonl')
        from itertools import zip_longest
        state_cases = state_calls = 0
        with (out/'state-original.jsonl').open() as original_state, (out/'state-native.jsonl').open() as native_state:
            for a, b in zip_longest(original_state, native_state):
                if a is None or b is None:
                    raise ValueError('Incomplete queue-state comparison')
                expected, actual = json.loads(a), json.loads(b)
                expected.pop('opening_clips')
                calls_for_state = []
                for call in expected['calls']:
                    if call['tag'] == 0x57dc60:
                        calls_for_state.append(dict(backend='clip', values=[call['ecx'], call['edx'], *call['args'][:2]]))
                    elif call['tag'] == 0x4d8ab0:
                        if call['ecx'] != 0x6ddf78:
                            raise ValueError('Wrong original postlude receiver')
                        calls_for_state.append(dict(backend='postlude', values=call['args'][:3]))
                    else:
                        call['ecx_object'] = call['ecx'] == 0
                        calls_for_state.append(normalize(call))
                expected['calls'] = calls_for_state
                if actual != expected:
                    (out/'state-mismatch.json').write_text(json.dumps(dict(native=actual, original=expected), indent=2)+'\n')
                    raise ValueError('Native queue state differs from original')
                state_cases += 1
                state_calls += len(calls_for_state)
        if state_cases != 50772:
            raise ValueError('Wrong queue-state sample count')
        for address in (0x54ac30,0x54ac40):
            for index in range(sections):
                _,va,size,offset=struct.unpack_from('<4I',raw,pe+24+optional+index*40+8)
                delta=address-0x400000-va
                if 0<=delta and delta+6<=size:
                    actual=raw[offset+delta:offset+delta+6].hex()
                    if actual!='6633c0c20800':raise ValueError('Empty kind8 method bytes changed')
                    report['anchors'][hex(address)]=actual;break
            else:raise ValueError('Empty kind8 method not file-backed')
        for name, flags in [('original', ['-m32', '-DMNM_ORIGINAL_REFERENCE', '-fno-pie', '-no-pie']),
                            ('native', [])]:
            run('kind8-'+name+'-build', ['g++', *flags, '-O2', '-std=c++17',
                '-Wall', '-Wextra', '-Werror', ROOT/'tests/kind8-method-test.cpp',
                '-o', out/('kind8-'+name)])
            run('kind8-'+name, [out/('kind8-'+name), *([args.executable] if flags else [])],
                'kind8-'+name+'.jsonl')
        methods = (out/'kind8-native.jsonl').read_bytes()
        if len(methods.splitlines()) != 1440 or methods != (out/'kind8-original.jsonl').read_bytes():
            raise ValueError('Native kind8 method/flags/owner gates differ from original')
        empty_rows=[json.loads(line) for line in methods.splitlines() if json.loads(line)['vtable'] in (0x5c7438,0x5c7450,0x5c7470)]
        if len(empty_rows)!=480:raise ValueError('Incomplete empty-method matrix')
        for row in empty_rows:
            expected=[i*0x01010101+0x123 for i in range(26)];expected[0]=row['vtable'];expected[24]=777 if row['owner'] else 0
            for at in (16,17):expected[at]=(expected[at]&0xffffff00)|row['seed']
            if row['result'] or row['calls'] or row['object']!=expected:raise ValueError('Empty method changed object/raster/return')
        flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding',
                 '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx',
                 '-Wall', '-Wextra', '-Werror', '-DMNM_SCENE_STARTUP_REPLAY_SELFTEST']
        objects = []
        for name in ('tests/kind8-hook-test.c', 'tests/canvas-startup-call.S',
                     'runtime/scene/world_trace.c', 'runtime/scene/world_entry.S'):
            obj = out/(Path(name).name+'.obj')
            run('hook-'+Path(name).name, [*flags, '-c', ROOT/name, '-o', obj])
            objects.append(obj)
        imports = set()
        for obj in objects:
            imports.update(re.findall(r'__imp__([A-Za-z0-9_]+@\d+)',
                subprocess.check_output(['llvm-nm', '--undefined-only', str(obj)], text=True)))
        definition = out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+'\n'.join(sorted(imports))+'\n')
        run('hook-imports', ['llvm-dlltool', '-m', 'i386', '-D', 'KERNEL32.dll', '-d',
            definition, '-l', out/'kernel32.lib', '--kill-at'])
        run('hook-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
            '/nodefaultlib', '/base:0x400000', '/dynamicbase:no', '/safeseh:no', '/timestamp:0',
            '/out:'+str(out/'hook.exe'), *objects, out/'kernel32.lib'])
        env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all')
        report['wine_prefix'] = str(prefix)
        run('hook-execute', ['wine', out/'hook.exe'], env=env)
        run('cmake-configure', ['cmake', '-S', ROOT/'reconstruction/rendering/world-dispatch',
            '-B', out/'cmake', '-DCMAKE_BUILD_TYPE=Release'])
        run('cmake-build', ['cmake', '--build', out/'cmake', '--parallel', '2'])
        run('cmake-tests', ['ctest', '--test-dir', out/'cmake', '--output-on-failure'])
        if sha(args.executable) != BUILD or any(sha(ROOT/p) != h for p, h in sources.items()):
            raise ValueError('Sources or executable changed during comparison')
        current = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        current_builds = {b['id']: b for b in current['builds']}
        current_behaviors = {b['id']: b for b in current['behaviors']}
        if any(behavior_contract(current_behaviors[c['behavior']], current_builds) !=
               c['contract_sha256'] for c in claims):
            raise ValueError('Declared contracts changed during execution')
        report.update(success=True, sources_stable=True, cases=len(native), calls_compared=calls,
                      routing_and_metadata_match=True,
                      class_method_cases=1440, class_methods_match=True, empty_method_cases=480, empty_methods_no_raster_or_writes=True, guarded_empty_object_admission=True,
                      active_kind8_admission_and_hook_abi=True,
                      native_ctests_passed=3,
                      queue_state_cases=state_cases, queue_state_calls=state_calls, queue_state_match=True,
                      artifacts={p.name: sha(p) for p in out.iterdir() if p.is_file()})
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)


if __name__ == '__main__':
    main()
