"""Select reviewed continuation identities and exact cumulative counts in final readers."""
import ast,json,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from broader_current_interaction_final_runner import READERS
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
MATRIX='broader_player_secondary_final_matrix'
GATE='broader_secondary_final_gate'
# These literal substitutions select the new global/case catalog and account for
# exactly one additional historical failed request. All other AST nodes persist.
LITERALS={
 'plan-43.json':'plan-45.json',
 'discovery-source-catalog-01.json':'secondary-source-catalog-02.json',
 'discovery-continuation-review-01.json':'secondary-continuation-review-02.json',
 197:198,246:247,107:108,148:149,
 'Changed exact final cumulative reservations':'Changed exact final cumulative reservations',
 'Complete finite 197row/246charge cohort required':'Complete finite 198row/247charge cohort required',
}
INSERT="secondary = checked(r, global_plan['secondary_caller_review']); secondary_row = checked(r, secondary['direct_refusal_diagnosis'])['failed_request']; historical[secondary_row['case_id']] = secondary_row"


def selected_source(source,kind):
    require(kind in READERS,'Known final reader required')
    tree=ast.parse(source);matrix=gate=0;changes={};inserted=0
    class Select(ast.NodeTransformer):
        def visit_ImportFrom(self,node):
            nonlocal matrix,gate
            if node.module=='broader_player_discovery_final_matrix':node.module=MATRIX;matrix+=1
            elif node.module=='broader_discovery_final_gate':node.module=GATE;gate+=1
            return node
        def visit_Constant(self,node):
            value=node.value
            if type(value)in(str,int) and value in LITERALS and LITERALS[value]!=value:
                changes[value]=changes.get(value,0)+1
                return ast.copy_location(ast.Constant(LITERALS[value]),node)
            return node
        def visit_Assign(self,node):
            nonlocal inserted
            self.generic_visit(node)
            if kind=='acceptance' and len(node.targets)==1 and isinstance(node.targets[0],ast.Name) and node.targets[0].id=='wanted':
                inserted+=1;return ast.parse(INSERT).body+[node]
            return node
    tree=Select().visit(tree)
    require(matrix==1 and gate==(3 if kind=='failure' else 1),'Changed final reader imports')
    expected={'acceptance':{'plan-43.json':1,'discovery-source-catalog-01.json':1,'discovery-continuation-review-01.json':1,197:1,246:1,107:1,148:1},
              'failure':{'plan-43.json':1,197:1,246:1,'Complete finite 197row/246charge cohort required':1},'boundary':{}}[kind]
    require(changes==expected and inserted==(kind=='acceptance'),'Unexpected final predicate literal/insertion domain')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'


def run(kind,caller):
    require(kind in READERS and sys.argv.count('--report')==1,'Known final reader/report required')
    target=sys.argv[sys.argv.index('--report')+1];path=ROOT/'tools'/READERS[kind]
    with Reader(ROOT,memoize=True)as reader:
        names=['tools/broader_secondary_final_runner.py','tools/broader_secondary_final_gate.py',
               'tools/broader_player_secondary_final_matrix.py',caller]
        sources={n:reader.digest(n)for n in names};original=dict(record=path.relative_to(ROOT).as_posix(),sha256=reader.digest(path))
        review=dict(record='research/runtime/broader-player-acceptance-results-20261110/secondary-continuation-review-02.json',sha256=reader.digest('research/runtime/broader-player-acceptance-results-20261110/secondary-continuation-review-02.json'))
    write=coverage_evidence.write_report
    def writer(root,name,body):
        if str(name)==target:
            result=json.loads(body);result['sources']=result.get('sources',{})|sources
            result['secondary_final_binding']=dict(original_reader=original,review=review,
                changed_predicates='Explicit selected imports, plan/catalog identities,198/247 ledger and108/149 historical snapshot; one exact actual failed row added to historical set. Other AST nodes unchanged.',
                actual_ledger_rows_renamed=False,source_waivers=[],original_game_runs=0)
            with Reader(ROOT,memoize=True)as reader:stable=all(reader.digest(n)==h for n,h in result['sources'].items())
            if 'sources_stable'in result:result['sources_stable'] &= stable
            result['success'] &= stable;body=(json.dumps(result,indent=2)+'\n').encode()
        return write(root,name,body)
    coverage_evidence.write_report=writer
    try:
        namespace=dict(__name__='secondary_final_reader',__file__=str(path))
        exec(compile(selected_source(path.read_text(),kind),str(path),'exec'),namespace)
        return namespace['main']()
    finally:coverage_evidence.write_report=write
