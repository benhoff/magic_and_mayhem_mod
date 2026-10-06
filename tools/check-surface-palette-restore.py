#!/usr/bin/env python3
"""Offline indexed driver observations; restored pixels are not an oracle."""
import argparse,gzip,hashlib,io,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-palette-restore-capture-20261006.json'
LOST=0x887601c2
WRONGMODE=0x8876024b

def require(ok,message):
 if not ok:raise ValueError(message)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(path=DEFAULT):
 r=json.loads(path.read_text());require(r['schema']==1 and r['success'] is True and r['environment']['display_bpp']==8 and r['environment']['override']=='ddraw=b','Unsupported capture')
 m=r['corpus'];p=ROOT/m['path'];require(not Path(m['path']).is_absolute() and p.resolve().is_relative_to(ROOT) and sha(p)==m['sha256'],'Corpus path/hash')
 with gzip.GzipFile(fileobj=io.BytesIO(p.read_bytes())) as f:raw=f.read(1024*1024+1)
 require(len(raw)<=1024*1024 and len(raw)==m['raw_bytes'] and len(raw)%24==0 and hashlib.sha256(raw).hexdigest()==m['raw_sha256']==r['outputs_sha256'],'Raw output size/hash')
 rows=[list(x) for x in struct.iter_unpack('<6I',raw)];require(len(rows)==r['row_count'],'Row count');return r,rows

def color(i,id,generation):return ((i*17+id*37+generation*11)&255)|(((i*29+3+generation*19)&255)<<8)|(((i*43+9+generation*23)&255)<<16)
def validate(r,rows):
 require(r['surface_caps']==[0x840,0x4040,0x40,0x200] and 0<len(rows)<15000 and all(len(x)==6 and all(type(v)==int and 0<=v<=0xffffffff for v in x) for x in rows),'Malformed trace')
 pos=0;palettes=[[color(i,id,0) for i in range(256)] for id in range(2)];states=[]
 def take(kind,phase,id,h=None):
  nonlocal pos
  require(pos<len(rows) and rows[pos][:3]==[kind,phase,id],f'Incomplete/reordered trace at {pos}: expected {kind,phase,id}')
  row=rows[pos];pos+=1
  if h is not None:require(row[3]==h,f'HRESULT mismatch for {kind,phase,id}')
  return row
 def activate(phase):require(take(14,phase,0)[3:5]==[1,1],'Foreground ownership')
 def mode(phase,w,h):
  require(take(10,phase,0,0)[4:]==[w,h],'Mode input');require(take(18,phase,0,0)[4:]==[8,0],'Reported format');require(take(19,phase,0)[3:]==[0,w,h],'Reported dimensions');activate(phase)
 def fill(phase,id,value):require(take(8,phase,id,0)[4:]==[value,0],'Fill input')
 def update(phase,id,first,count,generation):
  require(take(23,phase,id,0)[4:]==[first,count],'Palette update range')
  palettes[id][first:first+count]=[color(i,id,generation) for i in range(first,first+count)]
 def bind(phase,id,palette):require(take(24,phase,id,0)[4:]==[palette,0],'Palette bind')
 def inspect(phase,id):
  lost=LOST if id and (phase in (1,3) or (id==3 and phase==2)) else 0;palette=1 if id==2 or phase==9 else 0
  row=take(21,phase,id,lost);require(row[4:]==[0 if lost else 1,palette],'Palette identity/attachment')
  colors=[]
  if not lost:
   require(take(25,phase,id,0)[4:]==[0,256],'GetEntries range')
   for i in range(256):
    row=take(22,phase,id,0);require(row[4:]==[i,palettes[palette][i]],'Palette entry retention/update');colors.append(row[5])
  take(2,phase,id,lost);require(take(3,phase,id,0)[4:]==[0x35,0x35],'Key retention');row=take(4,phase,id,lost)
  if lost:return
  require(row[4]==8 and row[5]>=8,'Lock indexed format/pitch');require(take(15,phase,id,8)[4:]==([8,6] if id<3 else [640,480]),'Surface dimensions');require(take(16,phase,id)[3:]==[0,0,0],'Indexed masks')
  values=[]
  for i in range(48):
   row=take(5,phase,id,0);require(row[4]==i and row[5]<256,'Index order/range');values.append(row[5])
  if phase==0:require(values==[0x12]*48,'Initial fill indices')
  if phase in (6,8,9):require(values==[(i*37+id*11+6*13)&255 for i in range(48)],'Explicit reload indices')
  h=2166136261
  for v in values:h=((h^v)*16777619)&0xffffffff
  require(take(6,phase,id,h)[4:]==[0,0],'Index hash masks');take(7,phase,id,0)
  states.append(dict(phase=phase,surface=id,palette=palette,colors=colors,indices=values))
 activate(8);require(take(9,0,0,0)[4:]==[0x11,0],'Exclusive flags');mode(0,640,480)
 for id in range(2):require(take(20,0,id,0)[4:]==[0x44,256],'CreatePalette flags/count')
 for id,caps in enumerate(r['surface_caps']):
  require(take(1,0,id,0)[4:]==[caps,0],'CreateSurface caps');take(12,0,id,0);bind(0,id,1 if id==2 else 0);require(take(11,0,id,0)[4:]==[0x35,0x35],'Key input');fill(0,id,0x12);inspect(0,id)
 mode(1,800,600);update(1,0,37,7,1)
 for id in range(4):inspect(1,id);fill(1,id,0xab)
 for id in range(4):take(12,2,id,WRONGMODE if id==3 else 0);inspect(2,id)
 mode(3,640,480)
 for id in range(4):inspect(3,id)
 for id in range(4):
  for phase in (4,5):take(12,phase,id,0);inspect(phase,id)
  row=take(26,6,id,0);require(row[4]==8 and row[5]>=8,'Reload Lock');take(27,6,id,0);inspect(6,id)
 for id in range(2):update(8,id,0,256,2)
 for id in range(4):inspect(8,id)
 bind(9,1,1);inspect(9,1);take(13,7,0,0);require(take(9,7,0,0)[4:]==[8,0],'Normal cleanup');require(take(17,7,0,0)[4:]==[0,0],'Completion');require(pos==len(rows),'Trailing rows')
 return states

def check(path=DEFAULT):
 r,rows=load(path);states=validate(r,rows);sources={str(path.relative_to(ROOT)):sha(path)}
 for n,h in r['sources'].items():require(not Path(n).is_absolute() and (ROOT/n).resolve().is_relative_to(ROOT) and sha(ROOT/n)==h,'Capture source freshness: '+n);sources[n]=h
 for n in ['tools/check-surface-palette-restore.py','tests/test-surface-palette-restore.py']:sources[n]=sha(ROOT/n)
 return dict(schema=1,success=True,sources=sources,checked_rows=len(rows),known_states=len(states),palette_entries=sum(len(s['colors']) for s in states),reload_states=sum(s['phase'] in (6,8,9) for s in states),original_wrapper_executed=False,native_renderer_executed=False,scope='Offline ordered real indexed Wine API observation/provenance verification. Palette identities/entries, lost GetPalette results, explicit reload indices, post-reload palette updates and rebind checked. Unspecified post-Restore pixels have only internal hash validation; no original asset reloader, Windows driver or live equivalence.')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing overwrite')
 r=check();a.report.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps({k:r[k] for k in ['success','checked_rows','palette_entries','reload_states']}))
