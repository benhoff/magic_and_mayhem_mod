#!/usr/bin/env python3
"""Source-bound V5 owned snapshot/return/queue controls; no live gameplay claim."""
import argparse,hashlib,json,os,struct,subprocess,tempfile,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):
 with p.open('rb') as source:return hashlib.file_digest(source,'sha256').hexdigest()
def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--capture',type=Path,help='Owned captured prefix; defaults to the historical fixture when no direct inputs are supplied')
 parser.add_argument('--source',type=Path,help='Owned source bytes; requires --request')
 parser.add_argument('--request',type=Path,help='Owned request bytes; requires --source')
 parser.add_argument('--prefix-template',type=Path,help='Clone an admitted stopped private prefix')
 a=parser.parse_args()
 if bool(a.source)!=bool(a.request):parser.error('Supply both --source and --request')
 if a.source and a.capture:parser.error('Choose direct inputs or a captured prefix')
 parent=ROOT/'working/tests/world-owned-snapshots';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Owned snapshot evidence:',out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.world-owned-snapshot-delivery','--scenario','NR.world-owned-snapshot-delivery.fixture','--output',str(out/'claims.json')],cwd=ROOT,check=True)
 d=json.loads((out/'claims.json').read_text());r=dict(success=False,scope=__doc__,sources=d['sources'],claims=d['claims']);env=dict(os.environ,TMPDIR=str(ROOT/'working/coverage-hook-tmp'),WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all');prefix=out/'wineprefix';(out/'tmp').mkdir();env['TMPDIR']=str(out/'tmp')
 try:
  with (out/'original-before.log').open('x') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
  inputs={}
  if a.source:
   supplied=[a.source.resolve(),a.request.resolve()]
   for path,limit in zip(supplied,(16777216,197184)):
    if not path.is_relative_to(ROOT/'working') or not 64<=path.stat().st_size<=limit:raise ValueError('Invalid bounded owned fixture input')
    inputs[str(path)]=sha(path)
   data=supplied[0].read_bytes()+supplied[1].read_bytes();s=supplied[0].stat().st_size;q=supplied[1].stat().st_size
   origin=dict(source=str(supplied[0]),request=str(supplied[1]))
  else:
   capture=(a.capture or ROOT/'working/tests/native-campaign/run-u4m1wj53/resident-source.bin').resolve()
   if not capture.is_relative_to(ROOT/'working'):raise ValueError('Captured input must be owned working data')
   inputs[str(capture)]=sha(capture)
   with capture.open('rb') as incoming:
    magic=incoming.read(8)
    if magic not in (b'MNMPROF1',b'MNMPROF2'):raise ValueError('Unsupported captured input')
    h=incoming.read(20 if magic==b'MNMPROF2' else 16);g,s,q=struct.unpack('<QII',h[:16])
    if g!=1 or not 64<=s<=16777216 or not 64<=q<=197184:raise ValueError('Captured input bounds')
    if magic==b'MNMPROF2':
     n=struct.unpack('<I',h[16:])[0]
     if not 4<=n<=s+q+65536:raise ValueError('Compressed input bounds')
     packed=incoming.read(n)
     if len(packed)!=n or struct.unpack('>I',packed[:4])[0]!=s+q:raise ValueError('Compressed input extent')
     decoder=zlib.decompressobj();data=decoder.decompress(packed[4:],s+q+1)
     if not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:raise ValueError('Compressed input overflow or trailing bytes')
    else:data=incoming.read(s+q)
   if len(data)!=s+q:raise ValueError('Captured input extent')
   origin=dict(capture=str(capture))
  (out/'source.bin').write_bytes(data[:s]);(out/'request.bin').write_bytes(data[s:]);r['fixture_input']=dict(**origin,source_bytes=s,request_bytes=q,source_sha256=sha(out/'source.bin'),request_sha256=sha(out/'request.bin'),scope='Explicit owned fixture inputs, not new original engine comparison or live replacement.')
  r['inputs']=inputs
  if a.prefix_template:
   from native_prefix_template import clone
   r['prefix_template']=clone(a.prefix_template,prefix,env)
  else:prefix.mkdir(mode=0o700)
  build=ROOT/'working/build/qt-shell'
  with (out/'build.log').open('x') as log:
   subprocess.run(['cmake','-S',str(ROOT/'apps/qt-shell'),'-B',str(build),'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
   subprocess.run(['cmake','--build',str(build),'--target','world-snapshot-queue-test','world-gpu-ack-test','--parallel','4'],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
  with (out/'queue.log').open('x') as log:subprocess.run([str(build/'world-rolling/world-snapshot-queue-test'),str(out/'source.bin'),str(out/'request.bin')],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
  r['queue_controls']=json.loads((out/'queue.log').read_text().splitlines()[-1])
  with (out/'v4.log').open('x') as log:subprocess.run([str(build/'world-rolling/world-gpu-ack-test')],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
  (out/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\nExitProcess@4\n');subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(out/'kernel32.def'),'-l',str(out/'kernel32.lib'),'--kill-at'],env=env,check=True)
  subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/world-snapshot-ack-reference.c'),'-o',str(out/'ack.obj')],env=env,check=True)
  exe=out/'ack.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(exe),str(out/'ack.obj'),str(out/'kernel32.lib')],env=env,check=True)
  with (out/'pe.log').open('x') as log:subprocess.run(['xvfb-run','-a','-e',str(out/'xvfb.log'),'wine',str(exe)],env=env,cwd=out,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
  r['pe_controls']=dict(exit_code=0,exact_ack_bytes=80,byte_mutations=80,legacy_mutual_refusal=True,binary_sha256=sha(exe));r['legacy_v4_controls']=512;r['success']=True
 except (OSError,ValueError,AssertionError,IndexError,struct.error,zlib.error,subprocess.SubprocessError) as e:r['error']=str(e)
 finally:
  try:
   stopped=subprocess.run(['wineserver','-k'],env=env,timeout=10);assert stopped.returncode in (0,1);subprocess.run(['wineserver','-w'],env=env,timeout=10,check=True);r['private_wine_stopped']=True
   with (out/'original-after.log').open('x') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
   r['original_manifest_verified_after']=True
  except (OSError,AssertionError,subprocess.SubprocessError) as e:r.update(success=False,cleanup_error=str(e))
  r['inputs_unchanged']=all(sha(Path(p))==h for p,h in r.get('inputs',{}).items())
  if not r['inputs_unchanged']:r.update(success=False,error='Fixture inputs changed during execution')
  r['sources_unchanged']=all(sha(ROOT/p)==h for p,h in d['sources'].items())
  if not r['sources_unchanged']:r.update(success=False,error='Fixture sources changed during execution')
  (out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print('Report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
