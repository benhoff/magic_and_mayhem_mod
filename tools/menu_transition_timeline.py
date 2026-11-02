"""Bounded read-only host/menu observations with committed SQLite persistence.

Original callback rows have ordinals, not timestamps. Their visibility intervals
are bounded by successive reads; they never acquire invented execution times.
"""
import os
from pathlib import Path
import sqlite3
import struct
import time

MAGIC=b'MNMQLT01';CAP=512;SIZE=64+64*CAP
EVENTS={1:'created',2:'request_before',3:'request_published',4:'engine_read',5:'destination_ready',6:'fallback',7:'retire_before',8:'retire_after',9:'request_refused',10:'read_suppressed'}
def now():return time.clock_gettime_ns(time.CLOCK_MONOTONIC)
def host_rows(raw):
 if len(raw)!=SIZE:raise ValueError('Host journal extent')
 h=struct.unpack_from('<16I',raw)
 if raw[:8]!=MAGIC or h[2:5]!=(1,64,CAP) or h[5]>CAP or h[6] or not h[7] or h[8]>1 or h[9]!=1 or h[11]>1 or any(h[12:]):raise ValueError('Host journal header/overflow')
 rows=[]
 for i in range(h[5]):
  r=struct.unpack_from('<16I',raw,64+64*i)
  if r[0]!=i+1 or r[1] not in EVENTS or r[15] or r[11]&~31:raise ValueError('Host journal record')
  t=r[2]+(r[3]<<32)
  if not t or (rows and t<rows[-1][2]):raise ValueError('Host journal clock order')
  rows.append((r[0],EVENTS[r[1]],t,*r[4:15],raw[64+64*i:128+64*i]))
 return h,rows

def callback_rows(raw,final=False):
 if len(raw)<16:
  if final:raise ValueError('Callback header incomplete')
  return []
 if raw[:16]!=b'MNMMENU1'+struct.pack('<II',1,64) or len(raw)>16+256*64:raise ValueError('Callback header/extent')
 if final and (len(raw)-16)%64:raise ValueError('Callback tail incomplete')
 rows=[]
 for at in range(16,len(raw)-63,64):
  r=struct.unpack_from('<16I',raw,at)
  if r[0]!=len(rows)+1 or r[1] not in (1,2,3,4):raise ValueError('Callback ordinal/event')
  rows.append((r[0],r[1],r[3],r[9],raw[at:at+64]))
 return rows

def wire_words(raw):
 if len(raw)!=160 or raw[:16]!=b'MNMMCMD5'+struct.pack('<II',5,73728):raise ValueError('Menu V5 header')
 w=struct.unpack('<40I',raw)
 if w[4]&1 or w[32]&1:return None
 if w[5]>1 or w[35]>1 or w[37]>5 or w[39]>4:raise ValueError('Menu V5 fields')
 return (w[4],w[5],w[6],w[7],w[8],w[9],*w[32:40])

