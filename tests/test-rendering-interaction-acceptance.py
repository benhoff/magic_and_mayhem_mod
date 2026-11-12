import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('view_admission', ROOT / 'tools/close-rendering-interaction-acceptance.py')
module = importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
from coverage_claims import fingerprint
from rendering_acceptance import requirements


class FourViewAdmission(unittest.TestCase):
    def setUp(self):
        self.plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        rows = [dict(r, state='pending') for r in requirements(self.plan)]
        self.parent = dict(requirements=rows, passed_requirements=0,
                           dossier_identity=fingerprint(requirements(self.plan)))
        cases = []
        for software in (False, True):
            for size in ([800,600], [640,480]):
                views, routes = [], []
                for i,(mode,view) in enumerate((m,v) for m in ('sharp','integer') for v in range(4)):
                    frames = [i*3+j+1 for j in range(3)]
                    views.append(dict(mode=mode, view=view, frames=frames))
                    for g in frames:routes.append(dict(generation=g,camera=dict(view=view,creature_markers=1,
                        terrain_sequence=1,camera_sequence=2,cell_sequences=[3],creature_sequence=4,
                        raster=[dict(backend=b) for b in (1,2,9,10)])))
                cases.append(dict(domain=[software,size],verified=True,cleanup=dict(verified=True,normal_quit=True),
                                  views=views,four_view_source_routes=routes,source_generations=24,world_presentations=24))
        self.matrix = dict(success=True,sources_stable=True,four_view_required_routes_proved=True,
                           mismatching_pixels=0,cases=cases)

    def test_only_two_exact_frozen_rows_are_admitted(self):
        rows = module.close_rows(self.plan,self.parent,self.matrix,dict(record='matrix',sha256='a'*64))
        self.assertEqual([r['id'] for r in rows if r['state']=='passed'],[module.LEAF,module.LEAF])
        self.assertEqual([r for r in rows if r['id']!=module.LEAF],
                         [r for r in self.parent['requirements'] if r['id']!=module.LEAF])

    def test_missing_duplicate_or_partial_domains_refuse(self):
        for modify in (lambda m:m['cases'].pop(),lambda m:m['cases'].__setitem__(3,copy.deepcopy(m['cases'][0])),
                       lambda m:m.__setitem__('mismatching_pixels',1),lambda m:m['cases'][0]['views'].pop()):
            matrix=copy.deepcopy(self.matrix);modify(matrix)
            with self.assertRaises(ValueError):module.close_rows(self.plan,self.parent,matrix,{})

    def test_missing_effect_or_minimap_route_refuses(self):
        for field,value in [('raster',[dict(backend=b) for b in (1,2,9)]),('cell_sequences',[]),('creature_markers',0)]:
            matrix=copy.deepcopy(self.matrix);matrix['cases'][0]['four_view_source_routes'][0]['camera'][field]=value
            with self.assertRaises(ValueError):module.close_rows(self.plan,self.parent,matrix,{})

    def test_changed_denominator_or_already_accepted_leaf_refuses(self):
        parent=copy.deepcopy(self.parent);parent['requirements'].pop()
        with self.assertRaises(ValueError):module.close_rows(self.plan,parent,self.matrix,{})
        parent=copy.deepcopy(self.parent);next(r for r in parent['requirements'] if r['id']==module.LEAF)['state']='passed';parent['passed_requirements']=1
        with self.assertRaises(ValueError):module.close_rows(self.plan,parent,self.matrix,{})


if __name__ == '__main__':
    unittest.main()
