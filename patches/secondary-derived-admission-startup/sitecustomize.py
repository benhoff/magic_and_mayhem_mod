"""Compose the verified preparer startup with explicit pre-Wine admission."""
import json,os,sys
from pathlib import Path
raw=os.environ.get('MNM_SECONDARY_SCENE_PREPARER')
if raw:
    try:
        config=json.loads(raw);root=Path(config['root'])
        # Execute the existing, separately pinned startup verbatim. It selects
        # only prepare-menu-observer.py and is inert for the owned runner.
        prior=root/'patches/secondary-scene-python-startup/sitecustomize.py'
        exec(compile(prior.read_bytes(),str(prior),'exec'),dict(__name__='explicit_prior_startup',__file__=str(prior)))
        if Path(sys.argv[0]).resolve()==root/'tools/owned-menu-game-runner.py':
            from broader_derived_admission import startup
            startup(root,config)
    except BaseException as error:
        sys.stderr.write('Explicit derived admission startup refused: '+repr(error)+'\n')
        sys.stderr.flush();os._exit(78)
