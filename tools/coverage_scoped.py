"""Fresh Git deltas and affected-claim checks, without whole-tree extraction.

The dependency index records provenance, never an audit pass. Existing entries
must equal the actual base; new entries are decoded from verified report bytes.
"""

from contextlib import contextmanager
import copy
import hashlib
import io
import json
import os
from pathlib import Path
import subprocess
import tempfile
from functools import lru_cache

from coverage_evidence import Reader, decode_bytes, identity, shared_cache, write_report
from coverage_gate import (
    REGISTER,
    REVIEWS,
    BASELINE,
    CONTROL_FILES,
    source_index,
    digest,
    changes,
    gap_items,
)
from coverage_reviews import read_journal
from coverage_audit import audit, source_hashes, pointer

INDEX = "change_scope_index"
ROLES = ("implementation", "tests", "documents", "validation_dependencies")


def git(root, *args):
    return subprocess.check_output(
        ["git", "-C", str(root), *args], stderr=subprocess.PIPE
    )


def safe(name):
    path = Path(name)
    if path.is_absolute() or ".." in path.parts or not path.parts:
        raise ValueError("Unsafe Git path: " + name)
    return path.parts[0] not in {"original", "working", ".git"}


def canonical_path(name):
    normalized = os.path.normpath(name)
    if not safe(normalized):
        raise ValueError("Unsafe evidence dependency: " + name)
    return normalized


