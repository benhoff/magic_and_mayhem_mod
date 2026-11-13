"""Resolve the retained staged JSON manifest separately from the input TSV."""
import ast,hashlib,inspect,json
from pathlib import Path
from broader_secondary_staged_scene import verify_staged as original,checked_artifact
from rendering_pilot import require
OLD="manifest_ref = acquired['launch_manifest']"
def retained_manifest(reader,acquired):
    input_ref=acquired['launch_manifest'];raw=reader.read_bytes(input_ref['record'])
    require(hashlib.sha256(raw).hexdigest()==input_ref['sha256'],'Original launch input manifest changed')
    require(raw.startswith(b'sha256\tsize\tpath\n'),'Expected original installation TSV manifest')
    raw,reference=checked_artifact(reader,acquired,Path(acquired['session'])/'manifest.json')
    stage=json.loads(raw);require(isinstance(stage,dict)and stage['native_render']and stage['resident_world'],'Missing retained native staged manifest')
    return reference

def selected_source(source):
    tree=ast.parse(source);wanted=ast.parse(OLD).body[0];count=0
    class Select(ast.NodeTransformer):
        def visit_Assign(self,node):
            nonlocal count
            if ast.dump(node,include_attributes=False)==ast.dump(wanted,include_attributes=False):
                count+=1;return ast.parse('manifest_ref = retained_manifest(reader, acquired)').body[0]
            return self.generic_visit(node)
    tree=Select().visit(tree);require(count==1,'Original staged manifest assignment changed')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'
namespace=dict(original.__globals__,retained_manifest=retained_manifest)
exec(compile(selected_source(inspect.getsource(original)),__file__,'exec'),namespace)
verify_staged=namespace['verify_staged']
