#!/usr/bin/env python3
"""Retain production palette observer controls, bytes and source in SQLite."""
import argparse,json,os,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/palette-cycling-results-20261108/'
NAMES=('runtime/scene/palette_observation.h','runtime/scene/palette_observation.S',
       'runtime/scene/canvas_producers.c','runtime/scene/observer.c',
       'tools/build-scene-observer.py','tools/palette_observation.py',
       'tools/check-palette-observation.py','tests/palette-observation-fixture.c',
       'tests/test-palette-observation.py','research/formats/palette-observation-v1.md',
       'tests/palette-observation-abi-fixture.c','tests/palette-observation-abi.S',
       'protocols/include/mnm/world_continuous_checksum.h',
       'research/formats/palette-observation-v2.md','research/formats/palette-profile-observation-v1.md','research/formats/palette-observation-v3.md')

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
 if not a.report.startswith(PREFIX)or '..'in Path(a.report).parts or(ROOT/a.report).exists():p.error('Fresh owned report required')
 out=ROOT/'working/palette-cycling'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False)
 (out/'.retain').write_text('Palette production controls, negatives, exact tested bytes. Preserve.\n')
 def put(suffix,raw):
  name=a.report.removesuffix('.json')+'-'+suffix;return dict(record=name,sha256=write_report(ROOT,name,raw))
 with Reader(ROOT)as reader:sources={n:reader.digest(n)for n in NAMES}
 source_inputs={n:put('source-'+str(i)+'.bin',(ROOT/n).read_bytes())for i,n in enumerate(NAMES)}
 command=[sys.executable,'-B',str(ROOT/'tests/test-palette-observation.py')]
 result=subprocess.run(command,cwd=ROOT,capture_output=True,timeout=60,env=dict(os.environ,MNM_PALETTE_CONTROL_OUTPUT=str(out/'encoder')))
 artifacts={str(f.relative_to(out)):put('artifact-'+str(i)+'.bin',f.read_bytes())for i,f in enumerate(sorted(out.rglob('*')))if f.is_file()}
 log=put('log.bin',result.stdout+result.stderr)
 with Reader(ROOT)as reader:stable=all(reader.digest(n)==h for n,h in sources.items())
 q=dict(schema=1,kind='palette_observation_production_controls',success=result.returncode==0 and stable,
        exit_code=result.returncode,command=command,sources=sources,sources_stable=stable,source_inputs=source_inputs,
        artifacts=artifacts,log=log,original_game_runs=0,original_oracle_runs=0,native_player_runs=0,
        closes_palette_rows=False,scope='Production encoder/decoder and signature, bounds, order, owner, identity, selection, mutation and lifetime refusals on owned inputs; actual production i386 hook assembly with authored RET4/cdecl callees and deliberately clobbered x87/SSE callbacks. No unchanged original function, actual installed palette cycling or native display credit.')
 # The named immutable report is the canonical typed result, not an expanded file.
 digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
 print(json.dumps(dict(success=q['success'],record=a.report,sha256=digest,log=log)),flush=True)
 return int(not q['success'])

if __name__=='__main__':raise SystemExit(main())
