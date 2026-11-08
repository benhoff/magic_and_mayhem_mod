"""Read-only bounded observations for a second Quick/Spellbox handoff."""
from pathlib import Path
from contextlib import closing
import json
import os
import sqlite3
import struct
import threading
import time

PREFIX = 'research/runtime/rendering-handoff-results-20261107/'
CONTROLS = ('tools/rendering_handoff.py', 'tools/prepare-rendering-handoff.py',
            'tools/run-rendering-handoff.py', 'tools/check-rendering-handoff.py',
            'tests/test-rendering-handoff.py')


def rebuilt_frame(frame, size, previous_world):
    """An old displayed frame is expected while the new World is not ready."""
    return bool(frame and [frame['frame_width'], frame['frame_height']] == size
                and int(frame['world']) >= previous_world+5)


def menu_header(raw):
    if len(raw) != 160:
        raise ValueError('Incomplete menu header')
    w = struct.unpack('<40I', raw)
    magic = (('MNMMCMD' if w[2] < 10 else 'MNMMCM') + str(w[2])).encode()
    if not 2 <= w[2] <= 22 or raw[:8] != magic or w[3] < 540:
        raise ValueError('Unknown menu wire')
    if w[4] & 1 or w[32] & 1:
        return None
    if w[5] not in (0, 1) or w[35] not in (0, 1) or w[37] > 6:
        raise ValueError('Invalid menu state')
    return dict(host_sequence=w[4], alive=w[5], heartbeat=w[6], request=w[7], action=w[8],
                expected_generation=w[9], engine_sequence=w[32], generation=w[33],
                screen=w[34], ready=w[35], ack=w[36], status=w[37], thread=w[38], handoff=w[39])


def classify(rows):
    """Observation intervals only; never infer callback absence from no journal row."""
    if not rows:
        raise ValueError('No stable menu observations')
    pending = [r for r in rows if r['alive'] and r['screen'] == 7 and r['ready']
               and r['action'] == 12 and r['request'] > r['ack']]
    if not pending:
        return dict(pending_finish_observed=False, engine_freeze_observed=False)
    last_request = max(r['request'] for r in pending)
    selected = [r for r in pending if r['request'] == last_request]
    # The longest uninterrupted engine-publication interval is a concrete lead,
    # independent of whether the command ring is blocked or drained.
    first = selected[-1]
    for row in reversed(selected):
        if row['engine_sequence'] != first['engine_sequence']:
            break
        first = row
    last = selected[-1]
    seconds = (last['ns'] - first['ns']) / 1e9
    retired = [r for r in rows if not r['alive'] and r['request'] == last_request
               and r['ns'] > first['ns']]
    result = dict(pending_finish_observed=True, request=last_request, ack=last['ack'],
                action=12, screen=7, thread=last['thread'],
                engine_sequence=last['engine_sequence'], freeze_seconds=seconds,
                heartbeat_advanced=last['heartbeat'] > first['heartbeat'],
                engine_freeze_observed=seconds >= 2 and last['heartbeat'] > first['heartbeat'],
                first_ns=first['ns'], last_ns=last['ns'])
    if retired:
        # First visibility brackets publication; it is not its exact time.
        prior = [r for r in rows if r['ns'] < selected[0]['ns'] and r['request'] < last_request]
        result['retired_visible_by_ns'] = retired[0]['ns']
        if prior:
            result['request_to_retirement_upper_seconds'] = (retired[0]['ns']-prior[-1]['ns'])/1e9
    return result


def mapped_identity(maps, identity):
    """File device/inode joins across mount namespaces, without guest pointers."""
    device, inode = identity
    for line in maps.splitlines():
        fields = line.split(None, 5)
        if len(fields) < 5:
            continue
        try:
            major, minor = (int(x, 16) for x in fields[3].split(':'))
            if os.makedev(major, minor) == device and int(fields[4]) == inode:
                return True
        except ValueError:
            continue
    return False


