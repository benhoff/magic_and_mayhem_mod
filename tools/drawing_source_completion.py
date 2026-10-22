"""Bounded completion admission; does not infer semantic outcomes or frame ownership."""
import math,struct,time
from drawing_observed_inputs import regular_bytes


def decode_completion(raw, expected):
    if type(expected) is not int or not 1<=expected<=120:raise ValueError('Invalid source queue bound')
    if len(raw)!=32 or raw[:8]!=b'MNMPDONE':raise ValueError('Invalid source completion marker')
    words=struct.unpack('<8I',raw)
    if (words[2]!=1 or words[3]!=expected or not 1<=words[4]<=1048576 or words[5]
        or not 1<=words[6]<=min(9999,words[4]) or not 64+words[4]*96<=words[7]<=512*1024**2):raise ValueError('Source completion counts/refusal')
    return dict(expected_queues=expected,records=words[4],oracles=words[6],bytes=words[7],semantic_outcomes_assumed=False)


def wait_for_source(path,expected,process,heartbeat,timeout=90,monotonic=time.monotonic,sleep=time.sleep):
    if type(expected) is not int or not 1<=expected<=120 or type(timeout) not in (int,float) or not math.isfinite(timeout) or not 0<timeout<=90:raise ValueError('Invalid source wait bounds')
    deadline=monotonic()+timeout
    while not path.exists():
        if process.poll() is not None:raise ValueError('Original exited before source completion')
        if monotonic()>=deadline:raise ValueError('Whole update source completion timed out')
        heartbeat();sleep(.05)
    raw=regular_bytes(path,32)
    return decode_completion(raw, expected)
