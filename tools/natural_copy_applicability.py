"""Preserve observation proof when only post-retirement host close changes."""
import ast,hashlib
from rendering_pilot import require

COLLECTOR='tools/run-rendering-copy-branches.py'


def collector_review(old,new):
    before=ast.parse(old);after=ast.parse(new)
    helpers=[n for n in after.body if isinstance(n,ast.FunctionDef) and n.name=='close_after_retirement']
    require(len(helpers)==1 and not any(isinstance(n,ast.FunctionDef) and n.name=='close_after_retirement' for n in before.body),
            'Unexpected host-close helper phase')
    after.body.remove(helpers[0])
    dump=lambda n:ast.dump(n,include_attributes=False)
    previous=ast.parse("if child.poll() is None:\n    journey.input.close_window()\nrequire(child.wait(timeout=10) == 0, 'Native host failed')").body
    current=ast.parse('close_after_retirement(child,journey.input)').body[0]
    guard=ast.parse("require(result.get('retirement', {}).get('state') == 'completed', 'Original Quit did not retire normally')").body[0]
    counts=[0,0]
    def normalize(node,phase):
        for field,value in ast.iter_fields(node):
            if isinstance(value,list):
                output=[];i=0
                while i<len(value):
                    item=value[i]
                    old_match=phase==0 and i+1<len(value) and dump(item)==dump(previous[0]) and dump(value[i+1])==dump(previous[1])
                    new_match=phase==1 and dump(item)==dump(current)
                    if old_match or new_match:
                        require(i>0 and dump(value[i-1])==dump(guard),'Host-close change precedes original retirement')
                        output.append(ast.Expr(value=ast.Constant(value='reviewed post-retirement host closure')))
                        counts[phase]+=1;i+=2 if old_match else 1;continue
                    if isinstance(item,ast.AST):normalize(item,phase)
                    output.append(item);i+=1
                setattr(node,field,output)
            elif isinstance(value,ast.AST):normalize(value,phase)
    normalize(before,0);normalize(after,1)
    require(counts==[1,1] and dump(before)==dump(after),'Observation/source semantics changed outside post-retirement host close')
    return dict(verified=True,old_sha256=hashlib.sha256(old).hexdigest(),current_sha256=hashlib.sha256(new).hexdigest(),
        normalized_module_sha256=hashlib.sha256(dump(before).encode()).hexdigest(),
        scope='All module initialization, physical Journey methods, acquisition preparation, native configuration, observation/retention and failure paths have identical AST. Only one host-close block after observed completed original retirement is replaced and one helper added. Preserve raw file drift; no new execution, domain, original-driver or host-close execution credit.')


def execution_review(reader,acquired):
    drift={n:dict(recorded=h,current=reader.digest(n)) for n,h in acquired['sources'].items() if reader.digest(n)!=h}
    require(set(drift)<= {COLLECTOR},'Execution source changed outside the scoped collector close')
    if not drift:return dict(verified=True,raw_source_drift={},source_applicability=None)
    plan=reader.json(acquired['plan']['record'])
    require(reader.digest(acquired['plan']['record'])==acquired['plan']['sha256'],'Frozen collector plan changed')
    source=plan['collector_source'];raw=reader.read_bytes(source['record'])
    require(reader.digest(source['record'])==source['sha256']==acquired['sources'][COLLECTOR], 'Original collector source bytes unavailable')
    checked=collector_review(raw,reader.read_bytes(COLLECTOR))
    return dict(verified=True,raw_source_drift=drift,source_applicability=checked)
