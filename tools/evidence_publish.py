"""Upload immutable release assets using SSH and a short-lived Actions branch.

No personal token is stored locally. The transport branch contains bounded asset
parts and an upload-only workflow; it deletes itself after a successful upload.
"""
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import time
import urllib.error
import urllib.request
import uuid

from coverage_evidence import CHUNK, digest_file, fetch_archive

WORKFLOW = '''name: Publish immutable evidence
on:
  push:
    branches: ['evidence-upload/**']
permissions:
  contents: write
concurrency:
  group: evidence-publication-${{ github.ref_name }}
  cancel-in-progress: false
jobs:
  upload:
    runs-on: ubuntu-latest
    timeout-minutes: 45
    steps:
      - uses: actions/checkout@v4
        with:
          persist-credentials: false
      - name: Verify and upload exact assets
        env:
          GH_TOKEN: ${{ github.token }}
          GH_REPO: ${{ github.repository }}
        run: |
          python3 - <<'PY'
          import hashlib,json,os,re,subprocess
          from pathlib import Path
          spec=json.loads(Path('.upload.json').read_text())
          assert re.fullmatch(r'evidence-[A-Za-z0-9_.-]+',spec['tag'])
          paths=[]
          for item in spec['assets']:
              assert re.fullmatch(r'[A-Za-z0-9_.-]+',item['name'])
              p=Path('assets')/item['name']
              assert p.stat().st_size==item['size']
              assert hashlib.file_digest(p.open('rb'),'sha256').hexdigest()==item['sha256']
              paths.append(str(p))
          repo=os.environ['GH_REPO']; tag=spec['tag']
          endpoint=f'repos/{repo}/releases/tags/{tag}'
          view=subprocess.run(['gh','api',endpoint],capture_output=True)
          if view.returncode:
              subprocess.run(['gh','release','create',tag,'--verify-tag','--latest=false',
                  '--title',tag,'--notes','Immutable evidence and storage provenance. Checksums are pinned in the source repository.'],check=True)
              existing={}
          else:
              existing={a['name']:a for a in json.loads(view.stdout)['assets']}
          upload=[]
          for item,path in zip(spec['assets'],paths):
              prior=existing.get(item['name'])
              if prior:
                  assert prior.get('digest')=='sha256:'+item['sha256'] and prior['size']==item['size'],'Refusing to overwrite release asset'
              else:upload.append(path)
          if upload:subprocess.run(['gh','release','upload',tag,*upload],check=True)
          uploaded=json.loads(subprocess.check_output(['gh','api',endpoint]))
          found={a['name']:a for a in uploaded['assets']}
          for item in spec['assets']:
              assert found[item['name']]['size']==item['size']
              assert found[item['name']].get('digest')=='sha256:'+item['sha256']
          branch=os.environ['GITHUB_REF_NAME']
          assert branch.startswith('evidence-upload/')
          ref=f'repos/{repo}/git/refs/heads/{branch}'
          current=json.loads(subprocess.check_output(['gh','api',ref]))['object']['sha']
          if current==os.environ['GITHUB_SHA']:
              subprocess.run(['gh','api','--method','DELETE',ref],check=True)
          PY
'''


def git(root, *args, data=None, env=None):
    return subprocess.check_output(['git', '-C', str(root), *args], input=data, env=env).strip().decode()


