#!/usr/bin/env python3
"""Print a deterministic extension and signature inventory for a game tree."""

from __future__ import annotations

import argparse
import collections
import hashlib
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Inventory working/game-clean by default; this command is read-only."
    )
    parser.add_argument(
        "directory", nargs="?", type=Path, help="game tree (default: working/game-clean)"
    )
    return parser.parse_args()


def extension(path: Path) -> str:
    return path.suffix[1:].lower() if path.suffix else "[none]"


def signature(path: Path) -> str:
    data = path.read_bytes()[:16]
    if not data:
        return "empty"
    known = {
        b"BM": "BMP",
        b"RIFF": "RIFF",
        b"MZ": "PE/DOS",
        b"\xff\xd8\xff": "JPEG",
    }
    for prefix, name in known.items():
        if data.startswith(prefix):
            return name
    if len(data) >= 4 and data[0] == 0x0A and data[1] in {0, 2, 3, 5} and data[2] == 1:
        return "PCX"
    if all(byte in b"\t\n\r" or 32 <= byte < 127 for byte in data):
        return "text"
    return data[:8].hex()


def main() -> int:
    args = parse_args()
    repo = Path(__file__).resolve().parent.parent
    root = args.directory or repo / "working/game-clean"
    if not root.is_absolute():
        root = Path.cwd() / root
    if not root.is_dir():
        raise SystemExit(f"Error: directory does not exist: {root}")

    files = sorted((path for path in root.rglob("*") if path.is_file()))
    groups: dict[str, list[Path]] = collections.defaultdict(list)
    for path in files:
        groups[extension(path)].append(path)

    print(f"# Game file inventory: `{root}`\n")
    print(f"Files: {len(files)}  ")
    print(f"Bytes: {sum(path.stat().st_size for path in files)}\n")
    print("| Extension | Files | Bytes | Signatures | Example |")
    print("|---|---:|---:|---|---|")
    for suffix, paths in sorted(groups.items(), key=lambda item: (-len(item[1]), item[0])):
        signatures = collections.Counter(signature(path) for path in paths)
        signature_text = ", ".join(
            f"{name} ({count})" for name, count in signatures.most_common(3)
        )
        example = paths[0].relative_to(root).as_posix().replace("|", "\\|")
        print(
            f"| `{suffix}` | {len(paths)} | "
            f"{sum(path.stat().st_size for path in paths)} | {signature_text} | `{example}` |"
        )

    digest = hashlib.sha256()
    for path in files:
        relative = path.relative_to(root).as_posix().encode()
        digest.update(relative + b"\0" + str(path.stat().st_size).encode() + b"\n")
    print(f"\nPath/size inventory SHA-256: `{digest.hexdigest()}`")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
