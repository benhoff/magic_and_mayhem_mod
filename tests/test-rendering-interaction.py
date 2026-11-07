import copy,struct,sys,unittest,importlib.util
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_interaction import decode
from rendering_interaction_live import sprite_target
from rendering_interaction_budget import admit


def rows():
    w=[0]*96;w[:8]=[1,0,1,1,100,77,0,256];w[45]=4
    w[56:64]=[1,32,0x40209ca7,1,0,123,456,77]
    w[64]=1;w[65:81]=[1,64,0x40209ca7,1,77,1,2,99,400,265,1,1,0x4fd123,0,0,0]
    a=copy.copy(w);a[:4]=[2,1,2,1];a[8:10]=[400,265]
    b=copy.copy(a);b[0]=3;b[2]=3;b[4]=101
    return [w,a,b]


def wire(rs):return b'MNMIFR01'+struct.pack('<II',1,384)+b''.join(struct.pack('<96I',*r) for r in rs)


class FrameInput(unittest.TestCase):
    def test_positive(self):
        r=decode(wire(rows()));self.assertEqual(r['callbacks'],1);self.assertFalse(r['allocation_generations_proved']);self.assertEqual(r['rows'][0]['frame']['generation'],1)

    def test_invalid_streams(self):
        raw=wire(rows())
        for bad in (b'',raw[:-1],b'BADMAGIC'+raw[8:],raw+raw[16:400]):
            with self.subTest(length=len(bad)),self.assertRaises(ValueError):decode(bad)

    def test_corrupted_fields(self):
        for row,index,value in [(0,0,2),(1,5,78),(1,4,99),(0,7,0x80000100),(0,95,1),
                (0,56,2),(0,57,64),(0,58,0),(0,59,0),(1,61,999),(0,63,78),
                (0,64,2),(0,65,0),(0,68,0),(0,69,78),(0,70,0),(0,71,3),(0,76,0),
                (0,78,1),(0,45,0),(0,55,1),(0,81,2),(1,1,0),(2,1,2),(2,8,399),
                (0,8,1),(0,24,3),(0,2,4)]:
            rs=rows();rs[row][index]=value
            with self.subTest(row=row,index=index,value=value),self.assertRaises(ValueError):decode(wire(rs))

    def test_no_stale_data_for_missing_observation(self):
        for index,value in ((7,0),(64,0)):
            rs=rows();rs[0][index]=value
            with self.subTest(index=index),self.assertRaises(ValueError):decode(wire(rs))

    def test_pending_allowed_only_by_live_reader(self):
        rs=rows()[:-1]
        with self.assertRaises(ValueError):decode(wire(rs))
        self.assertEqual(len(decode(wire(rs),allow_pending=True)['rows']),2)

    def test_same_actor_disagreement(self):
        rs=rows();r=rs[0];r[24:32]=[1,1,20,1,0,0,1,100];r[42]=1
        r[45:55]=[7,1,20,1,0,0,1,100,0,0]
        with self.assertRaises(ValueError):decode(wire(rs))

    def test_foreign_pointer_has_no_actor_identity(self):
        rs=rows();rs[0][24]=2
        self.assertIsNone(decode(wire(rs))['rows'][0]['hit'])


