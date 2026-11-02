"""Strict MNMACE01 reader; campaign planes stay compressed between uses."""
import bisect
import hashlib
import json
import re
import struct
import numpy as np
from rendering_pilot import require, number, counter, inflate

CAMPAIGN_POLICY = 'campaign-lossless-viewport-v1'
CAMPAIGN_RAW = 64*1024**3
CAMPAIGN_ROWS = 131072


class Plane:
    """A verified decoded extent, reconstructed on demand from bounded keys."""
    def __init__(self, source, ordinal, size):
        self.source, self.ordinal, self.size = source, ordinal, size

    def __len__(self):
        return self.size

    def __call__(self):
        return self.source.get(self.ordinal)


class Planes:
    def __init__(self, raw):
        self.raw, self.entries, self.keys = raw, [], []
        self.reset()

    def reset(self):
        self.last, self.body, self.extent, self.index = -1, b'', None, -1

    def add(self, row, offset, packed_size, size):
        ordinal = len(self.entries)
        require(ordinal % 128 != 0 or row.get('capture_body_encoding') == 'full-rgba-v1',
                'Campaign keyframe interval exceeded')
        if row.get('capture_body_encoding') == 'full-rgba-v1':
            self.keys.append(ordinal)
        self.entries.append((row, offset, packed_size, size))
        # Validate every reconstructed paint now, including unpresented paints.
        self.get(ordinal)
        return Plane(self, ordinal, size)

    def get(self, ordinal):
        require(0 <= ordinal < len(self.entries), 'Missing campaign plane')
        if ordinal == self.last:
            return self.body
        if ordinal < self.last or self.last < 0:
            key = bisect.bisect_right(self.keys, ordinal)-1
            require(key >= 0, 'Campaign plane lacks a full keyframe')
            self.reset()
            begin = self.keys[key]
        else:
            begin = self.last+1
        for current in range(begin, ordinal+1):
            row, offset, packed, size = self.entries[current]
            encoded = inflate(self.raw[offset:offset+packed])
            width, height = number(row['width'], 1, 4096), number(row['height'], 1, 4096)
            extent = width, height
            require(row['kind'] == 'paint' and len(encoded) == size == width*height*4,
                    'Campaign paint body extent')
            encoding, base = row.get('capture_body_encoding'), row.get('capture_body_base')
            require(type(base) is int, 'Campaign plane base type')
            if encoding == 'full-rgba-v1':
                require(base == -1, 'Full campaign plane has a base')
                body = encoded
            else:
                require(encoding == 'xor-previous-paint-v1' and self.body
                        and base == self.index and extent == self.extent and len(self.body) == size,
                        'Campaign delta base/extent mismatch')
                body = np.bitwise_xor(np.frombuffer(encoded, dtype='u1'),
                                      np.frombuffer(self.body, dtype='u1')).tobytes()
            require(isinstance(row.get('capture_body_sha256'), str)
                    and re.fullmatch('[0-9a-f]{64}', row['capture_body_sha256'])
                    and hashlib.sha256(body).hexdigest() == row['capture_body_sha256'],
                    'Campaign reconstructed plane hash mismatch')
            self.last, self.body, self.extent, self.index = current, body, extent, row['index']
        return self.body


def capture(raw):
    require(raw[:8] == b'MNMACE01' and len(raw) <= 1024**3+8, 'Capture envelope')
    rows, at, total, last_time = [], 8, 0, 0
    row_limit, raw_limit, campaign = 16384, 8*1024**3, False
    planes = Planes(raw)
    while at < len(raw):
        require(len(rows) < row_limit and len(raw)-at >= 8, 'Capture row budget/header')
        m, n = struct.unpack_from('<II', raw, at)
        at += 8
        require(1 <= m <= 65536 and 0 <= n <= 33*1024**2 and m+n <= len(raw)-at,
                'Capture row extent')
        row = json.loads(raw[at:at+m]); at += m
        require(isinstance(row, dict) and number(row['index']) == len(rows)
                and type(row['kind']) is str, 'Capture row identity')
        if not rows:
            require(row['kind'] == 'open' and type(row['version']) is int
                    and row['version'] in (1, 2), 'Capture open version')
            if row['version'] == 2:
                require(row['role'] == 'viewport' and row.get('policy') == CAMPAIGN_POLICY
                        and type(row.get('raw_limit_bytes')) is int and row['raw_limit_bytes'] == CAMPAIGN_RAW
                        and type(row.get('row_limit')) is int and row['row_limit'] == CAMPAIGN_ROWS
                        and type(row.get('keyframe_paints')) is int and row['keyframe_paints'] == 128
                        and type(row.get('stored_limit_bytes')) is int and row['stored_limit_bytes'] == 1024**3,
                        'Unreviewed campaign capture policy')
                campaign, row_limit, raw_limit = True, CAMPAIGN_ROWS, CAMPAIGN_RAW
        observed = counter(row['host_us'])
        require(observed >= last_time, 'Capture clock regressed'); last_time = observed
        if n and campaign:
            require(row['kind'] == 'paint' and n >= 5, 'Campaign payload outside paint')
            size = struct.unpack_from('>I', raw, at)[0]
            require(0 < size <= 32*1024**2, 'Inflation budget')
            data = planes.add(row, at, n, size)
        else:
            require(not any(k.startswith('capture_body_') for k in row),
                    'Plane encoding requires campaign payload policy')
            data = inflate(raw[at:at+n]) if n else b''
        at += n
        total += len(data)
        require(total <= raw_limit, 'Capture raw budget')
        rows.append((row, data))
    require(len(rows) >= 2 and rows[-1][0]['kind'] == 'close', 'Capture is not closed')
    first, last = rows[0][0], rows[-1][0]
    limit = first.get('stored_limit_bytes', 256*1024**2)
    require(type(limit) is int and limit in (256*1024**2, 1024**3) and len(raw) <= limit+8,
            'Unreviewed stored capture budget')
    require(first['role'] in ('viewport', 'exports', 'commands') and type(first['session']) is str
            and 1 <= len(first['session']) <= 128, 'Capture open identity')
    require(last['complete'] is True and type(last['error']) is str and last['error'] == ''
            and number(last['prior_rows']) == len(rows)-1 and counter(last['raw_bytes']) == total,
            'Capture incomplete or truncated')
    require(all(r['kind'] not in ('open', 'close') for r, _ in rows[1:-1]), 'Nested capture lifetime')
    planes.reset()
    return rows
