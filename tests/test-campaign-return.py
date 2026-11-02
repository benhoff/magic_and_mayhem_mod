#!/usr/bin/env python3
"""First Realm tutorial input-window controls; no original game execution."""
import struct,sys,unittest,importlib.util,json,os,shutil,subprocess,tempfile,time
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_return import realm_return_state

class ReturnTests(unittest.TestCase):
    def setUp(self):
        self.data=bytearray(524288);self.data[:16]=b'MNMMCM22'+struct.pack('<II',22,524288)
        struct.pack_into('<I',self.data,28,6);struct.pack_into('<8I',self.data,128,100,25,0,0,6,1,316,3)
        self.report=dict(return_sequence=26,thread=316)
        pop=[0]*16;pop[0]=31;pop[1]=28;pop[2]=316;pop[3]=2;pop[11]=0x659408;pop[12]=3;self.events=[pop]
    def state(self):return realm_return_state(self.data,self.events,self.report)
    def test_complete_original_report_and_realm_pop_required(self):
        self.assertEqual(self.state(),'dismiss')
        self.report['return_sequence']=None;self.assertEqual(self.state(),'wait')
        self.report['return_sequence']=26;self.events=[];self.assertEqual(self.state(),'wait')
    def test_fresh_settled_realm_is_the_only_success(self):
        struct.pack_into('<8I',self.data,128,102,26,4,1,6,1,316,0)
        self.assertEqual(self.state(),'ready')
        for screen in (3,6,18,20):
            struct.pack_into('<I',self.data,136,screen);self.assertEqual(self.state(),'wait')
    def test_torn_unacknowledged_wrong_handoff_and_stack_do_not_click(self):
        for offset,value in ((128,101),(144,5),(156,0),(136,4),(140,1)):
            old=struct.unpack_from('<I',self.data,offset)[0];struct.pack_into('<I',self.data,offset,value)
            self.assertEqual(self.state(),'wait');struct.pack_into('<I',self.data,offset,old)
        self.events[0][11]=0x657ce0;self.assertEqual(self.state(),'wait')
    def test_wire_owner_status_and_readiness_refuse(self):
        with self.assertRaises(ValueError):realm_return_state(self.data[:-1],self.events,self.report)
        for offset,value in ((152,317),(148,5),(140,2)):
            old=struct.unpack_from('<I',self.data,offset)[0];struct.pack_into('<I',self.data,offset,value)
            with self.assertRaises(ValueError):self.state()
            struct.pack_into('<I',self.data,offset,old)

class HelperReturnProcess(unittest.TestCase):
    @unittest.skipUnless(shutil.which('xvfb-run'),'Xvfb required for physical helper fixture')
    def test_victory_helper_dismisses_then_waits_for_settled_realm(self):
        root=Path(__file__).resolve().parents[1]
        spec=importlib.util.spec_from_file_location('owned_return_display',root/'tools/run-native-owned.py')
        launcher=importlib.util.module_from_spec(spec);spec.loader.exec_module(launcher)
        with tempfile.TemporaryDirectory() as directory:
            out=Path(directory);fixture=ReturnTests();fixture.setUp()
            begin=[0]*16;begin[0]=25;begin[1]=2;begin[2]=316;begin[3]=6;begin[4]=begin[11]=0x6db967;begin[5]=0x5c5ebc;begin[6]=1;begin[9]=21;begin[12]=5
            end=begin.copy();end[0]=26;end[1]=3
            (out/'events.bin').write_bytes(b'MNMMENU1'+struct.pack('<II',1,64)+b''.join(struct.pack('<16I',*r) for r in (begin,end,*fixture.events)))
            channel=out/'channel.bin';channel.write_bytes(fixture.data)
            host=out/'host';host.mkdir();server,owner,_=launcher.start_display(host)
            process=None;record=out/'input.json'
            try:
                process=subprocess.Popen([sys.executable,root/'tools/campaign-outcome-input.py','--rect','0','0','800','600','--case','progression','--experiment',out,'--seconds','30','--output',record],stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=dict(os.environ,DISPLAY=owner['display'],XAUTHORITY=owner['authority']))
                deadline=time.monotonic()+8;clicked=False
                while time.monotonic()<deadline and process.poll() is None:
                    if record.exists():
                        item=json.loads(record.read_text())
                        if item.get('realm_tutorial_clicks',0):clicked=True;break
                    time.sleep(.02)
                self.assertTrue(clicked,'No bounded ordinary tutorial click')
                self.assertIsNone(process.poll(),'Helper retired before original Realm readiness')
                struct.pack_into('<8I',fixture.data,128,102,26,4,1,6,1,316,0);channel.write_bytes(fixture.data)
                _,stderr=process.communicate(timeout=5)
                self.assertEqual(process.returncode,0,stderr.decode())
                item=json.loads(record.read_text());self.assertTrue(item['success']);self.assertEqual(item['realm_return_state'],'ready')
                self.assertFalse(item['simulation_writes']);self.assertEqual(item['actions'][0]['button'],1)
            finally:
                if process and process.poll() is None:process.kill();process.wait(timeout=5)
                server.terminate();server.wait(timeout=5)

if __name__=='__main__':unittest.main()
