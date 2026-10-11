#!/usr/bin/env python3
"""Check the public Qt glyph default, explicit overrides and isolated launchers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from unittest.mock import patch
from glyph_backend_launch import authorize, mode_from_environment, stage

ROOT = Path(__file__).resolve().parents[1]
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    sources = {p: sha(ROOT / p) for p in claims['sources']}
    if sources != claims['sources']:
        raise ValueError('Prospective sources changed')
    parent = ROOT / 'working/tests/glyph-default-launch'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, claims=claims['claims'], sources=sources,
                  scope='Public run-qt-shell.sh default/overrides and both private staging/authorization routes. No interactive Qt gameplay or new pixel/ABI equivalence claim.')
    commands = []
    def run(label, argv, env=None):
        result = subprocess.run([str(a) for a in argv], env=env, capture_output=True,
                                text=True, timeout=180)
        log = out / (label + '.log')
        log.write_text(result.stdout + result.stderr)
        commands.append(dict(label=label, argv=[str(a) for a in argv], returncode=result.returncode,
                             log=str(log.relative_to(ROOT)), sha256=sha(log)))
        result.check_returncode()
        return result.stdout
    try:
        # Execute the real public wrapper with substitutes only for build/Qt.
        fixture = out / 'launcher'
        (fixture / 'tools').mkdir(parents=True)
        (fixture / 'tools/run-qt-shell.sh').write_bytes((ROOT / 'tools/run-qt-shell.sh').read_bytes())
        build = fixture / 'working/build/qt-shell'
        build.mkdir(parents=True)
        probe = build / 'mnm-qt-shell'
        probe.write_text('#!' + sys.executable + '\nimport json,os,sys\nprint(json.dumps(dict(mode=os.environ.get("MNM_GLYPH_BACKEND"),args=sys.argv[1:])))\n')
        probe.chmod(0o755)
        bin_dir = fixture / 'bin'
        bin_dir.mkdir()
        cmake = bin_dir / 'cmake'
        cmake.write_text('#!/bin/sh\nexit 0\n')
        cmake.chmod(0o755)
        clean = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        wrapper_env = dict(clean, PATH=str(bin_dir) + os.pathsep + clean['PATH'])
        report['selection'] = {}
        for label, value, expected in [('default', None, 'takeover'), ('off', 'off', 'off'),
                                       ('shadow', 'shadow', 'shadow'), ('takeover', 'takeover', 'takeover'),
                                       ('empty', '', '')]:
            env = dict(wrapper_env)
            if value is not None:
                env['MNM_GLYPH_BACKEND'] = value
            selected = json.loads(run('wrapper-' + label,
                ['bash', fixture / 'tools/run-qt-shell.sh', '--live-menus', '--native-commands'], env))
            assert selected == dict(mode=expected, args=['--repo', str(fixture), '--live-menus', '--native-commands'])
            with patch.dict(os.environ, {'MNM_GLYPH_BACKEND': selected['mode']}, clear=True):
                mode = mode_from_environment()
            assert mode == (expected if expected in ('shadow', 'takeover') else None)
            report['selection'][label] = dict(exported=expected, staged_mode=mode)
        with patch.dict(os.environ, {}, clear=True):
            assert mode_from_environment() is None
        with patch.dict(os.environ, {'MNM_GLYPH_BACKEND':'invalid'}, clear=True):
            try:
                mode_from_environment()
            except ValueError:
                pass
            else:
                raise AssertionError('Invalid mode accepted')
        assert stage(out / 'unused', b'unsupported', b'unchanged', None) == (b'unchanged', {})
        disabled = dict(MNM_GLYPH_BACKEND='takeover', MNM_GLYPH_DIR='untrusted')
        authorize(out, {}, disabled)
        assert not disabled
        report['diagnostic_default_original'] = True
        report['off_does_not_stage_or_authorize'] = True
        run('manifest-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        sys.path.insert(0, str(ROOT / 'protocols/python'))
        from mnm_protocols import frame_v1
        frame = out / 'frame.bin'
        with frame.open('xb') as stream:
            stream.write(frame_v1.initial_header())
            stream.truncate(frame_v1.SIZE)
        report['public_staging'] = {}
        # Feed actual wrapper selections into each unchanged Qt child launcher.
        for label in ('default', 'off', 'shadow'):
            env = dict(clean, MNM_GLYPH_BACKEND=report['selection'][label]['exported'])
            for route, argv, prefix in [
                ('menu', ['python3', ROOT / 'tools/prepare-menu-observer.py', '--actions', '--native-render'], 'Evidence directory: '),
                ('normal', ['python3', ROOT / 'tools/run-opengl-game.py', '--stream', frame, '--skip-movies', '--stage-only'], 'Render experiment: ')]:
                output = run(label + '-' + route, argv, env)
                roots = [Path(line.removeprefix(prefix)) for line in output.splitlines() if line.startswith(prefix)]
                assert len(roots) == 1
                root = roots[0]
                metadata = json.loads((root / 'manifest.json').read_text())
                child = dict(env)
                authorize(root, metadata, child)
                expected = report['selection'][label]['staged_mode']
                assert metadata.get('glyph_backend_mode') == expected
                assert child.get('MNM_GLYPH_BACKEND') == expected
                assert sha(root / 'game/Chaos.exe') == metadata['staged_sha256']
                if expected is None:
                    assert not (root / 'glyph-backend').exists()
                    assert not (root / 'game/MnmScene.dll').exists()
                    assert b'MnmScene.dll' not in (root / 'game/Chaos.exe').read_bytes()
                else:
                    assert sha(root / 'game/MnmScene.dll') == metadata['glyph_dll_sha256']
                report['public_staging'][label + '-' + route] = dict(
                    experiment=str(root.relative_to(ROOT)), mode=expected,
                    manifest_sha256=sha(root / 'manifest.json'),
                    interactive_qt_gameplay_validated=False)
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['original_manifest_verified_before_after'] = all(
            any(c['label'] == label and c['returncode'] == 0 for c in commands)
            for label in ('manifest-before', 'manifest-after'))
        report['sources_stable'] = all(sha(ROOT / p) == h for p,h in sources.items())
        report['commands'] = commands
        report['success'] = report['success'] and report['sources_stable'] and report['original_manifest_verified_before_after']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps({k:report[k] for k in ('success', 'sources_stable')}))
    return 0 if report['success'] else 1

if __name__ == '__main__':
    raise SystemExit(main())
