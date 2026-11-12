#!/usr/bin/env python3
import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_provenance_policy import drawing_only


class Policy(unittest.TestCase):
    def setUp(self):
        self.acquisition=dict(kind='public_native_watchdog_diagnostic_acquisition',scenario='normal-quick',player_accepted=False,
                              configuration=dict(diagnostic_only=True,operations_provenance=dict(copy_scope='all',cursor_state_version=1,qt_cursor_exercise=True)))

    def test_default_preserves_strict_input_guard(self):
        self.assertIsNone(drawing_only({},False))

    def test_drawing_result_explicitly_withholds_input_admission(self):
        result=drawing_only(self.acquisition,True)
        self.assertFalse(result['verified']);self.assertFalse(result['admitted'])

    def test_nominal_incomplete_other_scenario_or_accepted_input_refuses(self):
        for field,value in [('kind','public_native_normal_acquisition'),('scenario','normal-storage'),('player_accepted',True),('player_accepted',0)]:
            candidate=copy.deepcopy(self.acquisition);candidate[field]=value
            with self.subTest(field=field,value=value),self.assertRaises(ValueError):drawing_only(candidate,True)

    def test_diagnostic_only_and_provenance_binding_required(self):
        for field,value in [('diagnostic_only',False),('diagnostic_only',1),('operations_provenance',{})]:
            candidate=copy.deepcopy(self.acquisition);candidate['configuration'][field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):drawing_only(candidate,True)

    def test_missing_or_nonboolean_selection_refuses(self):
        with self.assertRaises(ValueError):drawing_only({},True)
        for value in [0,1,None,'true']:
            with self.subTest(value=value),self.assertRaises(ValueError):drawing_only(self.acquisition,value)

    def test_protocol_boolean_aliases_and_unknown_fields_refuse(self):
        for field,value in [('cursor_state_version',True),('qt_cursor_exercise',1),('extra',True)]:
            candidate=copy.deepcopy(self.acquisition);candidate['configuration']['operations_provenance'][field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):drawing_only(candidate,True)
        self.acquisition['configuration']=None
        with self.assertRaises(ValueError):drawing_only(self.acquisition,True)


if __name__=='__main__':unittest.main(verbosity=2)
