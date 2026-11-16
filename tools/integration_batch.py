"""Prepare and validate exact candidates before short, checked master promotion."""
import json
import os
from pathlib import Path
import subprocess
import sys
import secrets
import re

from coverage_evidence import write_report
from integration_queue import integration_lock, status, FD_VARIABLE, _current
from integration_review import managed_report, reconcile, review_history


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], text=True).strip()


def preview(root, tips):
    root = Path(root).resolve()
    return dict(status(root), tips=[dict(name=tip, commit=git(root, 'rev-parse', '--verify',
                '--end-of-options', tip+'^{commit}')) for tip in tips])


def master_checkout(root):
    for record in git(root, 'worktree', 'list', '--porcelain').split('\n\n'):
        lines = record.splitlines()
        if 'branch refs/heads/master' in lines:
            return Path(lines[0].removeprefix('worktree '))
    return None


def execute(root, tips, *, branch, report, reason, validation_reason, index_reason,
            message, database, history_reason='', behaviors=(), expected_master=None):
    root = Path(root).resolve()
    if os.environ.get(FD_VARIABLE)or _current.get()is not None:
        raise ValueError('Candidate preparation must run outside promotion admission')
    if not tips or not branch.startswith('integrate/'):
        raise ValueError('Provide ready tips and a new integrate/ branch name')
    if not all(value.strip() for value in (reason, validation_reason, index_reason, message)):
        raise ValueError('Concrete review, validation, census reasons and message are required')
    report = managed_report(root, report)
    pins = list(dict.fromkeys(item['commit'] for item in preview(root, tips)['tips']))
    details = dict(schema=1, kind='candidate_integration_preparation', tips=pins, branch=branch,
                   scope='Preparation and custody only. Normal finish, backups and staged hook still required.')
    base = git(root, 'rev-parse', 'master')
    if expected_master is not None and git(root, 'rev-parse', '--verify', '--end-of-options', expected_master+'^{commit}') != base:
        raise ValueError('Master changed before candidate preparation')
    if git(root, 'symbolic-ref', '--short', 'HEAD') == 'master':
        raise ValueError('Run from your clean owned task worktree, preserving the master checkout')
    if git(root, 'status', '--porcelain') or subprocess.run(
            ['git', '-C', str(root), 'rev-parse', '--verify', 'MERGE_HEAD'],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0:
        raise ValueError('Candidate worktree must be clean with no pending merge')
    checkout = master_checkout(root)
    if checkout and git(checkout, 'status', '--porcelain'):
        raise ValueError('Master checkout has other work; preserve it and retry later')
    active = [tip for tip in pins if subprocess.run(
        ['git', '-C', str(root), 'merge-base', '--is-ancestor', tip, base]).returncode != 0]
    if not active:
        raise ValueError('All requested tips are already included in master')
    for tip in active:
        if git(root, 'diff', '--name-only', base, tip, '--', 'original/'):
            raise ValueError('Ready tips must preserve immutable original inputs')
    details.update(master=base, active_tips=active, validation_outside_lock=True)
    try:
        subprocess.run(['git', '-C', str(root), 'switch', '-c', branch, base], check=True)
        strategy = ['-s', 'mm-coverage.sh'] if len(active) > 1 else []
        environment = dict(os.environ, PATH=str(root/'tools')+os.pathsep+os.environ.get('PATH',''))
        result = subprocess.run(['git', '-C', str(root), 'merge', '--no-ff', '--no-commit', *strategy, *active],
                           capture_output=True, text=True, env=environment)
        details.update(merge_returncode=result.returncode, stdout=result.stdout, stderr=result.stderr)
        if result.returncode:
            if len(active) != 1:
                raise ValueError('Batch merge needs conflict review. Prepared work is preserved; '
                                 'integrate conflicting tips separately.')
            metadata = str(Path(report).with_name(Path(report).stem+'-metadata.json'))
            details['metadata'] = reconcile(root, execute=True, reason=reason, report=metadata)
        if subprocess.run(['git', '-C', str(root), 'rev-parse', '--verify', 'MERGE_HEAD'],
                          stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode:
            raise ValueError('Git did not leave a pending combined merge')
        common = git(root, 'merge-base', '--octopus', base, *active)
        history = review_history(root, common, [base, *active])
        if history['missing_receipt_count']:
            if not history_reason.strip() or not behaviors:
                raise ValueError('Committed history needs exact custody review; supply '
                                 '--history-reason and every affected --behavior, or review it manually')
            history_report = str(Path(report).with_name(Path(report).stem+'-history.json'))
            history = review_history(root, common, [base, *active], record=True,
                reason=history_reason, database=database, report=history_report, behaviors=behaviors)
        details.update(history=history, prepared=True)
    except BaseException as error:
        details.update(prepared=False, error=str(error))
        write_report(root, report, (json.dumps(details, indent=2)+'\n').encode())
        raise
    write_report(root, report, (json.dumps(details, indent=2)+'\n').encode())
    command = [sys.executable, '-B', str(root/'tools/finish-work.py'), '--root', str(root),
               '--db', str(database), '--base', 'HEAD', '--reason', reason,
               '--validation-reason', validation_reason, '--index-review-reason', index_reason,
               '--message', message, '--timings', '--keep-review-cache']
    token=secrets.token_hex(16)
    completed=subprocess.run(command+['--result-token',token],cwd=root,stdout=subprocess.PIPE,text=True)
    print(completed.stdout,end='',flush=True)
    completed.check_returncode()
    records=[line.split()for line in completed.stdout.splitlines()if line.startswith('MM_FINISH_CHECKED '+token+' ')]
    if len(records)!=1 or len(records[0])!=5:
        raise ValueError('Finish did not return this invocation checked commit/tree/base; preserve candidate')
    _,_,commit,tree,checked_base=records[0]
    if (checked_base!=base or not re.fullmatch(r'[0-9a-f]{40,64}',commit)
            or not re.fullmatch(r'[0-9a-f]{40,64}',tree)
            or git(root,'rev-parse','HEAD')!=commit
            or git(root,'rev-parse',commit+'^{tree}')!=tree):
        raise ValueError('Candidate changed after validation; preserve the checked result')
    if git(root, 'status', '--porcelain'):
        raise ValueError('Candidate changed after validation; preserve and review it')
    # Nothing below performs evidence acquisition, review, backups or hooks.
    # A changed master invalidates the prepared candidate, never extends admission.
    with integration_lock(root, expected_master=base) as lease:
        if (git(root, 'rev-parse', 'HEAD') != commit
                or git(root, 'symbolic-ref', '--short', 'HEAD') != branch
                or git(root, 'write-tree') != tree or git(root, 'status', '--porcelain')):
            raise ValueError('Candidate changed before promotion; preserve the checked commit')
        if checkout:
            if git(checkout, 'status', '--porcelain') or git(checkout, 'rev-parse', 'HEAD') != base:
                raise ValueError('Master checkout changed; preserve the checked candidate')
            lease.run(['git', '-C', str(checkout), 'merge', '--ff-only', commit], check=True)
        else:
            lease.run(['git', '-C', str(root), 'update-ref', 'refs/heads/master', commit, base], check=True)
    from evidence_lifecycle import policy, integrated
    clocks = integrated(root, [commit, *pins]) if policy(root) else {}
    return dict(master=commit, previous_master=base, tips=pins, preparation_report=report,
                evidence_review_clocks=clocks,
                candidate_tree=tree, checked=True, validation_outside_lock=True,
                admission_wait_seconds=lease.wait_seconds, publication='local only')
