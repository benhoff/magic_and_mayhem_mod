"""Bounded Git filter-process v2 transport for the existing evidence cleaner.

Only clean and identity smudge are advertised. No cached source, evidence or
approval decisions survive requests; failures are returned to Git as errors.
"""
import os
import shutil
import sys
import tempfile

from coverage_evidence import CHUNK, clean

MAX_PACKET = 65520


def read_packet(stream):
    header = stream.read(4)
    if not header:
        raise EOFError
    if len(header) != 4 or any(c not in b'0123456789abcdefABCDEF' for c in header):
        raise ValueError('Invalid filter packet header')
    size = int(header, 16)
    if size == 0:
        return None
    if size < 4 or size > MAX_PACKET:
        raise ValueError('Invalid filter packet size')
    data = stream.read(size - 4)
    if len(data) != size - 4:
        raise ValueError('Truncated filter packet')
    return data


def read_list(stream):
    items = []
    while (item := read_packet(stream)) is not None:
        items.append(item)
        if len(items) > 32:
            raise ValueError('Too many filter headers')
    return items


def write_packet(stream, data):
    if len(data) > MAX_PACKET - 4:
        raise ValueError('Filter packet too large')
    stream.write(f'{len(data) + 4:04x}'.encode() + data)


def write_list(stream, items):
    for item in items:
        write_packet(stream, item)
    stream.write(b'0000')
    stream.flush()


def serve(root, source, destination):
    if read_list(source) != [b'git-filter-client\n', b'version=2\n']:
        raise ValueError('Unsupported Git filter handshake')
    write_list(destination, [b'git-filter-server\n', b'version=2\n'])
    capabilities = set(read_list(source))
    supported = [c for c in (b'capability=clean\n', b'capability=smudge\n') if c in capabilities]
    write_list(destination, supported)
    while True:
        # EOF between requests is normal; truncated requests are fatal.
        try:
            first = read_packet(source)
        except EOFError:
            return
        if first is None:
            raise ValueError('Empty filter request')
        headers = [first] + read_list(source)
        fields = {}
        for header in headers:
            key, separator, value = header.removesuffix(b'\n').partition(b'=')
            if not separator or key in fields:
                raise ValueError('Invalid filter request header')
            fields[key] = value
        command = fields.get(b'command')
        if command not in {b'clean', b'smudge'} or b'capability=' + command + b'\n' not in supported:
            raise ValueError('Unsupported filter command')
        if b'pathname' not in fields:
            raise ValueError('Missing filter pathname')
        with tempfile.SpooledTemporaryFile(max_size=CHUNK) as incoming, \
                tempfile.SpooledTemporaryFile(max_size=CHUNK) as outgoing:
            while (packet := read_packet(source)) is not None:
                incoming.write(packet)
            incoming.seek(0)
            try:
                if command == b'clean':
                    clean(root, os.fsdecode(fields[b'pathname']), incoming, outgoing)
                else:
                    shutil.copyfileobj(incoming, outgoing, CHUNK)
            except Exception as exc:
                print(f'Evidence filter failed: {exc}', file=sys.stderr, flush=True)
                write_list(destination, [b'status=error\n'])
                continue
            write_list(destination, [b'status=success\n'])
            outgoing.seek(0)
            while data := outgoing.read(MAX_PACKET - 4):
                write_packet(destination, data)
            destination.write(b'0000')
            write_list(destination, [])