class SpriteTarget(unittest.TestCase):
    def source(self):
        frame=bytearray(struct.pack('<10I',79,4,3,0xfffffffe,0xfffffffb,0,0,0,0,0))
        for y in range(3):frame+=struct.pack('<II',64+y*3,73+y*2)
        frame+=bytes([1,2,1])*3+bytes([9])*6
        f=[0]*24;f[1]=1;f[2]=9;f[8:10]=[10,20];f[15]=1;f[19]=len(frame);f[22]=0x500512
        return f,bytes(frame)

    def test_origin_and_opaque_centre(self):
        target=sprite_target([self.source()],[10,20,192])
        self.assertEqual(target['point'],[13,26]);self.assertEqual(target['origin'],[-2,-5]);self.assertEqual(target['opaque_pixel'],[1,1])

    def test_two_clipped_calls_same_sprite(self):
        a,b=self.source();c=copy.copy(a);c[1]=2;c[11]=2
        self.assertEqual(sprite_target([(a,b),(c,b)],[10,20,192])['raster_sequences'],[1,2])

    def test_visible_body_blend_geometry(self):
        for backend,caller in [(4,0x5005a2),(4,0x500aca),(5,0x5005c0),(5,0x500ae9),(6,0x500584),(6,0x500aab)]:
            a,b=self.source();a[15]=backend;a[22]=caller
            with self.subTest(backend=backend,caller=caller):self.assertEqual(sprite_target([(a,b)],[10,20,192])['point'],[13,26])

    def test_wrong_body_caller_pair_refused(self):
        a,b=self.source();a[15]=4
        with self.assertRaises(ValueError):sprite_target([(a,b)],[10,20,192])

    def test_missing_wrong_anchor_and_ambiguous_refused(self):
        a,b=self.source();other=bytearray(b);other[-1]=8
        for rs,h in [([],[10,20,192]),([(a,b)],[11,20,192]),([(a,b),(a,other)],[10,20,192])]:
            with self.subTest(history=h),self.assertRaises(ValueError):sprite_target(rs,h)

    def test_bad_runs_and_transparent_refused(self):
        a,b=self.source()
        for change in ('overflow','transparent','short','table'):
            q=bytearray(b)
            if change=='overflow':q[65]=8
            elif change=='transparent':q[64:73]=bytes([4,0,0])*3
            elif change=='short':q=q[:-1]
            else:q[40:44]=struct.pack('<I',20)
            with self.subTest(change=change),self.assertRaises(ValueError):sprite_target([(a,q)],[10,20,192])


class AttemptBudget(unittest.TestCase):
    def case(self,name,parent=None,games=1,success=False,proved=False,comparison_failed=False,preparation=False):
        return dict(report=name,repair_of=parent,result=dict(original_game_runs=games,success=success),proved=proved,comparison_failed=comparison_failed,preparation_fixed=preparation)

    def test_empty(self):self.assertEqual(admit([],None,7,6)['requests_before'],0)

    def test_unrelated_failure_blocks(self):
        with self.assertRaises(ValueError):admit([self.case('a')],None,7,6)

    def test_repair_chain_and_transitive_success(self):
        cases=[self.case('a',games=0),self.case('b','a',preparation=True),self.case('c','b')]
        self.assertEqual(admit(cases,'c',7,6)['original_games_before'],2)
        cases.append(self.case('d','c',success=True,proved=True))
        self.assertEqual(admit(cases,None,7,6)['resolved_ancestors'],['a','b','c'])

    def test_uncompared_success_blocks(self):
        cases=[self.case('a',success=True)]
        with self.assertRaises(ValueError):admit(cases,None,7,6)
        with self.assertRaises(ValueError):admit(cases,'a',7,6)
        cases[0]['comparison_failed']=True;admit(cases,'a',7,6)

    def test_each_budget_is_hard(self):
        for cases in ([self.case(str(i),games=0,success=True,proved=True) for i in range(7)],
                      [self.case(str(i),success=True,proved=True) for i in range(6)]):
            with self.assertRaises(ValueError):admit(cases,None,7,6)

    def test_unknown_game_count_refused(self):
        with self.assertRaises(ValueError):admit([self.case('a',games=None)],'a',7,6)

    def test_missing_and_cyclic_ancestry_refused(self):
        for cases,target in [([], 'missing'),([self.case('a','b'),self.case('b','a')],'a')]:
            with self.assertRaises(ValueError):admit(cases,target,7,6)


