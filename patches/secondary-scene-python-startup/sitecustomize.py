"""Explicit, task-owned startup for Qt's unchanged Python preparer only."""
import json,os,sys
from pathlib import Path
raw=os.environ.get('MNM_SECONDARY_SCENE_PREPARER')
if raw:
    try:
        config=json.loads(raw)
        expected=Path(config['root'])/'tools/prepare-menu-observer.py'
        if Path(sys.argv[0]).resolve()==expected:
            from broader_secondary_subprocess import startup
            startup(config['root'],config)
    except BaseException as error:
        # Python normally ignores sitecustomize failures. Refuse before staging
        # instead of silently falling back to the base scene.
        sys.stderr.write('Explicit secondary preparer startup refused: '+repr(error)+'\n')
        sys.stderr.flush();os._exit(78)
