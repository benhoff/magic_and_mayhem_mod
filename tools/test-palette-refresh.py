#!/usr/bin/env python3
"""Compare recovered in-place World palette refresh at observed and default x87 precision."""
import argparse,hashlib,json,os,shlex,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/palette-cycling-results-20261108/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned refresh matrix required')
 out=ROOT/'working/palette-cycling'/Path(a.report).stem;out.mkdir();(out/'.retain').write_text('Exact tested refresh binaries, guards, source and negative diagnostics. Compiler objects/dependency files are temporary after verified inputs and commands retained.\n')
 names={f.relative_to(ROOT).as_posix()for d in('reconstruction','tests')for f in(ROOT/d).rglob('*')if f.is_file()and f.suffix in('.cpp','.hpp','.h','.S','.inc')};names|={'tools/test-palette-refresh.py','tools/original-manifest.sh'}
 with Reader(ROOT)as r:prospective={n:r.digest(n)for n in names}
 q=dict(schema=1,kind='world_palette_refresh_original_native_matrix',success=False,original_game_runs=0,original_oracle_runs=1,native_player_runs=0,commands=[],artifacts=[],compiler_sources={},sources={},executions={})
 def put(label,b):
  n=a.report.removesuffix('.json')+'-'+label;return dict(record=n,sha256=write_report(ROOT,n,b))
 def run(label,command):
  q['commands'].append(list(map(str,command)));p=subprocess.run(command,cwd=ROOT,capture_output=True,timeout=300,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1'))
  q['artifacts'].append(put(label+'.log',p.stdout+p.stderr));require(p.returncode==0,label+' failed');return p.stdout
 verified=False
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  require(hashlib.sha256((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Original build differs')
  for label,extra in [('original',['-m32','-O2','-DMNM_ORIGINAL_REFERENCE']),('native64',['-O2']),('sanitized',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   obj=out/(label+'.o');dep=out/(label+'.d');binary=out/label
   flags=['g++','-std=c++17','-mstackrealign','-Wall','-Wextra','-Werror','-Wno-unused-function','-fno-pie','-no-pie',*extra]
   run(label+'-compile',flags+['-MMD','-MF',str(dep),'-c',str(ROOT/'tests/palette-refresh-reference.cpp'),'-o',str(obj)])
   sources=set()
   for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
    f=Path(token).resolve()
    if f.is_relative_to(ROOT):sources.add(f.relative_to(ROOT).as_posix())
   if label=='original':sources|={'tests/actor-services-call.S','tests/screen-provider-call.S'}
   require(sources<=prospective.keys(),'Undeclared refresh compiler input');q['compiler_sources'][label]={n:prospective[n]for n in sorted(sources)};q['sources'].update(q['compiler_sources'][label])
   run(label+'-link',flags+[str(obj),*([str(ROOT/n)for n in('tests/actor-services-call.S','tests/screen-provider-call.S')]if label=='original'else[]),'-o',str(binary)])
   q['artifacts'].append(put(label+'-binary.bin',binary.read_bytes()));q['executions'][label]=json.loads(run(label+'-execution',[binary,*([ROOT/'original/Arcane_Nocd/Chaos.exe']if label=='original'else[])]))
  require(q['executions']['original']==q['executions']['native64']==q['executions']['sanitized'],'Refresh cross-configuration result differs');q['success']=True
 except Exception as e:q['error']=str(e)
 finally:
  if verified:
   try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);q['original_manifest_before_after']=True
   except Exception as e:q.update(success=False,original_error=str(e))
  q['sources'].update({n:prospective[n]for n in('tools/test-palette-refresh.py','tools/original-manifest.sh')})
  with Reader(ROOT)as r:q['sources_stable']=all(r.digest(n)==h for n,h in q['sources'].items())
  q['success']&=q['sources_stable'];q['source_inputs']={n:put('source-'+str(i)+'.bin',(ROOT/n).read_bytes())for i,n in enumerate(sorted(q['sources']))};q['scope']='Whole unchanged original WORD/DWORD in-place palette refresh functions with input coefficients, bothx87CW027f/037f, safe and refused shade counts, RGB555/565, normal and actual captured lighting domains; native64 and ASan/UBSan. No new original game or actual runtime input/output credit.'
  h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h,executions=q['executions'])))
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
