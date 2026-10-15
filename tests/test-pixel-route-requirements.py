#!/usr/bin/env python3
"""Route requirements cannot be satisfied by installation or original forwards."""
import copy
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from native_pixel_counters import ROUTES
from pixel_route_requirements import validate_routes
from borrowed_pixel_backend_launch import selection, reject_retired_dc_hook, FEATURES
from native_pixel_counters import read_counters
import struct
import tempfile


class RouteRequirementsTests(unittest.TestCase):
    def fixture(self):
        return dict(families={f:[dict(route=r,installed=1,native=int(f=='minimap' and r=='camera3'),forwarded=0)
                                for r in routes] for f,routes in ROUTES.items()},
                    native=1,forwarded=0,history_refused=False)

    def test_only_executed_native_route_meets_requirement(self):
        r=validate_routes(self.fixture(),['minimap/camera3'])
        self.assertEqual(r['exercised'],['minimap/camera3'])
        self.assertEqual(len(r['unexercised']),42)
        self.assertFalse(r['complete_replacement'])
        with self.assertRaises(ValueError):validate_routes(self.fixture(),['raster/splice22'])

    def test_forward_outside_required_route_also_fails(self):
        c=self.fixture();c['families']['raster'][0]['forwarded']=1;c['forwarded']=1
        with self.assertRaises(ValueError):validate_routes(c,['minimap/camera3'])

    def test_incomplete_refused_or_inconsistent_diagnostics_fail(self):
        for case in ('empty','refusal','missing','reordered','install','negative','total','unknown','duplicate'):
            c=copy.deepcopy(self.fixture());required=['minimap/camera3']
            if case=='empty':c['families']['minimap'][-1]['native']=0;c['native']=0
            elif case=='refusal':c['history_refused']=True
            elif case=='missing':c['families']['raster'].pop()
            elif case=='reordered':c['families']['raster'].reverse()
            elif case=='install':c['families']['panel'][0]['installed']=0
            elif case=='negative':c['families']['panel'][0]['native']=-1
            elif case=='total':c['native']=2
            elif case=='unknown':required=['minimap/unknown']
            else:required*=2
            with self.subTest(case=case),self.assertRaises(ValueError):validate_routes(c,required)

    def test_retired_client_to_screen_sites_cannot_be_selected_or_authorized(self):
        env=selection({'MNM_OTHER_PIXEL_PRODUCERS':'1'})
        self.assertNotIn('dc_copy',FEATURES)
        self.assertNotIn('MNM_DC_COPY_TAKEOVER',env)
        for flag in ('1','2','garbage'):
            with self.assertRaises(ValueError):selection({'MNM_DC_COPY_TAKEOVER':flag})
        with self.assertRaises(ValueError):reject_retired_dc_hook({},dict(native_dc_copy_takeover=True))

    def test_v2_requires_exact_extent_and_rejects_legacy_invalid_routes(self):
        with tempfile.TemporaryDirectory() as d:
            game=Path(d);values=[0]*(4+6*32*3);values[:4]=[0x50504e4d,2,len(values)*4,6]
            for i,routes in enumerate(ROUTES.values()):
                for j in range(len(routes)):values[4+(i*32+j)*3]=1
            raw=struct.pack('<'+'I'*len(values),*values)
            path=game/'native-pixel-producers.bin';path.write_bytes(raw)
            self.assertEqual(read_counters(game)['schema'],2)
            for bad in (raw[:-4],raw+b'\0'*4,raw[:4]+struct.pack('<I',1)+raw[8:]):
                path.write_bytes(bad)
                with self.assertRaises(ValueError):read_counters(game)

    def test_camera_scenario_requires_counter_progress_for_every_orientation(self):
        from campaign_pixel_routes import camera_scenarios
        calls=[];samples=[];counts=[0]*4
        def counters(game):
            return dict(forwarded=0,history_refused=False,families=dict(minimap=[dict(route=f'camera{i}',native=n) for i,n in enumerate(counts)]))
        def key(sym,seconds):
            calls.append(sym);counts[(0,2,0,1)[(len(calls)-1)%4]]+=1
        with patch('campaign_pixel_routes.read_counters',side_effect=counters),patch('campaign_pixel_routes.time.sleep'):
            result=camera_scenarios(Path('.'),key,lambda action,**kw:samples.append((action,kw)))
            self.assertEqual(result['observed'],['camera0','camera1','camera2'])
            self.assertEqual(len(calls),4)
            counts[:]=[0]*4
            with self.assertRaises(ValueError):camera_scenarios(Path('.'),lambda *a:None,lambda *a,**k:None)


if __name__=='__main__':unittest.main()
