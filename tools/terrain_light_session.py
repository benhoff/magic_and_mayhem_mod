"""Opt-in normal-session lighting: fixed shared channel and owned native worker.

Graphics ownership and the original world scheduler are independent. Selecting
this backend never asserts equivalence for an unobserved map or a whole session.
"""
import contextlib,hashlib,importlib.util,json,mmap,os,secrets,shutil,signal,sqlite3,struct,subprocess,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
CAPACITY=8*1024*1024
SIZE=128+3*CAPACITY
RAW=('MNM_LIGHT_DIR','MNM_LIGHT_MODE','MNM_LIGHT_LIMIT','MNM_LIGHT_ARM_RELOAD','MNM_LIGHT_SESSION_FILE')
STATS=['calls','applied','fallback','refused','disabled','processed','rejected','compared','mismatches','max_wait_ms','max_compute_us','last_caller','last_clock','last_changed','error','records','trace_active']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def selection(environment):
 value=environment.get('MNM_TERRAIN_LIGHTING','original')
 if value not in ('original','observe','native'):raise ValueError('MNM_TERRAIN_LIGHTING must be original, observe or native')
 capture=environment.get('MNM_TERRAIN_LIGHT_CAPTURE','none')
 if capture not in ('none','first32','reload32'):raise ValueError('MNM_TERRAIN_LIGHT_CAPTURE must be none, first32 or reload32')
 if value=='original' and capture!='none':raise ValueError('Lighting capture requires an admitted lighting session')
 return value,capture

def stage(root,original,patched,environment):
 mode,capture=selection(environment)
 if mode=='original':return patched,{}
 if hashlib.sha256(original).hexdigest()!=HASH:raise ValueError('Unsupported lighting executable')
 if environment.get('MNM_MENU_DIRECTPLAY_DX6')!='1':raise ValueError('Lighting sessions require the admitted game-process wait boundary (MNM_MENU_DIRECTPLAY_DX6=1)')
 if any(k in environment for k in RAW):raise ValueError('Session lighting excludes legacy diagnostic channel overrides')
 game=root/'game';target=game/'MnmLight.dll'
 if game.is_symlink() or target.is_symlink() or not target.resolve().is_relative_to(game.resolve()):raise ValueError('Lighting DLL escapes disposable game')
 exporter=load('lighting_session_offsets','tools/export-menu-support.py');at=exporter.image_offset(original,0x4f27a0,8)
 if original[at:at+8]!=bytes.fromhex('568bf1e828000000'):raise ValueError('Lighting updater entry differs')
 dll,worker,build=load('lighting_session_build','tools/build-terrain-light-session.py').build(root/'lighting-build');shutil.copy2(dll,target)
 patched=load('lighting_session_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmLight.dll','LightAnchor',b'.mnlight')
 directory=root/'terrain-light-session';directory.mkdir(exist_ok=False);(directory/'.retain').write_text('Session lighting channel, bounded diagnostic corpus, native worker logs and SQLite outcome. Preserve.\n')
 channel=directory/'channel.bin';nonce=secrets.token_bytes(8)
 if not any(nonce):nonce=b'\1'+nonce[1:]
 header=bytearray(128);header[:8]=b'MNMLS001';struct.pack_into('<3I',header,8,1,CAPACITY,7);header[36:44]=nonce;struct.pack_into('<2I',header,44,int(mode=='native'),{'none':0,'first32':1,'reload32':2}[capture])
 with channel.open('xb') as f:f.write(header);f.truncate(SIZE)
 channel.chmod(0o600)
 return patched,dict(terrain_light_session=dict(schema=1,mode=mode,capture=capture,channel=str(channel),nonce=nonce.hex(),dll_sha256=build['dll_sha256'],worker_sha256=build['worker_sha256'],sources=build['sources'],scope='Opt-in persistent native lighting with source-bound requests, original fallback latched after refusal/timeout, fixed storage and session-owned worker lifetime; no default rollout or whole-world scheduling claim.'))