class View:
    """Read only exact Git blobs, index blobs or current owned working bytes."""

    def __init__(self, root, revision=None, staged=False):
        self.root = Path(root).resolve()
        self.revision = revision
        self.staged = staged
        args = (
            ("ls-tree", "-rz", "--full-tree", revision)
            if revision
            else ("ls-files", "--stage", "-z")
        )
        self.entries = {}
        for entry in filter(None, git(root, *args).split(b"\0")):
            metadata, name = entry.split(b"\t", 1)
            mode, kind, oid = metadata.split()
            name = os.fsdecode(name)
            if not safe(name):
                continue
            if revision:
                sha = oid
                valid = kind == b"blob"
            else:
                sha = kind
                valid = oid == b"0"
            if not valid or mode not in (b"100644", b"100755"):
                raise ValueError("Resolve unsupported/conflicted Git entry: " + name)
            self.entries[name] = (mode, sha)
        if not revision and not staged:
            for name in filter(
                None,
                git(root, "ls-files", "--others", "--exclude-standard", "-z").split(
                    b"\0"
                ),
            ):
                name = os.fsdecode(name)
                if safe(name):
                    self.entries[name] = (b"100644", None)
            deleted = {
                os.fsdecode(n)
                for n in git(root, "ls-files", "--deleted", "-z").split(b"\0")
                if n
            }
            for n in deleted:
                self.entries.pop(n, None)
        self.batch = subprocess.Popen(
            ["git", "-C", str(root), "cat-file", "--batch"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
        )
        self.cache = {}
        self.reader = Reader(self.root)

    def close(self):
        self.batch.stdin.close()
        self.batch.stdout.close()
        self.batch.wait()
        self.reader.close()

    def raw(self, name):
        name = canonical_path(name)
        if not safe(name) or name not in self.entries:
            raise ValueError("Missing scoped input: " + name)
        if not self.revision and not self.staged:
            path = self.root / name
            if path.is_symlink() or not path.resolve().is_relative_to(self.root):
                raise ValueError("Escaping input: " + name)
            return path.read_bytes()
        sha = self.entries[name][1]
        if sha not in self.cache:
            self.batch.stdin.write(sha + b"\n")
            self.batch.stdin.flush()
            header = self.batch.stdout.readline().split()
            if len(header) != 3 or header[1] != b"blob":
                raise ValueError("Git blob unavailable")
            size = int(header[2])
            body = self.batch.stdout.read(size)
            if len(body) != size or self.batch.stdout.read(1) != b"\n":
                raise ValueError("Truncated Git blob")
            self.cache[sha] = body
        return self.cache[sha]

    def payload(self, name):
        data = self.raw(name)
        ref = identity(data)
        if ref:
            out = io.BytesIO()
            self.reader.resolver.copy(*ref, out)
            return out.getvalue()
        return data

    def value(self, name):
        return json.loads(self.payload(name))

    def sha(self, name):
        data = self.raw(name)
        ref = identity(data)
        return ref[0] if ref else hashlib.sha256(data).hexdigest()


def dependency(ev, view):
    if view.sha(ev["record"]) != ev["sha256"]:
        raise ValueError("Evidence record hash mismatch: " + ev["id"])
    record = view.value(ev["record"])
    sources = source_hashes(pointer(record, ev["sources_pointer"]))
    return dict(definition_sha256=digest(ev), paths=tuple(sorted(sources)))


def dependencies(view, register, before=None, prior=None):
    ref = register.get(INDEX)
    catalog = decode_index(view.value(ref["path"])) if ref else None
    if catalog is not None:
        if view.sha(ref["path"]) != ref["sha256"]:
            raise ValueError("Evidence dependency index hash mismatch")
        if set(catalog) != {e["id"] for e in register["evidence"]}:
            raise ValueError("Evidence dependency index membership differs")
    for section in ("builds", "behaviors", "evidence", "scenarios", "recovered_ranges"):
        rows = register.get(section, [])
        ids = [v["id"] for v in rows]
        if len(ids) != len(set(ids)):
            raise ValueError("Duplicate " + section + " ID")
    for ref in register.get("register_imports", []):
        if not isinstance(ref.get("scope"), str) or not ref["scope"].strip():
            raise ValueError("Focused-register import scope missing")
    for scenario in register["scenarios"]:
        if not scenario.get("evidence") or not scenario.get("tests"):
            raise ValueError("Scenario missing evidence/tests: " + scenario["id"])
    result = {}
    old = {e["id"]: e for e in (before or {}).get("evidence", [])}
    for ev in register["evidence"]:
        eid = ev["id"]
        row = catalog.get(eid) if catalog else None
        if row is not None and row["definition_sha256"] != digest(ev):
            raise ValueError("Stale evidence dependency entry: " + eid)
        if before is not None and old.get(eid) == ev and prior is not None:
            expected = prior[eid]
            if row is not None and row != expected:
                raise ValueError("Historical evidence dependencies rewritten: " + eid)
        elif before is None and catalog is not None:
            # The exact base's pinned provenance map is trusted metadata, not a
            # validation result. Its report identity is independently verified.
            expected = row
        else:
            expected = dependency(ev, view)
        if row is not None and row != expected:
            raise ValueError("Evidence dependencies do not match report: " + eid)
        result[eid] = expected
    return result


def decode_index(catalog):
    if catalog.get("schema") == 1:
        # Compatibility for the uncommitted adoption prototype; do not copy
        # its repeated source-hash maps into the new dependency representation.
        return {
            eid: dict(
                definition_sha256=row["definition_sha256"],
                paths=tuple(sorted(row["sources"])),
            )
            for eid, row in catalog["evidence"].items()
        }
    if catalog.get("schema") not in (2, 3):
        raise ValueError("Unsupported dependency index schema")
    path_table = catalog.get("paths")
    if catalog["schema"] == 3 and (
        not isinstance(path_table, list) or path_table != sorted(set(path_table))
    ):
        raise ValueError("Malformed dependency path table")
    sets = {}
    for sha, names in catalog["path_sets"].items():
        if catalog["schema"] == 3:
            if not isinstance(names, list) or any(
                type(i) is not int or i < 0 or i >= len(path_table) for i in names
            ):
                raise ValueError("Invalid dependency path identifier")
            names = [path_table[i] for i in names]
        if (
            not isinstance(names, list)
            or names != sorted(set(names))
            or digest(names) != sha
        ):
            raise ValueError("Dependency path-set identity differs")
        sets[sha] = tuple(names)
    rows = {}
    for eid, row in catalog["evidence"].items():
        if set(row) != {"definition_sha256", "path_set"}:
            raise ValueError("Malformed dependency row")
        rows[eid] = dict(
            definition_sha256=row["definition_sha256"], paths=sets[row["path_set"]]
        )
    return rows


@lru_cache(maxsize=256)
def canonical_dependencies(names, bindings, inputs):
    ignore = set(inputs)
    paths = (
        (p for k, p in bindings if k not in ignore)
        if bindings
        else (n for n in names if n not in ignore)
    )
    return frozenset(canonical_path(p) for p in paths)


def dependency_paths(ev, row):
    return canonical_dependencies(
        row["paths"],
        tuple(sorted(ev.get("source_bindings", {}).items())),
        tuple(sorted(ev.get("recorded_inputs", []))),
    )


def bound_sources(ev, sources):
    inputs = set(ev.get("recorded_inputs", []))
    return (
        {p: sources[k] for k, p in ev["source_bindings"].items() if k not in inputs}
        if "source_bindings" in ev
        else {k: v for k, v in sources.items() if k not in inputs}
    )


def affected(before, after, oldreg, newreg, olddeps, newdeps):
    if after.revision or after.staged:
        changed = {
            n
            for n in set(before.entries) | set(after.entries)
            if before.entries.get(n) != after.entries.get(n)
        }
    else:
        changed = {
            os.fsdecode(n)
            for n in git(
                after.root,
                "diff",
                "--name-only",
                "--no-renames",
                "-z",
                before.revision,
                "--",
            ).split(b"\0")
            if n
        }
        changed.update(set(before.entries) ^ set(after.entries))
    if any(Path(n).parts[0] == "original" for n in changed):
        raise ValueError("Immutable original inputs changed")
    selected = set()
    evs = set()
    oldb = {b["id"]: b for b in oldreg["behaviors"]}
    newb = {b["id"]: b for b in newreg["behaviors"]}
    selected.update(k for k in set(oldb) | set(newb) if oldb.get(k) != newb.get(k))
    olde = {e["id"]: e for e in oldreg["evidence"]}
    newe = {e["id"]: e for e in newreg["evidence"]}
    evs.update(k for k in set(olde) | set(newe) if olde.get(k) != newe.get(k))
    for reg, deps in ((oldreg, olddeps), (newreg, newdeps)):
        for e in reg["evidence"]:
            if (
                canonical_path(e["record"]) in changed
                or dependency_paths(e, deps[e["id"]]) & changed
            ):
                evs.add(e["id"])
        for b in reg["behaviors"]:
            if any(
                {canonical_path(n) for n in b.get(role, [])} & changed for role in ROLES
            ):
                selected.add(b["id"])
    # Metadata outside behavior rows can change claims too. Conservatively
    # inspect all behavior contracts for changed build/discovery/applicability.
    broad = (
        "builds",
        "classifications",
        "api_bindings",
        "dispatch_tables",
        "recovered_ranges",
        "evidence_applicability",
        "implementation_reviews",
        "implementation_review_history",
        "claim_bindings_required",
        "evidence_id_imports",
    )
    if any(oldreg.get(k) != newreg.get(k) for k in broad):
        selected.update(set(oldb) | set(newb))
    olds = {s["id"]: s for s in oldreg["scenarios"]}
    news = {s["id"]: s for s in newreg["scenarios"]}
    for sid in set(olds) | set(news):
        if olds.get(sid) != news.get(sid):
            for s in (olds.get(sid), news.get(sid)):
                if s:
                    selected.update(s["behaviors"])
                    evs.update(s["evidence"])
    # Close shared scenarios/evidence/dispatch links on BOTH sides. This keeps
    # canonical contracts intact rather than trimming their fields to fit scope.
    while True:
        size = (len(selected), len(evs))
        for reg in (oldreg, newreg):
            for b in reg["behaviors"]:
                if b["id"] in selected or set(b["evidence"]) & evs:
                    selected.add(b["id"])
                    evs.update(b["evidence"])
            for s in reg["scenarios"]:
                if set(s["behaviors"]) & selected or set(s["evidence"]) & evs:
                    selected.update(s["behaviors"])
                    evs.update(s["evidence"])
            for t in reg["dispatch_tables"]:
                ids = {b for e in t["entries"] for b in e["behaviors"]}
                if ids & selected:
                    selected.update(ids)
            for a in reg["api_bindings"]:
                if set(a["behaviors"]) & selected:
                    selected.update(a["behaviors"])
            ranges = {
                ref.get("recovered_range")
                for b in reg["behaviors"]
                if b["id"] in selected
                for ref in b["original"]
            }
            ranges.update(
                t["original"].get("recovered_range")
                for t in reg["dispatch_tables"]
                if any(set(e["behaviors"]) & selected for e in t["entries"])
            )
            for recovered in reg["recovered_ranges"]:
                if recovered["id"] in ranges:
                    evs.add(recovered["evidence"])
        if size == (len(selected), len(evs)):
            break
    return changed, selected, evs


def subset(reg, bids, eids, wide=False):
    value = copy.deepcopy(reg)
    value["behaviors"] = [b for b in reg["behaviors"] if b["id"] in bids]
    value["evidence"] = [e for e in reg["evidence"] if e["id"] in eids]
    value["scenarios"] = [
        s
        for s in reg["scenarios"]
        if set(s["behaviors"]) & bids or set(s["evidence"]) & eids
    ]
    builds = {r["build"] for b in value["behaviors"] for r in b["original"]}
    builds.update(e["build"] for e in value["evidence"] if "build" in e)
    builds.update(
        a["build"] for a in reg["api_bindings"] if wide or set(a["behaviors"]) & bids
    )
    builds.update(
        t["original"]["build"]
        for t in reg["dispatch_tables"]
        if wide or any(set(e["behaviors"]) & bids for e in t["entries"])
    )
    if wide:
        builds.update(b["id"] for b in reg["builds"])
    value["builds"] = [b for b in reg["builds"] if b["id"] in builds]
    ranges = {
        ref.get("recovered_range") for b in value["behaviors"] for ref in b["original"]
    }
    ranges.update(
        t["original"].get("recovered_range")
        for t in reg["dispatch_tables"]
        if wide or any(set(e["behaviors"]) & bids for e in t["entries"])
    )
    value["recovered_ranges"] = [
        r for r in reg["recovered_ranges"] if wide or r["id"] in ranges
    ]
    value["classifications"] = [
        r for r in reg["classifications"] if r["build"] in builds
    ]
    value["api_bindings"] = [
        a for a in reg["api_bindings"] if wide or set(a["behaviors"]) & bids
    ]
    value["dispatch_tables"] = [
        t
        for t in reg["dispatch_tables"]
        if wide or any(set(e["behaviors"]) & bids for e in t["entries"])
    ]
    value.pop("code_index", None)
    value.pop("register_imports", None)
    return value


def strings(value):
    if isinstance(value, str):
        yield value
    elif isinstance(value, dict):
        for k, v in value.items():
            yield from strings(k)
            yield from strings(v)
    elif isinstance(value, list):
        for v in value:
            yield from strings(v)


def scoped_snapshot(view, reg, deps, changed, bids, eids, wide=False):
    selected = subset(reg, bids, eids, wide)
    names = {n for n in view.entries if source_index.in_scope(n)}
    names.update(
        n
        for n in (*CONTROL_FILES, REGISTER, REVIEWS, BASELINE, ".evidence-store.json")
        if n in view.entries
    )
    docs = {p for b in reg["behaviors"] for p in b["documents"]}
    names.update(docs)
    names.update(changed & set(view.entries))
    for e in selected["evidence"]:
        names.add(e["record"])
        names.update(dependency_paths(e, deps[e["id"]]))
    for b in selected["builds"]:
        names.add(b["inventory"])
    for b in selected["behaviors"]:
        for role in ROLES:
            names.update(b.get(role, []))
    for recovered in selected["recovered_ranges"]:
        names.add(recovered["document"])
    for r in selected["classifications"]:
        names.update(r["documents"])
    for t in selected["dispatch_tables"]:
        names.add(t["document"])
    for s in selected["scenarios"]:
        names.update(s["tests"])
    refs = []
    for ref in reg.get("evidence_applicability", []):
        data = view.value(ref["record"])
        if wide or data["behavior"] in bids:
            refs.append(ref)
            names.add(ref["record"])
            names.update(n for n in strings(data) if n in view.entries)
    selected["evidence_applicability"] = refs
    journal, paths = read_journal(view.payload)
    names.update(paths)
    for ref in reg.get("register_imports", []):
        names.add(ref["path"])
    if "code_index" in reg:
        names.add(reg["code_index"]["path"])
    if INDEX in reg:
        names.add(reg[INDEX]["path"])
    with shared_cache(view.root), tempfile.TemporaryDirectory(
        prefix="coverage-scoped-"
    ) as temporary:
        target = Path(temporary)
        for n in sorted(names):
            p = target / canonical_path(n)
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(view.raw(n))
        with Reader(target, memoize=True) as reader:
            observed = {}
            report = audit(target, selected, recorded_sources=observed, reader=reader)
            if report["errors"]:
                raise ValueError(
                    "Affected coverage audit failed: " + "; ".join(report["errors"])
                )
            census = source_index.index(target, reg)
            if "code_index" not in reg:
                raise ValueError("Coverage gate requires a pinned code_index")
            ref = reg["code_index"]
            catalog = reader.json(ref["path"])
            if reader.digest(ref["path"]) != ref["sha256"]:
                raise ValueError("Source index hash mismatch")
            indexed = {r["path"]: r for r in catalog["files"]}
            actual = {r["path"]: r for r in census["files"]}
            if indexed != actual:
                raise ValueError("Reconcile source census drift before checking")
            for key in ("schema", "roots", "suffixes", "build_names", "build_suffixes"):
                if catalog.get(key) != census.get(key):
                    raise ValueError("Source census scope differs: " + key)
            for ref in reg.get("register_imports", []):
                if reader.digest(ref["path"]) != ref["sha256"]:
                    raise ValueError("Reconcile focused-register drift: " + ref["path"])
            sources = {r["path"]: r for r in census["files"]}
            for n in sorted(
                (docs | changed | set(CONTROL_FILES))
                & set(view.entries) - paths - {BASELINE}
            ):
                if n not in sources:
                    sources[n] = dict(
                        path=n,
                        sha256=(reader.digest(n) if n in changed else view.sha(n)),
                        role="control" if n in CONTROL_FILES else "research",
                        references=[],
                    )
            for b in reg["behaviors"]:
                for n in b["documents"]:
                    sources[n]["references"].append(
                        dict(behavior=b["id"], role="documents")
                    )
            # Evidence provenance supplies impact links, not invented semantic
            # implementation mappings in the authored source census.
            linked = {e["id"]: set() for e in reg["evidence"]}
            for b in reg["behaviors"]:
                for eid in b["evidence"]:
                    linked[eid].add(b["id"])
            for e in reg["evidence"]:
                for n in dependency_paths(e, deps[e["id"]]) & changed:
                    if n in sources:
                        for bid in sorted(linked[e["id"]]):
                            ref = dict(behavior=bid, role="evidence_dependency")
                            if ref not in sources[n]["references"]:
                                sources[n]["references"].append(ref)
            for n, row in sources.items():
                mode = view.entries[canonical_path(n)][0].decode()
                if n in changed and not view.revision and not view.staged:
                    mode = (
                        "100755" if (view.root / n).stat().st_mode & 0o111 else "100644"
                    )
                row["git_mode"] = mode
            builds = {b["id"]: b for b in reg["builds"]}
            evidence = {e["id"]: e for e in reg["evidence"]}
            behaviors = {}
            for b in reg["behaviors"]:
                # Only affected contracts need fresh source fingerprints. Every
                # directly linked changed path has already entered the scope.
                contract_paths = {p for role in ROLES for p in b.get(role, [])}
                fingerprints = {p: view.sha(p) for p in contract_paths}
                contract = dict(
                    behavior=b,
                    sources=fingerprints,
                    builds=[
                        builds[k] for k in sorted({r["build"] for r in b["original"]})
                    ],
                    evidence=sorted(
                        (evidence[e] for e in b["evidence"]), key=lambda e: e["id"]
                    ),
                    scenarios=sorted(
                        (s for s in reg["scenarios"] if b["id"] in s["behaviors"]),
                        key=lambda s: s["id"],
                    ),
                )
                behaviors[b["id"]] = dict(sha256=digest(contract), record=b)
            artifacts = {
                n: (reader.digest(n) if n in changed else view.sha(n))
                for n in names
                if n.startswith("research/")
                and n.endswith(".json")
                and n not in paths | {BASELINE}
            }
            result = dict(
                schema=1,
                scope="Fresh affected-claim accounting; unrelated evidence is not audited.",
                sources=sources,
                behaviors=behaviors,
                evidence=evidence,
                evidence_keys={
                    e["id"]: list(deps[e["id"]]["paths"]) for e in reg["evidence"]
                },
                evidence_sources={
                    e["id"]: (
                        bound_sources(e, observed[e["id"]])
                        if e["id"] in observed
                        else {}
                    )
                    for e in reg["evidence"]
                },
                evidence_id_imports=reg.get("evidence_id_imports", []),
                inventories={
                    b["inventory"]: b["inventory_sha256"] for b in reg["builds"]
                },
                research_artifacts=artifacts,
                builds={b["id"]: b["source_sha256"] for b in reg["builds"]},
                historical_reviews=journal["reviews"],
                claim_bindings_required=reg.get("claim_bindings_required", False),
                evidence_applicability=reg.get("evidence_applicability", []),
                gaps=gap_items(report),
            )
            # Preserve all original inventory identities even when their builds
            # are outside this semantic audit. Changed inventory bytes are read.
            for b in reg["builds"]:
                result["research_artifacts"][b["inventory"]] = view.sha(b["inventory"])
    report["findings"]["sources_without_behavior_links"] = [
        r["path"] for r in census["files"] if not r["references"]
    ]
    result["gaps"] = gap_items(report)
    report["checked_evidence"] = sorted(eids)
    report["checked_scope"] = dict(
        behaviors=sorted(bids),
        evidence=sorted(eids),
        changed_paths=sorted(changed),
        full_corpus=False,
    )
    return result, report, journal


def _capture(root, base, staged=False):
    before = View(
        root,
        git(root, "rev-parse", "--verify", "--end-of-options", base + "^{commit}")
        .decode()
        .strip(),
    )
    after = View(root, staged=staged)
    try:
        original_changes = git(
            root,
            "diff",
            "--cached" if staged else "--no-ext-diff",
            "--name-only",
            before.revision,
            "--",
            "original/",
        ).strip()
        if original_changes:
            raise ValueError("Immutable original inputs changed")
        if BASELINE not in before.entries:
            raise ValueError("Base predates gate baseline; bootstrap is required")
        old = before.value(REGISTER)
        new = after.value(REGISTER)
        olddeps = dependencies(before, old)
        newdeps = dependencies(after, new, old, olddeps)
        changed, bids, eids = affected(before, after, old, new, olddeps, newdeps)
        metadata = (
            "builds",
            "classifications",
            "api_bindings",
            "dispatch_tables",
            "recovered_ranges",
            "evidence_applicability",
            "implementation_reviews",
            "implementation_review_history",
            "claim_bindings_required",
            "evidence_id_imports",
        )
        wide = any(old.get(k) != new.get(k) for k in metadata)
        if wide:
            eids.update(e["id"] for r in (old, new) for e in r["evidence"])
        first, _, _ = scoped_snapshot(before, old, olddeps, changed, bids, eids, wide)
        second, report, journal = scoped_snapshot(
            after, new, newdeps, changed, bids, eids, wide
        )
        return first, second, report, journal, before.revision
    finally:
        before.close()
        after.close()


def _write_index(root):
    view = View(root)
    try:
        reg = view.value(REGISTER)
        prior = decode_index(view.value(reg[INDEX]["path"])) if INDEX in reg else None
        rows = {
            e["id"]: (
                prior[e["id"]]
                if prior
                and prior.get(e["id"], {}).get("definition_sha256") == digest(e)
                else dependency(e, view)
            )
            for e in reg["evidence"]
        }
        path_sets = {}
        compact = {}
        for eid, row in rows.items():
            key = digest(row["paths"])
            path_sets.setdefault(key, list(row["paths"]))
            compact[eid] = dict(
                definition_sha256=row["definition_sha256"], path_set=key
            )
        paths = sorted({n for row in rows.values() for n in row["paths"]})
        ids = {n: i for i, n in enumerate(paths)}
        encoded_sets = {
            sha: [ids[n] for n in names] for sha, names in path_sets.items()
        }
        body = (
            json.dumps(
                dict(
                    schema=3,
                    scope="Provenance dependency paths only; original source hashes are verified in consumed reports, never cached validation.",
                    evidence=compact,
                    paths=paths,
                    path_sets=encoded_sets,
                ),
                sort_keys=True,
                separators=(",", ":"),
            )
            + "\n"
        ).encode()
        sha = hashlib.sha256(body).hexdigest()
        name = "research/runtime/coverage/evidence-dependencies-" + sha + ".json"
        if name not in view.entries:
            write_report(root, name, body)
        path = Path(root) / REGISTER
        old = path.read_bytes()
        reg[INDEX] = dict(path=name, sha256=sha)
        from coverage_bookkeeping import atomic_replace

        atomic_replace(path, (json.dumps(reg, indent=2) + "\n").encode(), old)
        return name
    finally:
        view.close()


def capture(root, base, staged=False):
    from evidence_local import local_evidence

    from verification_budget import workers

    with workers(root), local_evidence(root, verify_archives=False):
        return _capture(root, base, staged)


def enabled(root, staged=False):
    try:
        body = (
            git(root, "show", ":" + REGISTER)
            if staged
            else (Path(root) / REGISTER).read_bytes()
        )
        return INDEX in json.loads(body)
    except (OSError, ValueError, subprocess.CalledProcessError):
        return False


def write_index(root):
    from evidence_local import local_evidence

    from verification_budget import workers

    with workers(root), local_evidence(root, verify_archives=False):
        return _write_index(root)
