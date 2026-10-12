#!/usr/bin/env python3
"""Revalidate immutable original mutex/API journals and bind their exact prefixes."""
import argparse
import json
from pathlib import Path
from drawing_api_session import sha
from drawing_cursor_mutex_binding import analyze

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-reference', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
        raise ValueError('Prospective analysis sources changed')
    reference_raw = args.original_reference.read_bytes()
    reference = json.loads(reference_raw)
    if reference.get('success') is not True or not reference.get('normal_original_quit_verified'):
        raise ValueError('Reference lacks admitted normal original Quit')
    capture = Path(reference['experiment']) / 'capture'
    mutex_raw = (capture / 'cursor-mutex.bin').read_bytes()
    api_raw = (capture / 'session.bin').read_bytes()
    if sha(mutex_raw) != reference['cursor_mutex_sha256'] or sha(api_raw) != reference['observation']['journal_sha256']:
        raise ValueError('Immutable mutex/API journal hash differs')
    if (capture / 'session.done').read_bytes() != api_raw[-128:]:
        raise ValueError('Immutable API terminal differs')
    result = analyze(mutex_raw, api_raw)
    if args.original_reference.read_bytes() != reference_raw or (capture / 'cursor-mutex.bin').read_bytes() != mutex_raw or (capture / 'session.bin').read_bytes() != api_raw or any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
        raise ValueError('Sources/reference changed during analysis')
    report = dict(schema=1, success=True, sources=sources, claims=declaration['claims'], sources_stable=True, reference=dict(path=str(args.original_reference), sha256=sha(reference_raw)), mutex_sha256=sha(mutex_raw), api_sha256=sha(api_raw), analysis=result, original_game_executed=False, scope='Current immutable offline analysis of retained original journals; no new game execution, atomic event/frame binding, ownership or safe source synchronization/native parity claim.')
    with args.report.open('x') as out:
        json.dump(report, out, indent=2)
        out.write('\n')
    print(json.dumps(dict(success=True, calls=result['calls'], points=result['sampled_points'], inactive=result['inactive_points'], refused=result['query_refusals'])))


if __name__ == '__main__':
    main()