def validate(root,metadata,environment,phase):
 root=root.resolve()
 if not root.is_relative_to(ROOT/'working'):raise ValueError('Lighting session must be owned below working/')
 policy=metadata.get('terrain_light_session')
 mode,capture=selection(environment)
 if policy is None:
  if mode!='original':raise ValueError('Lighting session was not staged')
  return None
 if policy.get('schema')!=1 or policy.get('mode')!=mode or policy.get('capture')!=capture or metadata.get('directplay_dx6') is None:raise ValueError('Lighting selection differs from staged policy')
 path=root/'terrain-light-session/channel.bin';worker=root/'lighting-build/mnm-terrain-light-session';dll=root/'game/MnmLight.dll'
 for p in (path,worker,dll):
  if p.is_symlink() or not p.resolve().is_relative_to(root) or not p.is_file():raise ValueError('Lighting resource path changed')
 if policy['channel']!=str(path) or path.stat().st_size!=SIZE or sha(worker)!=policy['worker_sha256'] or sha(dll)!=policy['dll_sha256']:raise ValueError('Lighting resource identity changed')
 for p,h in policy['sources'].items():
  source=(ROOT/p).resolve()
  if Path(p).is_absolute() or not source.is_relative_to(ROOT) or source.parts[len(ROOT.parts)] in ('working','original','.git') or sha(source)!=h:raise ValueError('Lighting build source changed: '+p)
 with path.open('rb') as f:header=f.read(128)
 words=struct.unpack('<32I',header)
 if header[:8]!=b'MNMLS001' or words[2:5]!=(1,CAPACITY,phase) or header[36:44].hex()!=policy['nonce'] or words[11]!=int(mode=='native') or words[12]!={'none':0,'first32':1,'reload32':2}[capture] or words[13] or any(words[14:]):raise ValueError('Lighting channel is stale or incompatible')
 return policy

def authorize(root,metadata,environment):
 # The child receives only the source-checked staged channel, including after
 # the native-output bwrap boundary. Arbitrary inherited diagnostic paths vanish.
 for name in RAW:environment.pop(name,None)
 if validate(root,metadata,environment,0) is not None:environment['MNM_LIGHT_SESSION_FILE']='Z:'+str(root.resolve()/'terrain-light-session/channel.bin').replace('/','\\')

class Session:
 def __init__(self,root,metadata,environment):
  self.root=Path(root).resolve();self.metadata=metadata;self.environment=environment;self.child=None;self.mapping=None;self.log=None;self.failed=None
 def __enter__(self):
  self.policy=validate(self.root,self.metadata,self.environment,7)
  if self.policy is None:return self
  directory=self.root/'terrain-light-session';self.directory=directory
  try:
   with (directory/'channel.bin').open('r+b') as f:self.mapping=mmap.mmap(f.fileno(),SIZE)
   self.log=(directory/'worker.log').open('xb')
   self.child=subprocess.Popen([str(self.root/'lighting-build/mnm-terrain-light-session'),'--session',str(directory/'channel.bin'),str(directory/'corpus.bin')],stdin=subprocess.DEVNULL,stdout=self.log,stderr=subprocess.STDOUT,close_fds=True,start_new_session=True)
   identity=Path('/proc',str(self.child.pid),'stat').read_text().rsplit(') ',1)[1].split()
   db=sqlite3.connect(directory/'outcome.sqlite3')
   with db:db.execute('CREATE TABLE IF NOT EXISTS lifecycle(pid INTEGER,start_ticks INTEGER,phase TEXT)');db.execute('INSERT INTO lifecycle VALUES (?,?,?)',(self.child.pid,int(identity[19]),'started'))
   db.close()
   deadline=time.monotonic()+3
   while time.monotonic()<deadline:
    if self.child.poll() is not None:raise RuntimeError('Lighting worker exited during startup')
    if struct.unpack_from('<I',self.mapping,16)[0]==0:return self
    time.sleep(.005)
   raise RuntimeError('Lighting worker startup timed out')
  except BaseException as e:self.failed=str(e);self.close();raise
 def close(self):
  if self.child is None:
   if self.mapping is not None:self.mapping.close();self.mapping=None
   if self.log is not None:self.log.close();self.log=None
   return
  early=self.child.poll() is not None
  if not early:
   os.killpg(self.child.pid,signal.SIGTERM)
   try:self.child.wait(timeout=3)
   except subprocess.TimeoutExpired:os.killpg(self.child.pid,signal.SIGKILL);self.child.wait(timeout=3)
  words=struct.unpack_from('<32I',self.mapping);self.mapping.close();self.mapping=None;self.log.close()
  result=dict(schema=1,success=self.failed is None and not early and self.child.returncode==0 and not words[18] and not words[22] and not words[28],mode=self.policy['mode'],worker_exit=self.child.returncode,worker_exited_early=early,error=self.failed,statistics=dict(zip(STATS,words[14:31])),original_pixel_equivalence=False,complete_replacement=False,source_bound=True,scope=self.policy['scope'])
  db=sqlite3.connect(self.directory/'outcome.sqlite3')
  with db:db.execute('CREATE TABLE IF NOT EXISTS results(payload TEXT NOT NULL)');db.execute('INSERT INTO results VALUES (?)',(json.dumps(result,sort_keys=True),))
  db.close();self.result=result;self.child=None
 def __exit__(self,kind,error,tb):
  if error:self.failed=str(error)
  self.close()
  if kind is None and self.policy is not None and not self.result['success']:raise RuntimeError('Native lighting session ended with a refused/stopped worker; original lighting fallback was retained. See session SQLite outcome.')

@contextlib.contextmanager
def supervise(root,metadata,environment):
 with Session(root,metadata,environment) as session:yield session
