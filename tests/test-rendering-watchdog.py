#!/usr/bin/env python3
"""Challenge retained-ring interpretation at truncation and wrap boundaries."""
import hashlib,importlib.util,json,struct,sys,unittest,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('watchdog',ROOT/'tools/inspect-rendering-watchdog.py');watchdog=importlib.util.module_from_spec(spec);spec.loader.exec_module(watchdog)


def envelope(body):
    rows=[(dict(kind='open',role='commands',session='fixture',version=1),b''),(dict(kind='fragment'),body),(dict(kind='close',complete=True,error='',raw_bytes=str(len(body)),prior_rows=2),b'')]
    raw=b'MNMACE01'
    for i,(row,data) in enumerate(rows):
        row.update(index=i,host_us=str(1000+i));meta=json.dumps(row).encode()
        packed=struct.pack('>I',len(data))+zlib.compress(data) if data else b''
        raw+=struct.pack('<II',len(meta),len(packed))+meta+packed
    return raw


class Reader:
    def __init__(self,ack=32):
        # One already consumed opaque operation, then an unread World command.
        self.consumed=b'MNMCMD05'+struct.pack('<II',5,16)+struct.pack('<III',2,1,ack-28)+bytes(ack-28)
        self.tail=struct.pack('<III5I',19,2,20,1,5,0,7,0)
        self.ack=ack;self.rebuild()
    def rebuild(self):
        ring=bytearray(struct.pack('<16I',0x524d4e4d,0x33304344,3,1048640,99,(self.ack+len(self.tail))&0xffffffff,3,4,0,self.ack,0,0,0,0,0,0)+bytes(1048576))
        for i,v in enumerate(self.tail):ring[64+(self.ack+i)%1048576]=v
        self.data={'ring':bytes(ring),'commands':envelope(self.consumed)}
        self.acquired=dict(success=False,artifacts=[dict(path='working/failure/render-commands.bin',record='ring',sha256=self.digest('ring')),dict(path='working/failure/capture/commands-1.bin',record='commands',sha256=self.digest('commands'))])
    def json(self,name):return self.acquired
    def read_bytes(self,name):return self.data[name]
    def digest(self,name):return hashlib.sha256(self.data.get(name,b'fixture')).hexdigest()


class RetainedRingTests(unittest.TestCase):
    def test_negative_execution_can_explain_complete_unread_world_without_end(self):
        r=watchdog.inspect(Reader(),'negative')
        self.assertFalse(r['acquisition_success']);self.assertEqual(r['ring']['unread_bytes'],32)
        self.assertEqual(r['ring']['consumed_last_sequence'],1);self.assertEqual(r['ring']['operation_counts'],{19:1})
        self.assertEqual(r['ring']['commands'][0]['words'],[1,5,0,7,0])
    def test_unread_command_crosses_ring_wrap(self):
        r=watchdog.inspect(Reader(1048568),'wrapped')
        self.assertEqual(r['ring']['commands'][0]['sequence'],2)
        self.assertEqual(r['ring']['commands'][0]['words'],[1,5,0,7,0])
    def test_partial_unread_record_is_rejected(self):
        reader=Reader();reader.tail=reader.tail[:-1];reader.rebuild()
        with self.assertRaisesRegex(ValueError,'Partial unread'):watchdog.inspect(reader,'partial')
    def test_sequence_gap_is_rejected(self):
        reader=Reader();reader.tail=reader.tail[:4]+struct.pack('<I',3)+reader.tail[8:];reader.rebuild()
        with self.assertRaisesRegex(ValueError,'order'):watchdog.inspect(reader,'gap')
    def test_changed_retained_bytes_are_rejected(self):
        reader=Reader();reader.data['ring']=reader.data['ring'][:-1]+b'!'
        with self.assertRaisesRegex(ValueError,'Changed ring'):watchdog.inspect(reader,'changed')


if __name__=='__main__':unittest.main()
