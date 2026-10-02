#!/usr/bin/env python3
"""Inspect or decode Magic & Mayhem's encrypted CFG containers."""

from __future__ import annotations

import argparse
import os
import struct
import sys
from pathlib import Path


MASK32 = 0xFFFFFFFF
DEFAULT_INPUT = Path("working/game-clean/CFG/Encrypted")


class FormatError(ValueError):
    pass


class CipherGenerator:
    """Generator used by Chaos.exe at 0x00538770/0x00538800."""

    def __init__(self, seed: int) -> None:
        self.state = [0] * 250
        value = seed
        for index in range(249, -1, -1):
            product = value * 0x41C64E6D
            low = product & MASK32
            high = ((product >> 32) << 16) & MASK32
            old_low = low
            low = (low + 0x3039) & MASK32
            high = (high + 0xFFFF + (low < old_low)) & MASK32
            value = low
            self.state[index] = (low >> 16) | (high & 0xFFFF0000)

        bit = 0x80000000
        keep = MASK32
        for index in range(3, 224, 7):
            self.state[index] = (self.state[index] & keep) | bit
            bit >>= 1
            keep >>= 1

        self.left = 0
        self.right = 103

    def next(self) -> int:
        value = self.state[self.left] ^ self.state[self.right]
        self.state[self.left] = value
        self.left = (self.left + 1) % 250
        self.right = (self.right + 1) % 250
        return value


class BitReader:
    def __init__(self, data: bytes) -> None:
        self.data = data
        self.offset = 0
        self.mask = 0x80

    def bit(self) -> int:
        if self.offset >= len(self.data):
            raise FormatError("compressed bitstream ended early")
        value = int(bool(self.data[self.offset] & self.mask))
        self.mask >>= 1
        if self.mask == 0:
            self.mask = 0x80
            self.offset += 1
        return value

    def integer(self, width: int) -> int:
        value = 0
        for _ in range(width):
            value = (value << 1) | self.bit()
        return value


