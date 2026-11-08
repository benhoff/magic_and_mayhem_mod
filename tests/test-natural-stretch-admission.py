"""Keep natural stretch admission bounded to two rows and all four domains."""
import copy,importlib.util,json,sys,unittest
from pathlib import Path
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'tools'))
spec=importlib.util.spec_from_file_location('stretch_admit',root/'tools/close-rendering-natural-stretch.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class Admission(unittest.TestCase):
    def cases(self):
        return [dict(identity=str(i),software=soft,size=list(size),verified=True,calls=1,pixels=size[0]*size[1],
            mismatches=0,normal_retirement=True,actual_gpu_planes=True,native_policy=True,original_driver_comparison=False,
            acquisition=dict(record='a'+str(i)),comparison=dict(record='c'+str(i)),provider='control')
            for i,(soft,size) in enumerate((s,d) for s in (False,True) for d in ((640,480),(800,600)))]

    def parent(self):
        plan=json.loads((root/'docs/native-rendering-closure.json').read_bytes())
        rows=[dict(r,state='pending',admitted_execution_evidence=[]) for r in m.requirements(plan)]
        rows[0]['state']='passed'
        return plan,dict(success=True,requirements=rows,dossier_identity=m.fingerprint(m.requirements(plan)),passed_requirements=1)

    def test_only_stretch_rows_change_and_alias_stays_pending(self):
        plan,parent=self.parent();rows=m.close_rows(plan,parent,self.cases())
        self.assertEqual(sum(r['state']=='passed' for r in rows),3)
        self.assertEqual([r for r in rows if r['id']!=m.LEAF],[r for r in parent['requirements'] if r['id']!=m.LEAF])
        self.assertTrue(all(r['state']=='pending' for r in rows if r['id']=='operation.copy-alias-overlap'))

    def test_missing_duplicate_and_reused_session_refuse(self):
        cases=self.cases()
        for bad in (cases[:3],cases[:3]+[copy.deepcopy(cases[0])]):
            with self.assertRaises(ValueError):m.validate_cases(bad)
        cases=self.cases();cases[1]['identity']=cases[0]['identity']
        with self.assertRaises(ValueError):m.validate_cases(cases)

    def test_failed_or_dependent_output_refuses(self):
        for key,value in [('verified',False),('calls',0),('pixels',0),('mismatches',1),('normal_retirement',False),
                          ('actual_gpu_planes',False),('native_policy',False),('original_driver_comparison',True)]:
            cases=self.cases();cases[0][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):m.validate_cases(cases)

    def test_domain_substitution_refuses(self):
        cases=self.cases();cases[0]['size']=[640,400]
        with self.assertRaises(ValueError):m.validate_cases(cases)

    def test_parent_row_or_count_substitution_refuses(self):
        for mutation in ('profile','count'):
            plan,parent=self.parent()
            if mutation=='profile':parent['requirements'][0]['configuration']='unknown'
            else:parent['passed_requirements']=2
            with self.subTest(mutation=mutation),self.assertRaises(ValueError):m.close_rows(plan,parent,self.cases())

    def test_already_admitted_target_refuses(self):
        plan,parent=self.parent();next(r for r in parent['requirements'] if r['id']==m.LEAF)['state']='passed';parent['passed_requirements']=2
        with self.assertRaises(ValueError):m.close_rows(plan,parent,self.cases())


if __name__=='__main__':unittest.main()
