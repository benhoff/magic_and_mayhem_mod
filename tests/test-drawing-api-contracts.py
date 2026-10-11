#!/usr/bin/env python3
"""Layouts require exact imports/sites; ambiguous offsets stay unreviewed."""
import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_contracts import contract,validate,UNKNOWN_WORDS,MOVIE


class ContractTests(unittest.TestCase):
    def test_stretchdibits_has_thirteen_formal_words_and_bounded_header(self):
        result=contract({'site':0x486aca,'exact_import':{'dll':'GDI32.dll','name':'StretchDIBits'}})
        self.assertEqual(result['formal_words'],13)
        self.assertEqual(result['snapshots'],[{'argument':10,'bytes':40,'events':1,'name':'BITMAPINFOHEADER prefix'}])

    def test_exact_import_dll_and_name_both_required(self):
        for imported in [None,{'dll':'other.dll','name':'StretchDIBits'},{'dll':'gdi32.dll','name':'Unknown'},{'dll':'jpeg.dll','name':'?jpeg_read@@YI_NPADPAXHHHH@Z'}]:
            result=contract({'site':0x401000,'exact_import':imported})
            self.assertEqual((result['formal_words'],result['snapshots']),(UNKNOWN_WORDS,[]))

    def test_movie_exact_sites_match_original_fixture_prototypes(self):
        for site,(raw,name,words,slots) in MOVIE.items():
            result=contract({'site':site,'call':{'bytes':raw}})
            self.assertEqual((result['name'],result['formal_words'],result['snapshots']),(name,words,slots))

    def test_same_offset_at_another_site_gets_no_com_type(self):
        result=contract({'site':0x401000,'call':{'bytes':'ff5208'}})
        self.assertEqual(result['formal_words'],UNKNOWN_WORDS)

    def test_movie_signature_mutation_refuses(self):
        for site in MOVIE:
            with self.assertRaises(ValueError):contract({'site':site,'call':{'bytes':'ffd0'}})

    def test_release_never_has_post_object_snapshots(self):
        for site,(raw,name,words,slots) in MOVIE.items():
            if name.endswith('.Release'):
                result=contract({'site':site,'call':{'bytes':raw}});self.assertEqual(result['snapshots'],[])
                result['snapshots']=[{'argument':0,'bytes':4,'events':2,'name':'unsafe object'}]
                with self.assertRaises(ValueError):validate(result)

    def test_snapshot_bounds_and_boolean_layout_mutations_refuse(self):
        base=contract({'site':0x401000,'exact_import':{'dll':'USER32.dll','name':'ClientToScreen'}})
        for key,value in [('argument',2),('argument',True),('bytes',0),('bytes',129),('bytes',True),('events',0),('events',4),('events',True)]:
            bad=copy.deepcopy(base);bad['snapshots'][0][key]=value
            with self.assertRaises(ValueError):validate(bad)
        for words in [True,-1,17]:
            bad=copy.deepcopy(base);bad['formal_words']=words
            with self.assertRaises(ValueError):validate(bad)
        bad=copy.deepcopy(base);bad['formal_words']=UNKNOWN_WORDS
        with self.assertRaises(ValueError):validate(bad)


if __name__=='__main__':unittest.main()
