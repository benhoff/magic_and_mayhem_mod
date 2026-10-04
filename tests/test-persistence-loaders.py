#!/usr/bin/env python3
"""Independent codec fixtures and optional installed persistence inputs."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import random
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('cfg_codec', REPO / 'tools/decode-cfg.py')
codec = importlib.util.module_from_spec(spec)
spec.loader.exec_module(codec)


def container(plain, payload, mode, seed, no_cd=False):
    raw = struct.pack('<5I', seed, len(plain), codec.alternating_checksum(payload),
                      codec.alternating_checksum(plain), mode) + payload
    if not no_cd:
        return codec.decrypt_container(raw)  # XOR transform is its own inverse.
    data=bytearray(raw)
    generator=codec.CipherGenerator(seed)
    offset=4
    while offset+4<=len(data):
        struct.pack_into('<I',data,offset,struct.unpack_from('<I',data,offset)[0]^generator.next())
        offset+=4
    for _ in range(len(data)-offset):
        data[offset]^=generator.next() & 255  # No-CD routine does not advance ESI.
    return bytes(data)


def literal_bits(plain):
    bits = ''.join('1' + f'{v:08b}' for v in plain)
    return int(bits.ljust((len(bits)+7)//8*8, '0') or '0', 2).to_bytes((len(bits)+7)//8, 'big')


def inspect(binary, root, kind, path):
    run = subprocess.run([str(binary), str(root), kind, str(path)], capture_output=True, text=True)
    if run.returncode:
        raise AssertionError(f'{kind} {path}: {run.stderr}')
    return json.loads(run.stdout)


def run(binary, installation=None):
    rng = random.Random(1998)
    count = 0
    with tempfile.TemporaryDirectory(prefix='mnm-persistence-') as temp:
        root = Path(temp)
        for size in (0, 1, 2, 3, 4, 7, 8, 15, 127, 128, 250, 4097):
            plain = bytes(rng.randrange(256) for _ in range(size))
            rle = b''.join(bytes([256-len(plain[p:p+128])])+plain[p:p+128] for p in range(0, size, 128))
            for mode, payload in enumerate((plain, rle, literal_bits(plain))):
                packed = container(plain, payload, mode, rng.getrandbits(32))
                (root/'fixture.bin').write_bytes(packed)
                reference, expected = codec.decode(packed)
                actual = inspect(binary, root, 'container', 'fixture.bin')
                assert actual['mode'] == reference['mode']
                assert actual['decoded_bytes'] == len(expected)
                assert actual['decoded_sha256'] == hashlib.sha256(expected).hexdigest()
                count += 1
                (root/'save-container.bin').write_bytes(container(plain,payload,mode,seed=0xdeadbeef,no_cd=True))
                recovered=inspect(binary,root,'save-container','save-container.bin')
                assert recovered['decoded_sha256'] == hashlib.sha256(plain).hexdigest()
        malformed = [
            container(b'A', b'A', 3, 1),
            container(b'A', bytes([2, 65]), 1, 1), # Oversized RLE run.
            container(b'AA', bytes([1]), 1, 1), # Missing repeat byte.
            container(b'A', bytes([254, 65]), 1, 1), # Missing literal byte.
            container(b'AA', b'\0\0', 2, 1), # Early terminator.
            container(b'A'*7, bytes([0xA0, 0x80, 0x05, 0x40]), 2, 1),
        ]
        for packed in malformed:
            (root/'bad.bin').write_bytes(packed)
            failure = subprocess.run([str(binary), str(root), 'container', 'bad.bin'], capture_output=True)
            assert failure.returncode == 2, 'Malformed container accepted'
        # Full version-20 stream generated without the native parser; no actual game save.
        wizard = bytes(0x14a) + struct.pack('<I', 0) + bytes(1000) + struct.pack('<3I', 11, 22, 0) + b'\0'
        decoded = struct.pack('<3I', 0x564153, 20, 123) + bytes(0x200) + wizard*80 + bytes(0x13dc+0x16c+0x18+0x3268) + struct.pack('<3I', 8, 9, 0)
        (root/'synthetic.vas').write_bytes(decoded)
        (root/'synthetic.sav').write_bytes(container(decoded, decoded, 0, 0xfedcba98, no_cd=True))
        vas = inspect(binary, root, 'vas', 'synthetic.vas')
        sav = inspect(binary, root, 'sav', 'synthetic.sav')
        assert {k:v for k,v in vas.items() if k not in ('type','path')} == {k:v for k,v in sav.items() if k not in ('type','path')}
        assert sav['accounting_value'] == 123 and sav['wizard_records'] == 80
        for tail in (b'X', b'XY', b'XYZ'):
            world=decoded[:-4]+struct.pack('<I',0x17)+bytes(24)+tail
            (root/'world.sav').write_bytes(container(world,world,0,0x1234,no_cd=True))
            recovered=inspect(binary,root,'sav','world.sav')
            assert recovered['opaque_world_bytes'] == len(tail)
            assert recovered['world_sha256'] == hashlib.sha256(tail).hexdigest()
        (root/'duplicate.cfg').write_text('[A]\nx=1\nX=2\n')
        rejected = subprocess.run([str(binary), str(root), 'cfg', 'duplicate.cfg'], capture_output=True)
        assert rejected.returncode == 2
    result = {'synthetic_container_matches': count, 'synthetic_no_cd_container_matches': count, 'synthetic_save_layers_match': True}
    if installation:
        containers = sorted((installation/'CFG/Encrypted').glob('*.cfg'))
        assert containers, 'No installed CFGs'
        container_evidence=[]
        for path in containers:
            _, expected = codec.decode(path.read_bytes())
            actual = inspect(binary, installation, 'container', path.relative_to(installation))
            assert actual['decoded_sha256'] == hashlib.sha256(expected).hexdigest()
            parsed=inspect(binary, installation, 'packed-cfg', path.relative_to(installation))
            assert parsed['sections']
            container_evidence.append({'path':str(path.relative_to(installation)), 'input_sha256':hashlib.sha256(path.read_bytes()).hexdigest(), 'decoded_sha256':actual['decoded_sha256'], 'annotations':parsed['annotation_lines']})
        realms=[]
        for name, w, r in [('Celtic',9,10),('Greek',13,15),('Medieval',17,17)]:
            info=inspect(binary,installation,'realm',f'Realms/{name}/RealmView.cfg')
            assert (info['wizards'],info['regions']) == (w,r)
            info['input_sha256']=hashlib.sha256((installation/info['path']).read_bytes()).hexdigest()
            realms.append(info)
        names=[]
        for path in sorted((installation/'Realms').glob('*/*regionNames.txt')):
            info=inspect(binary,installation,'regions',path.relative_to(installation))
            assert info['lines'] == len(path.read_bytes().splitlines())
            info['input_sha256']=hashlib.sha256(path.read_bytes()).hexdigest()
            names.append(info)
        result.update(installed_container_matches=len(containers), containers=container_evidence, realms=realms, region_names=names)
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary',type=Path)
    parser.add_argument('--installation',type=Path)
    args=parser.parse_args()
    if args.installation:
        subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True,stdout=subprocess.DEVNULL)
    try:
        print(json.dumps(run(args.binary.resolve(), args.installation.resolve() if args.installation else None),indent=2))
    finally:
        if args.installation:
            subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True,stdout=subprocess.DEVNULL)

if __name__=='__main__':
    main()
