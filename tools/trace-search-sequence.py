"""Common 54b800 entry/return capture, invoked by trace-route-experiment.py."""
import importlib.util
import json
from pathlib import Path
import re
import time

POINTS = {
    "entry": (0x54b800, bytes.fromhex("81 ec d4 00 00 00")),
    "return": (0x54be45, bytes.fromhex("c2 18 00")),
}


def capture_search(debugger, base, seconds, maximum, evidence, breakpoints):
    spec = importlib.util.spec_from_file_location("world_sequence", Path(__file__).with_name("route-world-snapshot.py"))
    world = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(world)
    live = lambda address: base + address - 0x400000
    signed = lambda n: n - 0x100000000 if n & 0x80000000 else n
    debugger.register("eip")
    for address, expected in POINTS.values():
        if bytes(debugger.memory(live(address), len(expected), 1)) != expected:
            raise RuntimeError(f"Unexpected common-search instruction bytes at {live(address):x}")
    for name, (address, _) in POINTS.items():
        response = debugger.command(f"break *0x{live(address):x}")
        match = re.search(r"Breakpoint\s+(\d+)\s+at", response, re.IGNORECASE)
        if not match:
            raise RuntimeError(f"Could not install common search {name} breakpoint")
        breakpoints.append(int(match[1]))
    print("Common search tracing is armed. Load a map/save and issue movement orders.", flush=True)
    print("World snapshots pause the game; timings are NOT benchmark data.", flush=True)
    deadline = time.monotonic() + seconds
    pending = {}
    contexts_in_flight = set()
    completed = []
    sequence = 0
    with (evidence / "search-calls.jsonl").open("w") as output:
        while len(completed) < maximum:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError("Common search capture deadline reached")
            debugger.command("cont", remaining)
            pc = debugger.register("eip")
            thread, sp = debugger.thread(), debugger.register("esp")
            key = thread, sp
            if pc == live(POINTS["entry"][0]):
                if key in pending:
                    raise RuntimeError("Duplicate common search thread/stack entry")
                stack = debugger.memory(sp, 7)
                context = debugger.register("ecx")
                if context in contexts_in_flight:
                    raise RuntimeError("Overlapping searches reuse the same context; refusing ambiguous sequence")
                sequence += 1
                search = {"context": context, "object": stack[1], "unknown_argument": stack[2],
                          "coordinates": stack[3:6], "budget_pointer": stack[6],
                          "budget_before": signed(debugger.memory(stack[6], 1)[0]),
                          "flag_before": debugger.memory(context + 0x20c, 1, 1)[0]}
                record = {"sequence": sequence, "thread": thread, "entry_sp": sp,
                          "return_address": stack[0], "image_base": base, "search": search,
                          "context_before_hex": world.read_bytes(debugger, context, 0x269, deadline).hex()}
                name = f"world-{sequence:04d}.bin"
                print(f"Capturing search {sequence}, context 0x{context:x}, flag {search['flag_before']}...", flush=True)
                search["world_snapshot"] = world.capture_world(debugger, base, search,
                    evidence / name, deadline, allow_continuation=True)
                pending[key] = record
                contexts_in_flight.add(context)
            elif pc == live(POINTS["return"][0]):
                record = pending.get(key)
                if record is None:
                    # Tracing can begin while an already-entered search is running.
                    print("Skipped unmatched search return from before capture began.", flush=True)
                    continue
                search = record["search"]
                context = search["context"]
                if debugger.memory(sp, 1)[0] != record["return_address"]:
                    raise RuntimeError("Common search return address changed")
                record["snapshot_from_search"] = debugger.snapshot(context)
                record["context_after_hex"] = world.read_bytes(debugger, context, 0x269, deadline).hex()
                record["budget_after"] = signed(debugger.memory(search["budget_pointer"], 1)[0])
                record["flag_after_search"] = debugger.memory(context + 0x20c, 1, 1)[0]
                record["completion"] = len(completed) + 1
                output.write(json.dumps(record) + "\n")
                output.flush()
                completed.append(record)
                del pending[key]
                contexts_in_flight.remove(context)
                print(f"Captured search call {len(completed)}/{maximum}", flush=True)
            else:
                raise RuntimeError(f"Unexpected common search stop at 0x{pc:x}; inspect debugger.log")
    return completed
