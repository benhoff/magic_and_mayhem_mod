"""Select the retained strict pilot receipt with every prior predicate preserved."""
import inspect
import broader_strict_binding_final_gate as previous
from rendering_pilot import require
from broader_strict_binding_final_gate import P,FAILED,PILOT,same_context,guarded_contexts,caller_binding,prior_context,project_rows,_FADING,_CALLER,_PRIOR,_WITNESS,_REBUILD,_UNCOVERED,_PAN,_PROJECT,_COLLECTION
_original_source=inspect.getsource(previous.supplement)
_source=_original_source
RECEIPTS={
 'strict-binding42-single-pilot-01.json':'strict-binding42-single-pilot-02.json',
 'strict-binding-final-reader-controls-01.json':'strict-provenance-final-reader-controls-01.json',
}
for old,new in RECEIPTS.items():
 require(_source.count(old)==4,'Unexpected strict receipt source');_source=_source.replace(old,new)
_restored=_source
for old,new in reversed(list(RECEIPTS.items())):_restored=_restored.replace(new,old)
require(_restored==_original_source,'Non-receipt strict final predicate changed')
_namespace=dict(previous.supplement.__globals__)
exec(compile(_source,__file__,'exec'),_namespace)
supplement=_namespace['supplement']
