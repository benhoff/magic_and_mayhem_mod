#!/usr/bin/env python3
import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_route import hud_point,project,aim_point,approach_point,validate_opening_anchors,bridge_entry_ready,bridge_defender_slot,portal_points,follow_staging_goal_reached,boss_candidate_points


class Route(unittest.TestCase):
    def test_boss_hover_includes_feet_when_body_projection_hits_rear_guard(self):
        hero=dict(x=42,y=31,z=4);boss=dict(x=42,y=26,z=4)
        self.assertEqual(boss_candidate_points(hero,boss),[(560,198),(560,220),(560,180),(560,166)])
        self.assertEqual(boss_candidate_points(hero,dict(x=200,y=200,z=4)),[])
        for size in ((640,480),(800,600)):
            self.assertTrue(all(64<=y<size[1]-90 for x,y in boss_candidate_points(hero,boss,size)))
    def test_retained_quantized_follow_goal_does_not_repeat_the_current_cell(self):
        hero=dict(x=26,y=55,z=4);goal=dict(x=26.391993216020353,y=54.88000423998728,z=4)
        self.assertTrue(follow_staging_goal_reached(hero,goal))
        for changed in (dict(x=25),dict(y=54),dict(z=5)):
            self.assertFalse(follow_staging_goal_reached(dict(hero,**changed),goal))
        self.assertFalse(follow_staging_goal_reached(hero,None))
        self.assertFalse(follow_staging_goal_reached(hero,dict(goal,x=float('nan'))))
    def test_portal_alternatives_are_bounded_physical_points_in_both_frames(self):
        hero=dict(x=43,y=28,z=4);portal=dict(x=43,y=26,z=4)
        points=portal_points(hero,portal)
        self.assertEqual(points[:3],[(464,268),(464,300),(496,284)])
        for size in ((640,480),(800,600)):
            values=portal_points(hero,portal,size)
            self.assertTrue(1<=len(values)<=12)
            self.assertTrue(all(48<=x<size[0]-40 and 64<=y<size[1]-90 for x,y in values))
        self.assertEqual(portal_points(hero,dict(x=400,y=400,z=4)),[])
    def test_refilled_bridge_entry_requires_the_waypoint_before_final_approach(self):
        bridge=dict(x=31,y=52,z=4)
        # Retained v25 left staging at29,53 then never reached Camulos.
        short=dict(x=29,y=53,z=4)
        self.assertFalse(bridge_entry_ready(short,bridge,False))
        self.assertFalse(bridge_entry_ready(short,bridge,True))
        for point in (dict(x=30,y=52,z=4),dict(x=31,y=51,z=4),dict(x=31,y=52,z=5)):
            self.assertFalse(bridge_entry_ready(point,bridge,True))
        self.assertTrue(bridge_entry_ready(bridge,bridge,True))
        self.assertFalse(bridge_entry_ready(dict(x=20,y=52,z=4),bridge,False))

    def test_initial_bridge_refuses_the_retained_high_resolution_stall_pose(self):
        bridge=dict(x=31,y=52,z=4)
        # Original800x600 attempt advanced here, drifted to20,49 and timed out.
        self.assertFalse(bridge_entry_ready(dict(x=28,y=52,z=4),bridge,False))
        self.assertFalse(bridge_entry_ready(dict(x=29,y=52,z=4),bridge,False))
        self.assertTrue(bridge_entry_ready(dict(x=30,y=52,z=4),bridge,False))
        self.assertTrue(bridge_entry_ready(dict(x=30,y=51,z=4),bridge,False))
        self.assertTrue(bridge_entry_ready(bridge,bridge,False))

    def test_hud_is_anchored_to_native_right_bottom_edges(self):
        self.assertEqual(hud_point((477,575),(640,480)),(317,455))
        self.assertEqual(hud_point((523,575),(640,480)),(363,455))
        self.assertEqual(hud_point((744,522),(640,480)),(584,402))
        self.assertEqual(hud_point((477,575)),(477,575))

    def test_aim_never_scales_a_visible_creature_to_different_ground(self):
        hero=dict(x=0,y=0,z=4);target=dict(x=5,y=-4,z=4)
        self.assertEqual(project(hero,target,body=True),(688,294))
        self.assertEqual(aim_point(hero,target,body=True),(688,294))
        self.assertNotEqual(approach_point(hero,target),(688,316))
        self.assertIsNone(aim_point(hero,target,(640,480),True))
        self.assertEqual(project(hero,dict(x=1,y=0,z=4),(640,480)),(352,256))

    def test_remote_ground_orders_stay_bounded_in_both_frame_modes(self):
        for size in ((640,480),(800,600)):
            for x,y,z in ((100,-100,40),(-80,-90,0),(0,0,4)):
                point=approach_point(dict(x=0,y=0,z=4),dict(x=x,y=y,z=z),size)
                self.assertTrue(0<=point[0]<size[0] and 0<=point[1]<size[1])

    def test_spawn_anchor_proof_does_not_assume_slot_numbers(self):
        route=dict(points={'CF50 Redcap1_1':dict(x=7,y=67,z=4),'CF52 Bridge':dict(x=31,y=52,z=4)})
        rows=[]
        for slot,point in ((15,(7,67,4)),(70,(31,52,4))):
            r=[0]*16;r[1]=1;r[4]=slot;r[5]=10;r[6]=2;r[9:12]=point;rows.append(r)
        self.assertTrue(validate_opening_anchors(route,rows)['verified'])
        self.assertEqual(bridge_defender_slot(route,rows),70)
        # Later movement does not change the spawn identity.
        moved=list(rows[1]);moved[9:12]=(35,48,4)
        self.assertEqual(bridge_defender_slot(route,rows+[moved]),70)
        duplicate=list(rows[1]);duplicate[4]=71
        with self.assertRaises(ValueError):bridge_defender_slot(route,rows+[duplicate])
        with self.assertRaises(ValueError):bridge_defender_slot(route,rows[:1])
        rows[0][9]=8
        with self.assertRaises(ValueError):validate_opening_anchors(route,rows)


if __name__=='__main__':unittest.main()
