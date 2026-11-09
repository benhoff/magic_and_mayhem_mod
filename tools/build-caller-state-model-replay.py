#!/usr/bin/env python3
"""Build and retain the private publication-state replay service; no execution."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import tempfile
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/caller-state-results-20261108/'


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--name',required=True);a=p.parse_args()
 if not a.name or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-'for c in a.name):p.error('Unique lowercase name')
 target=PREFIX+a.name+'.json'
 if (ROOT/target).exists():p.error('Preserve immutable build results')
 out=Path(tempfile.mkdtemp(prefix='mnm-caller-model-',dir='/tmp'));sha=lambda b:hashlib.sha256(b).hexdigest()
 frozen={}
 for directory in ('runtime','reconstruction','protocols','tests','tools'):
  for path in (ROOT/directory).rglob('*'):
   if path.is_file()and path.suffix in('.c','.h','.cpp','.hpp','.S','.py'):frozen[path.relative_to(ROOT).as_posix()]=path.read_bytes()
 report=dict(schema=1,kind='caller_state_publication_model_build',success=False,original_game_runs=0,
  scope='Compile and retain a private i386 publication-state replay service. Recovered models require their separate current original comparisons; the build supplies no equivalence or installed acceptance.',
  temporary_directory=str(out),temporary_classification='RAM compiler objects/depfiles are temporary. Exact tested candidate, compiler logs, source preimages and build closure are durable below; no automatic scratch cleanup.',commands=[],source_preimages={})
 def retain(label,data):
  path=PREFIX+a.name+'-'+label;write_report(ROOT,path,data);return dict(path=path,sha256=sha(data),bytes=len(data))
 def run(label,argv):
  r=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,timeout=180)
  report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=r.returncode,log=retain(label+'.log.bin',r.stdout+r.stderr)))
  if r.returncode:raise RuntimeError(label+': exit '+str(r.returncode))
 used={'tools/build-caller-state-model-replay.py','tests/caller-state-model-replay.cpp'}
 try:
  # Freeze changed inputs before the compiler, including failed candidates.
  changed=subprocess.check_output(['git','ls-files','--modified','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
  for name in changed:
   if name in frozen:report['source_preimages'][name]=retain('source-'+sha(name.encode())[:16]+'.bin',frozen[name])
  run('clang-version',['clang','--version']);run('cxx-version',['clang++','--version'])
  objects=[]
  for i,name in enumerate(('reconstruction/rendering/word_backend_state.c','reconstruction/rendering/glyph_backend_state.c',
        'reconstruction/rendering/glyph_backend.c','reconstruction/rendering/glyph_backend_x87.S')):
   used.add(name);obj=out/(str(i)+'.o');objects.append(obj)
   run('compile-'+str(i),['clang','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
       '-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj])
  binary=out/'caller-state-model-replay'
  run('link',['clang++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
      '-MMD','-MF',out/'model.d',ROOT/'tests/caller-state-model-replay.cpp',*objects,'-o',binary])
  for path in out.glob('*.d'):
   for token in shlex.split(path.read_text().replace('\\\n',' ').split(':',1)[1]):
    source=Path(token).resolve()
    if source.is_relative_to(ROOT)and not source.is_relative_to(ROOT/'working'):used.add(source.relative_to(ROOT).as_posix())
  if used-frozen.keys():raise ValueError('Unfrozen compiler source closure')
  report['binary']=retain('replay.bin',binary.read_bytes());report['physical_binary']=str(binary);report['success']=True
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError)as error:report['error']=str(error)
 finally:
  report['sources']={n:sha(frozen[n])for n in sorted(used&frozen.keys())}
  report['sources_stable']=not(used-frozen.keys())and all((ROOT/n).read_bytes()==frozen[n]for n in used&frozen.keys())
  report['success'] &= report['sources_stable'];write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps({k:report[k]for k in('success','error','physical_binary')if k in report}),flush=True)
 return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
