#!/usr/bin/env python3
"""Encode plaintext as a deterministic mode-0 Magic & Mayhem CFG container."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import os
import struct
import sys
from pathlib import Path


class EncodeError(RuntimeError):
    pass


def load_decoder(repo: Path):
    path = repo / "tools/decode-cfg.py"
    spec = importlib.util.spec_from_file_location("mnm_decode_cfg", path)
    if spec is None or spec.loader is None:
        raise EncodeError(f"could not load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def default_seed(plaintext: bytes) -> int:
    """Make identical input produce identical output."""
    return int.from_bytes(hashlib.sha256(plaintext).digest()[:4], "little")


def encode(plaintext: bytes, decoder, seed: int | None = None) -> tuple[bytes, int]:
    if len(plaintext) > 0xFFFFFFFF:
        raise EncodeError("input is too large for the 32-bit CFG length field")
    chosen_seed = default_seed(plaintext) if seed is None else seed
    if not 0 <= chosen_seed <= 0xFFFFFFFF:
        raise EncodeError("seed must fit in an unsigned 32-bit integer")
    checksum = decoder.alternating_checksum(plaintext)
    clear = struct.pack(
        "<5I", chosen_seed, len(plaintext), checksum, checksum, 0
    ) + plaintext
    container = decoder.decrypt_container(clear)
    return container, chosen_seed


def confirm_overwrite(path: Path, force: bool) -> None:
    if not path.exists() or force:
        return
    if not sys.stdin.isatty():
        raise EncodeError(f"output exists: {path}; use --force to replace it")
    answer = input(f"Overwrite {path}? [y/N] ")
    if answer.lower() not in {"y", "yes"}:
        raise EncodeError("encode cancelled")


def parse_seed(value: str) -> int:
    try:
        return int(value, 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError("seed must be an integer") from error


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Encode a plaintext CFG using uncompressed mode 0. With no arguments, "
            "preview and round-trip-check runtime tables.cfg without writing."
        )
    )
    parser.add_argument(
        "input",
        nargs="?",
        type=Path,
        help="plaintext CFG (default: working/runtime/game-nocd/CFG/tables.cfg)",
    )
    parser.add_argument("-o", "--output", type=Path, help="container output path")
    parser.add_argument(
        "--seed", type=parse_seed, help="32-bit seed; default is derived from SHA-256"
    )
    parser.add_argument(
        "-f", "--force", action="store_true", help="replace an existing output"
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo = Path(__file__).resolve().parent.parent
    source = args.input or repo / "working/runtime/game-nocd/CFG/tables.cfg"
    if not source.is_absolute():
        source = Path.cwd() / source
    if not source.is_file():
        raise EncodeError(f"input is not a file: {source}")

    decoder = load_decoder(repo)
    plaintext = source.read_bytes()
    container, seed = encode(plaintext, decoder, args.seed)
    metadata, round_trip = decoder.decode(container)
    if round_trip != plaintext:
        raise EncodeError("internal round-trip validation failed")

    print(f"input={source}")
    print(f"plaintext_bytes={len(plaintext)}")
    print(f"container_bytes={len(container)}")
    print(f"mode={metadata['mode']}")
    print(f"seed=0x{seed:08x}")
    print("round_trip=valid")

    if args.output:
        target = args.output
        if not target.is_absolute():
            target = Path.cwd() / target
        confirm_overwrite(target, args.force)
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target.with_name(f".{target.name}.tmp-{os.getpid()}")
        temporary.write_bytes(container)
        temporary.replace(target)
        print(f"output={target}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (EncodeError, OSError) as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1)
