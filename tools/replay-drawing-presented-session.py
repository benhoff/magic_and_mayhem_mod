#!/usr/bin/env python3
"""DRAW-02 owned presented-session backend; unsupported contracts exit64."""
import argparse
import json
import os
from pathlib import Path
from drawing_presented_session import replay, Unsupported

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('owned',type=Path);p.add_argument('output',type=Path)
    p.add_argument('--composition',type=Path,default=os.environ.get('MNM_DRAW_COMPOSITION'))
    p.add_argument('--presenter',type=Path,default=os.environ.get('MNM_DRAW_PRESENTER'))
    p.add_argument('--display-conversion',choices=['bit-replicated','normalized-nearest'],default='bit-replicated')
    a=p.parse_args()
    try:
        if not a.composition or not a.presenter:raise Unsupported('Configure native composition and physical presenter binaries')
        result=replay(a.owned,a.output,a.composition,a.presenter,a.display_conversion)
        print(json.dumps(dict(success=True,frames=len(result['frames']),physical_presentation=True)))
        return 0
    except Unsupported as e:
        print(json.dumps(dict(success=False,unsupported=str(e))))
        return 64
    except (ValueError,KeyError,TypeError,OSError) as e:
        print(json.dumps(dict(success=False,error=str(e))))
        return 1

if __name__=='__main__':raise SystemExit(main())
