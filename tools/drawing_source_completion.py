"""Bounded completion admission; does not infer semantic outcomes or frame ownership."""
import struct,time


def decode_completion(raw, expected):
    if type(expected) is not int or not 1<=expected<=120:raise ValueError('Invalid source queue bound')
    if len(raw)!=32 or raw[:8]!=b'MNMPDONE':raise ValueError('Invalid source completion marker')
    words=struct.unpack('<8I',raw)
    if words[2]!=1 or words[3]!=expected or not words[4] or words[5] or not words[6] or not words[7]:raise ValueError('Source completion counts/refusal')
    return dict(expected_queues=expected,records=words[4],oracles=words[6],bytes=words[7],semantic_outcomes_assumed=False)


def wait_for_source(path,expected,process,heartbeat,timeout=90,monotonic=time.monotonic,sleep=time.sleep):
    if type(expected) is not int or not 1<=expected<=120 or not 0<timeout<=90:raise ValueError('Invalid source wait bounds')
    deadline=monotonic()+timeout
    while not path.exists():
        if process.poll() is not None:raise ValueError('Original exited before source completion')
        if monotonic()>=deadline:raise ValueError('Whole update source completion timed out')
        heartbeat();sleep(.05)
    raw=path.read_bytes()
    return decode_completion(raw, expected)
