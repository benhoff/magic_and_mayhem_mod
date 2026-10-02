#!/usr/bin/env python3
"""Bounded WineDbg route capture; no on-disk executable patch or gameplay edit."""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import pty
import platform
import re
import select
import shutil
import signal
import struct
import subprocess
import sys
import tempfile
import time
import tty

REPO = Path(__file__).resolve().parent.parent
HASH = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"
POINTS = {
    "entry": (0x512800, bytes.fromhex("51 8b 54 24 10")),
    "search_call": (0x51284d, bytes.fromhex("e8 ae 8f 03 00")),
    "search_done": (0x512852, bytes.fromhex("8d bd 6b 09 00 00")),
    "return": (0x512892, bytes.fromhex("c2 10 00")),
}
ANSI = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")


def signed(value: int) -> int:
    return value - 0x100000000 if value & 0x80000000 else value


def module_base(text: str) -> int:
    matches = re.findall(r"(?im)^\s*PE\s+([0-9a-f]+)-\s*[0-9a-f]+[^\n]*\bchaos(?:\.exe)?\s*$", text)
    if len(matches) != 1:
        raise RuntimeError("Cannot identify one loaded Chaos PE module; refusing breakpoints")
    return int(matches[0], 16)


def memory_values(text: str, address: int, count: int, width: int) -> list[int]:
    values = []
    expected_address = address
    for line in ANSI.sub("", text).splitlines():
        match = re.match(r"\s*(?:0x)?([0-9a-fA-F]{4,16}):\s+(.*)", line)
        if not match or int(match[1], 16) != expected_address:
            continue
        tokens = match[2].split()
        for token in tokens:
            if not re.fullmatch(r"(?:0x)?[0-9a-fA-F]{%d}" % (width * 2), token):
                break
            values.append(int(token, 16))
            expected_address += width
    if len(values) != count:
        raise RuntimeError(f"Expected {count} memory values at {address:x}, got {len(values)}")
    return values


class Debugger:
    def __init__(self, command: list[str], cwd: Path, environment: dict, evidence: Path):
        self.master, slave = pty.openpty()
        # Prevent the Unix terminal from translating CR into LF (^J in Wine).
        tty.setraw(slave)
        self.log = (evidence / "debugger.log").open("wb")
        self.commands = (evidence / "commands.log").open("w")
        self.buffer = b""
        self.stopped = False
        try:
            self.process = subprocess.Popen(command, cwd=cwd, env=environment,
                stdin=slave, stdout=slave, stderr=slave, start_new_session=True)
        except BaseException:
            os.close(self.master)
            self.log.close()
            self.commands.close()
            raise
        finally:
            os.close(slave)

    def prompt(self, seconds: float) -> str:
        deadline = time.monotonic() + seconds
        while True:
            match = re.search(rb"Wine-dbg>\s*", self.buffer)
            if match:
                data, self.buffer = self.buffer[:match.start()], self.buffer[match.end():]
                self.stopped = True
                return ANSI.sub("", data.decode(errors="replace"))
            if time.monotonic() >= deadline:
                raise TimeoutError("Timed out waiting for WineDbg prompt")
            ready, _, _ = select.select([self.master], [], [], min(0.2, deadline - time.monotonic()))
            if ready:
                try:
                    data = os.read(self.master, 65536)
                except OSError:
                    data = b""
                if not data:
                    raise RuntimeError("WineDbg exited before completing the capture")
                self.log.write(data)
                self.log.flush()
                self.buffer += data
            elif self.process.poll() is not None:
                raise RuntimeError(f"WineDbg exited with status {self.process.returncode}")

    def command(self, command: str, seconds: float = 10) -> str:
        self.commands.write(command + "\n")
        self.commands.flush()
        # Wine's console input treats CR as Enter; LF is displayed as ^J.
        os.write(self.master, (command + "\r").encode())
        if command == "cont":
            self.stopped = False
        return self.prompt(seconds)

    def register(self, name: str) -> int:
        text = self.command(f"print /x ${name}")
        # Ignore the PTY's command echo and require a standalone value.
        lines = [line.strip() for line in text.splitlines() if not line.startswith("print ")]
        for line in reversed(lines):
            match = re.fullmatch(r"(?:0x)?([0-9a-fA-F]{1,16})", line)
            if match:
                value = int(match[1], 16)
                if value <= 0xffffffff:
                    return value
        raise RuntimeError(f"Cannot read 32-bit ${name}; runtime may not expose x86 context")

    def memory(self, address: int, count: int, width: int = 4) -> list[int]:
        return memory_values(self.command(f"x /{count}{'b' if width == 1 else 'x'} 0x{address:x}"),
                             address, count, width)

    def thread(self) -> int:
        text = self.command("info thread")
        match = re.search(r"(?m)^\s*([0-9a-fA-F]{8})\s+[^\n]*<==", text)
        if not match:
            raise RuntimeError("Cannot identify stopped thread; refusing to pair unrelated calls")
        return int(match[1], 16)

    def snapshot(self, address: int) -> str:
        return struct.pack("<131I", *self.memory(address, 131)).hex()

    def cleanup(self, breakpoints: list[int], keep_game: bool) -> str:
        try:
            if self.process.poll() is not None:
                return "debugger_exited"
            if not self.stopped:
                os.killpg(self.process.pid, signal.SIGINT)
                self.prompt(5)
            for number in breakpoints:
                self.command(f"delete {number}")
            # Never leave a failed/paused experiment's game behind. A successful
            # capture detaches and lets the user exit normally instead.
            self.command("detach" if keep_game else "kill")
            os.write(self.master, b"quit\r")
            self.process.wait(timeout=5)
            return "breakpoints_removed_and_detached" if keep_game else "owned_game_terminated"
        except (OSError, RuntimeError, TimeoutError, subprocess.TimeoutExpired):
            # Only the process group spawned by this experiment; never wineserver
            # or other Wine applications. Do not leave an unresponsive traced game.
            if self.process.poll() is None:
                os.killpg(self.process.pid, signal.SIGTERM)
                try:
                    self.process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(self.process.pid, signal.SIGKILL)
                    self.process.wait(timeout=5)
            return "owned_debugger_group_terminated_cleanup_unverified"
        finally:
            os.close(self.master)
            self.log.close()
            self.commands.close()


