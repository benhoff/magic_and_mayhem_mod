#!/usr/bin/env python3
"""Reject false crowded claims using original observation wire rows."""
import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_crowded import crowded_metrics


class CrowdedTests(unittest.TestCase):
    def fixture(self):
        rows=[]
        for tick in range(0,41000,250):
            for slot in range(4 if tick == 0 else 14):
                r=[0]*16;r[:9]=[len(rows)+1,1,99,tick,slot,0 if slot==0 else 14,0 if slot<10 else 1,1,800 if slot==0 else 100]
                r[9:11]=[slot%4,slot//4];rows.append(r)
        hit=[0]*16;hit[:14]=[len(rows)+1,2,99,40000,4,14,0,100,10,14,1,100,95,1];rows.append(hit)
        spells=[]
        for i in range(8):
            r=[0]*16;r[:10]=[i+1,1,99,1000+i*100,14,0,0,0,200*256,(200-17)*256];r[15]=1;spells.append(r)
        return rows,spells

    def test_population_requires_original_summons_and_combat(self):
        rows,spells=self.fixture();self.assertGreaterEqual(crowded_metrics(rows,spells,0,0)['crowded_seconds'],30)
        for mutation in ('small','one-owner','remote','dead','short','no-melee','no-debit','thread','gap'):
            r,s=copy.deepcopy(rows),copy.deepcopy(spells)
            if mutation=='small':r=[v for v in r if v[1]!=1 or v[4]<10]
            if mutation=='one-owner':
                for v in r:
                    if v[1]==1:v[6]=0
            if mutation=='remote':
                for v in r:
                    if v[1]==1 and v[4]>0:v[9]=1000
            if mutation=='dead':r[-2][4:9]=[0,0,0,1,0]
            if mutation=='short':r=[v for v in r if v[3]<25000]
            if mutation=='no-melee':r=r[:-1]
            if mutation=='no-debit':
                for v in s:v[9]=v[8]
            if mutation=='thread':s[0][2]=100
            if mutation=='gap':r=[v for v in r if not 10000<v[3]<20000]
            with self.subTest(mutation=mutation),self.assertRaises(ValueError):crowded_metrics(r,s,0,0)

if __name__=='__main__':unittest.main()
