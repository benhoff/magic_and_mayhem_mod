#!/usr/bin/env python3
"""Watch original global lighting storage and inspect pinned installed inputs."""
import argparse
import configparser
import hashlib
import json
import os
from pathlib import Path
import runpy
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import read_bytes, read_json, write_report

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BEHAVIOR = 'RE.terrain-single-cell-global-storage'
COHORTS = ['fix-observe-reload01', 'observe-stress01']


def sha(data):
    return hashlib.sha256(data).hexdigest()


def packet(raw):
    if len(raw) < 300 or raw[:8] != b'MNMLIT01':
        raise ValueError('Lighting packet header')
    w = struct.unpack_from('<32I', raw)
    if w[2:5] != (1, 128, len(raw)):
        raise ValueError('Lighting packet extent/version')
    width, height, layers, count, creatures, objects = w[8], w[9], w[10], w[11], w[19], w[21]
    if (not 0 < width <= 128 or not 0 < height <= 128 or not 0 < layers <= 32
            or count != width * height * ((layers + 1) // 2)
            or creatures > 65536 or objects > 65536
            or len(raw) != 300 + creatures * 28 + objects * 32 + count * 5):
        raise ValueError('Lighting packet source/buffer shape')
    inherited = struct.unpack_from('<i', raw, 56)[0]
    if not -128 <= inherited <= 127:
        raise ValueError('Noncanonical inherited byte')
    checksum = 2166136261
    for i, value in enumerate(raw):
        checksum = ((checksum ^ (0 if 120 <= i < 124 else value)) * 16777619) & 0xffffffff
    if checksum != w[30]:
        raise ValueError('Lighting packet checksum')
    indices = {}
    for at in range(300 + creatures * 28, 300 + creatures * 28 + objects * 32, 32):
        active, index = struct.unpack_from('<2I', raw, at)
        if active:
            indices[str(index)] = indices.get(str(index), 0) + 1
    return dict(inherited=inherited, creatures=creatures, objects=objects,
                active_static_indices=indices, caller=w[27], checksum=checksum)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--main-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    main_root, out, target = args.main_root.resolve(), args.output.resolve(), ROOT / args.report
    if out.exists() or target.exists() or not target.is_relative_to(ROOT / 'research/runtime'):
        parser.error('Use fresh output and immutable research report paths')
    out.mkdir(parents=True)
    reg = json.loads((ROOT / 'research/runtime/coverage/register.json').read_bytes())
    behavior = next(b for b in reg['behaviors'] if b['id'] == BEHAVIOR)
    builds = {b['id']: b for b in reg['builds']}
    paths = required_sources(behavior)
    sources = {p: sha(read_bytes(ROOT / p, root=ROOT)) for p in sorted(paths)}
    code_sources = dict(sources)
    report = dict(schema=1, success=False, sources=sources, recorded_inputs=[],
                  claims=[dict(behavior=BEHAVIOR, contract_sha256=behavior_contract(behavior, builds))],
                  original_sha256=BUILD, commands=[], artifacts=[],
                  new_live_session=False, native_production_changed=False,
                  scope='Global PE zero-fill origin, CPU write-watch of unchanged initializer/kernel/clear with supplied delete endpoint, original installed FX loader/selector with profile endpoints, and current analysis of64 retained historical original-observation packets. Authored diameter-one controls remain separate; no whole-program writer exclusion or natural index-one discovery.')

    def retain(label, data):
        path = target.parent / (target.stem + '-' + label)
        digest = write_report(ROOT, path, data)
        assert sha(read_bytes(path, root=ROOT)) == digest
        row = dict(path=str(path.relative_to(ROOT)), sha256=digest, bytes=len(data))
        report['artifacts'].append(row)
        return row

    def command(label, argv, timeout=180):
        print(label, flush=True)
        result = subprocess.run(list(map(str, argv)), cwd=ROOT, env=env,
                                capture_output=True, timeout=timeout)
        report['commands'].append(dict(label=label, exit_code=result.returncode,
            log=retain(label + '.log', result.stdout + result.stderr)))
        if result.returncode:
            raise RuntimeError(label + ': ' + (result.stdout + result.stderr)[-3000:].decode(errors='replace'))
        return result.stdout

    env = dict(os.environ, TMPDIR=str(out), UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    exe = main_root / 'original/Arcane_Nocd/Chaos.exe'
    packed_path = main_root / 'working/game-nocd/CFG/Encrypted/effects.cfg'
    try:
        for p, digest in code_sources.items():
            retain('source-' + sha(p.encode())[:16] + '.bin', read_bytes(ROOT / p, root=ROOT))
        command('manifest-before', [main_root / 'tools/original-manifest.sh', 'verify'])
        if sha(exe.read_bytes()) != BUILD:
            raise ValueError('Pinned executable differs')
        packed = packed_path.read_bytes()
        stored = retain('installed-effects.cfg.bin', packed)
        sources[stored['path']] = stored['sha256']
        report['recorded_inputs'].append(stored['path'])
        decoder = runpy.run_path(str(ROOT / 'tools/decode-cfg.py'))
        _, text = decoder['decode'](packed)
        cfg = configparser.ConfigParser(interpolation=None)
        cfg.read_string(text.decode('ascii'))
        keys = ['LightSourceDiameter', 'LightSourceAffected', 'ClippedToHeight']
        fields = [[cfg.get('FXA_' + str(i), key, fallback='').strip().strip('\"\'')[:255]
                   for key in keys] for i in range(89)]
        fixture = out / 'installed-fields.bin'
        fixture.write_bytes(b''.join(struct.pack('<I', len(v.encode('ascii'))) + v.encode('ascii')
                                    for row in fields for v in row))
        retain('decoded-fields.bin', fixture.read_bytes())
        report['installed_nonzero_diameters'] = {str(i): int(row[0]) for i, row in enumerate(fields) if row[0] and int(row[0])}
        results = []
        for variant, flags in [('optimized32', ['-O2']),
                               ('ubsan32', ['-O1', '-g', '-fsanitize=undefined', '-fno-sanitize-recover=all'])]:
            binary = out / variant
            command(variant + '-build', ['g++', '-std=c++17', '-m32', '-fno-pie', '-no-pie',
                                        *flags, ROOT / 'tests/terrain-single-cell-storage-reference.cpp', '-o', binary])
            retain(variant + '-binary.bin', binary.read_bytes())
            result = json.loads(command(variant, [binary, exe, fixture], timeout=240))
            assert (result['cases'], result['positive_same_value_and_overlapping_writes'],
                    result['selected_byte_writes'], result['installed_selectors']) == (257, 514, 0, 89)
            report.setdefault('variants', {})[variant] = result
            results.append({k: v for k, v in result.items() if k != 'control_post_write_pcs'})
        if results[0] != results[1]:
            raise ValueError('Original probe variants differ')
        cohorts = []
        example = None
        for name in COHORTS:
            path = 'research/runtime/terrain-light-qt-results-20261026/' + name + '.json'
            raw = read_bytes(ROOT / path, root=ROOT)
            prior = json.loads(raw)
            assert prior['success'] and prior['mode'] == 'observe' and not prior['live_replacement']
            assert prior['original_sha256'] == BUILD and len(prior['results']) == 32
            sources[path] = sha(raw)
            report['recorded_inputs'].append(path)
            rows = []
            for row in prior['results']:
                reference = row['input']
                raw = read_bytes(ROOT / reference['path'], root=ROOT)
                assert sha(raw) == reference['sha256']
                decoded = packet(raw)
                assert decoded['inherited'] == 0 and decoded['caller'] == row['caller']
                sources[reference['path']] = reference['sha256']
                report['recorded_inputs'].append(reference['path'])
                rows.append(dict(path=reference['path'], sha256=reference['sha256'], **decoded))
                example = raw
            cohorts.append(dict(name=name, provenance='Retained prior original observation; no new live execution', rows=rows))
        report['historical_cohorts'] = cohorts
        refusals = []
        for label, bad in [('truncated', example[:-1]), ('bad-checksum', example[:56] + bytes([example[56] ^ 1]) + example[57:]),
                           ('noncanonical-byte', example[:56] + struct.pack('<I', 256) + example[60:])]:
            try:
                packet(bad)
            except ValueError as error:
                refusals.append(dict(control=label, refusal=str(error)))
            else:
                raise ValueError('Malformed packet accepted: ' + label)
        report['packet_negative_controls'] = refusals
        assert packed_path.read_bytes() == packed
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            command('manifest-after', [main_root / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = any(c['label'] == 'manifest-before' and c['exit_code'] == 0 for c in report['commands'])
        except Exception as error:
            report['success'] = False
            report['manifest_error'] = repr(error)
        report['code_sources_stable'] = all(sha(read_bytes(ROOT / p, root=ROOT)) == h for p, h in code_sources.items())
        report['success'] &= report['code_sources_stable']
        digest = write_report(ROOT, target, (json.dumps(report, indent=2) + '\n').encode())
        print(json.dumps(dict(success=report['success'], sha256=digest, error=report.get('error'))), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
