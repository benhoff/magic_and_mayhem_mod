#!/usr/bin/env python3
"""Compare a current CPU replay to precise original entries from a frozen capture.

The capture is historical input provenance, not fresh hook/live validation. Only
the original comparator reads the PE and captured destination checkpoints; the
native consumer reads closed producer records and owned encoded assets.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import time

ORIGINAL_SHA = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def fnv(data):
    value = 2166136261
    for byte in data:
        value = ((value ^ byte) * 16777619) & 0xffffffff
    return value

def compare(source, binary, pe, capture_path, inputs, assets, output):
    output.mkdir()
    pinned = {}
    def pin(path):
        pinned[str(path.resolve())] = sha(path)
    pin(capture_path); capture = json.loads(capture_path.read_text())
    assert capture['success'] and capture['sources_stable'] and capture['canvas_guard_verified']
    assert capture['source_executable_sha256'] == ORIGINAL_SHA and sha(pe) == ORIGINAL_SHA
    assert capture['world_raster_batch'] and capture['original_work_bypassed']
    directory = Path(capture['experiment']) / 'capture'
    stream = inputs / 'canvas-producers.bin'; pin(stream); pin(inputs/'canvas-producers.done'); pin(pe); pin(binary)
    for name in ('canvas-producers.bin', 'canvas-producers.done'):
        pin(directory/name); assert (inputs/name).read_bytes() == (directory/name).read_bytes()
    raw = stream.read_bytes(); at = 64; operations = {}; expected = {}; queue = 0; returns = {}
    while at < len(raw):
        fields = struct.unpack_from('<24I', raw, at)
        size = fields[0]; assert size >= 96 and size <= len(raw)-at
        record = raw[at:at+size]; at += size
        assert fields[1] not in operations; operations[fields[1]] = (fields, record)
        if fields[2] == 11:
            assert not queue; queue = fields[14]; expected[queue] = []
        elif fields[2] == 12:
            assert queue == fields[14]; returns[queue] = fields; queue = 0
        elif queue and fields[2] == 9:
            expected[queue].append(fields[1])
    assert not queue and list(expected) == list(range(1, 17))
    actual = []; cumulative = 0
    batches = capture['native_producers']['native_batch_replies']
    assert len(batches) == 16
    for ordinal, batch in enumerate(batches, 1):
        request = directory/(batch['wire_base']+'.request'); reply = directory/(batch['wire_base']+'.reply')
        pin(request); pin(reply); data = request.read_bytes(); h = struct.unpack_from('<16I', data)
        q = batch['queue']; rows = list(struct.iter_unpack('<4I', data[64:])); cumulative += len(rows)
        assert data[:8] == b'MNMWBQ03' and h[2:6] == (3,64,ordinal,q)
        assert h[8:11] == (800,600,800) and h[13] == 1 and h[14] == len(rows) and h[15] == cumulative
        assert h[11] == len(rows)*16 and len(data) == 64+h[11] and fnv(data[64:]) == h[12]
        assert [r[0] for r in rows] == expected[q] == batch['sequences']
        assert returns[q][15] == cumulative and returns[q][17] == ordinal and returns[q][19:21] == (len(rows),1)
        for sequence, entry, ax, checksum in rows:
            fields, record = operations[sequence]
            assert ax == fields[21] and fnv(record) == checksum
            actual.append((sequence, entry, ax))
        data = reply.read_bytes(); rh = struct.unpack_from('<16I', data)
        assert data[:8] == b'MNMWBR03' and rh[2:11] == h[2:11] and rh[14:] == h[14:]
        assert rh[11] == 960000 and len(data) == 960064 and fnv(data[64:]) == rh[12] and rh[13] == 1
    assert cumulative == capture['bypassed_rasters']
    entries = output/'actual-entries.tsv'; entries.write_text(''.join(f'{seq} {entry} {ax}\n' for seq,entry,ax in actual)); pin(entries)
    reference = output/'original-reference'
    with (output/'reference-build.log').open('x') as log:
        subprocess.run(['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',
                        str(source/'tests/world-raster-batch-reference.cpp'),'-o',str(reference)],
                       stdout=log, stderr=subprocess.STDOUT, check=True, timeout=120)
    pin(reference); fifo = output/'native-proof.fifo'; os.mkfifo(fifo); result = output/'precise-results.tsv'
    native = output/'native'; children = []; started = time.monotonic()
    try:
        with (output/'original.log').open('x') as original_log, (output/'native.log').open('x') as native_log:
            original = subprocess.Popen([str(p) for p in (reference,pe,stream,entries,fifo,result)], stdout=original_log, stderr=subprocess.STDOUT)
            children.append(original)
            viewer = subprocess.Popen(['xvfb-run','-a','-s','-screen 0 1280x1024x24',str(binary),str(stream),str(assets),str(native),'--world-handoff','--world-proof',str(fifo)],
                env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'}, stdout=native_log, stderr=subprocess.STDOUT)
            children.append(viewer)
            while any(p.poll() is None for p in children):
                if any(p.poll() not in (None,0) for p in children): raise RuntimeError('CPU/original comparison refused; inspect '+str(output))
                if time.monotonic()-started > 1800: raise TimeoutError('Bounded CPU/original proof timed out')
                time.sleep(.2)
            assert all(p.returncode == 0 for p in children)
    finally:
        for child in children:
            if child.poll() is None:
                child.terminate()
                try: child.wait(timeout=5)
                except subprocess.TimeoutExpired: child.kill(); child.wait(timeout=5)
    viewer = json.loads((native/'live-report.json').read_text())
    assert viewer['success'] and viewer['world_queues'] == viewer['world_readbacks'] == 16
    assert not viewer['remaining_surfaces'] and not viewer['original_oracles_read'] and not viewer['original_pixels_used_as_native_inputs']
    checks = []
    for old, new in zip(capture['canvas_producers']['checkpoints'],viewer['checkpoints'],strict=True):
        assert (old['sequence'],old['canvas'],old['oracle']) == (new['sequence'],new['canvas'],new['oracle'])
        original_path = directory/old['path']; pin(original_path)
        checks.append({'sequence':old['sequence'],'equal':original_path.read_bytes() == (native/new['path']).read_bytes()})
    assert all(c['equal'] for c in checks)
    rows = [line.split('\t') for line in result.read_text().splitlines()]
    assert len(rows) == cumulative and all(len(r)==5 and r[-1]=='1' for r in rows)
    assert all(sha(Path(n)) == h for n,h in pinned.items())
    return dict(success=True,scope='Current source-only native replay of immutable historical guarded inputs; precise original entries and low16 AX compared offline. No fresh hook, caller ABI, live replacement or frame-rate claim.',
        source_executable_sha256=ORIGINAL_SHA,precise_rasters_equal=cumulative,caller_ax_equal=cumulative,complete_raster_queues=list(expected),
        checkpoints_equal=True,completed=len(checks),precise_results_sha256=sha(result),
        comparison_elapsed_seconds=time.monotonic()-started,inputs=pinned,inputs_stable=True,original_pixels_used_as_native_inputs=False)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('source','binary','pe','capture-report','inputs','assets','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    args = parser.parse_args()
    manifest = args.pe.resolve().parents[2]/'tools/original-manifest.sh'
    subprocess.run([str(manifest),'verify'],check=True)
    try:
        report = compare(args.source,args.binary,args.pe,args.capture_report,args.inputs,args.assets,args.output)
    finally:
        subprocess.run([str(manifest),'verify'],check=True)
    (args.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')

if __name__ == '__main__':
    main()