def transport(root, output, assets, tag, anchor=None):
    if not re.fullmatch(r'evidence-[A-Za-z0-9_.-]+', tag):
        raise ValueError('Invalid evidence release tag')
    root, output = Path(root).resolve(), Path(output).resolve()
    remote = git(root, 'remote', 'get-url', 'origin')
    if remote not in {'git@github.com:benhoff/magic_and_mayhem_mod.git',
                      'https://github.com/benhoff/magic_and_mayhem_mod.git'}:
        raise ValueError('Unexpected evidence publication repository')
    output.mkdir(parents=True, exist_ok=False)
    bare = output/'transport.git'
    subprocess.run(['git', 'init', '--bare', '-q', str(bare)], check=True)
    for setting in ('user.name', 'user.email'):
        git(bare, 'config', setting, git(root, 'config', '--get', setting))
    entries = []
    spec = {'schema': 1, 'tag': tag, 'assets': []}
    for path in assets:
        path = Path(path)
        name = path.name.lstrip('.')  # GitHub normalizes leading dots in asset names.
        if (not re.fullmatch('[A-Za-z0-9_.-]+', name) or path.stat().st_size > 64*CHUNK
                or any(item['name'] == name for item in spec['assets'])):
            raise ValueError('Unsafe or oversized transport asset')
        spec['assets'].append({'name': name, 'size': path.stat().st_size, 'sha256': digest_file(path)})
        entries.append(('assets/'+name, git(bare, 'hash-object', '-w', str(path))))
    for name, data in (('.upload.json', json.dumps(spec, sort_keys=True).encode()),
                       ('.github/workflows/evidence-upload.yml', WORKFLOW.encode())):
        entries.append((name, git(bare, 'hash-object', '-w', '--stdin', data=data)))
    env = dict(os.environ, GIT_INDEX_FILE=str(output/'transport.index'))
    git(bare, 'read-tree', '--empty', env=env)
    for name, oid in entries:
        git(bare, 'update-index', '--add', '--cacheinfo', '100644,'+oid+','+name, env=env)
    tree = git(bare, 'write-tree', env=env)
    # Give the upload commit a real parent. An orphan root push can fail to
    # schedule its newly introduced workflow; an ordinary tree change does not
    # depend on GitHub discovering workflows in a root-commit creation event.
    empty = git(bare, 'mktree', data=b'')
    seed = git(bare, 'commit-tree', empty, data=b'Initialize disposable evidence transport\n')
    commit = git(bare, 'commit-tree', tree, '-p', seed, data=b'Upload immutable evidence assets\n')
    branch = 'evidence-upload/'+uuid.uuid4().hex
    git(bare, 'update-ref', 'refs/heads/'+branch, commit)
    anchor = anchor or git(root, 'ls-remote', 'origin', 'refs/heads/master').split()[0]
    existing = git(root, 'ls-remote', 'origin', 'refs/tags/'+tag)
    if not existing:
        # A release tag is created against the existing default branch. The
        # history migration subsequently moves it to the corresponding new ID.
        subprocess.run(['git', '-C', str(root), 'push', '--force-with-lease=refs/tags/'+tag+':',
                        'origin', anchor+':refs/tags/'+tag], check=True)
    # A normal update after creation reliably schedules the upload even when
    # GitHub suppresses a new-branch event. Serialize its jobs and let only the
    # current tip delete the branch; both executions verify identical assets.
    heartbeat = git(bare, 'commit-tree', tree, '-p', commit, data=b'Run verified evidence publication\n')
    report = dict(spec, branch=branch, commit=heartbeat, upload_commit=commit,
                  anchor=anchor, status='prepared')
    (output/'receipt.json').write_text(json.dumps(report, indent=2)+'\n')
    subprocess.run(['git', '-C', str(bare), 'push', '--force-with-lease=refs/heads/'+branch+':', remote,
                    'refs/heads/'+branch+':refs/heads/'+branch], check=True)
    git(bare, 'update-ref', 'refs/heads/'+branch, heartbeat, commit)
    subprocess.run(['git', '-C', str(bare), 'push', '--force-with-lease=refs/heads/'+branch+':'+commit,
                    remote, heartbeat+':refs/heads/'+branch], check=True)
    report['status']='pushed'
    (output/'receipt.json').write_text(json.dumps(report, indent=2)+'\n')
    return report


def split_file(path, output, prefix, part_mib=48):
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    paths = []
    with Path(path).open('rb') as source:
        while True:
            data = source.read(part_mib*CHUNK)
            if not data:
                return paths
            part = output/(prefix+'.part'+str(len(paths)).zfill(4))
            with part.open('xb') as dest:
                dest.write(data)
            paths.append(part)


def await_publication(root, manifest, timeout=2700):
    """Verify actual downloaded bytes before a caller may pin the new archive."""
    deadline = time.monotonic()+timeout
    spec = manifest['archives'][0]
    while True:
        try:
            for part in spec['parts']:
                request = urllib.request.Request(part['url'], method='HEAD')
                with urllib.request.urlopen(request, timeout=30) as response:
                    if int(response.headers.get('Content-Length', -1)) != part['size']:
                        raise ValueError('Published asset size differs from its manifest')
            return fetch_archive(root, spec)
        except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError) as exc:
            if time.monotonic() >= deadline:
                raise ValueError('Release upload did not finish; preserve the transport receipt and inspect its Actions run') from exc
            print(json.dumps({'publication': 'waiting for verified release assets'}), flush=True)
            time.sleep(20)
