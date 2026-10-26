#!/usr/bin/env python3
"""Restore exact retired tracked sources to a new directory; keep old outputs."""
import argparse,sqlite3
from contextlib import closing
from pathlib import Path
from source_archive import restore
p=argparse.ArgumentParser(description=__doc__)
p.add_argument("--catalog",type=Path,required=True);p.add_argument("--snapshot",required=True);p.add_argument("--output",type=Path,required=True)
a=p.parse_args()
with closing(sqlite3.connect(a.catalog.resolve().as_uri()+"?mode=ro",uri=True)) as db:
    print("Restored source files:",restore(db,a.snapshot,a.output))
