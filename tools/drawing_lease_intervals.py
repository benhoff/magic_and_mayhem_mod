"""Exact observed lease overlaps with bounded listings; unknown endings stay open."""
from bisect import bisect_left, bisect_right


class LeaseIntervals:
    def __init__(self, leases):
        self.leases = leases
        self.starts = [lease['acquire_event'] for lease in leases]
        if self.starts != sorted(set(self.starts)):
            raise ValueError('Lease acquisition order/identity')
        self.ends = sorted(lease['release_event'] for lease in leases if lease['release_event'] is not None)
        self.width = 1
        while self.width < len(leases):
            self.width *= 2
        self.maximum = [0] * (2 * self.width)
        for i, lease in enumerate(leases):
            end = lease['release_event']
            if type(lease['acquire_event']) is not int or lease['acquire_event'] <= 0 or end is not None and (type(end) is not int or end <= lease['acquire_event']):
                raise ValueError('Invalid observed lease interval')
            self.maximum[self.width + i] = end if end is not None else 2**64
        for i in range(self.width - 1, 0, -1):
            self.maximum[i] = max(self.maximum[2*i], self.maximum[2*i+1])

    def overlaps(self, enter, ack, limit=256):
        if any(type(v) is not int or v <= 0 for v in (enter, ack, limit)) or enter >= ack:
            raise ValueError('Invalid observed witness interval')
        right = bisect_left(self.starts, ack)
        count = right - bisect_right(self.ends, enter)
        entries = []

        def visit(node, lo, hi):
            if lo >= right or self.maximum[node] <= enter or len(entries) >= limit:
                return
            if hi - lo == 1:
                entries.append(self.leases[lo]['entry'])
                return
            middle = (lo + hi) // 2
            visit(node*2, lo, middle)
            visit(node*2+1, middle, hi)

        visit(1, 0, self.width)
        return count, entries
