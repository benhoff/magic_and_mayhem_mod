#!/usr/bin/env python3
"""Native terminal-state helper guards; no original program is executed."""
import argparse,hashlib,json,os,struct,subprocess
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/rendering-transition-recovery-results-20261107/'
SOURCES=['tools/test-rendering-transition-fault.py','tools/rendering-transition-fault.cpp','protocols/include/mnm/render_command_ring.h','protocols/include/mnm/render_commands_v2.h','protocols/include/mnm/render_commands_v3.h']
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args();out=a.output.resolve()
 if not out.is_relative_to(ROOT/'working') or out.exists() or not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('Fresh owned controls required')
 out.mkdir(parents=True);(out/'.retain').touch();q=dict(schema=1,kind='terminal_fault_helper_controls',success=False,sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in SOURCES},checks=[],original_game_runs=0)
 try:
  for mode,flags in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   binary=out/mode
   for phase,cmd in [('build',['g++','-std=c++17','-Wall','-Wextra','-Werror',*flags,str(ROOT/'tools/rendering-transition-fault.cpp'),'-o',str(binary)]),('self-test',[str(binary),'--self-test'])]:
    r=subprocess.run(cmd,capture_output=True,text=True,cwd=ROOT,timeout=120,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'));q['checks'].append(dict(mode=mode,phase=phase,command=cmd,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr));assert r.returncode==0,r.stderr
    if phase=='self-test':assert json.loads(r.stdout)==dict(success=True,cases=7,only_state_reason_changed=True)
   fake=out/(mode+'-repo');channel=fake/'working/experiments/menu-observer/run-control/render-commands.bin';channel.parent.mkdir(parents=True)
   # The native helper's ABI fixes the size. Retain the compiled constant through
   # a tiny compile-time probe rather than duplicate it in a Python fixture.
   probe=out/'size.cpp';probe.write_text('#include "'+str(ROOT/'protocols/include/mnm/render_commands_v3.h')+'"\n#include <cstdio>\nint main(){std::printf("%u",unsigned(MNM_RENDER_COMMANDS_V3_SIZE));}\n')
   subprocess.run(['g++',str(probe),'-o',str(out/'size')],check=True,capture_output=True);size=int(subprocess.check_output([out/'size']))
   header=bytearray(64);header[:8]=b'MNMRGL03';struct.pack_into('<8I',header,8,3,size,37,0,1,0,0,0)
   # Use the actual public magic from the header; no hand-authored substitute.
   import re
   data=(ROOT/'protocols/include/mnm/render_commands_v3.h').read_text();magic=re.search(r'MNM_RENDER_COMMANDS_V3_MAGIC\s+"([^"\\]+)"',data).group(1).encode();header[:8]=magic
   with channel.open('wb') as f:f.write(header);f.truncate(size)
   r=subprocess.run([binary,str(fake),str(channel),'37'],capture_output=True,text=True,timeout=30,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'));q['checks'].append(dict(mode=mode,phase='owned-mapped-input',exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr));assert r.returncode==0,r.stderr
   result=json.loads(r.stdout);after=channel.read_bytes();expected=bytearray(header);struct.pack_into('<2I',expected,24,3,5);assert after[:64]==expected and not any(after[64:]) and result['cursor_or_payload_written'] is False
   before=channel.read_bytes();r=subprocess.run([binary,str(fake),str(channel),'37'],capture_output=True,text=True,timeout=30);assert r.returncode!=0 and channel.read_bytes()==before
   q['checks'].append(dict(mode=mode,phase='duplicate-terminal-refused',exit_code=r.returncode,stderr=r.stderr,unchanged=True))
  q['success']=True;q['native_self_test_cases']=14;q['mapped_cases']=4
 except Exception as e:q['error']=str(e)
 finally:
  q['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in q['sources'].items());q['success']&=q['sources_stable'];q['artifacts']=[]
  for f in sorted(out.rglob('*')):
   if f.is_file():
    raw=f.read_bytes();name=a.report.removesuffix('.json')+'-raw-'+hashlib.sha256(str(f.relative_to(out)).encode()).hexdigest()[:20]+'.bin';q['artifacts'].append(dict(path=str(f.relative_to(out)),record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw)))
  sha=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=sha)))
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
