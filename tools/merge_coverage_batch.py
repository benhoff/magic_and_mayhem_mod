"""Git octopus strategy with conservative central metadata union and no commits.

Git owns the merge lifecycle/parent list. This strategy writes only candidate
trees and index entries, following the ordinary octopus read-tree/merge-index
sequence. The final combined commit still requires normal finish and hooks.
"""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
from coverage_bookkeeping import encoded
from integration_review import ALLOWED, REGISTER, JOURNAL, LEDGER, merge_json


def git(*args):
    return subprocess.check_output(['git', *args])


def resolve_metadata():
    conflicts = [p.decode() for p in git('diff', '--name-only', '--diff-filter=U', '-z').split(b'\0') if p]
    for name in conflicts:
        if name not in ALLOWED:
            continue
        values = [git('show', ':'+str(stage)+':'+name) for stage in (1, 2, 3)]
        if name == LEDGER:
            base, ours, theirs = values
            if not ours.startswith(base) or not theirs.startswith(base):
                raise ValueError('Batch ledger changes must be append-only')
            result = ours if ours == theirs else ours+b'\n\n'+theirs[len(base):]
        else:
            value = merge_json(*(json.loads(v) for v in values), census=name == REGISTER)
            result = encoded(value) if name == JOURNAL else (json.dumps(value, indent=2)+'\n').encode()
        # These are authored central metadata, rather than generated reports.
        Path(name).write_bytes(result)
        subprocess.run(['git', 'add', '--', name], check=True)
        print(json.dumps(dict(metadata_union=name, parent_sha256=[hashlib.sha256(v).hexdigest() for v in values],
                              result_sha256=hashlib.sha256(result).hexdigest())), flush=True)


def main():
    arguments = sys.argv[1:]
    separator = arguments.index('--')
    head, *tips = arguments[separator+1:]
    merged = [head]
    tree = git('write-tree').decode().strip()
    for tip in tips:
        if any(subprocess.run(['git', 'merge-base', '--is-ancestor', tip, parent],
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0
               for parent in merged):
            continue
        common = git('merge-base', '--all', tip, *merged).decode().splitlines()
        if len(common) != 1:
            raise ValueError('Multiple batch merge bases require manual review')
        subprocess.run(['git', 'read-tree', '-u', '-m', common[0], tree, tip], check=True)
        resolve_metadata()
        # Standard Git text merging handles remaining code/doc paths. Failed
        # merges preserve their diagnostics/index for explicit review.
        if git('diff', '--name-only', '--diff-filter=U'):
            subprocess.run(['git', 'merge-index', '-o', 'git-merge-one-file', '-a'], check=True)
        tree = git('write-tree').decode().strip()
        merged.append(tip)
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print('Batch merge requires review: '+str(error), file=sys.stderr)
        raise SystemExit(2)
