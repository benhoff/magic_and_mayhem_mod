#!/usr/bin/env python3
"""Test a generated mode-0 CFG container in the game, then restore it."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def load_encoder(repo: Path):
    path = repo / "tools/encode-cfg.py"
    spec = importlib.util.spec_from_file_location("mnm_encode_cfg", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Install a comment-only mode-0 tables.cfg container, run a bounded "
            "windowed launch, verify its decoded plaintext, and restore both files."
        )
    )
    parser.add_argument(
        "--seconds", type=int, default=20, help="launch duration (default: 20)"
    )
    parser.add_argument(
        "--fullscreen", action="store_true", help="use native fullscreen"
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.seconds < 1:
        raise RuntimeError("--seconds must be at least 1")

    repo = Path(__file__).resolve().parent.parent
    runtime = repo / "working/runtime/game-nocd"
    plaintext_path = runtime / "CFG/tables.cfg"
    encrypted_path = runtime / "CFG/Encrypted/tables.cfg"
    if not plaintext_path.is_file() or not encrypted_path.is_file():
        raise RuntimeError(
            "runtime CFG files are missing; launch the game once with ./run-asahi.sh"
        )
    active = subprocess.run(
        ["pgrep", "-f", "[C]haos.exe"],
        check=False,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if active.returncode == 0:
        raise RuntimeError("Chaos.exe is already running; close it before this experiment")

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    evidence_root = repo / "working/experiments/cfg-writer"
    evidence_root.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix=f"run-{stamp}-", dir=evidence_root))
    before_plain = plaintext_path.read_bytes()
    before_encrypted = encrypted_path.read_bytes()
    shutil.copy2(plaintext_path, evidence / "tables.before.cfg")
    shutil.copy2(encrypted_path, evidence / "tables.before.encrypted.cfg")

    marker = f"; MNM_CFG_WRITER_PROBE={stamp}".encode("ascii")
    separator = b"" if before_plain.endswith((b"\n", b"\r")) else b"\r\n"
    modified_plain = before_plain + separator + marker + b"\r\n"
    encoder = load_encoder(repo)
    decoder = encoder.load_decoder(repo)
    generated, seed = encoder.encode(modified_plain, decoder)
    metadata, round_trip = decoder.decode(generated)
    if round_trip != modified_plain or metadata["mode"] != 0:
        raise RuntimeError("generated container failed validation before launch")
    (evidence / "tables.generated.encrypted.cfg").write_bytes(generated)

    launch_status = -1
    marker_in_plaintext = False
    marker_in_encrypted = False
    plaintext_existed = False
    after_plain = before_plain
    after_encrypted = generated
    decode_error = ""
    try:
        encrypted_path.write_bytes(generated)
        print(f"Evidence directory: {evidence.relative_to(repo)}")
        print(f"Probe marker: {marker.decode()}")
        launch = [
            str(repo / "tools/run-game.sh"),
            "smoke",
            "--seconds",
            str(args.seconds),
            "--fullscreen" if args.fullscreen else "--window",
        ]
        launch_status = subprocess.run(launch, cwd=repo, check=False).returncode
        after_encrypted = encrypted_path.read_bytes()
        (evidence / "tables.after.encrypted.cfg").write_bytes(after_encrypted)
        plaintext_existed = plaintext_path.is_file()
        if plaintext_existed:
            after_plain = plaintext_path.read_bytes()
            marker_in_plaintext = marker in after_plain
            (evidence / "tables.after.cfg").write_bytes(after_plain)
        else:
            after_plain = b""
        try:
            _, decoded_after = decoder.decode(after_encrypted)
            marker_in_encrypted = marker in decoded_after
            (evidence / "tables.decoded-after.cfg").write_bytes(decoded_after)
        except Exception as error:  # Preserve evidence even for malformed output.
            decode_error = str(error)
    finally:
        plaintext_path.write_bytes(before_plain)
        encrypted_path.write_bytes(before_encrypted)

    confirmed = marker_in_plaintext or marker_in_encrypted
    if marker_in_encrypted and not plaintext_existed:
        conclusion = (
            "Confirmed: Chaos.exe accepted the generated mode-0 container, then a "
            "clean shutdown repacked the marker and removed the plaintext CFG."
        )
    elif marker_in_plaintext:
        conclusion = (
            "Confirmed: Chaos.exe accepted the generated mode-0 container and decoded "
            "its marker into the plaintext CFG."
        )
    else:
        conclusion = (
            "Inconclusive: neither the startup plaintext nor the post-launch encrypted "
            "container contained the marker."
        )
    report = "\n".join(
        [
            "# Mode-0 CFG writer experiment",
            "",
            f"- Probe: `{marker.decode()}`",
            f"- Seed: `0x{seed:08x}`",
            f"- Launch exit status: `{launch_status}`",
            f"- Generated container bytes: `{len(generated)}`",
            f"- Plaintext existed after launch: `{str(plaintext_existed).lower()}`",
            f"- Marker found in plaintext: `{str(marker_in_plaintext).lower()}`",
            f"- Marker found in post-launch encrypted container: "
            f"`{str(marker_in_encrypted).lower()}`",
            f"- Post-launch decoder error: `{decode_error or 'none'}`",
            f"- Plaintext before SHA-256: `{digest(before_plain)}`",
            f"- Plaintext after SHA-256: `{digest(after_plain)}`",
            "- Runtime files restored: `true`",
            "",
            conclusion,
            "",
        ]
    )
    (evidence / "report.md").write_text(report, encoding="utf-8")
    print(report)
    print(f"Evidence saved to {evidence.relative_to(repo)}")
    return 0 if confirmed else 2


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError) as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1)
