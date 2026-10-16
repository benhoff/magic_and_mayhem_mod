#!/usr/bin/env python3
"""Replay every portable recorded frame through native source/GPU/X11 output."""
import argparse,json
from pathlib import Path
from drawing_corpus import ROOT,local,sha
from drawing_recorded_session import CATALOG,check

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--catalog',default=CATALOG);p.add_argument('--native',type=Path,required=True);p.add_argument('--presenter',type=Path,required=True);p.add_argument('--headless',action='store_true');p.add_argument('--claims',type=Path);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing result overwrite')
 claims=json.loads(a.claims.read_text()) if a.claims else {'claims':[],'sources':{}};sources=dict(claims['sources'])
 names=['tools/check-recorded-drawing-session.py','tools/drawing_recorded_session.py','tools/recorded_drawing_plan.py','tools/compare-drawing-quick-battle.py','tools/drawing_corpus.py','tools/drawing_scene_corpus.py','tools/drawing_producer_points.py','tools/drawing_call_intervals.py','tools/drawing_api_session.py','tools/drawing_session.py','tools/drawing_journeys.py','tools/rendering_coverage.py','tools/inspect-canvas-producers.py','apps/drawing-session/main.cpp','apps/drawing-session/recorded_composition.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp']
 for directory in ('tests/fixtures/drawing-recorded-session','tests/fixtures/drawing-storage-session','renderer','assets','compat/legacy','protocols/include','reconstruction/rendering'):
  names.extend(str(f.relative_to(ROOT)) for f in (ROOT/directory).rglob('*') if f.is_file() and (f.suffix in ('.cpp','.hpp','.h','.c','.S','.cmake','.json','.gz') or f.name=='CMakeLists.txt'))
 for name in names:
  digest=sha(local(ROOT,name).read_bytes())
  if name in sources and sources[name]!=digest:raise ValueError('Prospective recorded-session source changed')
  sources[name]=digest
 result=check(root=ROOT,catalog_path=a.catalog,native=a.native,presenter=a.presenter,headless=a.headless)
 if any(sha(local(ROOT,n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Recorded-session source/fixture changed')
 result.update(sources=sources,sources_stable=True,claims=claims['claims']);a.report.parent.mkdir(parents=True,exist_ok=True)
 with a.report.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
 print(json.dumps({k:result[k] for k in ('success','frames','pixels_compared','mismatches','artifacts')}));return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
