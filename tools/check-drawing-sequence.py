#!/usr/bin/env python3
"""Compare every retained independent producer checkpoint with native OpenGL."""
import argparse
import json
from pathlib import Path
import subprocess
from drawing_corpus import ROOT, sha
from drawing_sequence import CATALOG, check
from importlib import import_module


def sources():
    paths=set()
    for directory in ('assets','renderer','compat/legacy','protocols/include'):
        for p in (ROOT/directory).rglob('*'):
            if p.is_file() and (p.suffix in {'.cpp','.hpp','.c','.h','.S','.cmake'} or p.name=='CMakeLists.txt'):
                paths.add(p)
    paths.update(p for p in (ROOT/'tests/fixtures/drawing-sequence').iterdir() if p.is_file())
    paths.update(ROOT/p for p in ('tools/drawing_sequence.py','tools/check-drawing-sequence.py',
        'tools/retain-drawing-sequence.py','tools/drawing_corpus.py','tools/drawing_scene_corpus.py',
        'tools/retain-drawing-scene.py','tools/inspect-canvas-producers.py','tools/rendering_coverage.py',
        'tools/coverage_audit.py','tests/test-drawing-sequence.py','research/runtime/draw02-full-session.md'))
    return {str(p.relative_to(ROOT)):sha(p.read_bytes()) for p in sorted(paths)}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--corpus',default=CATALOG);p.add_argument('--executable',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True);p.add_argument('--headless',action='store_true')
    p.add_argument('--claims',type=Path)
    a=p.parse_args()
    try:
        output=import_module('check-drawing-corpus').output_path(a.report)
        fingerprints=sources();claims=json.loads(a.claims.read_text()) if a.claims else None
        if claims:
            if not isinstance(claims['claims'],list) or not claims['claims']:
                raise ValueError('Execution claims must be a nonempty list')
            for name,digest in claims['sources'].items():
                from drawing_corpus import local
                if sha(local(ROOT,name).read_bytes())!=digest:raise ValueError('Prospective sequence source changed')
                fingerprints[name]=digest
        result=check(catalog_path=a.corpus,executable=a.executable,headless=a.headless)
        if any(sha((ROOT/name).read_bytes())!=h for name,h in fingerprints.items()):
            raise ValueError('Sequence comparison sources changed')
        result.update(sources=fingerprints,sources_stable=True)
        if claims:result['claims']=claims['claims']
        output.parent.mkdir(parents=True,exist_ok=True)
        with output.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
        print(json.dumps({k:result[k] for k in ('success','pixels_compared','mismatches','draw02_complete','artifacts')}))
        return int(not result['success'])
    except (ValueError,OSError,KeyError,TypeError,subprocess.TimeoutExpired) as e:p.exit(2,str(e)+'\n')

if __name__=='__main__':raise SystemExit(main())