def capture(debugger: Debugger, base: int, seconds: int, maximum: int,
            evidence: Path, breakpoints: list[int]) -> list[dict]:
    live = lambda address: base + address - 0x400000
    debugger.register("eip") # refuse non-x86 debug contexts before setting breakpoints
    for _, (address, expected) in POINTS.items():
        if bytes(debugger.memory(live(address), len(expected), 1)) != expected:
            raise RuntimeError(f"Unexpected loaded instruction bytes at {live(address):x}")
    for name, (address, _) in POINTS.items():
        text = debugger.command(f"break *0x{live(address):x}")
        match = re.search(r"Breakpoint\s+(\d+)\s+at", text, re.IGNORECASE)
        if not match:
            raise RuntimeError(f"Could not install {name} breakpoint: {text.strip()}")
        breakpoints.append(int(match[1]))
    print("Tracing is armed. Load a map/save and issue movement orders; AI calls are also sampled.", flush=True)
    print("Capture pauses the game briefly at each site; timings are NOT benchmark data.", flush=True)
    deadline = time.monotonic() + seconds
    pending: dict[tuple[int, int], dict] = {}
    completed = []
    with (evidence / "calls.jsonl").open("w") as trace:
        while len(completed) < maximum:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError("Capture deadline reached")
            debugger.command("cont", remaining)
            pc = debugger.register("eip")
            site = next((name for name, (address, _) in POINTS.items() if live(address) == pc), None)
            if site is None:
                raise RuntimeError(f"Unexpected stop at {pc:x}; inspect debugger.log before proceeding")
            thread = debugger.thread()
            sp = debugger.register("esp")
            entry_sp = sp + {"entry": 0, "search_call": 40, "search_done": 16, "return": 0}[site]
            key = thread, entry_sp
            if site == "entry":
                obj = debugger.register("ecx")
                stack = debugger.memory(sp, 5)
                pending[key] = {"thread": thread, "entry_sp": sp, "object": obj,
                    "return_address": stack[0], "arguments": stack[1:],
                    "dimensions": [signed(debugger.memory(live(0x6c5494), 1)[0]),
                                   signed(debugger.memory(live(0x6c5498), 1)[0])],
                    "configured_budget": signed(debugger.memory(live(0x5e174c), 1)[0]),
                    "snapshot_before": debugger.snapshot(obj + 0x96b)}
                continue
            record = pending.get(key)
            if record is None:
                raise RuntimeError(f"Unpaired {site} stop for thread/stack {key}")
            if site == "search_call":
                stack = debugger.memory(sp, 6)
                record["search"] = {"context": debugger.register("ecx"),
                    "object": stack[0], "unknown_argument": stack[1], "coordinates": stack[2:5],
                    "budget_pointer": stack[5], "budget_before": signed(debugger.memory(stack[5], 1)[0]),
                    "flag_before": debugger.memory(live(0x690354), 1, 1)[0]}
            elif site == "search_done":
                if "search" not in record:
                    raise RuntimeError("Missing search-call observation")
                record["snapshot_from_search"] = debugger.snapshot(live(0x690148))
                record["budget_after"] = signed(debugger.memory(record["search"]["budget_pointer"], 1)[0])
                record["flag_after_search"] = debugger.memory(live(0x690354), 1, 1)[0]
            else:
                if "snapshot_from_search" not in record:
                    raise RuntimeError("Missing search-result observation")
                obj = record["object"]
                record["return"] = {"eax": debugger.register("eax"),
                    "snapshot": debugger.snapshot(obj + 0x96b),
                    "route_present": debugger.memory(obj + 0xb8b, 1)[0],
                    "unknown_d03": debugger.memory(obj + 0xd03, 1)[0],
                    "configured_budget": signed(debugger.memory(live(0x5e174c), 1)[0]),
                    "flag": debugger.memory(live(0x690354), 1, 1)[0]}
                record["image_base"] = base
                trace.write(json.dumps(record) + "\n")
                trace.flush()
                completed.append(record)
                del pending[key]
                print(f"Captured route call {len(completed)}/{maximum}", flush=True)
    return completed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seconds", type=int, default=180, help="capture deadline (default: 180)")
    parser.add_argument("--calls", type=int, default=10, help="complete-call limit (default: 10)")
    parser.add_argument("--debugger", help="WineDbg launcher or PE debugger executable")
    parser.add_argument("--prepare-only", action="store_true", help="stage evidence without launching")
    args = parser.parse_args()
    if args.seconds < 1 or args.calls < 1:
        raise ValueError("--seconds and --calls must be positive")
    source = REPO / "working/game-nocd"
    if hashlib.sha256((source / "Chaos.exe").read_bytes()).hexdigest() != HASH:
        raise RuntimeError("No-CD executable hash mismatch; refusing build-specific breakpoints")
    debugger_path = args.debugger or shutil.which("winedbg")
    if args.debugger is None and platform.machine() in ("aarch64", "arm64"):
        # Use Wine's i386 debugger for the i386 target rather than ARM host registers.
        for candidate in (Path("/usr/lib64/wine/i386-windows/winedbg.exe"),
                          Path("/usr/lib/wine/i386-windows/winedbg.exe")):
            if candidate.is_file() and shutil.which("wine"):
                debugger_path = str(candidate)
                break
    if not debugger_path and not args.prepare_only:
        raise RuntimeError("WineDbg not found; install it or pass --debugger PATH")
    active = subprocess.run(["pgrep", "-f", "[C]haos.exe"], stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL, check=False)
    if active.returncode == 0:
        raise RuntimeError("Chaos.exe is already running; close it before tracing")
    root = REPO / "working/experiments/route-trace"
    root.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix="run-", dir=root))
    game = evidence / "game"
    subprocess.run(["cp", "-a", "--reflink=auto", str(source), str(game)], check=True)
    if hashlib.sha256((game / "Chaos.exe").read_bytes()).hexdigest() != HASH:
        raise RuntimeError("Disposable game-copy hash mismatch")
    manifest = {"sha256": HASH, "utc": datetime.now(timezone.utc).isoformat(),
                "status": "prepared", "calls_limit": args.calls, "seconds": args.seconds,
                "scenario": "unclassified; human scenario labels not inferred",
                "game_copy": str(game), "cleanup": "not_started"}
    print(f"Evidence directory: {evidence}", flush=True)
    if args.prepare_only:
        (evidence / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        return 0
    environment = os.environ.copy()
    environment["WINEPREFIX"] = str(REPO / "working/wineprefix")
    environment["WINEDEBUG"] = "fixme-all"
    if debugger_path.endswith(".exe"):
        # Otherwise Wine may substitute its ARM built-in winedbg for the named
        # i386 PE, leaving only ARM register names visible to the controller.
        environment["WINEDLLOVERRIDES"] = (
            environment.get("WINEDLLOVERRIDES", "") + ";winedbg=n").lstrip(";")
    command = ([shutil.which("wine"), debugger_path] if debugger_path.endswith(".exe")
               else [debugger_path]) + [str(game / "Chaos.exe")]
    manifest["command"] = command
    debugger = None
    breakpoints: list[int] = []
    error = ""
    try:
        (evidence / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        debugger = Debugger(command, game, environment, evidence)
        debugger.prompt(30)
        base = module_base(debugger.command("info share"))
        manifest["image_base"] = base
        capture(debugger, base, args.seconds, args.calls, evidence, breakpoints)
        manifest["status"] = "captured"
    except (OSError, RuntimeError, TimeoutError, KeyboardInterrupt) as exception:
        error = str(exception) or "Interrupted"
        manifest["status"] = "incomplete"
        manifest["error"] = error
    finally:
        if debugger is not None:
            manifest["cleanup"] = debugger.cleanup(breakpoints, keep_game=not bool(error))
        manifest["input_unchanged"] = hashlib.sha256((source / "Chaos.exe").read_bytes()).hexdigest() == HASH
        manifest["copy_executable_unchanged"] = hashlib.sha256((game / "Chaos.exe").read_bytes()).hexdigest() == HASH
        (evidence / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    if (evidence / "calls.jsonl").exists() and (evidence / "calls.jsonl").stat().st_size:
        result = subprocess.run([sys.executable, str(REPO / "tools/validate-route-trace.py"),
                                 str(evidence)], check=False)
        if result.returncode:
            return result.returncode
    if error:
        print(f"Inconclusive: {error}; inspect {evidence / 'debugger.log'}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1)
