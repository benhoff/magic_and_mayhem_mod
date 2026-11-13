"""Add only the unchanged collector's existing explicit repair ancestry flags."""
from pathlib import Path
from rendering_pilot import require

def arguments(root,argv,failed,reason):
 require(argv and Path(argv[0])==Path(root)/'tools/run-rendering-transition.py','Only the existing wide transition collector supports this repair binding')
 require('--repair-of'not in argv and '--repair-reason'not in argv and '--low'not in argv and '--software'not in argv,'Repair binding cannot duplicate ancestry or change the800NVIDIA context')
 require(isinstance(failed,str)and failed.endswith('-acquisition.json')and isinstance(reason,str)and reason,'Explicit retained failure and concrete reason required')
 return [*argv,'--repair-of',failed,'--repair-reason',reason]
