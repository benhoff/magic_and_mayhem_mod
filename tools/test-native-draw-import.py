#!/usr/bin/env python3
"""Prove pinned import redirection and byte guards without launching a game."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    parent = ROOT/'working/tests/native-draw-import'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(success=False, sources=claims['sources'], claims=claims['claims'],
                  scope='Offline guarded pinned PE32 import redirection only. Eight staging compositions preserve original IAT and section bytes, with only the explicit enumeration variant changing the two guarded12-byte DLL names, change the graphics descriptor name to native and add the anchor. Ten mutations and duplicate staging refuse. No executable instructions or loader/game integration are executed.',
                  real_game_directdraw_retired=False)
    before = False
    try:
        if any(sha((ROOT/n).read_bytes()) != h for n, h in claims['sources'].items()):
            raise ValueError('Prospective source changed')
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        source = ROOT/'working/game-nocd/Chaos.exe'
        data = source.read_bytes()
        report['source_executable_sha256'] = sha(data)
        if sha(data) != '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':
            raise ValueError('Pinned executable differs')
        native = load('native_draw_stage', 'tools/prepare-native-draw.py')
        imports = load('native_draw_test_import', 'tools/prepare-shadow-experiment.py')
        report['cases'] = []
        for case in range(8):
            index=case%4
            enumeration=case>=4
            incoming = data
            for dll, symbol, section in [
                    ('MnmMenu.dll', 'MenuAnchor', b'.mnmenu'),
                    ('MnmScene.dll', 'SceneAnchor', b'.mnscene'),
                    ('MnmRender.dll', 'RenderAnchor', b'.mnmgl')][:index]:
                incoming = imports.add_import(incoming, dll, symbol, section)
            patched = native.redirect(incoming,enumeration=enumeration)
            expected = native.remap_enumerator(incoming) if enumeration else incoming
            sections, offset, old = native.layout(incoming)
            _, new_offset, new = native.layout(patched)
            for name, _, _, size, raw in sections:
                if patched[raw:raw+size] != expected[raw:raw+size]:
                    raise ValueError('Existing section modified: '+repr(name))
            for i, (old_at, new_at) in enumerate(zip(old, new)):
                a = incoming[old_at:old_at+20]
                b = patched[new_at:new_at+20]
                if i == 3:
                    if a[:12] != b[:12] or a[16:] != b[16:]:
                        raise ValueError('Original graphics lookup/IAT changed')
                elif a != b:
                    raise ValueError('Other import descriptor changed')
            dlls = []
            for at in new:
                rva = struct.unpack_from('<I', patched, at+12)[0]
                start = new_offset(rva, 1)
                dlls.append(patched[start:patched.index(b'\0', start)].decode())
            if any(n.lower() == 'ddraw.dll' for n in dlls) or dlls[3] != 'MnmDraw.dll' or dlls[-1] != 'MnmDraw.dll':
                raise ValueError('Graphics import retirement incomplete')
            try:
                native.redirect(patched,enumeration=enumeration)
            except ValueError:
                pass
            else:
                raise ValueError('Duplicate native staging admitted')
            (out/f'case-{case}.exe').write_bytes(patched)
            report['cases'].append(dict(existing_additions=index, output_sha256=sha(patched),
                                        original_sections_unchanged=not enumeration, dynamic_loader_redirect=enumeration, guarded_data_bytes_changed=24 if enumeration else 0, all_other_section_bytes_unchanged=True, original_iat_unchanged=True,
                                        directdraw_import_absent=True, duplicate_refused=True))
        sections, offset, descriptors = native.layout(data)
        mutations = [raw for _, _, _, _, raw in sections]
        mutations += [offset(0x1e1264,12)+10,offset(0x1e1264,12)+11,offset(0x1f1a84,12)+10,offset(0x1f1a84,12)+11,descriptors[3]+12, offset(struct.unpack('<5I', native.DESCRIPTOR)[0], 12)]
        for position in mutations:
            damaged = bytearray(data)
            damaged[position] ^= 1
            try:
                native.redirect(bytes(damaged),enumeration=True)
            except ValueError:
                pass
            else:
                raise ValueError('Changed original bytes admitted')
        report.update(mutation_refusals=len(mutations), duplicate_refusals=8, success=True)
        if sha(source.read_bytes()) != sha(data):
            raise ValueError('Input executable changed')
    except Exception as error:
        report.update(success=False, error=str(error))
    finally:
        if before:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
            report['original_manifest_verified_before_after'] = True
        report['sources_stable'] = all(sha((ROOT/n).read_bytes()) == h for n, h in report['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
