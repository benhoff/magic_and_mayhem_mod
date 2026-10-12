#!/usr/bin/env python3
"""Prepare native music WAV assets without changing the supplied disc image."""
import argparse
import json
from pathlib import Path
import subprocess
from cd_audio import extract,sha

ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cue',type=Path,default=ROOT/'original/MagicMayhem_ISO/MagicMayhem.cue')
    parser.add_argument('--output',type=Path,required=True,help='New directory under working/')
    parser.add_argument('--claims',type=Path,help='Prospective coverage declarations prepared before execution')
    args=parser.parse_args();output=args.output.resolve()
    if not output.is_relative_to(ROOT/'working'):parser.error('Output must be under working/')
    declaration=json.loads(args.claims.read_text()) if args.claims else None
    if declaration and not any(c['behavior']=='HOST.native-cd-assets' for c in declaration['claims']):
        parser.error('Missing native CD asset behavior declaration')
    sources={str(path.relative_to(ROOT)):sha(path) for path in (ROOT/'tools/cd_audio.py',ROOT/'tools/extract-cd-audio.py',ROOT/'tests/test-cd-audio.py')}
    if declaration:
        sources.update(declaration['sources'])
        if any(sha(ROOT/path)!=fingerprint for path,fingerprint in sources.items()):raise ValueError('Prospective CD source fingerprints changed')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        report=extract(args.cue,output)
        report['original_manifest_verified_before']=True
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    report['original_manifest_verified_after']=True
    report['sources']=sources
    report['sources_unchanged']=all(sha(ROOT/path)==fingerprint for path,fingerprint in sources.items())
    if declaration:report['claims']=declaration['claims']
    if not report['sources_unchanged']:raise ValueError('CD extraction sources changed during execution')
    with (output/'manifest.json').open('x') as stream:json.dump(report,stream,indent=2);stream.write('\n')
    print(output/'manifest.json')
if __name__=='__main__':main()
