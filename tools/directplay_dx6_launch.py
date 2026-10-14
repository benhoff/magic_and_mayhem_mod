"""Stage supplied DirectX6 DirectPlay in a disposable menu experiment only."""
import hashlib
import json
import fcntl
import os
from pathlib import Path
import shutil
import subprocess
import time

REPO = Path(__file__).resolve().parents[1]
SOURCE = 'original/MagicMayhem_CD/DirectX6/Win95/Dx6eng.exe'
SOURCE_SHA256 = 'a9daa6ec591514337ccfa3df83e363bfa29010ffccf654c0e14ff55dabf98f81'
FILES = {
    'DPLAYX.DLL': '230a761372d6974d144aac75f77a01ee4aa970647221e8dd4b2c88d9331f692d',
    'DPWSOCKX.DLL': 'bb0c9be765fe037874514a2c18b39ae56ada288c09eceafac8fb9bac6d081977',
    'DPLAYSVR.EXE': '5b1a5758c65a09ffbb1aa594e56b3e9c72cc904398a4d37cf1147961a3f4154c',
    'DPLAY.DLL': '7f3978149dd401c33ab0cf34bf776d794005dccf8307cce5006710976c6df545',
}
BUILTIN = {
    'DPLAYX.DLL': '57201fc156715c85676d369bea7507d5fd019c7b8c94bb622cf60b9d1102a897',
    'DPLAY.DLL': '1081f9ebefaa0ecc6da88d65c79148062e5472eaec71042b5cb40188feb10846',
    'DPLAYSVR.EXE': '27274d6ba81f2e60955f39a11b38b465938c016312a6f25c54144479be28b2e4',
}


def prefix_identity(prefix):
    return dict(schema=1, prefix=str(prefix), source_sha256=SOURCE_SHA256,
                files={name: FILES[name] for name in BUILTIN})


def admit_prefix(root, metadata, prefix):
    prefix = prefix.resolve()
    if not prefix.is_relative_to(REPO / 'working'):
        raise ValueError('Supplied DirectPlay requires a private working prefix')
    marker = prefix / '.mnm-directplay-dx6.json'
    fresh = not prefix.exists()
    if not fresh:
        if marker.is_symlink() or not marker.is_file() or json.loads(marker.read_text()) != prefix_identity(prefix):
            raise ValueError('Refusing an existing prefix without matching DirectPlay ownership')
        check_prefix(prefix)
    metadata['directplay_prefix'] = dict(path=str(prefix), initialize=fresh)
    (root / 'manifest.json').write_text(json.dumps(metadata, indent=2) + '\n')


def check_prefix(prefix):
    for name in BUILTIN:
        path = prefix / 'drive_c/windows/syswow64' / name.lower()
        if path.is_symlink() or hashlib.sha256(path.read_bytes()).hexdigest() != FILES[name]:
            raise ValueError('Installed DirectPlay input changed: ' + name)


def install_prefix(root, metadata, environment):
    admission = metadata['directplay_prefix']
    prefix = Path(environment['WINEPREFIX']).resolve()
    if admission['path'] != str(prefix):
        raise ValueError('DirectPlay prefix differs from admission')
    marker = prefix / '.mnm-directplay-dx6.json'
    if admission['initialize']:
        if marker.exists() or marker.is_symlink():
            raise ValueError('Fresh DirectPlay prefix already has an owner')
        paths = {name: prefix / 'drive_c/windows/syswow64' / name.lower() for name in BUILTIN}
        if not all(path.is_file() for path in paths.values()):
            # The normal launcher initializes Wine lazily on its first exec.
            # Finish that initialization before verifying or replacing builtins.
            with (root / 'directplay-prefix-bootstrap.log').open('xb') as log:
                subprocess.run(['wineboot', '-i'], env=dict(environment, WINEDEBUG='-all'),
                               check=True, timeout=120, stdout=log, stderr=subprocess.STDOUT)
        # Check every expected Wine11.16 input before writing any binary.
        for name, path in paths.items():
            if path.is_symlink() or hashlib.sha256(path.read_bytes()).hexdigest() != BUILTIN[name]:
                raise ValueError('Unexpected fresh Wine DirectPlay input: ' + name)
        backup = root / 'directplay-prefix-before'
        backup.mkdir(exist_ok=False)
        for name, path in paths.items():
            shutil.copy2(path, backup / name)
            temporary = path.with_name(path.name + '.mnm-dx6')
            if temporary.exists() or temporary.is_symlink():
                raise ValueError('Refusing stale DirectPlay install output')
            shutil.copy2(root / 'game' / name, temporary)
            os.replace(temporary, path)
        check_prefix(prefix)
        marker.write_text(json.dumps(prefix_identity(prefix), indent=2) + '\n')
    else:
        if marker.is_symlink() or json.loads(marker.read_text()) != prefix_identity(prefix):
            raise ValueError('DirectPlay prefix ownership changed')
        check_prefix(prefix)
    metadata['directplay_prefix_result'] = prefix_identity(prefix)
    (root / 'manifest.json').write_text(json.dumps(metadata, indent=2) + '\n')


def requested():
    value = os.environ.get('MNM_MENU_DIRECTPLAY_DX6', '0')
    if value not in ('0', '1'):
        raise ValueError('Invalid supplied DirectPlay selection')
    return value == '1'


