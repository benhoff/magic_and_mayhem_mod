"""Clone a stopped, admitted DirectPlay prefix into a new test-owned location."""
import hashlib
import json
from pathlib import Path
import subprocess
from directplay_dx6_launch import prefix_identity,check_prefix
ROOT=Path(__file__).resolve().parents[1]


def sha(path):
    with path.open('rb') as file:return hashlib.file_digest(file,'sha256').hexdigest()


def clone(template,target,environment):
    template=template.resolve();target=Path(target)
    if not target.resolve().is_relative_to((ROOT/'working').resolve()) or target.exists() or target.is_symlink():
        raise ValueError('Prefix clone requires a fresh test-owned working target')
    if not template.is_dir() or 'original' in template.parts:
        raise ValueError('Prefix template must be a disposable owned Wine prefix')
    marker=template/'.mnm-directplay-dx6.json'
    if marker.is_symlink() or not marker.is_file() or json.loads(marker.read_text())!=prefix_identity(template):
        raise ValueError('Prefix template lacks exact DirectPlay ownership; use an admitted native-session prefix')
    check_prefix(template)
    before={name:sha(template/name) for name in ('.mnm-directplay-dx6.json','system.reg','user.reg')}
    # This wait never kills a template user's server. Active prefixes refuse.
    subprocess.run(['wineserver','-w'],env=dict(environment,WINEPREFIX=str(template)),check=True,timeout=5)
    subprocess.run(['cp','-a','--reflink=auto',str(template),str(target)],check=True,timeout=120)
    check_prefix(target)
    copied=target/'.mnm-directplay-dx6.json'
    if copied.is_symlink() or sha(copied)!=before['.mnm-directplay-dx6.json']:
        raise ValueError('Cloned DirectPlay ownership differs from the validated source')
    if any(sha(template/name)!=digest for name,digest in before.items()):
        raise ValueError('Prefix template changed during cloning')
    # Only the new private owner path changes. Native DLL bytes stay hash-checked.
    copied.write_text(json.dumps(prefix_identity(target.resolve()),indent=2)+'\n')
    check_prefix(template);check_prefix(target)
    return dict(path=str(template),target=str(target.resolve()),stopped_before_copy=True,
                source_fingerprints=before,target_owner_sha256=sha(copied),
                original_template_modified=False)
