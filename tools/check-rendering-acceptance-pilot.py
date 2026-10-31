#!/usr/bin/env python3
"""Compare a real public-player acquisition with an independent original/history oracle."""
import argparse,hashlib,importlib.util,json,os,shlex,subprocess
from pathlib import Path
import numpy as np
from coverage_evidence import Reader,write_report
from rendering_pilot import acquisition,compare,inflate,require,sha,input_acknowledgements
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-acceptance-pilot-results-20261031/'
def digest(path):
 with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);a=p.parse_args()
 if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('New immutable owned report required')
 output=ROOT/'working/pilot'/(Path(a.report).stem+'-verification');output.mkdir(parents=True,exist_ok=False)
 report=dict(schema=1,kind='public_native_pilot_comparison',success=False,player_accepted=False,original_pixels_used_as_native_inputs=False,scope=__doc__)
 before=False
 def execute(label,command):
  with (output/(label+'.log')).open('xb') as log:subprocess.run([str(v) for v in command],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=300)
 with Reader(ROOT) as reader:
  acquired=reader.json(a.acquisition);report['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
  sources=dict(acquired['sources']);sources.update(acquired.get('native_admission',{}).get('sources',{}))
  sources.update({n:digest(ROOT/n) for n in ['tools/rendering_pilot.py','tools/check-rendering-acceptance-pilot.py','tests/acceptance-world-history.cpp','tests/acceptance-world-reference.cpp','tests/sprite-binary-reference.cpp','tests/world-raster-reference.hpp','tests/clipped-word-grayscale-cases.hpp','tools/native_player_quick_acceptance.py','tools/test-menu-observer.py']});report['sources']=sources
  try:
   require(acquired['success'] is True and acquired['sources_stable'] is True and acquired['private_wine_stopped'] is True and acquired['original_manifest_verified_before_after'] is True,'Incomplete actual public acquisition')
   require(all(digest(ROOT/n)==h for n,h in sources.items()),'Changed current pilot sources')
   artifacts={v['path']:v for v in acquired['artifacts']}
   capture_files=[v for path,v in artifacts.items() if '/capture/' in path and path.endswith('.bin')]
   observed=acquisition([reader.read_bytes(v['record']) for v in capture_files],acquired['session_identity']);report['observations']=observed['observations']
   report['captured_command_stream_sha256']=sha(observed['commands'])
   from native_player_quick_acceptance import validate_quick
   report['original_menu_actions']=validate_quick(Path(acquired['session']),acquired['flow'],0)
   ack=next(v for path,v in artifacts.items() if path.endswith('/render-input.bin.acceptance'))
   require(reader.digest(ack['record'])==ack['sha256'],'Original input receipt changed')
   report['input_acknowledgements']=input_acknowledgements(reader.read_bytes(ack['record']),acquired['flow'],observed)
   root=Path(acquired['temporary_root']);source=root/'worlds.bin';require(digest(source)==artifacts[str(source.relative_to(ROOT))]['sha256'],'World source capture differs')
   history=ROOT/'working/build/qt-shell/mnm-acceptance-world-history';original=ROOT/'working/original-reference';pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
   require(digest(pe)=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Original build mismatch')
   report['binaries']={str(v.relative_to(ROOT)):digest(v) for v in [history,original]}
   # These are oracle-side executables. Their actual compiler closure is kept
   # distinct from the already stopped public player and its runtime adapters.
   dependencies=[ROOT/'working/original-reference.d',*list((ROOT/'working/build/qt-shell').glob('CMakeFiles/mnm-acceptance-world-history.dir/**/*.o.d'))]
   compiled=set()
   for f in dependencies:
    for word in shlex.split(f.read_text().replace('\\\n',' ')):
     candidate=Path(word).resolve()
     if candidate.is_file() and candidate.is_relative_to(ROOT) and not candidate.is_relative_to(ROOT/'working'):compiled.add(str(candidate.relative_to(ROOT)))
   require('tests/acceptance-world-history.cpp' in compiled and 'tests/acceptance-world-reference.cpp' in compiled,'Independent compiler source closure missing')
   sources.update({n:digest(ROOT/n) for n in sorted(compiled)})
   execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
   assets=Path(acquired['session'])/'game'
   extensions={'.spr','.jpg','.jpeg','.bmp','.pcx','.sft','.fp','.ttd','.wav'}
   asset_hashes={str(f.relative_to(assets)):digest(f) for f in sorted(assets.rglob('*')) if f.is_file() and f.suffix.lower() in extensions}
   require(all(n in acquired['installation_inputs'] and h==acquired['installation_inputs'][n]['sha256'] for n,h in asset_hashes.items()),'Reference asset differs from the named installation')
   report['asset_inputs']=asset_hashes
   reference=output/'reference'
   execute('original-history',[history,source,Path(acquired['session'])/'game',original,pe,reference])
   index=json.loads((reference/'worlds.json').read_text());require(index['success'] is True and index['native_player_pixels_read'] is False and index['original_destination_capture_read'] is False,'Oracle history provenance')
   worlds={}
   for row in index['worlds']:
    g=int(row['generation']);require(g==len(worlds)+1 and Path(row['path']).name==row['path'],'Original World order/path')
    raw=inflate((reference/row['path']).read_bytes());require(len(raw)==row['width']*row['height']*2,'Original World full plane');worlds[g]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
   report['original_reference']=dict(worlds=len(worlds),draws=index['draws'],native_player_pixels_read=False,original_destination_capture_read=False,continuous_from_owned_initial_history=True)
   require(all(digest(assets/n)==h for n,h in asset_hashes.items()),'Reference asset changed');report['asset_inputs_stable']=True
   report['comparison']=compare(observed,worlds);report['success']=report['comparison']['success']
  except Exception as e:report['error']=str(e)
  finally:
   if before:
    try:execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
    except Exception as e:report.update(success=False,original_verification_error=str(e))
   report['sources_stable']=all(digest(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
   report['artifacts']=[]
   for f in sorted(output.rglob('*')):
    if f.is_file():
     name=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(output)).encode()).hexdigest()[:20]+'.bin';data=f.read_bytes();h=write_report(ROOT,name,data);report['artifacts'].append(dict(path=str(f.relative_to(output)),record=name,sha256=h,bytes=len(data)))
   h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','observations','original_reference']}|{'record':a.report,'sha256':h}),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
