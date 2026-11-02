#!/usr/bin/env python3
"""Synthetic callback ordering based on the observed report-before-World-pop path."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_report import report_callback,REPORT


def rows():
    begin=[0]*16;begin[0]=25;begin[1]=2;begin[2]=316;begin[3]=6;begin[4]=REPORT
    begin[5]=0x5c5ebc;begin[6]=1;begin[9]=21;begin[11]=REPORT;begin[12]=5
    end=begin.copy();end[0]=26;end[1]=3
    popped=[0]*16;popped[0]=31;popped[1]=28;popped[2]=316;popped[3]=2;popped[11]=0x659408
    return [begin,end,popped]


class ReportBoundary(unittest.TestCase):
    def test_callback_before_world_pop_is_admitted_with_distinct_return_proof(self):
        r=rows();self.assertEqual(report_callback(r[:1])['begin_sequence'],25)
        self.assertIsNone(report_callback(r[:1],require_return=True))
        result=report_callback(r,316,True);self.assertEqual(result['return_sequence'],26)
        self.assertNotEqual(r[-1][11],REPORT)

    def test_world_pop_alone_and_foreign_callback_identity_cannot_supply_report(self):
        self.assertIsNone(report_callback(rows()[2:]))
        for index,value in ((1,3),(2,0),(3,17),(4,REPORT+1),(5,0),(6,0),(9,4),(11,0x659408),(12,4)):
            r=rows();r[0][index]=value
            with self.subTest(field=index):self.assertIsNone(report_callback(r,316,True))

    def test_return_must_match_dispatched_owner_object_and_vtable(self):
        for index in (2,3,4,5,9):
            r=rows();r[1][index]+=1
            with self.subTest(field=index):self.assertIsNone(report_callback(r,316,True))
        self.assertIsNone(report_callback(rows(),999,True))

    def test_abandon_before_report_is_not_natural_report(self):
        r=rows();quit_row=[0]*16;quit_row[0]=24;quit_row[1]=3;quit_row[3]=17;quit_row[9]=3
        self.assertIsNone(report_callback([quit_row,*r],316,True))


if __name__=='__main__':unittest.main()
