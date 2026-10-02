#!/usr/bin/env python3
"""Read-only checksum verification; optionally check assembly against the PE."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import sys

BASELINE = Path(__file__).resolve().parent.parent / "research/runtime/decompiled/nocd"


def verify(baseline: Path, executable: Path | None = None) -> tuple[int, int]:
    manifest = json.loads((baseline / "manifest.json").read_text())
    for relative, expected in manifest["artifacts"].items():
        path = (baseline / relative).resolve()
        if not path.is_relative_to(baseline.resolve()):
            raise ValueError(f"Manifest path escapes baseline: {relative}")
        actual = hashlib.sha256(path.read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError(f"Baseline checksum mismatch: {relative}")
    instruction_chunks = 0
    if executable is not None:
        data = executable.read_bytes()
        if hashlib.sha256(data).hexdigest() != manifest["executable"]["sha256"]:
            raise ValueError("Unsupported executable hash")
        pe = struct.unpack_from("<I", data, 0x3c)[0]
        count = struct.unpack_from("<H", data, pe + 6)[0]
        optional_size = struct.unpack_from("<H", data, pe + 20)[0]
        image_base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
        sections = []
        for index in range(count):
            section = pe + 24 + optional_size + index * 40
            _, rva, raw_size, raw_offset = struct.unpack_from("<IIII", data, section + 8)
            sections.append((image_base + rva, raw_size, raw_offset))
        for path in (baseline / "assembly").glob("*.asm"):
            for line in path.read_text().splitlines():
                match = re.match(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}(?:[ \t]+|$))+)", line)
                if not match:
                    continue
                address = int(match[1], 16)
                expected = bytes.fromhex(match[2])
                section = next((s for s in sections if s[0] <= address and
                                address + len(expected) <= s[0] + s[1]), None)
                if section is None:
                    raise ValueError(f"Assembly address outside file-backed section: {address:x}")
                offset = section[2] + address - section[0]
                if data[offset:offset + len(expected)] != expected:
                    raise ValueError(f"Assembly bytes disagree with PE: {path.name}:{address:x}")
                instruction_chunks += 1
        if instruction_chunks == 0:
            raise ValueError("No assembly bytes were checked")
    return len(manifest["artifacts"]), instruction_chunks


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, help="also verify assembly bytes against this no-CD PE")
    args = parser.parse_args()
    artifacts, chunks = verify(BASELINE, args.executable)
    print(f"Verified {artifacts} baseline artifacts." +
          (f" Checked {chunks} assembly byte chunks against PE." if args.executable else ""))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, struct.error) as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1)
