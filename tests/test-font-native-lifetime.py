#!/usr/bin/env python3
import copy,json,sys,unittest,hashlib
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from font_native_lifetime import decode,installed,PREFIX,SOURCES

def fixture():
 rows=[]
 def add(kind,**fields):rows.append(dict(schema=1,sequence=str(len(rows)+1),pid=123,gui_thread=True,host_ns=str(len(rows)+1),kind=kind,**fields))
 add('create',generation='1',root='/owned/game',key_sha256=hashlib.sha256(b''.join(bytes.fromhex(str(i)*64)for i in range(4))).hexdigest())
 for role in range(4):add('register',generation='1',role=role,id=role,source=SOURCES[role],family='MNM'+str(role),ttf_sha256=str(role)*64)
 add('attach',generation='1',owner='1',widget='menu',**{'class':'Menu'})
 add('reuse',generation='1',root='/owned/game')
 add('paint',generation='1',owner='1',widget='mainMenuAction4',**{'class':'QPushButton'},text='Quit',role=0,family='MNM0',resolved_family='MNM0',font_valid=True,font_size=32,truncated=False,glyphs=[2,3,4,5],rect=[100,200,80,40])
 add('detach',generation='1',owner='1')
 for role in range(4):add('unregister',generation='1',id=role,success=True)
 add('destroy',generation='1',app_alive=True)
 return rows

def log(rows):return '\n'.join(PREFIX+json.dumps(r)for r in rows)
def installed_fixture():
 rows=fixture();paint=rows[7];other=[]
 for widget,role in [('quick',1),('spell',2),('restored',0),('quick',1),('spell',2)]:
  r=copy.deepcopy(paint);r.update(widget=widget,role=role,family='MNM'+str(role),resolved_family='MNM'+str(role));other.append(r)
 rows[8:8]=other
 for i,r in enumerate(rows,1):r.update(sequence=str(i),host_ns=str(i*100))
 shots=[]
 for event,label in zip(rows[7:13],['font-main-initial','font-quick-1','font-original-spellbox-1','font-main-restored','font-quick-2','font-original-spellbox-2']):
  shots.append(dict(label=label,host_ns=int(event['host_ns'])+10,sha256='a'*64,nodes=[dict(accessible_id=event['widget'],name=event['text'],rect=event['rect'])]))
 return rows,dict(player_pid=123,session='/owned',player_started_host_ns=1,screenshots=shots)

class Controls(unittest.TestCase):
 def test_complete_actual_families_and_retirement(self):
  q=decode(log(fixture()));self.assertTrue(q['complete']);self.assertEqual(q['peak_registered_fonts'],4);self.assertEqual(len(q['paints']),1);self.assertFalse(q['closes_acceptance'])
 def test_missing_unregister_or_owner_drop(self):
  for i in (8,9,13):
   rows=fixture();rows.pop(i)
   for n,r in enumerate(rows,1):r['sequence']=str(n)
   with self.subTest(i=i),self.assertRaises(ValueError):decode(log(rows))
 def test_fallback_overflow_foreign_process_thread(self):
  for fields in [dict(kind='overflow'),dict(kind='fallback'),dict(pid=124),dict(gui_thread=False)]:
   rows=fixture();rows[7].update(fields)
   with self.subTest(fields=fields),self.assertRaises(ValueError):decode(log(rows))
 def test_wrong_family_or_invalid_glyph(self):
  for fields in [dict(family='serif'),dict(resolved_family='serif'),dict(font_valid=False),dict(glyphs=[1]),dict(rect=[0,0,0,20]),dict(truncated=True)]:
   rows=fixture();rows[7].update(fields)
   with self.subTest(fields=fields),self.assertRaises(ValueError):decode(log(rows))
 def test_duplicate_font_id_and_wrong_source(self):
  for fields in [dict(id=0),dict(source='foreign.sft')]:
   rows=fixture();rows[2].update(fields)
   with self.subTest(fields=fields),self.assertRaises(ValueError):decode(log(rows))
 def test_font_unregistered_while_owned(self):
  rows=fixture();rows[8],rows[9]=rows[9],rows[8]
  for n,r in enumerate(rows,1):r['sequence']=str(n)
  with self.assertRaisesRegex(ValueError,'while owned'):decode(log(rows))
 def test_paint_after_retirement(self):
  rows=fixture();paint=copy.deepcopy(rows[7]);rows.append(paint);paint['sequence']=str(len(rows));paint['host_ns']=str(len(rows))
  with self.assertRaisesRegex(ValueError,'retired'):decode(log(rows))
 def test_catalog_key_and_typed_generation(self):
  for fields in [dict(key_sha256='a'*64),dict(generation=True),dict(generation='01')]:
   rows=fixture();rows[0].update(fields)
   with self.subTest(fields=fields),self.assertRaises(ValueError):decode(log(rows))
 def test_installed_caption_intervals_before_and_after_reload(self):
  rows,acquired=installed_fixture();q=installed(acquired,log(rows));self.assertTrue(q['verified']);self.assertEqual(len(q['caption_snapshot_joins']),6)
 def test_installed_future_paint_or_missing_reload_caption(self):
  for which in ('future','missing'):
   rows,acquired=installed_fixture()
   if which=='future':acquired['screenshots'][4]['host_ns']=int(rows[11]['host_ns'])-1
   else:acquired['screenshots'][4]['nodes']=[]
   with self.subTest(which=which),self.assertRaises(ValueError):installed(acquired,log(rows))
 def test_clock_missing_invalid_or_reversed(self):
  for value in (None,True,"01","0","6"):
   rows=fixture();rows[7]["host_ns"]=value
   with self.subTest(value=value),self.assertRaises(ValueError):decode(log(rows))
 def test_missing_footer_and_sequence(self):
  with self.assertRaises(ValueError):decode(log(fixture()[:-1]))
  rows=fixture();rows[7]['sequence']='99'
  with self.assertRaises(ValueError):decode(log(rows))
if __name__=='__main__':unittest.main()
