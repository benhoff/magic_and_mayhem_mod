#!/usr/bin/env python3
"""Run a reversible plaintext-versus-encrypted CFG precedence probe."""

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


def load_decoder(repo: Path):
    path = repo / "tools/decode-cfg.py"
    spec = importlib.util.spec_from_file_location("mnm_decode_cfg", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Temporarily append a comment to runtime tables.cfg, run a bounded "
            "windowed smoke test, inspect the rewritten encrypted CFG, and restore "
            "the runtime files. Evidence is retained under working/experiments."
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
    plaintext = runtime / "CFG/tables.cfg"
    encrypted = runtime / "CFG/Encrypted/tables.cfg"
    if not plaintext.is_file() or not encrypted.is_file():
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

    evidence_root = repo / "working/experiments/cfg-precedence"
    evidence_root.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    evidence = Path(tempfile.mkdtemp(prefix=f"run-{stamp}-", dir=evidence_root))
    before_plain = plaintext.read_bytes()
    before_encrypted = encrypted.read_bytes()
    shutil.copy2(plaintext, evidence / "tables.before.cfg")
    shutil.copy2(encrypted, evidence / "tables.before.encrypted.cfg")

    marker = f"; MNM_CFG_PRECEDENCE_PROBE={stamp}".encode("ascii")
    separator = b"" if before_plain.endswith((b"\n", b"\r")) else b"\r\n"
    staged_plain = before_plain + separator + marker + b"\r\n"
    plaintext.write_bytes(staged_plain)

    launch = [
        str(repo / "tools/run-game.sh"),
        "smoke",
        "--seconds",
        str(args.seconds),
        "--fullscreen" if args.fullscreen else "--window",
    ]
    launch_status = -1
    marker_found = False
    marker_in_plaintext = False
    decode_error = ""
    after_encrypted = before_encrypted
    after_plain = staged_plain
    try:
        print(f"Evidence directory: {evidence.relative_to(repo)}")
        print(f"Probe marker: {marker.decode()}")
        launch_status = subprocess.run(launch, cwd=repo, check=False).returncode
        after_encrypted = encrypted.read_bytes()
        after_plain = plaintext.read_bytes()
        marker_in_plaintext = marker in after_plain
        (evidence / "tables.after.cfg").write_bytes(after_plain)
        shutil.copy2(encrypted, evidence / "tables.after.encrypted.cfg")
        try:
            decoder = load_decoder(repo)
            _, decoded = decoder.decode(after_encrypted)
            (evidence / "tables.decoded-from-encrypted.cfg").write_bytes(decoded)
            marker_found = marker in decoded
        except Exception as error:  # Preserve evidence even for malformed output.
            decode_error = str(error)
    finally:
        plaintext.write_bytes(before_plain)
        encrypted.write_bytes(before_encrypted)

    changed = after_encrypted != before_encrypted
    if launch_status != 0:
        conclusion = (
            "Inconclusive: Wine exited before the bounded launch completed. See "
            "the corresponding working/logs run directory."
        )
        confirmed = False
    elif not marker_in_plaintext:
        conclusion = (
            "Confirmed: startup replaced the edited plaintext CFG from the "
            "encrypted container; the encrypted copy has precedence."
        )
        confirmed = True
    elif marker_found:
        conclusion = (
            "Confirmed: the game consumed the plaintext CFG and wrote the probe "
            "into the encrypted container."
        )
        confirmed = True
    elif changed:
        conclusion = (
            "Inconclusive: the encrypted container changed, but its decoded bytes "
            "did not contain the probe marker."
        )
        confirmed = False
    else:
        conclusion = (
            "Inconclusive: the encrypted container was not rewritten during the "
            "bounded launch. A normal launch-and-exit test is still required."
        )
        confirmed = False

    report = "\n".join(
        [
            "# CFG precedence experiment",
            "",
            f"- Probe: `{marker.decode()}`",
            f"- Launch exit status: `{launch_status}`",
            f"- Plaintext before SHA-256: `{digest(before_plain)}`",
            f"- Encrypted before SHA-256: `{digest(before_encrypted)}`",
            f"- Encrypted after SHA-256: `{digest(after_encrypted)}`",
            f"- Encrypted container changed: `{str(changed).lower()}`",
            f"- Probe remained in plaintext: `{str(marker_in_plaintext).lower()}`",
            f"- Probe found after decoding: `{str(marker_found).lower()}`",
            f"- Decoder error: `{decode_error or 'none'}`",
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
