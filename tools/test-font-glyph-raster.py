#!/usr/bin/env python3
"""Compare owned native glyph canvases with unmodified pinned PE32 glyph code."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import shlex
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def synthetic(seed):
    rng = random.Random(seed)
    w, h = 16, 8
    controls, planes = bytearray(), bytearray()
    rows = []
    for y in range(h):
        rows.append((len(controls), len(planes)))
        x = 0
        while x < w:
            skip = min(w-x, rng.randrange(4))
            controls.append(skip)
            x += skip
            if x == w:
                break
            run = min(w-x, rng.randrange(1, 6))
            controls.append(run)
            planes.extend((x+k+y*w+seed) % 64 for k in range(run))
            x += run
    base = 40+h*8
    header = struct.pack('<10I', base+len(controls)+len(planes), w, h, 2, 3, 0, 0, 0, 0, 0)
    return header + b''.join(struct.pack('<2I', base+a, base+len(controls)+b) for a,b in rows) + controls + planes


def fonts(root):
    result = []
    for path in sorted(root.rglob('*.sft')):
        raw = path.read_bytes()
        words = struct.unpack_from('<10I', raw)
        if words[0] != 0x00544653 or words[1] != len(raw) or words[2] != 3 or not words[8]:
            raise ValueError('Unsupported installed font: '+str(path))
        table = 40+768+words[4]*words[9]*8
        base = table+words[3]*4
        for i in range(words[3]):
            start = base+struct.unpack_from('<I', raw, table+i*4)[0]
            size = struct.unpack_from('<I', raw, start)[0]
            frame = raw[start:start+size]
            if len(frame) != size or size < 40:
                raise ValueError('Unclosed installed glyph')
            # Runtime palette pointer is irrelevant to this coverage raster.
            frame = frame[:28]+b'\0'*4+frame[32:]
            result.append((path, i, frame))
    if not result:
        raise ValueError('No installed fonts')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--font-root', type=Path, default=ROOT/'working/game-clean/Sprites')
    args = parser.parse_args()
    parent = ROOT/'working/tests/font-glyph-raster'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    # Capture source bytes before builds/execution; compiler closure is selected below.
    names = [p for root in ('assets', 'renderer', 'tests', 'tools') for p in (ROOT/root).rglob('*')
             if p.is_file() and p.suffix in ('.cpp', '.hpp', '.c', '.h', '.py')]
    names += [ROOT/'assets/CMakeLists.txt', ROOT/'tests/glyph-raster/CMakeLists.txt']
    before = {str(p.relative_to(ROOT)): sha(p) for p in names}
    report = {'success': False, 'sources': {}, 'source_executable_sha256': HASH,
              'original_pixels_used_as_native_inputs': False, 'live_replacement': False,
              'scope': 'Selected RGB565 initialized-table glyph pixels; isolated original code, synthetic and installed sources. No text layout, Win32 ABI, live admission or bypass.'}

    def run(label, command, env=None):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command], cwd=ROOT, env=env, stdout=log,
                           stderr=subprocess.STDOUT, timeout=180, check=True)

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        pe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Original executable hash changed')
        installed = fonts(args.font_root.resolve())
        inputs = {str(p.relative_to(ROOT)): sha(p) for p,_,_ in installed}
        inputs[str(pe.relative_to(ROOT))] = sha(pe)
        report['inputs'] = inputs
        records, manifest = [], []

        def case(frame, label, position, tint, table, pad=0):
            fw, fh, ox, oy = struct.unpack_from('<4i', frame, 4)
            w, h = max(32, fw+8), max(24, fh+8)
            if w > 256 or h > 256:
                raise ValueError('Glyph fixture canvas budget')
            x, y, clip = position(w, h, fw, fh)
            values = [316+len(frame), len(frame), w, h, w+pad, x+ox, y+oy, *clip, *tint, 113]
            records.append(struct.pack('<15I', *(v & 0xffffffff for v in values))+struct.pack('<64f', *table)+frame)
            manifest.append({'source': label, 'words': w*h, 'x': x, 'y': y, 'clip': clip, 'tint': tint, 'padding': pad})

        placements = [
            lambda w,h,fw,fh:(4,4,(0,0,w,h)),
            lambda w,h,fw,fh:(0,0,(0,0,w,h)),
            lambda w,h,fw,fh:(w-fw,h-fh,(0,0,w,h)),
            lambda w,h,fw,fh:(-1,4,(0,0,w,h)),
            lambda w,h,fw,fh:(w-fw+1,4,(0,0,w,h)),
            lambda w,h,fw,fh:(4,-3,(0,0,w,h)),
            lambda w,h,fw,fh:(4,h-3,(0,0,w,h)),
            lambda w,h,fw,fh:(4,4,(4,5,4+fw,h)),
            lambda w,h,fw,fh:(4,4,(5,0,w,h)),
            lambda w,h,fw,fh:(4,h,(0,0,w,h)),
        ]
        tables = [[i/64 for i in range(64)], [i/63 for i in range(64)],
                  [0 if i % 3 == 0 else 1 if i % 3 == 1 else 0.5 for i in range(64)]]
        tints = [(0,0,0), (255,255,255), (173,91,237), (7,3,7), (8,4,8), (248,252,248)]
        for seed in range(4):
            for position in placements:
                for tint in tints:
                    for table in tables:
                        case(synthetic(seed), 'synthetic-'+str(seed), position, tint, table, seed % 3)
        synthetic_cases = len(records)
        for path, index, frame in installed:
            for position in (placements[0], placements[5], placements[3]):
                case(frame, str(path.relative_to(ROOT))+':'+str(index), position, tints[2], tables[0], 2)
        corpus = out/'cases.bin'
        corpus.write_bytes(b'MNMGLY01'+struct.pack('<I',len(records))+b''.join(records))
        report.update(cases=len(records), synthetic_cases=synthetic_cases,
                      installed_cases=len(records)-synthetic_cases, installed_fonts=len(inputs)-1,
                      corpus_sha256=sha(corpus))
        (out/'cases.json').write_text(json.dumps(manifest,indent=2)+'\n')
        build = out/'build'
        run('configure', ['cmake','-S',ROOT/'tests/glyph-raster','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake','--build',build,'--target','glyph-raster-native','glyph-raster-test','-j4'])
        run('admission', ['ctest','--test-dir',build,'-R','^glyph-atomic-admission$','--output-on-failure'])
        reference = out/'glyph-reference'
        run('reference-build', ['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',
                                '-MMD','-MF',out/'reference.d',ROOT/'tests/glyph-raster-reference.cpp','-o',reference])
        qt = shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui'],text=True))
        sanitized = out/'glyph-admission-sanitized'
        run('sanitizer-build', ['c++','-std=c++17','-O1','-g','-fPIC','-fno-omit-frame-pointer',
            '-fsanitize=address,undefined',ROOT/'tests/glyph-raster-test.cpp',ROOT/'renderer/canvas_sequence.cpp',
            ROOT/'renderer/minimap/minimap.cpp',ROOT/'renderer/minimap/overlays.cpp',*qt,'-o',sanitized])
        run('sanitizer', [sanitized], env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1',
                                         'UBSAN_OPTIONS':'halt_on_error=1'})
        report['sanitizer_passed'] = True
        dependencies = { 'tools/test-font-glyph-raster.py', 'assets/CMakeLists.txt', 'tests/glyph-raster/CMakeLists.txt' }
        for path in [*build.rglob('*.o.d'), out/'reference.d']:
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                if not Path(name).is_absolute():
                    continue
                p = Path(name).resolve()
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    dependencies.add(str(p.relative_to(ROOT)))
        report['sources'] = {n:before[n] for n in sorted(dependencies)}
        registry = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        sys.path.insert(0,str(ROOT/'tools'))
        import coverage_claims
        behaviors = [b for b in registry['behaviors'] if b['id'] in ('RS.font-glyph-raster','NR.font-glyph-admission')]
        report['claims'] = [{'behavior':b['id'], 'contract_sha256':coverage_claims.behavior_contract(
            b,{build['id']:build for build in registry['builds']}), 'scenarios':{
                s['id']:coverage_claims.scenario_contract(s) for s in registry['scenarios']
                if b['id'] in s['behaviors'] and s['id']=='font-glyph-atomic-20261010'}} for b in behaviors]
        run('original', [reference,pe,corpus,out/'original.565'])
        run('native', [build/'glyph-raster-native',corpus,out/'native.565'])
        old, new = (out/'original.565').read_bytes(), (out/'native.565').read_bytes()
        if len(old) != len(new) or len(old) != sum(c['words'] for c in manifest)*2:
            raise ValueError('Glyph output extent changed')
        unequal, offset = [], 0
        for index, c in enumerate(manifest):
            end = offset+c['words']*2
            if old[offset:end] != new[offset:end]:
                unequal.append(index)
            offset = end
        report.update(cases=len(records), synthetic_cases=synthetic_cases, installed_cases=len(records)-synthetic_cases,
                      installed_fonts=len(inputs)-1, equal=len(records)-len(unequal), mismatched_cases=unequal,
                      words_compared=len(old)//2, source_and_padding_guards_passed=True,
                      original_sha256=sha(out/'original.565'), native_sha256=sha(out/'native.565'),
                      native_binary_sha256=sha(build/'glyph-raster-native'), reference_binary_sha256=sha(reference),
                      atomic_refusals=8, corpus_sha256=sha(corpus), inputs=inputs)
        report['success'] = not unequal
    except Exception as exc:
        report['error'] = str(exc)
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_stable'] = all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['inputs_stable'] = all(sha(ROOT/n)==h for n,h in report.get('inputs',{}).items())
        report['success'] = report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
