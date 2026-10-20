#!/usr/bin/env python3
"""Validate explicit rendering review decisions; never infer implementation status."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
from coverage_claims import behavior_contract

ROOT=Path(__file__).resolve().parents[1]
LABELS={
    'bounded_code':'Bounded implementation located',
    'diagnostic_tooling':'Diagnostic / research tooling',
    'original_scope':'Original behavior still unresolved',
    'umbrella_scope':'Broad contract with narrower implementations',
    'acceptance_scope':'Acceptance / corpus target',
    'missing_code':'Implementation missing',
}
ROLES={'offline_reconstruction','native_capability','original_contract','diagnostic','acceptance'}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def safe(root,name):
    p=Path(name)
    if p.is_absolute() or not p.parts or p.parts[0] in {'original','working','.git'}:
        raise ValueError('Unsafe reconciliation path: '+name)
    result=(root/p).resolve()
    if (not result.is_relative_to(root.resolve()) or
            result.relative_to(root.resolve()).parts[0] in {'original','working','.git'}):
        raise ValueError('Escaping reconciliation path')
    return result

def load_reviews(root,register):
    """Return exact static reviews, visibly historical when their inputs change.

    Review metadata is deliberately outside execution contract identities. No
    comparison, integration, implementation-completion or replacement is granted.
    """
    root=Path(root);by_id={b['id']:b for b in register['behaviors']}
    builds={b['id']:b for b in register['builds']};hashes={};reviews=[];seen=set()
    for ref in register.get('implementation_reviews',[]):
        record=safe(root,ref['path'])
        if sha(record)!=ref['sha256']:raise ValueError('Reconciliation record hash mismatch')
        doc=json.loads(record.read_text())
        if doc.get('schema')!=1 or doc.get('static_review_only') is not True or doc['id']!=ref['id']:
            raise ValueError('Invalid static reconciliation header')
        entries=doc['entries'];ids=[e['behavior'] for e in entries]
        if len(ids)!=len(set(ids)) or set(ids)!=set(doc['reviewed_behaviors']):
            raise ValueError('Duplicate or incomplete reconciliation entries')
        if len(doc['reviewed_behaviors'])!=len(set(doc['reviewed_behaviors'])):
            raise ValueError('Duplicate reviewed behavior')
        if doc['reviewed_count']!=len(entries):raise ValueError('Reconciliation denominator changed')
        sources=doc['source_sha256']
        for name in sources:
            path=safe(root,name)
            if name not in hashes:hashes[name]=sha(path) if path.is_file() else None
        enriched=[]
        for e in entries:
            bid=e['behavior'];b=by_id.get(bid)
            if not b or bid in seen:raise ValueError('Unknown or multiply reviewed behavior: '+bid)
            seen.add(bid)
            if e['disposition'] not in LABELS or e['role'] not in ROLES:
                raise ValueError('Unknown reconciliation classification')
            if not e.get('finding','').strip() or not e.get('remaining','').strip():
                raise ValueError('Reconciliation needs concrete finding and remaining work')
            if e['disposition']=='missing_code' and e.get('code_witness'):
                raise ValueError('Missing code cannot have a claimed implementation witness')
            expected={'review':doc['id'],'entry':bid}
            if b.get('implementation_review')!=expected:
                raise ValueError('Missing exact register review link: '+bid)
            required=set(b['implementation'])|set(b['tests'])|set(b['documents'])|set(b.get('validation_dependencies',[]))
            if not required.issubset(set(e['reviewed_paths'])) or not set(e['reviewed_paths']).issubset(sources):
                raise ValueError('Incomplete reviewed source paths: '+bid)
            changed=[p for p in e['reviewed_paths'] if hashes[p]!=sources[p]]
            pending=e.get('pending_source_review',[])
            if not isinstance(pending,list) or not set(pending).issubset(e['reviewed_paths']):
                raise ValueError('Pending reconciliation paths must be explicitly pinned')
            contract_current=behavior_contract(b,builds)==e['contract_sha256']
            for relation in e['related_behaviors']:
                if relation not in by_id or relation==bid:raise ValueError('Invalid narrower behavior link')
            witness=e.get('code_witness')
            if e['disposition'] in {'bounded_code','diagnostic_tooling'} and not witness:
                raise ValueError('Code/tooling review requires a source witness')
            for w in [w for w in (witness,e.get('boundary_witness')) if w]:
                if w['path'] not in e['reviewed_paths'] or w['path'] not in sources:
                    raise ValueError('Unbound reconciliation witness')
                if w is witness and w['path'] not in b['implementation']:
                    raise ValueError('Code witness is not an implementation link')
                if not w.get('text','').strip() or not isinstance(w.get('line'),int) or w['line']<1:
                    raise ValueError('Invalid source witness')
                if hashes[w['path']]==sources[w['path']]:
                    lines=safe(root,w['path']).read_text(errors='replace').splitlines()
                    if w['line']>len(lines) or lines[w['line']-1]!=w['text']:
                        raise ValueError('Reconciliation witness differs from reviewed source')
            status_current=b['status']['implementation']==e['recorded_implementation']
            enriched.append({**e,'label':LABELS[e['disposition']],
                             'freshness':'current' if contract_current and status_current and not changed and not pending else 'historical',
                             'changed_sources':changed,'contract_current':contract_current})
        counts=dict(Counter(e['disposition'] for e in enriched))
        if doc['counts']!=counts:raise ValueError('Curated review counts disagree with entries')
        reviews.append({'id':doc['id'],'track':doc['track'],'record':ref['path'],
                        'document':doc['document'],'reviewed_count':len(entries),'counts':counts,
                        'labels':LABELS,'entries':enriched,
                        'freshness':'current' if all(e['freshness']=='current' for e in enriched) else 'historical'})
    linked={b['id'] for b in register['behaviors'] if b.get('implementation_review')}
    if linked!=seen:raise ValueError('Orphan reconciliation links')
    return reviews

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--json',type=Path)
    p.add_argument('--require-fresh',action='store_true');a=p.parse_args()
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    rows=load_reviews(ROOT,r)
    result={'static_review_only':True,'reviews':[{k:v for k,v in row.items() if k!='entries'} for row in rows]}
    if a.json:a.json.parent.mkdir(parents=True,exist_ok=True);a.json.open('x').write(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
    if a.require_fresh and any(row['freshness']!='current' for row in rows):raise SystemExit(1)
if __name__=='__main__':main()
