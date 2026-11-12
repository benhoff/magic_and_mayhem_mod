#!/usr/bin/env python3
"""Create reviewed preservation boundaries or inspect targeted revalidation needs."""
import argparse
import json
from pathlib import Path
import subprocess

from coverage_applicability import PREFIX, PHASES, STAGES, evaluate, plan, python_projection, require, sha, safe_relative
from coverage_claims import behavior_contract, fingerprint, scenario_contract, required_sources, validation_support, ORIGINAL_KINDS
from coverage_evidence import Reader, write_report, identity

ROOT = Path(__file__).resolve().parents[1]
REGISTER = 'research/runtime/coverage/register.json'


def pointer(value, path):
    for key in path.split('/')[1:]:
        key = key.replace('~1', '/').replace('~0', '~')
        value = value[int(key)] if isinstance(value, list) else value[key]
    return value


def inputs(reader, register):
    evidence = {v['id']: v for v in register['evidence']}
    sources, claims = {}, {}
    for eid, ev in evidence.items():
        # Scope only selected records; normal full audit remains authoritative.
        record = reader.json(ev['record'])
        require(reader.digest(ev['record']) == ev['sha256'], eid+': changed evidence payload')
        for assertion in ev['assertions']:
            require(pointer(record, assertion['pointer']) == assertion['equals'], eid+': failed original assertion')
        if ev['kind'] in ORIGINAL_KINDS:
            build=next(v for v in register['builds'] if v['id']==ev['build'])
            require(pointer(record,ev['original_hash_pointer'])==build['source_sha256'],eid+': original build differs')
        if ev['kind'] in {'live_equivalence','live_replacement'}:
            require(pointer(record,ev['equivalence_pointer']) is True,eid+': missing original equivalence')
        if ev['kind']=='live_replacement':
            require(pointer(record,ev['bypass_pointer']) is True,eid+': missing original bypass')
        raw = pointer(record, ev['sources_pointer'])
        raw = raw if isinstance(raw, dict) else {v['path']: v['sha256'] for v in raw}
        bindings = ev.get('source_bindings', {p: p for p in raw})
        sources[eid] = {path: raw[key] for key, path in bindings.items() if key not in ev.get('recorded_inputs', [])}
        claims[eid] = {v['behavior']: v for v in pointer(record, ev['claims_pointer'])} if 'claims_pointer' in ev else {}
    return evidence, sources, claims


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('operation', choices=('create', 'plan'))
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--evidence', required=True)
    parser.add_argument('--behavior', required=True)
    parser.add_argument('--baseline', help='Existing ancestor commit containing the accepted evidence and status')
    parser.add_argument('--id')
    parser.add_argument('--reason')
    parser.add_argument('--supersedes',help='Exact preceding review ID; prior reports remain immutable')
    parser.add_argument('--callables', nargs='+', action='append', default=[], metavar='PHASE_PATH_SYMBOLS')
    parser.add_argument('--full-source', nargs=2, action='append', default=[], metavar=('PHASE', 'PATH'))
    parser.add_argument('--report')
    parser.add_argument('--require-applicable',action='store_true',help='Fail if this evidence cannot preserve every existing declared milestone; never bypass the full audit/gate')
    args = parser.parse_args()
    root = args.root.resolve()
    register = json.loads((root/REGISTER).read_text())
    ev = next(v for v in register['evidence'] if v['id'] == args.evidence)
    b = next(v for v in register['behaviors'] if v['id'] == args.behavior)
    builds = {v['id']: v for v in register['builds']}
    scenarios = {v['id']: v for v in register['scenarios']}
    selected = dict(register, evidence=[ev])
    selected['evidence_applicability'] = list(register.get('evidence_applicability', []))
    with Reader(root) as reader:
        # Load other records only if a selected applicability report names them.
        selected['evidence_applicability'] = [r for r in selected['evidence_applicability']
            if (lambda q: q['evidence']['id'] == ev['id'] and q['behavior'] == b['id'])(reader.json(r['record']))]
        evidence, sources, claims = inputs(reader, selected)
        if args.operation == 'create':
            require(args.baseline and args.id and args.reason and args.reason.strip() and args.report,
                    'Creation needs a baseline, unique ID, concrete review reason and new report')
            require(safe_relative(args.report) and args.report.startswith(PREFIX) and not (root/args.report).exists(), 'New owned immutable report required')
            require(not any(v['id'] == args.id for v in register.get('evidence_applicability', [])), 'Review ID already registered')
            commit = subprocess.check_output(['git', 'rev-parse', '--verify', args.baseline+'^{commit}'], cwd=root, text=True).strip()
            subprocess.run(['git', 'merge-base', '--is-ancestor', commit, 'HEAD'], cwd=root, check=True)
            def old(path):
                require(not Path(path).is_absolute() and '..' not in Path(path).parts and '\n' not in path, 'Unsafe baseline path')
                return subprocess.check_output(['git', 'show', commit+':'+path], cwd=root)
            old_register = json.loads(old(REGISTER))
            old_ev = next(v for v in old_register['evidence'] if v['id'] == ev['id'])
            old_b = next(v for v in old_register['behaviors'] if v['id'] == b['id'])
            require(old_ev == ev and ev['id'] in old_b['evidence'], 'Baseline must contain the exact accepted evidence definition')
            claim = claims[ev['id']].get(b['id'])
            require(claim and claim['contract_sha256'] == behavior_contract(b, builds)
                    == behavior_contract(old_b, {v['id']: v for v in old_register['builds']}), 'No existing unchanged bound behavior claim')
            require(required_sources(b) <= sources[ev['id']].keys(), 'Incomplete original dependencies cannot be repaired by scoping')
            status = {k: v for k, v in old_b['status'].items() if k in STAGES and v != 'none'}
            require(status and all(b['status'][k] == v for k, v in status.items()), 'Applicability preserves existing milestones only')
            anchor = dict(schema=1,commit=commit,evidence=old_ev,behavior=old_b)
            name = args.report.removesuffix('.json')+'-anchor.json'
            anchor_raw=(json.dumps(anchor,indent=2)+'\n').encode();anchor_sha=sha(anchor_raw);pending={name:anchor_raw}
            entries = []
            existing_sources={}
            for p in sorted((root/PREFIX).glob('*.bin')):
                ident=identity(p.read_bytes())
                if ident:existing_sources.setdefault(ident[0],str(p.relative_to(root)))
            for row in args.callables + args.full_source:
                require(len(row) >= 2 and row[0] in PHASES, 'Boundary needs phase and recorded source path')
                phase, path, *symbols = row
                require(path in sources[ev['id']], 'Boundary not in complete original source provenance')
                raw = old(path); recorded = sources[ev['id']][path]
                require(sha(raw) == recorded, 'Git baseline does not reproduce original source bytes: '+path)
                kind = 'python-callables-v1' if symbols else 'full-source-v1'
                require(not symbols or (path.endswith('.py') and path.startswith(('tools/','tests/')) and phase in {'analysis','controls'}),
                        'Native/runtime code cannot use callable preservation')
                projection = python_projection(raw,symbols) if symbols else recorded
                name = existing_sources.get(recorded,PREFIX+'source-'+recorded+'.bin')
                if (root/name).exists():require(reader.digest(name)==recorded,'Source reference collision')
                else:pending[name]=raw
                entry = dict(path=path,phase=phase,kind=kind,recorded_sha256=recorded,
                             baseline_record=name,projection_sha256=projection)
                if symbols:entry['symbols']=symbols
                entries.append(entry)
            report = dict(schema=1,kind='scoped_source_applicability',id=args.id,preservation_only=True,
                reason=args.reason,supersedes=args.supersedes,evidence=dict(id=ev['id'],record=ev['record'],sha256=ev['sha256'],definition_sha256=fingerprint(ev)),
                behavior=b['id'],contract_sha256=claim['contract_sha256'],scenarios=claim.get('scenarios',{}),
                preserve_status=status,baseline_anchor=dict(record=args.report.removesuffix('.json')+'-anchor.json',sha256=anchor_sha),
                source_contracts=entries,original_execution_rerun=False,
                limits='Named callable semantics only; no CLI/test-discovery promise, inferred external closure, native-code waiver, new behavior claim or milestone promotion.')
            pending[args.report]=(json.dumps(report,indent=2)+'\n').encode()
            ref=dict(id=args.id,record=args.report,sha256=sha(pending[args.report]))
            preview=dict(selected,evidence_applicability=selected['evidence_applicability']+[ref])
            evaluate(root,preview,builds,evidence,sources,claims,scenarios,
                lambda p: json.loads(pending[p]) if p in pending else reader.json(p),
                lambda p: pending[p] if p in pending else reader.read_bytes(p),
                lambda p: sha(pending[p]) if p in pending else reader.digest(p))
            for path,raw in pending.items():write_report(root,path,raw)
            register.setdefault('evidence_applicability',[]).append(ref)
            (root/REGISTER).write_text(json.dumps(register,indent=2)+'\n')
            selected.setdefault('evidence_applicability',[]).append(ref)
        # A fresh pending SQLite store may have been created above. Reopen the
        # invocation reader so post-write validation observes that exact store.
        with Reader(root) as final_reader:
            compatible, diagnostics = evaluate(root,selected,builds,evidence,sources,claims,scenarios,
                                               final_reader.json,final_reader.read_bytes,final_reader.digest)
            changed=[p for p,h in sources[ev['id']].items() if final_reader.digest(p)!=h]
            review=compatible.get(b['id'],{}).get(ev['id'])
            support=validation_support(b,builds,evidence,sources,claims,scenarios,{ev['id']} if changed else set(),
                                       compatible.get(b['id'],{}),{ev['id']:changed})
            stages=[k for k in STAGES if b['status'][k]!='none']
            preserved=bool(stages) and all(ev['id'] in support[k].get('evidence',[]) for k in stages)
            print(json.dumps(dict(evidence=ev['id'],behavior=b['id'],raw_changed_sources=changed,
                                 source_actions=plan(changed,review),reviews=diagnostics,
                                 current_scope_preserved=preserved,preserved_stages=[k for k in stages if ev['id'] in support[k].get('evidence',[])],
                                 retained_input=dict(record=ev['record'],sha256=ev['sha256']),
                                 original_execution_rerun=False,scope='Source impact only; current audit/gate still checks contracts, source completeness, stage kinds, scenarios and promotions.'),indent=2))
            require(not args.require_applicable or preserved,'Retained source/contract is impacted; follow the phase plan rather than assuming a current pass')


if __name__ == '__main__':
    main()
