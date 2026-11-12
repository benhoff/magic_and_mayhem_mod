#!/usr/bin/env python3
"""Archive exact SQLite backups in two deduplicated stores, or restore one."""
import argparse,json
from pathlib import Path
from sqlite_snapshot_store import Store,archive

def main():
    p=argparse.ArgumentParser(description=__doc__)
    sub=p.add_subparsers(dest="command",required=True)
    a=sub.add_parser("archive")
    for key in ("source","local","cold","scratch"):a.add_argument("--"+key,type=Path,required=True)
    a.add_argument("--remove",action="store_true")
    a.add_argument("--expected-sha256")
    r=sub.add_parser("restore")
    r.add_argument("--catalog",type=Path,required=True)
    r.add_argument("--sha256",required=True)
    r.add_argument("--output",type=Path,required=True)
    args=p.parse_args()
    if args.command=="restore":
        store=Store(args.catalog,readonly=True)
        try:
            result=store.restore(args.sha256,args.output)
            print(json.dumps({"success":True,"path":str(result),"sha256":args.sha256}))
        finally:store.close()
    else:
        if args.local.resolve()==args.cold.resolve() or args.local.parent.stat().st_dev==args.cold.parent.stat().st_dev:
            p.error("Archive requires independent local and cold filesystems")
        print(json.dumps(archive(args.source,args.local,args.cold,args.scratch,90*1024**3,100*1024**3,args.remove,args.expected_sha256),sort_keys=True))
if __name__=="__main__":main()