class Sampler:
    """One SQLite writer thread, 20 Hz, 8,000 bounded samples. No IPC writes."""
    def __init__(self, path, session, prefix, host_pid):
        self.path, self.session, self.prefix = Path(path), session, Path(prefix).resolve()
        self.host_pid = host_pid
        self.stop = threading.Event()
        self.error = None
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()

    def run(self):
        try:
            with closing(sqlite3.connect(self.path)) as db:
                db.executescript('''PRAGMA journal_mode=WAL; PRAGMA synchronous=FULL;
                CREATE TABLE samples(ordinal INTEGER PRIMARY KEY,ns INTEGER NOT NULL,menu BLOB,ring BLOB,events_bytes INTEGER);
                CREATE TABLE processes(sample INTEGER,pid INTEGER,tid INTEGER,kind TEXT,field TEXT,body BLOB);
                CREATE TABLE errors(sample INTEGER,error TEXT);
                ''')
                count = 0
                last_process = 0
                while not self.stop.wait(.05):
                    path = self.session()
                    if path is None:
                        continue
                    path = Path(path)
                    try:
                        with (path / 'channel.bin').open('rb') as f:
                            menu = f.read(160)
                            f.seek(0)
                            if f.read(160) != menu or not any(menu):
                                continue
                        state = menu_header(menu)
                        if state is None:
                            continue
                        ring = (path / 'render-commands.bin').open('rb')
                        with ring:
                            header = ring.read(64)
                        events = (path / 'events.bin').stat().st_size
                        count += 1
                        if count > 8000:
                            raise ValueError('Diagnostic observation cap exhausted')
                        ns = time.monotonic_ns()
                        with db:
                            db.execute('INSERT INTO samples VALUES(?,?,?,?,?)', (count, ns, menu, header, events))
                        if state['screen'] == 7 and state['request'] > state['ack'] and ns-last_process >= 1_000_000_000:
                            self.processes(db, count)
                            last_process = time.monotonic_ns()
                    except FileNotFoundError:
                        continue
                db.execute('PRAGMA wal_checkpoint(TRUNCATE)')
        except Exception as error:
            self.error = repr(error)

    def processes(self, db, sample):
        """Bind Linux waits to the exact private prefix; failures remain explicit."""
        channel = Path(self.session())/'channel.bin'
        stat = channel.stat()
        identity = stat.st_dev, stat.st_ino
        for proc in Path('/proc').iterdir():
            if self.stop.is_set():
                return
            if not proc.name.isdecimal():
                continue
            try:
                pid = int(proc.name)
                body = (proc / 'cmdline').read_bytes()
                if pid != self.host_pid and b'Chaos.exe' not in body:
                    continue
                values = (proc / 'environ').read_bytes().split(b'\0')
                own = b'WINEPREFIX=' + os.fsencode(self.prefix) in values
                mapped = False
                if not own and pid != self.host_pid:
                    mapped = mapped_identity((proc/'maps').read_text(), identity)
                    own = mapped
                if pid != self.host_pid and not own:
                    continue
                kind = 'host' if pid == self.host_pid else 'mapped-menu-channel' if mapped else 'private-prefix'
                with db:
                    db.execute('INSERT INTO processes VALUES(?,?,?,?,?,?)', (sample, pid, pid, kind, 'cmdline', body))
                tasks = list((proc / 'task').iterdir())
                if len(tasks) > 128:
                    with db:
                        db.execute('INSERT INTO errors VALUES(?,?)', (sample, 'Thread observations truncated at128'))
                for task in tasks[:128]:
                    if self.stop.is_set():
                        return
                    for field in ('stat', 'wchan', 'syscall'):
                        try:
                            with (task / field).open('rb') as f:
                                raw = f.read(16385)
                            if len(raw) > 16384:
                                raise ValueError('Process observation byte cap')
                            with db:
                                db.execute('INSERT INTO processes VALUES(?,?,?,?,?,?)',
                                           (sample, pid, int(task.name), kind, field, raw))
                        except OSError as error:
                            with db:
                                db.execute('INSERT INTO errors VALUES(?,?)', (sample, str(error)))
            except OSError:
                continue

    def close(self):
        self.stop.set()
        self.thread.join(timeout=5)
        if self.thread.is_alive() or self.error:
            raise ValueError('Diagnostic sampler failed: ' + str(self.error))
        with closing(sqlite3.connect(self.path)) as db:
            if db.execute('PRAGMA integrity_check').fetchone() != ('ok',):
                raise ValueError('Diagnostic SQLite integrity failure')


def observed(database):
    with closing(sqlite3.connect(Path(database).as_uri() + '?mode=ro', uri=True)) as db:
        if db.execute('PRAGMA integrity_check').fetchone() != ('ok',):
            raise ValueError('Diagnostic SQLite integrity failure')
        rows = [dict(menu_header(raw), ns=ns) for ns, raw in db.execute('SELECT ns,menu FROM samples ORDER BY ordinal')]
        return dict(classify(rows), sample_count=len(rows),
                    process_rows=db.execute('SELECT COUNT(*) FROM processes').fetchone()[0],
                    process_read_errors=db.execute('SELECT COUNT(*) FROM errors').fetchone()[0])
