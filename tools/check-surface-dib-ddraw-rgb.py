#!/usr/bin/env python3
"""Offline full native-word and DC RGB comparison from independent Surface2 captures."""
import argparse,gzip,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-dib-ddraw-rgb-capture-20261006.json'
spec=importlib.util.spec_from_file_location('dc',ROOT/'tools/check-surface-dib-ddraw.py');dc=importlib.util.module_from_spec(spec);spec.loader.exec_module(dc)
require=dc.require;sha=dc.sha

def load(report=DEFAULT):
 c=json.loads(report.read_text());require(c['schema']==1 and c['success'] is True,'Invalid full DC capture');p=c['corpus']['path'];require(p=='tests/fixtures/surfaces/surface-dib-ddraw-rgb.json.gz','Unsafe full fixture path');require(sha(ROOT/p)==c['corpus']['sha256'],'Full DC fixture differs')
 with gzip.open(ROOT/p,'rb') as f:raw=f.read(48*1024*1024+1)
 require(len(raw)<=48*1024*1024,'Expanded full fixture limit');data=json.loads(raw);require(data['schema']==1 and len(data['rows'])==c['cases']==72,'Full DC case count');return c,data

def compare(payload,assets):
 counts,states=dc.compare(payload,assets);counts['dc_rgb_pixels']=0;counts['floor_rgb_differences']=0;levels=[set(),set(),set()]
 for row,state in zip(payload['rows'],states,strict=True):
  bits=row['input']['bits'];words=state['expected'];expected=[dc.replicated565(p) if bits==16 else p for p in words];actual=[dc.colorref(p) for p in row['dc_pixels']];require(actual==expected,f'Full DC RGB differs: case{row["input"]["id"]}')
  counts['dc_rgb_pixels']+=len(expected)
  if bits==16:
   counts['floor_rgb_differences']+=sum(dc.expand565(p)!=rgb for p,rgb in zip(words,expected,strict=True))
   for p in words:levels[0].add((p>>11)&31);levels[1].add((p>>5)&63);levels[2].add(p&31)
 counts['rgb565_channel_levels']=[len(x) for x in levels];require(counts['rgb565_channel_levels']==[32,64,32],'Incomplete RGB565 channel coverage');return counts,states

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path);a=p.parse_args();cap,data=load();original,_=dc.gdi.sentinel.load();counts,_=compare(data,original['assets']);names=['tools/check-surface-dib-ddraw-rgb.py','tests/test-surface-dib-ddraw-rgb.py','tools/check-surface-dib-ddraw.py','tools/check-surface-dib-gdi.py',str(DEFAULT.relative_to(ROOT)),cap['corpus']['path'],'tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz'];out=dict(schema=1,success=True,sources={n:sha(ROOT/n) for n in names},**counts,scope='72 real Surface2 GetDC/GDI/ReleaseDC native-word and complete DC GetPixel RGB frames match input-derived model; all32/64/32 RGB565 channel levels represented. High unusedRGB32 byte excluded. Floor-scaling mismatch counted separately. No original instructions, actual loss, indexed/DC clipping, Windows hardware or live/wire equivalence.')
 if a.report:
  with a.report.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
