"""Retain only this exact staged source batch and its report payloads.

Existing archive objects are not scrubbed or migrated as a side effect of a
commit. This is append-only custody and grants no permission to remove storage.
"""

from pathlib import Path
import io
import os
import tempfile
from coverage_scoped import View, git
from coverage_evidence import identity
from evidence_objects import configured, Sink


def retain_staged_batch(root, base):
    root = Path(root).resolve()
    if git(root, "rev-parse", "HEAD").decode().strip() != base:
        raise ValueError("Commit base changed before retention")
    paths = [
        os.fsdecode(n)
        for n in git(
            root, "diff", "--cached", "--name-only", "--no-renames", "-z", base, "--"
        ).split(b"\0")
        if n
    ]
    view = View(root, staged=True)
    try:
        members = []
        payloads = []
        tree = git(root, "write-tree").decode().strip()
        with configured(root) as pair:
            with pair.batch():
                for name in paths:
                    if Path(name).parts[0] in {"original", "working", ".git"}:
                        raise ValueError("Unsafe commit input: " + name)
                    if name not in view.entries:
                        members.append(dict(path=name, deleted=True))
                        continue
                    body = view.raw(name)
                    sha = pair.put(io.BytesIO(body), len(body), copies=2)
                    members.append(
                        dict(
                            path=name,
                            sha256=sha,
                            size=len(body),
                            git_blob=view.entries[name][1].decode(),
                            git_mode=view.entries[name][0].decode(),
                        )
                    )
                    ref = identity(body)
                    if ref:
                        if pair.object_copies(ref[0]) != 2 or not all(
                            s.has(*ref) for s in pair.stores
                        ):
                            scratch = root / "working/coverage/commit-retention"
                            scratch.mkdir(parents=True, exist_ok=True)
                            with tempfile.SpooledTemporaryFile(
                                max_size=8 * 1024**2, dir=scratch
                            ) as f:
                                view.reader.resolver.copy(*ref, f)
                                f.seek(0)
                                if pair.put(f, ref[1], copies=2) != ref[0]:
                                    raise ValueError("Report identity changed")
                        else:
                            for s in pair.stores:
                                s.copy(*ref, Sink())
                        payloads.append(dict(path=name, sha256=ref[0], size=ref[1]))
                manifest = pair.root(
                    "staged_commit_batch",
                    str(root),
                    dict(
                        schema=1,
                        base=base,
                        tree=tree,
                        source_members=members,
                        report_payloads=payloads,
                        copies=2,
                        scope="Exact staged source bytes and cited payloads only; no storage retirement or engine validation.",
                    ),
                )
        if git(root, "write-tree").decode().strip() != tree:
            raise ValueError("Index changed during commit retention")
        return dict(
            success=True,
            manifest=manifest,
            base=base,
            tree=tree,
            source_files=len(members),
            report_payloads=len(payloads),
            replicas_verified=2,
            retirement_authorized=False,
        )
    finally:
        view.close()
