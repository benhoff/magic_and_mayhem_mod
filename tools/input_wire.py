"""Admission for client-relative v1 and desktop-relative v2 input channels."""
import struct
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocols/python'))
from mnm_protocols import input_v1, input_v2


def valid_header(data, size):
    if input_v1.valid_header(data,size):
        return True
    return (input_v2.valid_header(data,size)
            and struct.unpack_from('<I',data,input_v2.COORDINATE_SPACE_OFFSET)[0]==input_v2.COORDINATE_SPACE_DESKTOP
            and not any(data[input_v2.RESERVED_OFFSET:input_v2.HEADER_SIZE]))
