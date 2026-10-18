"""Build and decode the bounded CPUID-context debugger; no process-memory writes."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = ('runtime/diagnostics/raster_cpu_debug.c', 'tests/raster-cpu-debug-target.c',
           'tools/raster_cpu_debug.py', 'runtime/diagnostics/win32_debug_min.h',
           'runtime/shadow/win32_min.h')
IMPORTS = {'CreateFileA': 28, 'WriteFile': 20, 'CloseHandle': 4, 'SetFilePointer': 16,
           'FlushFileBuffers': 4, 'ExitProcess': 4, 'GetEnvironmentVariableA': 12,
           'GetCommandLineA': 0, 'CreateProcessA': 40, 'WaitForDebugEvent': 8,
           'ContinueDebugEvent': 12, 'GetThreadContext': 8, 'SetThreadContext': 8,
           'ReadProcessMemory': 20, 'VirtualAlloc': 16, 'VirtualProtect': 16}
ALTERNATE_ROUTES = tuple('raster/splice' + str(i) for i in (0, 1, 4, 5, 11, 12, 14))


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def pinned_code(image):
    data = Path(image).read_bytes()
    if hashlib.sha256(data).hexdigest() != BUILD:
        raise ValueError('Unsupported original CPU initializer')
    pe = struct.unpack_from('<I', data, 60)[0]
    sections = pe + 24 + struct.unpack_from('<H', data, pe + 20)[0]
    for i in range(struct.unpack_from('<H', data, pe + 6)[0]):
        size, rva, length, raw = struct.unpack_from('<4I', data, sections + i * 40 + 8)
        offset = 0x484470 - 0x400000 - rva
        if 0 <= offset and offset + 0x231 <= min(size, length):
            code = data[raw + offset:raw + offset + 0x231]
            if code[0x192:0x194] != b'\x0f\xa2' or code[0x11a] != 0xc3:
                raise ValueError('Original CPU instruction boundaries changed')
            return code
    raise ValueError('Original initializer not backed by a PE section')


def build(out, image):
    """Caller owns original manifest bookends and the storage reservation."""
    out = Path(out)
    out.mkdir(parents=True, exist_ok=False)
    code = pinned_code(image)
    header = out / 'raster_cpu_pinned.h'
    header.write_text('static const unsigned char raster_cpu_pinned[] = {' +
                      ','.join(hex(b) for b in code) + '};\n')
    definition = out / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' +
                          ''.join(f'{name}@{size}\n' for name, size in IMPORTS.items()))
    commands = [['llvm-dlltool', '-m', 'i386', '-D', 'KERNEL32.dll', '-d', str(definition),
                 '-l', str(out / 'kernel32.lib'), '--kill-at']]
    for name, source in (('debugger', SOURCES[0]), ('target', SOURCES[1])):
        commands.extend([
            ['clang', '--target=i686-pc-windows-msvc', '-std=c11', '-O2', '-ffreestanding',
             '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx',
             '-Wall', '-Wextra', '-Werror', '-Wno-missing-field-initializers',
             '-I' + str(out), '-c', str(ROOT / source),
             '-o', str(out / (name + '.obj'))],
            ['lld-link', '/machine:x86', '/subsystem:console', '/entry:mainCRTStartup',
             '/nodefaultlib', '/safeseh:no', '/base:0x400000', '/fixed', '/timestamp:0',
             '/out:' + str(out / (name + '.exe')), str(out / (name + '.obj')),
             str(out / 'kernel32.lib')]])
    for i, command in enumerate(commands):
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=90)
        (out / f'build-{i}.log').write_text(result.stdout + result.stderr)
        result.check_returncode()
    # No API capable of writing child memory is linked. Check the built import table.
    imports = subprocess.check_output(['objdump', '-p', str(out / 'debugger.exe')], text=True)
    (out / 'debugger-imports.txt').write_text(imports)
    if 'WriteProcessMemory' in imports or 'SetThreadContext' not in imports:
        raise ValueError('Unexpected debugger memory-mutation interface')
    return dict(commands=commands, original_sha256=BUILD,
                code_sha256=hashlib.sha256(code).hexdigest(),
                artifacts={p.name: sha(p) for p in out.iterdir() if p.is_file()},
                sources={p: sha(ROOT / p) for p in SOURCES})


def decode(path, family):
    if family not in (6, 15):
        raise ValueError('Unsupported authored CPU family')
    data = Path(path).read_bytes()
    if len(data) != 256 or data[:8] != b'MNMCPU01':
        raise ValueError('Invalid CPU debugger proof extent/magic')
    q = struct.unpack('<62I', data[8:])
    if q[:2] != (1, 256) or q[2:5] != (1, 0, 0) or any(q[31:]):
        raise ValueError('Incomplete or failed CPU debugger acquisition')
    if not q[5] or not q[6] or q[7] != 0x400000 or q[8:12] != (family, 1, 1, 1):
        raise ValueError('Missing unique original initializer/helper execution')
    before, after, targets = q[14:18], q[18:22], q[22:26]
    if after != ((before[0] & ~0xf00) | family << 8, *before[1:]):
        raise ValueError('CPU context changed outside raw family bits')
    expected = ((0x59521a, 0x59603e, 0x596490, 0x596cb8) if family == 6 else
                (0x5947b2, 0x595b47, 0x5968a4, 0x597086))
    if q[26] == 1:
        expected = (0x57de00, *expected[1:])
    if targets != expected or q[27:30] != (family, 3, 1):
        raise ValueError('Original target selection/code checks differ')
    if not 0x401000 <= q[12] < 0x5c0000 or not q[13]:
        raise ValueError('Missing original caller/object identity')
    return dict(verified=True, base_family=family, host_cpuid=list(before), authored_cpuid=list(after),
                targets=list(targets), override=q[26], caller=q[12], object=q[13],
                pid=q[5], tid=q[6], code_checks=q[28], first_chance_exceptions=q[30],
                sha256=sha(path), child_exit=0, executable_bytes_unchanged=True,
                dispatch_pointers_written_by_debugger=False,
                scope='Game-selected raster dispatch under an authored CPUID.1 raw base family; other reported CPU bits unchanged. No physical alternate-CPU or pixel/session equivalence claim.')


def decode_rgb(path, cpu):
    """An observer record never independently grants native/empty admission."""
    data = Path(path).read_bytes()
    if len(data) != 512 or data[:8] != b'MNMRGB01' or cpu.get('verified') is not True:
        raise ValueError('Invalid RGB caller extent/magic or unverified CPU child')
    q = struct.unpack('<126I', data[8:])
    if (q[:5] != (1, 512, 1, 0, 0) or q[5:7] != (cpu['pid'], cpu['tid']) or
            q[10:12] != (0x545ce9, 8) or any(q[12:62]) or q[8] > q[7] or q[9] != min(q[8], 8)):
        raise ValueError('Incomplete or mismatched RGB caller proof')
    samples, sequence = [], 0
    signed = lambda value: value-0x100000000 if value & 0x80000000 else value
    for i in range(8):
        row = q[62+i*8:70+i*8]
        if i >= q[9]:
            if any(row): raise ValueError('Unexpected unused RGB sample')
            continue
        seq, bottom, top, stack, flags, right_raw, skip_raw, tid = row
        right, skip = signed(right_raw), signed(skip_raw)
        if not (sequence < seq <= q[7] and 0 <= top < bottom <= 2048 and
                0 < stack <= 0xffffffff-0x34 and -2048 <= right <= 2048 and
                0 <= skip <= 2048 and right <= skip and tid == cpu['tid']):
            raise ValueError('Invalid RGB empty geometry/order/thread')
        sequence = seq
        samples.append(dict(sequence=seq,bottom=bottom,top=top,stack=stack,flags=flags,
                            right=right,skip=skip,tid=tid))
    return dict(verified=True,pid=q[5],tid=q[6],calls=q[7],empty_calls=q[8],samples=samples,
                site=q[10],child_exit=0,sha256=sha(path),native_admission_proved=False,
                scope='Read-only hardware observation of prepared scalar geometry on the initializer thread. Native admission needs independent matching producer counters and an enabled candidate. No pixel/source read, game-side I/O or natural caller credit from fixtures.')


def wine_wrapper(directory, debugger, family, rgb_diagnostics=False, empty_rgb=False, rgb_caller_trace=False, disable_empty_rgb=False):
    """Intercept only the existing owned explorer/Chaos launch, preserving args."""
    if empty_rgb and disable_empty_rgb:
        raise ValueError('Conflicting RGB policy selection')
    directory = Path(directory)
    directory.mkdir()
    debugger = Path(debugger).resolve()
    body = f'''#!/usr/bin/env python3
import hashlib,json,os
from pathlib import Path
import sys
args=sys.argv[1:]
direct=any(Path(a).name.lower()=='chaos.exe' for a in args)
batch_launch=(len(args)==7 and args[:6]==['explorer','/desktop=MagicMayhem,800x600','cmd.exe','/d','/q','/c'])
if direct or batch_launch:
    debugger=Path({str(debugger)!r})
    if direct and (len(args)<3 or args[:2]!=['explorer','/desktop=MagicMayhem,800x600'] or Path(args[2]).name!='Chaos.exe'):
        raise ValueError('Refusing an unexpected game launch')
    if hashlib.sha256(debugger.read_bytes()).hexdigest()!={sha(debugger)!r}:raise ValueError('Debugger changed')
    record=dict(command=args[:],debugger=str(debugger),sha256=hashlib.sha256(debugger.read_bytes()).hexdigest(),family={family},empty_rgb={bool(empty_rgb)!r},rgb_caller_trace={bool(rgb_caller_trace)!r},empty_rgb_selection={'disabled0' if disable_empty_rgb else 'enabled1' if empty_rgb else 'default'!r})
    if batch_launch:
        if not args[6].startswith('Z:'):raise ValueError('Unsupported wait batch path')
        batch=Path(args[6][2:].replace(chr(92),'/'))
        if batch.name!='original-process-wait.cmd' or batch.is_symlink():raise ValueError('Unexpected wait batch')
        batch=batch.resolve()
        manifest=json.loads((batch.parent/'manifest.json').read_text())
        original=batch.read_bytes()
        if hashlib.sha256(original).hexdigest()!=manifest['original_process_wait']['batch_sha256']:
            raise ValueError('Wait batch changed')
        game=batch.parent/'game/Chaos.exe'
        def winepath(p):return 'Z:'+str(p).replace('/',chr(92))
        prefix=('@echo off'+chr(13)+chr(10)+'start "" /wait /b "'+winepath(game)+'"').encode('ascii')
        if not original.startswith(prefix):raise ValueError('Unexpected original wait command')
        replacement=('@echo off'+chr(13)+chr(10)+'start "" /wait /b "'+winepath(debugger)+'" "'+winepath(game)+'"').encode('ascii')
        decorated=batch.parent/'original-process-wait-cpu.cmd'
        adapted=replacement+original[len(prefix):]
        with decorated.open('xb') as stream:stream.write(adapted)
        args[6]=winepath(decorated)
        record['batch']=dict(original=str(batch),sha256=hashlib.sha256(original).hexdigest(),adapted=str(decorated),adapted_sha256=hashlib.sha256(adapted).hexdigest(),scope='Existing start /wait now waits for the CPU debugger, which reports success only after its actual game child exits with code zero and completes all three observations. The original batch and its manifest remain unchanged.')
    else:
        game=Path(args[2]).resolve()
        args=args[:2]+[str(debugger)]+args[2:]
    record.update(game=str(game),game_sha256=hashlib.sha256(game.read_bytes()).hexdigest(),executed_command=args)
    with Path({str(directory.parent / 'debugger-launch.json')!r}).open('x') as out:
        json.dump(record,out,indent=2)
    os.environ['MNM_RASTER_CPU_FAMILY']={str(family)!r}
    if {bool(rgb_diagnostics)!r}:os.environ['MNM_PIXEL_REFUSAL_DIAGNOSTICS']='1'
    else:os.environ.pop('MNM_PIXEL_REFUSAL_DIAGNOSTICS',None)
    if {bool(disable_empty_rgb)!r}:os.environ['MNM_PIXEL_EMPTY_RGB_NATIVE']='0'
    elif {bool(empty_rgb)!r}:os.environ['MNM_PIXEL_EMPTY_RGB_NATIVE']='1'
    else:os.environ.pop('MNM_PIXEL_EMPTY_RGB_NATIVE',None)
    if {bool(rgb_caller_trace)!r}:os.environ['MNM_RASTER_RGB_TRACE']='1'
    else:os.environ.pop('MNM_RASTER_RGB_TRACE',None)
os.execv('/usr/bin/wine',['/usr/bin/wine']+args)
'''
    target = directory / 'wine'
    target.write_text(body)
    target.chmod(0o755)
    return target
