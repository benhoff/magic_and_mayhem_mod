import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from pixel_forwarding_guard import FLAG,FAMILIES,selection,authorize,decode_refusal


class Guard(unittest.TestCase):
    def test_requested_guard_requires_all_six_and_owned_staging(self):
        env={FLAG:'1',**{f:'1' for f in FAMILIES}}
        self.assertTrue(selection(env,native_draw=True,native_render=True,actions=True))
        for flag in FAMILIES:
            e=env.copy();e.pop(flag)
            with self.assertRaises(ValueError):selection(e,native_draw=True,native_render=True,actions=True)
        for flag in ('native_draw','native_render','actions'):
            args=dict(native_draw=True,native_render=True,actions=True);args[flag]=False
            with self.assertRaises(ValueError):selection(env,**args)
        with self.assertRaises(ValueError):selection({FLAG:'yes'},native_draw=True,native_render=True,actions=True)

    def test_authorization_clears_ambient_flag_and_checks_fresh_outputs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary);(root/'game').mkdir()
            env={FLAG:'1'};authorize(root,{},env);self.assertNotIn(FLAG,env)
            metadata=dict(origin='menu_action_bridge',native_draw=True,native_render=True,native_pixel_no_forward=True)
            env={**{f:'1' for f in FAMILIES},'MNM_NATIVE_PIXEL_COUNTERS':'1'}
            authorize(root,metadata,env);self.assertEqual(env[FLAG],'1')
            (root/'game/native-pixel-forward-refusal.bin').symlink_to(root/'missing')
            with self.assertRaises(ValueError):authorize(root,metadata,env)
            with self.assertRaises(ValueError):authorize(root,dict(metadata,native_pixel_no_forward=1),env)

    def test_refusal_layout_and_no_complete_claim(self):
        r=[0]*32;r[:9]=[0x504d4e4d,0x31445247,1,128,6,0,0,2,1]
        r[9:15]=r[15:21]=[(1<<n)-1 for n in (3,8,1,7,1,23)]
        self.assertFalse(decode_refusal(struct.pack('<32I',*r))['complete_rendering_replacement'])
        for word in (0,2,3,4,8,15,21):
            q=copy.copy(r);q[word]=0 if q[word] else 1
            with self.assertRaises(ValueError):decode_refusal(struct.pack('<32I',*q))


if __name__=='__main__':unittest.main()