class Recorder:
 def __init__(self,path):
  self.path=Path(path)
  if self.path.exists():raise ValueError('New SQLite timeline required')
  self.db=sqlite3.connect(self.path,isolation_level=None)
  self.db.execute('PRAGMA journal_mode=WAL');self.db.execute('PRAGMA synchronous=FULL')
  self.db.executescript('''CREATE TABLE host_events(ordinal INTEGER PRIMARY KEY,event TEXT,ns INTEGER,request INTEGER,action INTEGER,ack INTEGER,generation INTEGER,screen INTEGER,status INTEGER,engine_seq INTEGER,flags INTEGER,request_age_ms INTEGER,state_age_ms INTEGER,target INTEGER,raw BLOB);
CREATE TABLE callbacks(ordinal INTEGER PRIMARY KEY,event INTEGER,menu INTEGER,argument INTEGER,visible_after_ns INTEGER,visible_by_ns INTEGER,raw BLOB);
CREATE TABLE wire(ordinal INTEGER PRIMARY KEY,begin_ns INTEGER,end_ns INTEGER,host_seq INTEGER,alive INTEGER,heartbeat INTEGER,request INTEGER,action INTEGER,expected_generation INTEGER,engine_seq INTEGER,generation INTEGER,screen INTEGER,ready INTEGER,ack INTEGER,status INTEGER,thread INTEGER,handoff INTEGER,raw BLOB);
CREATE TABLE payloads(ordinal INTEGER PRIMARY KEY,raw BLOB);
CREATE TABLE metadata(key TEXT PRIMARY KEY,value TEXT);''')
  self.identities={};self.host_count=0;self.callback_count=0;self.wire_count=0;self.callback_prior=0;self.last_key=None;self.last_sample=0;self.unstable=0;self.header=None;self.payloads=0
 def read(self,path,limit):
  path=Path(path)
  if not path.exists():return None
  if path.is_symlink():raise ValueError('Symlinked observation input')
  with path.open('rb') as f:
   st=os.fstat(f.fileno());key=(st.st_dev,st.st_ino)
   if path in self.identities and self.identities[path]!=key:raise ValueError('Observation input replaced')
   self.identities[path]=key
   if st.st_size>limit:raise ValueError('Observation input exceeds cap')
   raw=f.read(limit+1)
  return raw
 def observe(self,session,final=False):
  session=Path(session);begin=now();raw=self.read(session/'menu-host-timeline.bin',SIZE)
  if raw and any(raw) and (len(raw)==SIZE and struct.unpack_from('<I',raw,28)[0]):
   header,rows=host_rows(raw);self.header=header
   if len(rows)<self.host_count:raise ValueError('Host journal shrank')
   with self.db:self.db.executemany('INSERT INTO host_events VALUES('+','.join('?'*15)+')',rows[self.host_count:])
   self.host_count=len(rows)
  start=now();raw=self.read(session/'events.bin',16+256*64)
  if raw:
   rows=callback_rows(raw,final);end=now()
   if len(rows)<self.callback_count:raise ValueError('Callback journal shrank')
   values=[(r[0],r[1],r[2],r[3],self.callback_prior,end,r[4]) for r in rows[self.callback_count:]]
   with self.db:self.db.executemany('INSERT INTO callbacks VALUES(?,?,?,?,?,?,?)',values)
   self.callback_count=len(rows);self.callback_prior=start
  path=session/'channel.bin'
  if path.exists():
   if path.is_symlink():raise ValueError('Symlinked menu channel')
   with path.open('rb') as f:
    st=os.fstat(f.fileno());key=(st.st_dev,st.st_ino)
    if st.st_size!=73728:
     if not final and not self.wire_count:return
     raise ValueError('Menu V5 channel extent')
    if path in self.identities and self.identities[path]!=key:raise ValueError('Menu channel replaced')
    self.identities[path]=key;start=now();whole=f.read(73728);raw=whole[:160];f.seek(0);again=f.read(160);end=now()
   if raw!=again or (not any(raw) and not final):self.unstable+=1;return
   fields=wire_words(raw)
   if fields is None:self.unstable+=1;return
   # Heartbeat snapshots every250ms, plus all semantic changes. Engine seqlock
   # increments alone are not new semantic observations.
   semantic=fields[1:2]+fields[3:6]+fields[7:]
   if semantic!=self.last_key or end-self.last_sample>=250_000_000 or final:
    if self.wire_count>=4096:raise ValueError('Wire observation cap')
    self.wire_count+=1
    with self.db:self.db.execute('INSERT INTO wire VALUES('+','.join('?'*18)+')',(self.wire_count,start,end,*fields,raw))
    # Full payloads can explain a rejected MenuBridge decode. Keep them only
    # around the first transition, with an explicit eight-MiB-scale cap.
    terminal=self.db.execute("SELECT MIN(ns) FROM host_events WHERE event IN ('destination_ready','fallback')").fetchone()[0]
    if self.header and self.header[10] and (terminal is None or end<terminal+500_000_000):
     if self.payloads>=128:raise ValueError('Transition payload cap')
     with self.db:self.db.execute('INSERT INTO payloads VALUES(?,?)',(self.wire_count,whole))
     self.payloads+=1
    self.last_key=semantic;self.last_sample=end
 def summary(self):
  published=list(self.db.execute("SELECT request,ns FROM host_events WHERE event='request_published'"));dest=list(self.db.execute("SELECT request,ns FROM host_events WHERE event='destination_ready'"));fallback=list(self.db.execute("SELECT request,ns,state_age_ms,request_age_ms FROM host_events WHERE event='fallback'"))
  if len(published)!=1:raise ValueError('Exactly one first Main request publication required')
  req,pub=published[0];retire=self.db.execute("SELECT MIN(ns) FROM host_events WHERE event='retire_before' AND ns>=?",(pub,)).fetchone()[0]
  ack=self.db.execute('SELECT MIN(end_ns) FROM wire WHERE ack=? AND end_ns>=?',(req,pub)).fetchone()[0]
  callbacks=list(self.db.execute('SELECT ordinal,event,visible_after_ns,visible_by_ns FROM callbacks WHERE menu=3 AND argument=2 AND event IN (2,3) ORDER BY ordinal'))
  pair=callbacks[:2];pair_ok=len(pair)==2 and [r[1] for r in pair]==[2,3]
  return dict(request=req,published_ns=pub,destination_ready=bool(dest),fallback=fallback,retire_before_ns=retire,first_ack_visible_by_ns=ack,ack_observed_before_client_retire=bool(ack and retire and ack<retire),original_callback_pair_retained=pair_ok,original_callback_receipt_intervals=pair,original_callback_execution_timestamps_available=False,callback_log_cap_reached=self.callback_count==256,host_rows=self.host_count,wire_rows=self.wire_count,callback_rows=self.callback_count,unstable_wire_reads=self.unstable,payload_snapshots=self.payloads,header_closed=bool(self.header and self.header[8]),control=bool(self.header and self.header[11]),outcome='ready' if dest else 'timeout_or_fallback' if fallback else 'incomplete',scope='Callback ordinals/visibility intervals are observations, not exact execution timestamps. Missing records do not prove absence of a callback; the original observer can drop contended rows.')
 def close(self):
  # Every insertion above is committed on disk. No in-memory backup operation
  # and no live write transaction during checkpoint/close.
  if self.db is not None:
   checkpoint=self.db.execute('PRAGMA wal_checkpoint(TRUNCATE)').fetchone()
   if checkpoint[0]:raise RuntimeError('SQLite checkpoint remained busy; preserve WAL')
   self.db.close();self.db=None
