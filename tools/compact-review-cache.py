#!/usr/bin/env python3
"""Replace one inactive legacy review cache after dual exact archival and restore."""
from pathlib import Path
import argparse,json
from review_cache_storage import compact

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ("root","local","cold","scratch"):p.add_argument("--"+key,type=Path,required=True)
    p.add_argument("--accept-current-journal-order",action="store_true")
    a=p.parse_args()
    if a.local.parent.stat().st_dev==a.cold.parent.stat().st_dev:p.error("Independent archive filesystems required")
    print(json.dumps(compact(a.root,a.local,a.cold,a.scratch,accept_current_order=a.accept_current_journal_order),sort_keys=True))
if __name__=="__main__":main()
