"""Select derived-admission final readers with exact extra historical accounting."""
import ast,json,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from broader_secondary_final_runner import READERS,selected_source as previous_source
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
MATRIX='broader_player_derived_final_matrix'
GATE='broader_derived_final_gate'
LITERALS={'plan-45.json':'plan-48.json','secondary-source-catalog-02.json':'derived-source-catalog-03.json',
          'secondary-continuation-review-02.json':'derived-continuation-review-03.json',198:201,247:250,108:111,149:152,
          'Complete finite 198row/247charge cohort required':'Complete finite 201row/250charge cohort required'}
INSERT="derived = checked(r, global_plan['derived_admission_review']); derived_row = checked(r, derived['startup_diagnosis'])['failed_request']; historical[derived_row['case_id']] = derived_row; hover_row = checked(r, derived['hover_diagnosis'])['failed_request']; historical[hover_row['case_id']] = hover_row; death_row = checked(r, derived['death_diagnosis'])['failed_request']; historical[death_row['case_id']] = death_row"


def selected_source(source,kind):
    tree=ast.parse(previous_source(source,kind));counts={};matrix=gate=inserted=0
    class Select(ast.NodeTransformer):
        def visit_ImportFrom(self,node):
            nonlocal matrix,gate
            if node.module=='broader_player_secondary_final_matrix':node.module=MATRIX;matrix+=1
            elif node.module=='broader_secondary_final_gate':node.module=GATE;gate+=1
            return node
        def visit_Constant(self,node):
            if type(node.value)in(str,int)and node.value in LITERALS:
                counts[node.value]=counts.get(node.value,0)+1;return ast.copy_location(ast.Constant(LITERALS[node.value]),node)
            return node
        def visit_Assign(self,node):
            nonlocal inserted
            self.generic_visit(node)
            if kind=='acceptance'and len(node.targets)==1 and isinstance(node.targets[0],ast.Name)and node.targets[0].id=='wanted':
                inserted+=1;return ast.parse(INSERT).body+[node]
            return node
    tree=Select().visit(tree);expected={'acceptance':{'plan-45.json':1,'secondary-source-catalog-02.json':1,'secondary-continuation-review-02.json':1,198:1,247:1,108:1,149:1},'failure':{'plan-45.json':1,198:1,247:1,'Complete finite 198row/247charge cohort required':1},'boundary':{}}[kind]
    require(matrix==1 and gate==(3 if kind=='failure'else 1)and counts==expected and inserted==(kind=='acceptance'),
            'Changed derived final-reader import/literal/history-insertion domain')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'


def run(kind,caller):
    require(kind in READERS and sys.argv.count('--report')==1,'Known final-reader/report required')
    report=sys.argv[sys.argv.index('--report')+1];path=ROOT/'tools'/READERS[kind]
    with Reader(ROOT,memoize=True)as reader:
        names=['tools/broader_derived_final_runner.py','tools/broader_derived_final_gate.py','tools/broader_player_derived_final_matrix.py',caller]
        sources={p:reader.digest(p)for p in names};original=dict(record=path.relative_to(ROOT).as_posix(),sha256=reader.digest(path))
        review_name='research/runtime/broader-player-acceptance-results-20261110/derived-continuation-review-03.json';review=dict(record=review_name,sha256=reader.digest(review_name))
    write=coverage_evidence.write_report
    def writer(root,target,body):
        if str(target)==report:
            result=json.loads(body)
            with Reader(ROOT,memoize=True)as reader:
                actual_review=reader.json(review['record']);known=actual_review['terminal_relevance'];boundary=reader.json(known['record'])
                require(reader.digest(known['record'])==known['sha256']and boundary['new_required_row_excluded']is False,'Known terminal boundary review changed')
            result['known_unsupported_native_controls']=[dict(review=known,scope=boundary['explicit_known_unsupported_boundary'],required_row_excluded=False)]
            result['sources']=result.get('sources',{})|sources
            result['derived_final_binding']=dict(original_reader=original,review=review,actual_ledger_rows_renamed=False,
                explicit_changes='Prior selected imports plus new matrix/gate,plan48/catalog/selection review,201/250 cumulative and111/152historical counts,and all three exact additional failed rows. Other AST nodes unchanged.',
                source_waivers=[],original_game_runs=0)
            with Reader(ROOT,memoize=True)as reader:stable=all(reader.digest(p)==h for p,h in result['sources'].items())
            if 'sources_stable'in result:result['sources_stable'] &= stable
            result['success'] &= stable;body=(json.dumps(result,indent=2)+'\n').encode()
        return write(root,target,body)
    coverage_evidence.write_report=writer
    try:
        namespace=dict(__name__='derived_final_reader',__file__=str(path));exec(compile(selected_source(path.read_text(),kind),str(path),'exec'),namespace);return namespace['main']()
    finally:coverage_evidence.write_report=write
