"""One exact setup guard adaptation for four frozen physical controller families."""
import ast,hashlib
from pathlib import Path
from broader_attachment_plan_path import adapt as attachment_adapt,PREFIX
from rendering_pilot import require
SOURCES={
 'run-font-dc-lifetime.py':'5de5f6db2f4c827782d2129f617d78519f9a375a8c37713ae46afa8ff9184807',
 'run-palette-cycle.py':'145f14d03cad09b41211e61b995b42f9351cd3d5011bc6814a15e0eb4c0541ac',
 'run-rendering-picking-closeout.py':'172f5c7e33615749c8ee76db4f93685a95d0e881e51bdab6b94091bb756ea5dc'}
ENTRYPOINTS={'attachment':('run-scene-attachment.py','run-broader-history-world.py'),
 'font':('run-font-dc-lifetime.py','run-broader-font-visible-world.py'),
 'palette':('run-palette-cycle.py','run-broader-visible-world.py'),
 'picking':('run-rendering-picking-closeout.py','run-broader-visible-world.py')}

def adapt(raw,filename,phase):
    name=Path(filename).name
    if name=='run-scene-attachment.py':return attachment_adapt(raw,filename,phase)
    require(name in SOURCES and hashlib.sha256(raw).hexdigest()==SOURCES[name],'Unknown frozen canonical-plan controller source')
    require(isinstance(phase,str)and phase.startswith(PREFIX)and phase.endswith('.json')and '..'not in Path(phase).parts,'Unbound canonical controller phase')
    text=("base.require(args.plan.startswith(PREFIX), 'Owned prospective picking plan required')"if name=='run-rendering-picking-closeout.py'else"require(args.plan.startswith(PREFIX) and '..' not in Path(args.plan).parts, 'Owned prospective plan required')")
    tree=ast.parse(raw,filename=filename);expected=ast.parse(text).body[0].value
    found=[n for n in ast.walk(tree)if isinstance(n,ast.Call)and ast.dump(n)==ast.dump(expected)];require(len(found)==1,'Missing/duplicate exact canonical-plan guard')
    call=found[0];container=call.args[0].values if isinstance(call.args[0],ast.BoolOp)else call.args;old=container[0]
    changed=ast.copy_location(ast.Compare(left=ast.Attribute(value=ast.Name(id='args',ctx=ast.Load()),attr='plan',ctx=ast.Load()),ops=[ast.Eq()],comparators=[ast.Constant(phase)]),old);container[0]=changed
    body=(ast.unparse(ast.fix_missing_locations(tree))+'\n').encode();container[0]=old;require(ast.dump(tree)==ast.dump(ast.parse(raw,filename=filename)),'Non-guard canonical-plan adaptation');container[0]=changed
    return compile(ast.fix_missing_locations(tree),filename,'exec'),body,dict(schema=1,kind='exact_canonical_plan_guard_adaptation',source_sha256=SOURCES[name],adapted_source_sha256=hashlib.sha256(body).hexdigest(),phase=phase,line=call.lineno,changed_guards=1,all_other_AST_nodes_unchanged=True,report_prefix_guard_unchanged=True,actions_deadlines_budgets_Native_code_unchanged=True)

def binding_class(base_class,controller):
    require(controller in {'run-scene-attachment.py',*SOURCES},'Unknown guarded controller')
    class ReviewedBinding(base_class):
        def __init__(self,*args,**kwargs):super().__init__(*args,**kwargs);self.plan_guard=None;self.adapted_modules=set()
        def patch(self,module):
            if Path(getattr(module,'__file__','/')).name==controller and id(module)not in self.adapted_modules:
                from coverage_evidence import write_report
                self.adapted_modules.add(id(module));source=self.root/'tools'/controller;code,raw,self.plan_guard=adapt(source.read_bytes(),str(source),self.phase_name)
                n=PREFIX+Path(self.report).stem+'-original-controller.bin';self.plan_guard['original_source']=dict(record=n,sha256=write_report(self.root,n,source.read_bytes()))
                n=PREFIX+Path(self.report).stem+'-adapted-controller.bin';self.plan_guard['adapted_source']=dict(record=n,sha256=write_report(self.root,n,raw));exec(code,module.__dict__)
            return super().patch(module)
        def receipt(self):
            q=super().receipt();require(self.plan_guard is not None,'Missing canonical-plan guard adaptation');q['canonical_plan_guard_adaptation']=self.plan_guard;return q
    return ReviewedBinding