class MinimapPlaneValidation(unittest.TestCase):
    def test_complete_valid_and_bad_flag_planes(self):
        path=Path(__file__).resolve().parents[1]/'tools/inspect-canvas-producers.py'
        spec=importlib.util.spec_from_file_location('interaction_producers',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        h=[0]*16;h[:2]=struct.unpack('<II',b'MNMPRO03');h[2:7]=[3,64,0x40209ca7,131072,32]
        for width,height in [(2,1),(8,4),(128,128),(256,256)]:
            f=[0]*24;f[0]=96+width*height*3;f[1]=1;f[2]=22;f[14:16]=[width,height];f[18]=width*height*3;f[19]=0xfffffffe
            body=bytearray(width*height*3)
            for i in range(width*height):body[i*3+2]=i%2
            header=struct.pack('<16I',*h)+struct.pack('<24I',*f)
            module.decode(header+body,complete=False)
            for index in (0,width*height-1):
                bad=bytearray(body);bad[index*3+2]=2
                with self.subTest(width=width,height=height,index=index),self.assertRaises(ValueError):module.decode(header+bad,complete=False)


class AsynchronousFrameJoin(unittest.TestCase):
    def test_cursor_freshness_follows_call_order_across_slow_world(self):
        from rendering_interaction_assessment import cursor_after_action
        baseline=dict(tick=100,cursor=dict(poll=3,tick=90))
        row=dict(tick=4000,cursor=dict(poll=4,tick=110))
        self.assertTrue(cursor_after_action(row,baseline))
        for cursor in (None,dict(poll=3,tick=110),dict(poll=4,tick=99),dict(poll=4,tick=4001)):
            with self.subTest(cursor=cursor):self.assertFalse(cursor_after_action(dict(row,cursor=cursor),baseline))

    def test_public_scaling_transition_is_bounded(self):
        from rendering_interaction_assessment import presentation_scale_matches
        for tick,wanted in [(100,0),(200,2)]:
            self.assertTrue(presentation_scale_matches(dict(host_us=tick,scaling=wanted),100,200))
            self.assertFalse(presentation_scale_matches(dict(host_us=tick,scaling=2-wanted),100,200))
        for scaling in (0,2):self.assertTrue(presentation_scale_matches(dict(host_us=150,scaling=scaling),100,200))
        self.assertFalse(presentation_scale_matches(dict(host_us=150,scaling=1),100,200))

    def test_hit_window_stops_before_bounded_original_callback(self):
        from rendering_interaction_assessment import hit_window_end
        hover=dict(settled=dict(frame_input_row=10,original_frame_tick=100,host_us='1000'))
        following=dict(released=dict(frame_input_row=14,host_us='500000'),settled=dict(frame_input_row=20,original_frame_tick=2000,host_us='2000000'))
        entry=dict(kind=2,ordinal=15,tick=500)
        self.assertEqual(hit_window_end(hover,following,entry),dict(frame_input_row=14,host_us='2000000'))
        self.assertEqual(hit_window_end(hover,following,dict(entry,tick=1500))['frame_input_row'],14)
        for changes in [dict(kind=3),dict(ordinal=10),dict(ordinal=21),dict(tick=2001),dict(tick=99)]:
            with self.subTest(changes=changes),self.assertRaises(ValueError):hit_window_end(hover,following,dict(entry,**changes))
        late=dict(released=dict(frame_input_row=14,host_us='1001001'),settled=following['settled'])
        with self.assertRaises(ValueError):hit_window_end(hover,late,entry)
        late=dict(following,settled=dict(following['settled'],host_us='6001001'))
        with self.assertRaises(ValueError):hit_window_end(hover,late,entry)

    def test_completed_generation_may_swap_after_host_settle(self):
        from rendering_interaction_assessment import presented_interval
        rs=[dict(kind=1,frame=dict(generation=g)) for g in range(4)]
        released=dict(frame_input_row=1,host_us='100')
        settled=dict(frame_input_row=4,host_us='200')
        paints=[dict(world=1,host_us=150),dict(world=2,host_us=190),dict(world=3,host_us=220)]
        tail,shown=presented_interval(rs,paints,released,settled)
        self.assertEqual([r['frame']['generation'] for r in tail],[1,2,3])
        self.assertEqual(shown,paints)
        for bad in ([*paints[:2],dict(world=4,host_us=220)],
                    [*paints[:2],dict(world=3,host_us=500201)],
                    [*paints[:2],dict(world=3,host_us=100)],
                    paints[:2]):
            with self.subTest(bad=bad),self.assertRaises(ValueError):presented_interval(rs,bad,released,settled)
        released['scaling']=0
        mixed=[dict(p,scaling=0) for p in paints]+[dict(world=3,host_us=230,scaling=2)]
        self.assertEqual(len(presented_interval(rs,mixed,released,settled)[1]),3)
        with self.assertRaises(ValueError):presented_interval(rs,[dict(p,scaling=2) for p in paints],released,settled)


if __name__=='__main__':unittest.main()
