import sys,struct,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_realm_exit import HEADER,read_events,mode4_state,fresh_main,options_point,quit_point


def mini(modal=0):
    base=[1,13,7,17,0x6a5088,0x5c6644,1,0,0,4,5,0x6a5088,4,0,0,0]
    rows=[tuple(base)]
    for seq,event,arg,result in ((2,14,0,5),(3,15,modal,0)):
        row=list(base);row[0]=seq;row[1]=event;row[9:11]=arg,result;rows.append(tuple(row))
    return rows


class RealmExit(unittest.TestCase):
    def test_mode4_requires_fresh_typed_nonmodal_and_confirmation_triples(self):
        self.assertEqual(mode4_state(mini())['mode'],4)
        self.assertIsNone(mode4_state(mini(),after=1))
        self.assertIsNone(mode4_state(mini(123)))
        self.assertEqual(mode4_state(mini(123),modal=True)['modal_pointer'],123)
        self.assertIsNone(mode4_state(mini()[:-1]))

    def test_mode2_world_parent_and_mutated_identity_context_order_are_refused(self):
        for at,value in ((1,16),(3,2),(4,0x6cbb78),(5,0),(6,0),(7,1),(8,1),(9,2),(10,3),(11,0x659408),(12,5)):
            rows=mini();r=list(rows[0]);r[at]=value;rows[0]=tuple(r)
            with self.subTest(at=at,value=value):self.assertIsNone(mode4_state(rows))
        rows=mini();r=list(rows[1]);r[2]=8;rows[1]=tuple(r);self.assertIsNone(mode4_state(rows))

    def test_main_requires_later_initialized_original_root(self):
        row=(4,4,7,3,0x657ce0,0x5c63e4,1,0,0,0,0,0x657ce0,2,0,0,0)
        self.assertEqual(fresh_main([row],3)['sequence'],4)
        self.assertIsNone(fresh_main([row],4))
        for at,value in ((3,4),(4,0x659408),(5,0),(6,0),(7,1),(8,1),(11,0x659408),(12,3)):
            r=list(row);r[at]=value
            with self.subTest(at=at,value=value):self.assertIsNone(fresh_main([tuple(r)],3))

    def test_live_tail_complete_sequence_thread_and_capacity(self):
        with tempfile.TemporaryDirectory() as directory:
            p=Path(directory)/'events.bin';raw=HEADER+b''.join(struct.pack('<16I',*r) for r in mini())
            p.write_bytes(raw+b'partial');self.assertEqual(len(read_events(p,7)),3)
            with self.assertRaises(ValueError):read_events(p,7,True)
            p.write_bytes(raw)
            with self.assertRaises(ValueError):read_events(p,8)
            p.write_bytes(b'bad')
            with self.assertRaises(ValueError):read_events(p,7)
            rows=[(i+1,1,7,*([0]*13)) for i in range(256)];p.write_bytes(HEADER+b''.join(struct.pack('<16I',*r) for r in rows))
            with self.assertRaises(ValueError):read_events(p,7)

    def test_original_options_and_pinned_mini_quit_geometry_in_both_modes(self):
        self.assertEqual(options_point((800,600)),(616,28));self.assertEqual(options_point((640,480)),(456,28))
        with tempfile.TemporaryDirectory() as directory:
            game=Path(directory);p=game/'Interface/MiniMenu/screen (Mini Menu).cfg';p.parent.mkdir(parents=True)
            text='[TEXTBUTTON_4]\nRect2=250,330,550,370 ; high\nRect1=200,264,440,296\nText=50; Quit Game\n';p.write_text(text)
            self.assertEqual(quit_point(game,(800,600))[0],(400,350));self.assertEqual(quit_point(game,(640,480))[0],(320,280))
            p.write_text(text.replace('250,330','251,330'))
            with self.assertRaises(ValueError):quit_point(game,(800,600))


if __name__=='__main__':unittest.main()
