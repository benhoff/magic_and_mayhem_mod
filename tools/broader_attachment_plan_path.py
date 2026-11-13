"""Exact setup-only canonical-plan guard adaptation for the frozen attachment collector."""
import ast,hashlib
from pathlib import Path
from rendering_pilot import require
SOURCE_SHA256='0ac248ebdbfbe20c4d6ee7d66e3087bcdcb057eb7713bdb1b8b2e764ff11e15c'
PREFIX='research/runtime/broader-player-acceptance-results-20261110/'

def adapt(raw,filename,phase):
    require(hashlib.sha256(raw).hexdigest()==SOURCE_SHA256 and Path(filename).name=='run-scene-attachment.py',
            'Unknown attachment controller source')
    require(isinstance(phase,str)and phase.startswith(PREFIX)and phase.endswith('.json')
            and not Path(phase).is_absolute()and '..'not in Path(phase).parts,
            'Unbound canonical attachment phase')
    tree=ast.parse(raw,filename=filename)
    expected=ast.parse("require(args.plan.startswith(PREFIX) and '..' not in Path(args.plan).parts, 'Owned prospective plan required')").body[0].value
    found=[n for n in ast.walk(tree)if isinstance(n,ast.Call)and ast.dump(n)==ast.dump(expected)]
    require(len(found)==1,'Missing/duplicate exact attachment plan guard')
    call=found[0];old=call.args[0].values[0]
    call.args[0].values[0]=ast.copy_location(ast.Compare(left=ast.Attribute(value=ast.Name(id='args',ctx=ast.Load()),attr='plan',ctx=ast.Load()),ops=[ast.Eq()],comparators=[ast.Constant(phase)]),old)
    encoded=(ast.unparse(ast.fix_missing_locations(tree))+'\n').encode()
    # Reversing the single guard replacement must recover the entire original AST.
    modified=call.args[0].values[0];call.args[0].values[0]=old
    require(ast.dump(tree)==ast.dump(ast.parse(raw,filename=filename)),'Attachment adaptation changed more than its plan-path guard')
    call.args[0].values[0]=modified
    proof=dict(schema=1,kind='exact_attachment_canonical_plan_guard_adaptation',source_sha256=SOURCE_SHA256,
               adapted_source_sha256=hashlib.sha256(encoded).hexdigest(),phase=phase,line=call.lineno,
               changed_expression='args.plan.startswith(PREFIX)',replacement='args.plan == the exact hash-bound current phase',
               changed_guards=1,report_prefix_guard_unchanged=True,all_other_AST_nodes_unchanged=True,
               actions_deadlines_budgets_Native_code_unchanged=True)
    return compile(ast.fix_missing_locations(tree),filename,'exec'),encoded,proof
