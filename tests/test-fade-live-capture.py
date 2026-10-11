#!/usr/bin/env python3
"""Closed live fade diagnostics reject malformed/truncated/non-monotone records."""
import importlib.util
from pathlib import Path
import struct
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('fade_decode',ROOT/'tools/fade_live_capture.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
def main():
    before=struct.pack('<6H',0xffff,0x1234,0x5678,0xabcd,0x7777,0x8888)
    after=struct.pack('<6H',0x7bef,0x091a,0x2b2c,0x51e6,0x7777,0x8888)
    valid=b'MNMFDLV1'+struct.pack('<8I',1,3,2,3,0,6,4,0)+before+after+struct.pack('<4I',1,0,0,0x31444e45)
    rows=module.decode(valid);assert len(rows)==1 and rows[0]['before']==before and rows[0]['after']==after
    bad=[b'',valid+b'X',valid[:-1]]
    for offset,value in [(0,0),(8,2),(12,0),(12,2049),(16,0),(20,2),(20,4097),(28,7),(32,6),(36,1),(64,2),(68,1),(76,0)]:
        changed=bytearray(valid);struct.pack_into('<I',changed,offset,value);bad.append(bytes(changed))
    for value in bad:
        try:module.decode(value)
        except ValueError:pass
        else:raise AssertionError('Malformed record admitted')
    for end in range(8,len(valid)):
        assert module.decode(valid[:end],partial=True)==[]
    assert len(module.decode(valid,partial=True))==1
    print(f'{len(bad)} malformed records and {len(valid)-8} partial boundaries refused')
if __name__=='__main__':main()
