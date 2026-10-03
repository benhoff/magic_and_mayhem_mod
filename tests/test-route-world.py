#!/usr/bin/env python3
"""Capture/parser/replay integration using explicitly synthetic frozen memory."""
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
REPO = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("world", REPO / "tools/route-world-snapshot.py")
world = importlib.util.module_from_spec(spec)
spec.loader.exec_module(world)

class Memory:
    def __init__(self, image_base=0x400000):
        self.regions = {}
        self.image_base = image_base
    def put(self, address, data):
        self.regions[address] = bytes(data)
    def integer(self, address, value):
        self.put(address, struct.pack("<I", value & 0xffffffff))
    def memory(self, address, count, width=4):
        for base, data in self.regions.items():
            if base <= address and address + count * width <= base + len(data):
                return list(struct.unpack(f"<{count}{'I' if width == 4 else 'B'}",
                    data[address-base:address-base+count*width]))
        raise AssertionError(f"Unmapped fixture read: {address:x}/{count}/{width}")

def fixture(base=0x400000):
    m = Memory(base)
    live = lambda va: base + va - 0x400000
    for va, value in [(0x6c5494,6),(0x6c5498,6),(0x5e1780,3),(0x6c54a0,36),
                      (0x6c54dc,0x1000000),(0x65660c,0x1100000),(0x6c5c80,1),
                      (0x6c007d,720)]:
        m.integer(live(va), value)
    m.put(live(0x5e15f0), struct.pack("<f",0.969))
    m.put(live(0x6cb942),struct.pack("<6i",0,6,12,18,24,30))
    m.put(live(0x6cb8c2),struct.pack("<3i",0,36,72))
    cells = bytearray(108*12)
    for i in range(108):
        struct.pack_into("<H",cells,i*12+4,0xffff)
        if i < 36:
            struct.pack_into("<H",cells,i*12,1)
    m.put(0x1000000,cells)
    terrain = bytearray(2*0x164)
    struct.pack_into("<i",terrain,0x164+0x94,16)
    terrain[0x164+0xb0] = 8
    m.put(0x1100000,terrain)
    obj = bytearray(0xd07)
    struct.pack_into("<3i",obj,8,1,1,1)
    struct.pack_into("<I",obj,0xac,0x1300000)
    m.put(0x1200000,obj)
    scalar = bytearray(0x198)
    struct.pack_into("<3i",scalar,8,1,1,500)
    for i in range(48):
        struct.pack_into("<I",scalar,0xd8+i*4,40 if (i//12)%2 else 60)
    m.put(0x1300000,scalar)
    generator = bytearray(0x5c9)
    struct.pack_into("<i",generator,0x589,720)
    m.put(live(0x6a5f80),generator)
    return m, {"object":0x1200000,"budget_before":300,"unknown_argument":0,"coordinates":[3,1,1]}

replay_spec = importlib.util.spec_from_file_location("replay", REPO / "tools/replay-route-world.py")
replay = importlib.util.module_from_spec(replay_spec)
replay_spec.loader.exec_module(replay)

class WorldTests(unittest.TestCase):
    def test_reference_comparison_including_unused_bytes(self):
        raw = bytes(0x20c)
        report = {"snapshot_hex":raw.hex(),"budget_remaining":280,"flag":1}
        reference = {"snapshot_from_search":raw.hex(),"budget_after":280,"flag_after_search":1}
        self.assertEqual(replay.compare_reference(report,None)["status"],"no_reference")
        self.assertEqual(replay.compare_reference(report,reference)["status"],"match")
        changed = bytearray(raw)
        changed[-1] = 1
        reference["snapshot_from_search"] = changed.hex()
        reference["budget_after"] = 279
        reference["flag_after_search"] = 0
        result = replay.compare_reference(report,reference)
        self.assertEqual(result["status"],"mismatch")
        self.assertEqual(result["snapshot_byte_offsets"],[0x20b])
        self.assertEqual(result["fields"],["budget","flag"])

    def test_continuation_refused(self):
        m, search = fixture()
        search["flag_before"] = 0
        with tempfile.TemporaryDirectory() as temp:
            with self.assertRaisesRegex(ValueError,"fresh search"):
                world.capture_world(m,0x400000,search,Path(temp)/"bad.bin")

    def test_capture_roundtrip_and_full_replay(self):
        for base in (0x400000,0x500000):
            with self.subTest(base=base), tempfile.TemporaryDirectory() as temp:
                m, search = fixture(base)
                path = Path(temp)/"world.bin"
                world.capture_world(m,base,search,path)
                manifest = world.verify_snapshot(path)
                self.assertEqual(manifest["dimensions"],[6,6,3])
                self.assertNotEqual(manifest["scalar_type_pointer"],manifest["generator_type_pointer"])
                binary = Path(os.environ.get("MNM_ROUTE_REPLAY", REPO/"working/build/pathfinding/route-replay"))
                self.assertTrue(binary.exists(),"Build route-replay before running this integration test")
                result = subprocess.run([str(binary),str(path)],check=True,capture_output=True,text=True)
                report = json.loads(result.stdout)
                self.assertEqual(report["path"][-1]["xyz"],[3,1,1])
                self.assertEqual(report["waypoint_count"],2)
                self.assertEqual([p["scalar"] for p in report["path"]][1:],[720,720])
                path.write_bytes(path.read_bytes()[:-1])
                with self.assertRaisesRegex(ValueError,"hash/size"):
                    world.verify_snapshot(path)
                self.assertNotEqual(subprocess.run([str(binary),str(path)],capture_output=True).returncode,0)
    def test_sequence_replay_and_comparison(self):
        binary = Path(os.environ.get("MNM_ROUTE_REPLAY", REPO/"working/build/pathfinding/route-replay"))
        with tempfile.TemporaryDirectory(prefix='world sequence ') as temporary:
            root=Path(temporary)
            specifications=[(0x690148,1,1,[3,1,1]), (0x6c4bd0,1,1,[4,1,1]),
                            (0x690148,0,2,[3,1,1]), (0x690148,0,2,[3,1,1]),
                            (0x690148,1,300,[2,1,1]), (0x999999,0,2,[3,1,1])]
            calls=[]
            index=[]
            for number,(context,flag,budget,target) in enumerate(specifications,1):
                m, search=fixture()
                search.update(context=context,flag_before=flag,budget_before=budget,coordinates=target)
                name=f'world-{number:04d}.bin'
                world.capture_world(m,0x400000,search,root/name,allow_continuation=True)
                search['world_snapshot']=name
                calls.append({'sequence':number,'image_base':0x400000,'search':search,
                              'return_address':0x5131b8})
                index.append(f'{context} {flag} "{root/name}"\n')
            path=root/'index.txt'; path.write_text(''.join(index))
            process=subprocess.run([str(binary),'--sequence',str(path)],check=True,capture_output=True,text=True)
            reports=[json.loads(line) for line in process.stdout.splitlines()]
            self.assertEqual([r.get('flag') for r in reports[:5]],[0,0,0,1,1])
            self.assertEqual(reports[3]['path'][-1]['xyz'],[3,1,1])
            self.assertEqual(reports[4]['path'][-1]['xyz'],[2,1,1])
            self.assertEqual(reports[-1]['reason'],'missing_initialization')
            # Synthetic reference records exercise comparison and chain validity.
            for call,report in zip(calls,reports):
                call['snapshot_from_search']=report.get('snapshot_hex',bytes(0x20c).hex())
                call['budget_after']=report.get('budget_remaining',0)
                call['flag_after_search']=report.get('flag',0)
                if report['status']=='replayed':
                    context=bytearray(0x269)
                    context[:0x20c]=bytes.fromhex(report['snapshot_hex'])
                    context[0x20c]=report['flag']
                    for key,offset,fmt in [('best_heuristic',0x211,'i'),('best_node',0x215,'I'),('best_priority',0x265,'i')]:
                        struct.pack_into('<'+fmt,context,offset,report[key])
                    call['context_after_hex']=context.hex()
            (root/'search-calls.jsonl').write_text(''.join(json.dumps(c)+'\n' for c in reversed(calls)))
            ordered, rows=replay.load_sequence(root,world)
            self.assertEqual([c['sequence'] for c in ordered],list(range(1,7)))
            self.assertEqual(len(rows),6)
            result=replay.compare_sequence(reports,calls)
            self.assertEqual((result['matched'],result['mismatched'],result['inconclusive']),(5,0,1))
            # Exercise the launcher/index/report path without rebuilding inside CTest.
            with patch.object(replay,'build_replay',return_value=binary), \
                 patch.object(sys,'argv',['replay-route-world.py','--sequence',str(root)]), \
                 patch.object(sys,'stdout',io.StringIO()):
                with self.assertRaises(SystemExit) as stopped:
                    replay.main()
                self.assertEqual(stopped.exception.code,2)
            saved=json.loads((root/'sequence-replay.json').read_text())
            self.assertEqual((saved['matched'],saved['inconclusive']),(5,1))
            calls[0]['budget_after']=-1
            result=replay.compare_sequence(reports,calls)
            self.assertEqual((result['matched'],result['mismatched'],result['inconclusive']),(2,1,3))
            self.assertEqual(result['calls'][2]['native_comparison']['reason'],'prior_mismatch')
            self.assertEqual(result['calls'][4]['native_comparison']['status'],'match')
            # A mismatched snapshot/call pair is rejected before replay runs.
            calls[0]['search']['budget_before']=200
            (root/'search-calls.jsonl').write_text(''.join(json.dumps(c)+'\n' for c in calls))
            with self.assertRaisesRegex(ValueError,'header does not match'):
                replay.load_sequence(root,world)

    def test_deadline_before_memory_read(self):
        m, search = fixture()
        with tempfile.TemporaryDirectory() as temp:
            with self.assertRaisesRegex(TimeoutError,"deadline"):
                world.capture_world(m,0x400000,search,Path(temp)/"bad.bin",deadline=0)

    def test_dimension_guard_before_bulk_reads(self):
        m, search = fixture()
        m.integer(0x5e1780,33)
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp)/"bad.bin"
            with self.assertRaisesRegex(ValueError,"dimensions"):
                world.capture_world(m,0x400000,search,path)
            self.assertFalse(path.exists())
    def test_corrupt_block_lengths_refused(self):
        m, search = fixture()
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp)/"bad.bin"
            world.capture_world(m,0x400000,search,path)
            data=bytearray(path.read_bytes())
            struct.pack_into("<I",data,8+14*4,0xffffffff)
            path.write_bytes(data)
            manifest=json.loads(path.with_suffix('.json').read_text())
            manifest['sha256']=hashlib.sha256(data).hexdigest()
            path.with_suffix('.json').write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError,"block lengths"):
                world.verify_snapshot(path)
            binary=Path(os.environ.get("MNM_ROUTE_REPLAY", REPO/"working/build/pathfinding/route-replay"))
            self.assertNotEqual(subprocess.run([str(binary),str(path)],capture_output=True).returncode,0)

if __name__ == '__main__': unittest.main()