def check(root, profile):
    if profile != dict(source=SOURCE, source_sha256=SOURCE_SHA256, files=FILES):
        raise ValueError('Unsupported supplied DirectPlay profile')
    for name, digest in FILES.items():
        path = root / 'game' / name
        if path.is_symlink() or hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            raise ValueError('Supplied DirectPlay input changed: ' + name)


def stage(root):
    if not requested():
        return None
    source = REPO / SOURCE
    if hashlib.sha256(source.read_bytes()).hexdigest() != SOURCE_SHA256:
        raise ValueError('Original DirectX6 archive hash mismatch')
    for path in (root / 'game').iterdir():
        if path.name.upper() in FILES:
            raise ValueError('Refusing existing DirectPlay input: ' + path.name)
    # 7z reads the embedded cabinet. Never execute the installer or modify a prefix.
    subprocess.run(['7z', 'e', '-y', '-o' + str(root / 'game'), str(source), *FILES],
                   check=True, stdout=subprocess.DEVNULL)
    profile = dict(source=SOURCE, source_sha256=SOURCE_SHA256, files=FILES.copy())
    check(root, profile)
    return profile


def authorize(root, metadata, environment):
    profile = metadata.get('directplay_dx6')
    if requested() != (profile is not None):
        raise ValueError('Supplied DirectPlay selection differs from staging')
    if profile is None:
        return
    check(root, profile)
    install_prefix(root, metadata, environment)
    overrides = environment.get('WINEDLLOVERRIDES', '')
    environment['WINEDLLOVERRIDES'] = (overrides + ';' if overrides else '') + 'dplay,dplayx,dpwsockx=n'


def launch(root, metadata, arguments):
    """Wait for the game itself before closing its private DirectPlay helper."""
    prefix = Path(os.environ['WINEPREFIX']).resolve()
    if metadata['directplay_prefix_result'] != prefix_identity(prefix):
        raise ValueError('DirectPlay process launch lacks prefix ownership')
    check_prefix(prefix)
    with (prefix / '.mnm-directplay-session.lock').open('a') as lease:
        try:
            fcntl.flock(lease, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            raise ValueError('Another supplied DirectPlay session owns this prefix')
        def winepath(path):
            return 'Z:' + str(path).replace('/', '\\')
        exe = winepath(root / 'game/Chaos.exe')
        output = root / 'original-process-exit.txt'
        batch = root / 'original-process-wait.cmd'
        for value in [exe, winepath(output), *arguments]:
            if not value.isascii() or any(c in value for c in '\r\n%!&|<>^"'):
                raise ValueError('Unsupported command interpreter path/argument')
        if output.exists() or output.is_symlink() or batch.exists():
            raise ValueError('Refusing stale original process exit evidence')
        script = '@echo off\r\nstart "" /wait /b "' + exe + '"'
        if arguments:
            script += ' ' + subprocess.list2cmdline(arguments)
        script += '\r\nset MNM_ORIGINAL_EXIT=%ERRORLEVEL%\r\n> "' + winepath(output) + '" echo %MNM_ORIGINAL_EXIT%\r\nexit /b %MNM_ORIGINAL_EXIT%\r\n'
        batch.write_bytes(script.encode('ascii'))
        metadata['original_process_wait'] = dict(schema=1, batch_sha256=hashlib.sha256(batch.read_bytes()).hexdigest(),
                                               method='Wine start /wait then separate-line ERRORLEVEL capture', arguments=arguments)
        (root / 'manifest.json').write_text(json.dumps(metadata, indent=2) + '\n')
        child = subprocess.Popen(['wine', 'explorer', '/desktop=MagicMayhem,800x600',
                                  'cmd.exe', '/d', '/q', '/c', winepath(batch)])
        while not output.exists() or not output.stat().st_size:
            if child.poll() is not None:
                raise RuntimeError('Desktop exited without original process status')
            time.sleep(.1)
        raw = output.read_bytes()
        if len(raw) > 32:
            raise ValueError('Invalid original process exit record')
        code = int(raw.decode('ascii').strip())
        if not -(1 << 31) <= code < (1 << 32):
            raise ValueError('Invalid original process exit code')
        print('Original process exited with status ' + str(code) + '; closing its owned DirectPlay helper.', flush=True)
        # start /wait has returned from WaitForSingleObject/GetExitCodeProcess.
        # Only the now-idle helper in this leased, profile-owned prefix is closed.
        cleanup = subprocess.run(['wine', 'taskkill', '/f', '/im', 'dplaysvr.exe'],
                                 timeout=15, check=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (root / 'directplay-helper-cleanup.log').write_bytes(cleanup.stdout)
        desktop_code = child.wait(timeout=15)
        result = dict(schema=1, original_exit_code=code, desktop_exit_code=desktop_code,
                      helper_cleanup_returncode=cleanup.returncode, cleanup_after_original_exit=True,
                      prefix=str(prefix), prefix_session_lease=True)
        (root / 'original-process-result.json').write_text(json.dumps(result, indent=2) + '\n')
        check_prefix(prefix)
        return code if 0 <= code <= 255 and desktop_code == 0 else 1
