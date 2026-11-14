#!/usr/bin/env python3
"""Run the unchanged recovery comparison with only derived output moved to RAM."""
import argparse
import ast
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import types

import coverage_evidence
from coverage_evidence import Reader
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
SOURCE='tools/check-rendering-recovery-closeout.py'


def memory_output_code(raw,filename):
    """Change exactly the output directory initializer; preserve every assertion."""
    tree=ast.parse(raw,filename=filename);matches=[]
    expected=ast.parse("ROOT / ('working/surface-loss/comparisons' if surface_loss else 'working/recovery-closeout08/comparisons')",mode='eval').body
    for node in ast.walk(tree):
        if isinstance(node,ast.Assign)and len(node.targets)==1 and isinstance(node.targets[0],ast.Name)and node.targets[0].id=='output_root':
            require(ast.dump(node.value)==ast.dump(expected),'Recovery output initializer changed outside the reviewed binding')
            matches.append(node);node.value=ast.copy_location(ast.Name(id='BROADER_RAM_COMPARISON_ROOT',ctx=ast.Load()),node.value)
    require(len(matches)==1,'Missing/duplicate recovery output initializer')
    return compile(ast.fix_missing_locations(tree),filename,'exec'),hashlib.sha256(ast.dump(tree,include_attributes=False).encode()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kind',choices=('loss','transition'),required=True)
    parser.add_argument('--acquisition',required=True);parser.add_argument('--report',required=True);args=parser.parse_args()
    parent=Path('/tmp/mnm-broader-recovery-comparisons-20261110')
    require(subprocess.check_output(['findmnt','-T','/tmp','-n','-o','FSTYPE'],text=True).strip()=='tmpfs'
            and shutil.disk_usage('/tmp').free>=8*1024**3,'Insufficient RAM comparison scratch')
    parent.mkdir(mode=0o700,parents=True,exist_ok=True)
    require(not(parent/Path(args.report).stem).exists(),'Preserve prior RAM comparison')
    with Reader(ROOT)as r:
        raw=r.read_bytes(SOURCE);source_sha=r.digest(SOURCE)
        sources={n:r.digest(n)for n in('tools/check-broader-recovery-ram.py','tests/test-broader-recovery-ram.py')}
    code,ast_sha=memory_output_code(raw,str(ROOT/SOURCE))
    original_write=coverage_evidence.write_report
    def writer(root,path,body):
        if str(path)==args.report:
            q=json.loads(body);stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
            q['sources'].update(sources);q['sources_stable']=q['sources_stable']and stable;q['success']=q['success']and stable
            q['comparison_storage']=dict(path=str(parent/Path(args.report).stem),temporary=True,
                policy='task-owned-tmpfs-derived-comparison-v1',original_producer=dict(path=SOURCE,sha256=source_sha),
                output_bound_module_ast_sha256=ast_sha,only_output_directory_initializer_changed=True,
                input_source_assets_oracle_binary_assertions_and_deadlines_unchanged=True,
                original_game_runs_added=0,durable_artifacts='Every logical output member is verified by the unchanged producer and retained in its SHA-pinned SQLite archive before admission.',physical_RAM_files_removed=False)
            body=(json.dumps(q,indent=2)+'\n').encode()
        return original_write(root,path,body)
    coverage_evidence.write_report=writer
    module=types.ModuleType('broader_ram_recovery_comparison');module.__file__=str(ROOT/SOURCE)
    module.BROADER_RAM_COMPARISON_ROOT=parent
    try:
        exec(code,module.__dict__)
        sys.argv=[str(ROOT/SOURCE),'--acquisition',args.acquisition,'--report',args.report]
        return module.main(surface_loss=args.kind=='loss')
    finally:coverage_evidence.write_report=original_write


if __name__=='__main__':raise SystemExit(main())
