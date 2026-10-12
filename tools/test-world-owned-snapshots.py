#!/usr/bin/env python3
"""Source-bound V5 owned snapshot/return/queue controls; no live gameplay claim."""
import hashlib,json,os,struct,subprocess,tempfile,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parent=ROOT/'working/tests/world-owned-snapshots';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Owned snapshot evidence:',out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.world-owned-snapshot-delivery','--scenario','NR.world-owned-snapshot-delivery.fixture','--output',str(out/'claims.json')],cwd=ROOT,check=True)
 d=json.loads((out/'claims.json').read_text());r=dict(success=False,scope=__doc__,sources=d['sources'],claims=d['claims']);env=dict(os.environ,TMPDIR=str(ROOT/'working/coverage-hook-tmp'),WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all');prefix=out/'wineprefix';prefix.mkdir(mode=0o700)
 try:
  with (out/'original-before.log').open('x') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
  capture=ROOT/'working/tests/native-campaign/run-u4m1wj53/resident-source.bin'
  with capture.open('rb') as incoming:
   magic=incoming.read(8);assert magic in (b'MNMPROF1',b'MNMPROF2');h=incoming.read(20 if magic==b'MNMPROF2' else 16);g,s,q=struct.unpack('<QII',h[:16]);assert g==1 and 64<=s<=16777216 and 64<=q<=197184
   if magic==b'MNMPROF2':
    n=struct.unpack('<I',h[16:])[0];assert n<=s+q+65536;packed=incoming.read(n);assert struct.unpack('>I',packed[:4])[0]==s+q;data=zlib.decompress(packed[4:])
   else:data=incoming.read(s+q)
   assert len(data)==s+q
  (out/'source.bin').write_bytes(data[:s]);(out/'request.bin').write_bytes(data[s:]);r['fixture_input']=dict(capture=str(capture),source_bytes=s,request_bytes=q,source_sha256=sha(out/'source.bin'),request_sha256=sha(out/'request.bin'),scope='First source-owned record from historical captured gameplay prefix; authored original peer controls only.')
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
 except (OSError,ValueError,AssertionError,IndexError,subprocess.SubprocessError) as e:r['error']=str(e)
 finally:
  try:
   stopped=subprocess.run(['wineserver','-k'],env=env,timeout=10);assert stopped.returncode in (0,1);subprocess.run(['wineserver','-w'],env=env,timeout=10,check=True);r['private_wine_stopped']=True
   with (out/'original-after.log').open('x') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
   r['original_manifest_verified_after']=True
  except (OSError,AssertionError,subprocess.SubprocessError) as e:r.update(success=False,cleanup_error=str(e))
  r['sources_unchanged']=all(sha(ROOT/p)==h for p,h in d['sources'].items())
  if not r['sources_unchanged']:r.update(success=False,error='Fixture sources changed during execution')
  (out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 print('Report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
