"""Preserve an import-extended working install while deriving a pinned source."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
from rts_controls_launch import ROOT, HASH


def original_sections_match(original, candidate):
    if hashlib.sha256(original).hexdigest() != HASH:
        raise ValueError('Unsupported canonical controls image')
    pe = struct.unpack_from('<I',original,60)[0]
    optional = pe+24
    optional_size = struct.unpack_from('<H',original,pe+20)[0]
    table = optional+optional_size
    count = struct.unpack_from('<H',original,pe+6)[0]
    try:
        if (candidate[:2] != b'MZ' or candidate[pe:pe+4] != b'PE\0\0' or
            struct.unpack_from('<I',candidate,60)[0] != pe or
            candidate[optional+28:optional+40] != original[optional+28:optional+40] or
            struct.unpack_from('<H',candidate,pe+20)[0] != optional_size):
            return False
        candidate_count = struct.unpack_from('<H',candidate,pe+6)[0]
        if not count <= candidate_count <= count+16:
            return False
        for i in range(count):
            at = table+i*40
            row = original[at:at+40]
            if candidate[at:at+40] != row:
                return False
            size, offset = struct.unpack_from('<II',row,16)
            if candidate[offset:offset+size] != original[offset:offset+size]:
                return False
        for i in range(count,candidate_count):
            row = candidate[table+i*40:table+(i+1)*40]
            size, offset = struct.unpack_from('<II',row,16)
            if struct.unpack_from('<I',row,36)[0]&0x20000000 or offset+size>len(candidate):
                return False
        return True
    except (struct.error,IndexError):
        return False


def prepare_source(source):
    source = Path(source).resolve()
    if not source.is_relative_to(ROOT/'working') or not source.is_dir():
        raise ValueError('Controls source must be an installation under working/')
    manifest=[ROOT/'tools/original-manifest.sh','verify']
    subprocess.run(manifest,check=True,capture_output=True)
    try:
        original=(ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if hashlib.sha256(original).hexdigest() != HASH:
            raise ValueError('Canonical controls source hash mismatch')
        current=(source/'Chaos.exe').read_bytes()
        current_sha=hashlib.sha256(current).hexdigest()
        if current_sha==HASH:
            return source
        if not original_sections_match(original,current):
            raise ValueError('Working executable has modified original sections; refusing normalization')
        parent=ROOT/'working/experiments/rts-controls-source'
        parent.mkdir(parents=True,exist_ok=True)
        root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game'
        subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
        target=game/'Chaos.exe'
        if (target.is_symlink() or not target.resolve().is_relative_to(game) or
            hashlib.sha256(target.read_bytes()).hexdigest()!=current_sha or
            hashlib.sha256((source/'Chaos.exe').read_bytes()).hexdigest()!=current_sha):
            raise ValueError('Copied controls source changed or redirected')
        # Scripted normalization in the new copy only. All original sections
        # were independently verified byte-for-byte above; old imports are data.
        target.write_bytes(original)
        if hashlib.sha256(target.read_bytes()).hexdigest()!=HASH:
            raise ValueError('Normalized controls source did not verify')
        (root/'manifest.json').write_text(json.dumps({
            'source_game':str(source),'source_executable_sha256':current_sha,
            'canonical_executable_sha256':HASH,'game_copy':str(game),
            'original_sections_unchanged':True,'source_installation_modified':False,
            'scope':'Fresh source copy with canonical executable. Original sections preserved; inherited extra imports removed only from this disposable copy.'
        },indent=2)+'\n')
        return game
    finally:
        subprocess.run(manifest,check=True,capture_output=True)