def alternating_checksum(data: bytes) -> int:
    """XOR even and add odd complete little-endian words, as Chaos.exe does."""
    result = 0
    for index in range(len(data) // 4):
        value = struct.unpack_from("<I", data, index * 4)[0]
        if index & 1:
            result = (result + value) & MASK32
        else:
            result ^= value
    return result


def decrypt_container(data: bytes) -> bytes:
    if len(data) < 20:
        raise FormatError("container is too short")
    result = bytearray(data)
    generator = CipherGenerator(struct.unpack_from("<I", result)[0])
    offset = 4
    while offset + 4 <= len(result):
        value = struct.unpack_from("<I", result, offset)[0] ^ generator.next()
        struct.pack_into("<I", result, offset, value)
        offset += 4
    while offset < len(result):
        result[offset] ^= generator.next() & 0xFF
        offset += 1
    return bytes(result)


def decode_rle(payload: bytes, expected_size: int) -> bytes:
    output = bytearray()
    offset = 0
    while offset < len(payload) and len(output) < expected_size:
        control = struct.unpack_from("<b", payload, offset)[0]
        offset += 1
        if control > 0:
            if offset >= len(payload):
                raise FormatError("RLE repeat is missing its byte")
            output.extend(payload[offset : offset + 1] * control)
            offset += 1
        elif control < 0:
            count = -control
            if offset + count > len(payload):
                raise FormatError("RLE literal extends past the payload")
            output.extend(payload[offset : offset + count])
            offset += count
    return bytes(output[:expected_size])


def decode_lzss(payload: bytes, expected_size: int) -> bytes:
    reader = BitReader(payload)
    ring = bytearray(4096)
    write_position = 1
    output = bytearray()
    while len(output) < expected_size:
        if reader.bit():
            value = reader.integer(8)
            output.append(value)
            ring[write_position] = value
            write_position = (write_position + 1) & 0xFFF
            continue

        read_position = reader.integer(12)
        if read_position == 0:
            break
        count = reader.integer(4) + 2
        for delta in range(count):
            value = ring[(read_position + delta) & 0xFFF]
            output.append(value)
            ring[write_position] = value
            write_position = (write_position + 1) & 0xFFF
            if len(output) == expected_size:
                break
    return bytes(output)


def decode(data: bytes) -> tuple[dict[str, int | bool], bytes]:
    clear = decrypt_container(data)
    seed, original_size, packed_checksum, plain_checksum, mode = struct.unpack_from(
        "<5I", clear
    )
    payload = clear[20:]
    if alternating_checksum(payload) != packed_checksum:
        raise FormatError("packed-data checksum does not match")

    if mode == 0:
        output = payload[:original_size]
    elif mode == 1:
        output = decode_rle(payload, original_size)
    elif mode == 2:
        output = decode_lzss(payload, original_size)
    else:
        raise FormatError(f"unsupported compression mode {mode}")

    if len(output) != original_size:
        raise FormatError(f"decoded {len(output)} bytes; expected {original_size}")
    if alternating_checksum(output) != plain_checksum:
        raise FormatError("decoded-data checksum does not match")
    return {
        "seed": seed,
        "original_size": original_size,
        "packed_size": len(payload),
        "packed_checksum": packed_checksum,
        "plain_checksum": plain_checksum,
        "mode": mode,
        "valid": True,
    }, output


def default_inputs(repo: Path) -> list[Path]:
    directory = repo / DEFAULT_INPUT
    if not directory.is_dir():
        raise FormatError(
            f"default input is missing: {directory}; run tools/prepare-working.sh first"
        )
    return sorted(directory.glob("*.cfg"), key=lambda path: path.name.lower())


def confirm_overwrites(paths: list[Path], assume_yes: bool) -> None:
    existing = [path for path in paths if path.exists()]
    if not existing or assume_yes:
        return
    if not sys.stdin.isatty():
        names = ", ".join(str(path) for path in existing[:3])
        raise FormatError(f"output exists ({names}); use --force to replace it")
    answer = input(f"Overwrite {len(existing)} existing decoded file(s)? [y/N] ")
    if answer.lower() not in {"y", "yes"}:
        raise FormatError("decode cancelled")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Validate encrypted CFG files. With no arguments, inspect every CFG in "
            "working/game-clean/CFG/Encrypted without writing files."
        )
    )
    parser.add_argument("inputs", nargs="*", type=Path, help="container file(s)")
    parser.add_argument(
        "-o", "--output-dir", type=Path, help="write decoded files to this directory"
    )
    parser.add_argument(
        "-f", "--force", action="store_true", help="replace existing decoded files"
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo = Path(__file__).resolve().parent.parent
    inputs = args.inputs or default_inputs(repo)
    inputs = [path if path.is_absolute() else Path.cwd() / path for path in inputs]
    if not inputs:
        raise FormatError("no CFG containers found")

    decoded: list[tuple[Path, dict[str, int | bool], bytes]] = []
    for path in inputs:
        if not path.is_file():
            raise FormatError(f"input is not a file: {path}")
        metadata, output = decode(path.read_bytes())
        decoded.append((path, metadata, output))

    print("file\tmode\tpacked\tdecoded\tseed\tstatus")
    for path, metadata, _ in decoded:
        print(
            f"{path.name}\t{metadata['mode']}\t{metadata['packed_size']}\t"
            f"{metadata['original_size']}\t0x{metadata['seed']:08x}\tvalid"
        )

    if args.output_dir:
        output_dir = args.output_dir
        if not output_dir.is_absolute():
            output_dir = Path.cwd() / output_dir
        targets = [output_dir / path.name for path, _, _ in decoded]
        confirm_overwrites(targets, args.force)
        output_dir.mkdir(parents=True, exist_ok=True)
        for target, (_, _, output) in zip(targets, decoded, strict=True):
            temporary = target.with_name(f".{target.name}.tmp-{os.getpid()}")
            temporary.write_bytes(output)
            temporary.replace(target)
        print(f"Decoded {len(targets)} file(s) into {output_dir}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except FormatError as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1)
