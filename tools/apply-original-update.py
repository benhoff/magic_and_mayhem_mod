#!/usr/bin/env python3
"""Apply the supplied official update only to a fresh owned retail copy."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess

from coverage_evidence import evidence_digest, read_bytes, write_report
from drawing_fixture_prefix import prepare
from original_x86 import PE32

ROOT = Path(__file__).resolve().parents[1]
RETAIL = '124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214'
PAYLOAD = '308a503536018b2246f4802a4f51c25f0d78d2e34371f6c76132c890043ef770'


def patch_engine(raw):
    image = PE32(raw)
    base = image.base + image.u32(image.u32(60) + 24 + 112)
    at = base
    for name in (20530, 19274, 1033):
        header = image.read(at, 16)
        a, b = struct.unpack_from('<HH', header, 12)
        entries = [struct.unpack('<2I', image.read(at + 16 + 8*i, 8)) for i in range(a+b)]
        target, = [target for key, target in entries if key == name]
        at = base + (target & 0x7fffffff)
    rva, size, _, _ = struct.unpack('<4I', image.read(at, 16))
    data = image.read(image.base + rva, size)
    if struct.unpack_from('<I', data)[0] != len(data)-4:
        raise ValueError('Patch-engine length mismatch')
    engine = data[4:]
    PE32(engine)
    return engine


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--report', required=True)
    p.add_argument('--retail', type=Path, required=True)
    p.add_argument('--retail-manifest', type=Path, required=True)
    p.add_argument('--prefix-owner-root', type=Path, required=True)
    p.add_argument('--wine-prefix', type=Path, required=True)
    a = p.parse_args()
    out = a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):
        p.error('Require fresh owned working output')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Original official update, raw host logs and owned executable; preserve.\n')
    names = ['tools/apply-original-update.py', 'tests/original-update-host.c',
             'runtime/shadow/win32_min.h', 'tools/drawing_fixture_prefix.py',
             'tools/original_x86.py', 'tools/unpack-original-update.py']
    sources = {n: evidence_digest(ROOT/n, root=ROOT) for n in names}
    report = dict(schema=1, success=False, static_only=True, sources=sources,
                  original_game_executed=False, original_owner_recovered=False,
                  commands=[], retail_sha256=RETAIL, payload_sha256=PAYLOAD)
    env = {k:v for k,v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(WINEDEBUG='-all', DISPLAY='')
    lock = None
    before = False
    def run(label, cmd, timeout=60):
        path = out/(label+'.log')
        with path.open('x') as log:
            result = subprocess.run(list(map(str, cmd)), cwd=out, env=env,
                                    stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(label=label, argv=list(map(str, cmd)),
                                       exit=result.returncode, sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        if result.returncode:
            raise ValueError(label+': '+path.read_text(errors='replace')[-4000:])
    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        before = True
        raw = a.retail.read_bytes()
        if hashlib.sha256(raw).hexdigest() != RETAIL:
            raise ValueError('Retail executable changed')
        manifest = a.retail_manifest.read_bytes()
        entries = [line.split('\t') for line in manifest.decode().splitlines() if not line.startswith('sha256\t')]
        def verify_baseline():
            for sha, size, name in entries:
                path = a.retail.parent/name
                if not path.is_file() or path.is_symlink() or path.stat().st_size != int(size) or hashlib.sha256(path.read_bytes()).hexdigest() != sha:
                    raise ValueError('Clean installation manifest differs: '+name)
        verify_baseline()
        report['baseline'] = dict(files=len(entries), manifest_sha256=hashlib.sha256(manifest).hexdigest())
        payload = read_bytes(ROOT/'research/runtime/alternate-original-results-20261025/update04-payload.bin', root=ROOT)
        if hashlib.sha256(payload).hexdigest() != PAYLOAD:
            raise ValueError('Update payload changed')
        (out/'update.exe').write_bytes(payload)
        engine = patch_engine(payload)
        (out/'PATCHW32.dll').write_bytes(engine)
        game = out/'retail'
        run('stage', ['cp', '-a', '--reflink=auto', a.retail.parent.resolve(), game])
        imports = {'LoadLibraryA':4, 'GetProcAddress':8, 'GetEnvironmentVariableA':12, 'ExitProcess':4}
        definition = out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(v)+'\n' for n,v in imports.items()))
        run('imports', ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', out/'kernel32.lib'])
        run('compile', ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
                        '-fno-stack-protector', '-Wall', '-Wextra', '-Werror', '-c',
                        ROOT/'tests/original-update-host.c', '-o', out/'host.obj'])
        run('link', ['lld-link', '/machine:x86', '/entry:mainCRTStartup', '/subsystem:console',
                     '/nodefaultlib', '/out:'+str(out/'host.exe'), out/'host.obj', out/'kernel32.lib'])
        prefix, lock = prepare(a.prefix_owner_root.resolve(), out, a.wine_prefix, a.wine_prefix)
        env['WINEPREFIX'] = str(prefix)
        def win(path):
            return 'Z:'+str(path.resolve()).replace('/', '\\')
        command = '/z /noa "'+win(out/'update.exe')+'" "'+win(game)+'"'
        env['MNM_OFFICIAL_UPDATE_COMMAND'] = command
        run('apply', ['wine', out/'host.exe'])
        updated = (game/'Chaos.exe').read_bytes()
        PE32(updated)
        path = a.report.removesuffix('.json')+'-chaos.bin'
        sha = write_report(ROOT, path, updated)
        report.update(success=True, updated=dict(record=path, sha256=sha, bytes=len(updated)),
                      engine_sha256=hashlib.sha256(engine).hexdigest(), command=command)
        if a.retail.read_bytes() != raw:
            raise ValueError('Retail input changed')
        verify_baseline()
        report['baseline_verified_before_after'] = True
    except Exception as error:
        report.update(success=False, error=str(error))
    finally:
        if lock:
            try:
                subprocess.run(['wineserver', '-k'], env=env, check=True, timeout=10)
                subprocess.run(['wineserver', '-w'], env=env, check=True, timeout=10)
                report['private_clients_stopped'] = True
            finally:
                lock.close()
        if before:
            run('manifest-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        report['sources_stable'] = all(evidence_digest(ROOT/n, root=ROOT) == h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        sha = write_report(ROOT, a.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('success','error','updated')} | dict(sha256=sha)))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
