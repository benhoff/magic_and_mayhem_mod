"""Write only diagnostic captures in tmpfs, then copy exact closed bytes back."""
import hashlib,json,os,shutil,stat,subprocess
from pathlib import Path
from rendering_pilot import require

BASE=Path('/dev/shm/mnm-broader-player-20261110/captures')
PIN=b'Temporary diagnostic capture copy. Preserve until exact acquisition artifacts have two verified durable copies and a real restore; then this owned RAM copy may be retired.\n'

def digest(path):
    with path.open('rb')as stream:return hashlib.file_digest(stream,'sha256').hexdigest()

class CaptureProcess:
    def __init__(self,process,storage):self.process,self.storage=process,storage
    def __getattr__(self,name):return getattr(self.process,name)
    def poll(self):
        result=self.process.poll()
        if result is not None:self.storage.copy_closed()
        return result
    def wait(self,*a,**k):
        result=self.process.wait(*a,**k);self.storage.copy_closed();return result
    def communicate(self,*a,**k):
        result=self.process.communicate(*a,**k);self.storage.copy_closed();return result
    def __enter__(self):return self
    def __exit__(self,*a):
        result=self.process.__exit__(*a);self.storage.copy_closed();return result

class CaptureStorage:
    def __init__(self,root,report,enabled=False,*,base=BASE):
        self.root=Path(root).resolve();self.report=report;self.enabled=enabled;self.base=Path(base);self.entries=[]
    def prepare(self,environment):
        if not self.enabled or not environment.get('MNM_ACCEPTANCE_CAPTURE_DIRECTORY'):return False
        logical=Path(environment['MNM_ACCEPTANCE_CAPTURE_DIRECTORY'])
        require(logical.is_absolute()and logical.is_relative_to(self.root/'working')and logical.name=='capture',
                'Capture storage must be the existing owned logical capture directory')
        require(not logical.exists()and not logical.is_symlink()and logical.parent.is_dir(),
                'Preserve existing logical capture output')
        require(not any(p.is_symlink()for p in [logical.parent,*logical.parent.parents]if p==self.root or self.root in p.parents),
                'Capture output may not traverse aliases')
        require(subprocess.check_output(['findmnt','-T',str(self.base.parent),'-n','-o','FSTYPE'],text=True).strip()=='tmpfs',
                'Diagnostic RAM capture requires tmpfs')
        require(shutil.disk_usage(self.base.parent).free>=8*1024**3,'Insufficient RAM capture headroom')
        require(not any(p.is_symlink()for p in [self.base,*self.base.parents]),'Capture RAM base may not traverse aliases')
        self.base.mkdir(mode=0o700,parents=True,exist_ok=True)
        physical=self.base/hashlib.sha256(self.report.encode()).hexdigest()[:24]
        require(not physical.exists()and not physical.is_symlink(),'Preserve existing physical capture')
        physical.mkdir(mode=0o700);(physical/'.retain').write_bytes(PIN);logical.symlink_to(physical,target_is_directory=True)
        self.entries.append(dict(logical=str(logical),physical=str(physical),copied=False,files=[]))
        return True
    def wrap(self,process):return CaptureProcess(process,self)if self.entries else process
    def copy_closed(self):
        for entry in self.entries:
            if entry['copied']:continue
            logical,physical=Path(entry['logical']),Path(entry['physical'])
            require(logical.is_symlink()and logical.readlink()==physical and physical.is_dir(),
                    'Capture storage identity changed')
            from evidence_workspace import idle
            idle(physical)
            members=sorted(physical.iterdir());require(members and all(p.is_file()and not p.is_symlink()and p.stat().st_nlink==1 for p in members),
                    'Capture copy requires ordinary closed files only')
            total=sum(p.stat().st_size for p in members)
            require(shutil.disk_usage(logical.parent).free>=100*1024**3+total+2*1024**2,
                    'Capture copy would cross the cold-drive100GiB reserve')
            temporary=logical.with_name('capture.from-ram-'+physical.name);alias=logical.with_name('capture.ram-alias')
            require(not temporary.exists()and not alias.exists()and not alias.is_symlink(),'Preserve earlier capture copy')
            temporary.mkdir(mode=0o700)
            for source in members:
                target=temporary/source.name;before=source.stat();sha=digest(source)
                shutil.copy2(source,target)
                require(digest(target)==sha and tuple(getattr(source.stat(),n)for n in('st_dev','st_ino','st_size','st_mtime_ns','st_ctime_ns'))==tuple(getattr(before,n)for n in('st_dev','st_ino','st_size','st_mtime_ns','st_ctime_ns')),'Capture copy bytes or identity changed')
                entry['files'].append(dict(name=source.name,sha256=sha,bytes=before.st_size))
            require({p.name for p in physical.iterdir()}=={p.name for p in members},'Capture changed during closed copy')
            logical.rename(alias)
            try:temporary.rename(logical)
            except BaseException:alias.rename(logical);raise
            entry['copied']=True;entry['held_alias']=str(alias)
    def receipt(self):
        require(all(e['copied']for e in self.entries),'Capture process did not close and copy all diagnostic bytes')
        return dict(enabled=self.enabled,policy='owned-tmpfs-diagnostic-capture-exact-copy-v1',entries=self.entries,
            native_code_environment_capture_path_assertions_and_deadlines_changed=False,
            captured_bytes_preserved=True,physical_RAM_copies_removed=False,
            explanation='Only the backing filesystem used while Qt writes diagnostic captures changes. The environment retains its original logical capture pathname. After the player exits, every closed file is copied and hash-checked into that exact logical directory before the unchanged collector imports it. RAM copies remain held until separately verified durable handoff. This is not a proved root cause or renderer repair.')
