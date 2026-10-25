#!/usr/bin/env python3
"""Fresh bounded initializer request proof with a corrected prospective contract."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import read_bytes,write_report
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def digest(path):return hashlib.sha256(read_bytes(path,root=ROOT) if path.resolve().is_relative_to(ROOT) else path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--main-root',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve();target=a.report.absolute()
 if out.exists() or target.exists() or not out.is_relative_to(ROOT/'working/tests') or not target.is_relative_to(ROOT/'research/runtime'):p.error('Require new owned execution and immutable report paths')
 out.mkdir(parents=True);(out/'.retain').write_text('Corrected initializer contract, original requests, native policies, binaries and logs.\n');(out/'tmp').mkdir()
 env=dict(os.environ,TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='RS.alternate-canvas-initializer');builds={b['id']:b for b in r['builds']}
 sources=required_sources(b)|set(b['documents'])|{'tests/graphics-initializer-plan-test.cpp','tools/test-graphics-initializer-plan.py','tests/sprite-binary-reference.cpp'}
 files=['tests/graphics-initializer-plan-test.cpp','tests/alternate-canvas-initializer.cpp','reconstruction/rendering/graphics_initializer.cpp']
 flags=['g++','-std=c++17','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']
 deps=subprocess.check_output(flags+['-MM',*files],cwd=ROOT,env=env,text=True)
 for token in shlex.split(deps.replace('\\\n',' ')):
  path=ROOT/token
  if path.is_file():sources.add(path.resolve().relative_to(ROOT).as_posix())
 report=dict(success=False,sources={path:digest(ROOT/path) for path in sorted(sources)},claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds))],commands=[],original_sha256=BUILD,live_replacement=False,real_surface_pixels=False,scope='Successful16-bit owned initializer request plan over the existing recovered backend. Complete descriptors, mode/padding/clear effects and ordering compare with unchanged original initializer/clear/format wrappers at owned successful boundaries. Native extent overflow refusal is policy. No real surface allocation/clear pixels, masks/provider failure, alternate publisher or live acceptance.')
 def run(label,argv,structured=False):
  command=list(map(str,argv));v=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,timeout=300);payload=v.stdout+b'\n'+v.stderr;(out/(label+'.log')).write_bytes(payload);report['commands'].append(dict(label=label,argv=command,exit_code=v.returncode,output=payload.decode()));v.check_returncode();return json.loads(v.stdout) if structured else None
 before=False
 try:
  run('manifest-before',[a.main_root/'tools/original-manifest.sh','verify']);before=True
  exe=a.main_root/'original/Arcane_Nocd/Chaos.exe';report['input_sha256']=digest(exe);assert report['input_sha256']==BUILD
  for mode,options in [('release',['-O2','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])]:
   binary=out/mode;run(mode+'-build',flags+options+[ROOT/files[0],ROOT/files[2],'-o',binary]);report[mode]=run(mode,[binary],True);assert report[mode]['success']
  binary=out/'original';run('original-build',flags+['-m32','-O2','-fno-pie','-no-pie',ROOT/files[1],ROOT/files[2],'-o',binary]);report['original']=run('original',[binary,exe],True)
  assert report['original']['cases']==288 and report['original']['complete_descriptors_compared']==576 and report['original']['original_initializer_clear_format_unchanged'] and report['original']['alternate_storage_unchanged']
  report['sources_stable']=all(digest(ROOT/p)==h for p,h in report['sources'].items());report['inputs_stable']=digest(exe)==report['input_sha256'];assert report['sources_stable'] and report['inputs_stable'];report['success']=True
 except BaseException as error:report['failure']=dict(type=type(error).__name__,message=str(error));raise
 finally:
  try:run('manifest-after',[a.main_root/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=before
  except BaseException:report['success']=False;raise
  finally:write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps(dict(success=True,native_cases=report['release']['cases'],original_cases=288)))
if __name__=='__main__':main()
