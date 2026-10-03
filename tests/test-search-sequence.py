#!/usr/bin/env python3
"""Scripted common-search debugger events; never starts or attaches to a game."""
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest
REPO = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("world_fixtures", REPO / "tests/test-route-world.py")
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)
spec = importlib.util.spec_from_file_location("sequence", REPO / "tools/trace-search-sequence.py")
sequence = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sequence)

class Debugger:
    def __init__(self, events, base=0x400000):
        self.memory_image, _ = fixtures.fixture(base)
        self.events = iter(events)
        self.event = None
        self.base = base
        self.commands = []
        self.breaks = 0
        for address, data in sequence.POINTS.values():
            self.memory_image.put(base + address - 0x400000,data)
    def command(self, command, seconds=10):
        self.commands.append(command)
        if command.startswith("break "):
            self.breaks += 1
            return f"Breakpoint {self.breaks} at something"
        if command == "cont":
            self.event = next(self.events)
        return ""
    def register(self, name):
        if self.event is None:
            return 0
        return {"eip":self.base + sequence.POINTS[self.event['site']][0]-0x400000,
                "esp":self.event['sp'],"ecx":self.event['context']}[name]
    def thread(self):
        return self.event.get("thread",1)
    def memory(self, address, count, width=4):
        event = self.event
        if event is not None:
            if address == event['sp']:
                stack=[event.get('return_address',0x5131b8),0x1200000,0,3,1,1,0x1400000]
                return stack[:count]
            if address == 0x1400000:
                return [event.get('budget',2)]
            context=event['context']
            if context <= address and address+count*width <= context+0x269:
                data=bytearray(0x269)
                data[0x20c]=event.get('flag',1)
                return list(struct.unpack(f"<{count}{'I' if width==4 else 'B'}",
                    data[address-context:address-context+count*width]))
        return self.memory_image.memory(address,count,width)
    def snapshot(self, address):
        return bytes(0x20c).hex()


def event(site, context=0x6c4bd0, sp=0x1500000, **kwargs):
    return {'site':site,'context':context,'sp':sp,**kwargs}

class SequenceTests(unittest.TestCase):
    def test_common_entry_relocation_and_interleaved_returns(self):
        for base in (0x400000,0x500000):
            with self.subTest(base=base), tempfile.TemporaryDirectory() as temporary:
                events=[event('return'),event('entry',0x690148),
                        event('entry',sp=0x1600000,thread=2),
                        event('return',sp=0x1600000,thread=2,budget=0,flag=0),
                        event('return',0x690148,budget=1,flag=1)]
                debugger=Debugger(events,base)
                breaks=[]
                records=sequence.capture_search(debugger,base,10,2,Path(temporary),breaks)
                self.assertEqual(breaks,[1,2])
                self.assertEqual([r['sequence'] for r in records],[2,1])
                self.assertEqual([r['search']['context'] for r in records],[0x6c4bd0,0x690148])
                self.assertEqual(records[0]['search']['flag_before'],1)
                self.assertEqual(records[0]['flag_after_search'],0)
                calls, rows=fixtures.replay.load_sequence(Path(temporary),fixtures.world)
                self.assertEqual([r['sequence'] for r in calls],[1,2])
                self.assertEqual(len(rows),2)
    def test_orphan_continuation_is_recorded_without_inventing_state(self):
        with tempfile.TemporaryDirectory() as temporary:
            debugger=Debugger([event('entry',flag=0),event('return',flag=0,budget=0)])
            records=sequence.capture_search(debugger,0x400000,10,1,Path(temporary),[])
            self.assertEqual(records[0]['search']['flag_before'],0)
            metadata=json.loads((Path(temporary)/'world-0001.json').read_text())
            self.assertEqual(metadata['flag_before'],0)
    def test_reentrant_context_refused(self):
        with tempfile.TemporaryDirectory() as temporary:
            debugger=Debugger([event('entry'),event('entry',sp=0x1600000)])
            with self.assertRaisesRegex(RuntimeError,'Overlapping'):
                sequence.capture_search(debugger,0x400000,10,2,Path(temporary),[])
    def test_instruction_mismatch_before_breakpoints(self):
        with tempfile.TemporaryDirectory() as temporary:
            debugger=Debugger([])
            debugger.memory_image.put(0x54b800,bytes(6))
            breaks=[]
            with self.assertRaisesRegex(RuntimeError,'instruction bytes'):
                sequence.capture_search(debugger,0x400000,10,1,Path(temporary),breaks)
            self.assertEqual(breaks,[])
    def test_changed_return_address_refused(self):
        with tempfile.TemporaryDirectory() as temporary:
            debugger=Debugger([event('entry'),event('return',return_address=0x512852)])
            with self.assertRaisesRegex(RuntimeError,'return address changed'):
                sequence.capture_search(debugger,0x400000,10,1,Path(temporary),[])

if __name__ == '__main__': unittest.main()
