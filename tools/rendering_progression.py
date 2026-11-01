"""Bounded EP05 preparation and semantic assertions, separate from pixel admission."""
import configparser
import hashlib
import re
from pathlib import Path

BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PREFIX = 'research/runtime/rendering-progression-results-20261101/'
CASES = ('progression', 'victory', 'defeat')


def require(value, message):
    if not value:
        raise ValueError(message)


def sha(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def recipes(game):
    """Pin every possible section, preserving ordered Specific/Random recipe text.

    This is a candidate-input closure. Only execution can identify rotations,
    random section choices, the assembled grid, or an actual progression event.
    """
    game = Path(game)
    require(sha(game/'Chaos.exe') == BUILD, 'Unsupported original executable')
    cfg = configparser.ConfigParser(interpolation=None, strict=False)
    cfg.read_string((game/'Realms/Celtic/Celtic.cfg').read_text(encoding='cp1252'))
    result = []
    for region in (1, 2):
        row = cfg['REGION'+str(region)]
        directory = game/Path(row['Path'].strip().replace('\\', '/'))
        prefix = row['SectionPreFix'].strip()
        specific = [list(map(int, m)) for m in re.findall(r'\((-?\d+),(-?\d+),(-?\d+),(-?\d+)\)', row['Specific'])]
        random = [list(map(int, m)) for m in re.findall(r'(\d+)_(\d+)', row['Random'])]
        require(specific and random, 'Unrecognized campaign placement recipe')
        sections = sorted({v[0] for v in specific} | {v[0] for v in random})
        inputs = []
        for section in sections:
            for suffix in ('.map', '.nod', '.evt', '.mps'):
                path = directory/(prefix+f'{section:02}'+suffix)
                require(path.is_file(), 'Missing candidate map input: '+str(path))
                inputs.append(dict(path=path.relative_to(game).as_posix(), sha256=sha(path), bytes=path.stat().st_size))
        result.append(dict(realm='Celtic', region=region, name=row['Name'].strip(),
                           path=row['Path'].strip(), size=row['MapSize'].strip(),
                           specific=specific, random=random, inputs=inputs,
                           assembled_map_identity='pending execution'))
    return result


def validate_flow(flow, case):
    require(case in CASES and flow.get('case') == case, 'Wrong progression case')
    require(flow.get('success') is True and flow.get('normal_quit') is True, 'Driver did not finish through original Quit')
    require(flow.get('native_command_fallback') is False and not flow.get('native_error'), 'Renderer fallback/error')
    require(flow.get('size') in ('640x480','800x600'), 'Actual frame mode must be declared')
    dimensions = list(map(int, flow['size'].split('x')))
    rows = flow['states']
    require(rows and len({r['thread'] for r in rows}) == 1 and rows[0]['thread'] > 0, 'Changed engine owner thread')
    outcomes = [r for r in rows if r['event'] == 'natural-report']
    require(len(outcomes) == 1, 'Exactly one primary natural report required')
    report = outcomes[0]
    require(report['screen'] == 6 and report['outcome'] == (0 if case == 'defeat' else 1), 'Wrong battle outcome')
    require(report['title'] and len(report['texts']) == 21 and not any('quit the battle' in t.lower() for t in report['texts']), 'Quit cannot supply a natural result')
    returned = [r for r in rows if r['event'] == 'result-return']
    require(len(returned) == 1 and returned[0]['screen'] in (3, 4) and returned[0]['sequence'] > report['sequence'] and returned[0]['ack'] == report['ack']+1, 'Original result OK/return acknowledgement missing')
    worlds = flow['worlds']
    require(worlds and worlds[0]['region'] == 1 and worlds[0]['realm'] == 'Celtic', 'Wrong initial campaign map')
    require(all([w['width'],w['height']] == dimensions for w in worlds), 'Actual World extent differs from declared frame mode')
    require(all(w['paint'] > 0 and w['world'] > 0 and w['frames'] > 0 for w in worlds), 'World was never presented')
    if case in ('progression', 'victory'):
        require(returned[0]['screen'] == 4 and len(worlds) == 2, 'Victory must return to Realm and enter the later map')
        require(worlds[1]['region'] == 2 and worlds[1]['realm'] == 'Celtic' and worlds[1]['world'] > worlds[0]['world'], 'Reload/same-map re-entry is not progression')
        entries = [r for r in rows if r['event'] == 'region-entry']
        require([r['region'] for r in entries] == [1, 2], 'Missing original later-region admission')
    require(not any(r['event'] == 'load' for r in rows), 'Save reload is a different scenario')
    return dict(semantic_route_verified=True, case=case, natural_outcome=report['outcome'],
                later_map_entered=case != 'defeat', pixel_history_accepted=False,
                ownership_retirement_accepted=False, supported_player_accepted=False)


def validate_callbacks(data, flow):
    """Require original callbacks and exit provenance, never infer from UI clicks."""
    import struct
    require(data[:16] == b'MNMMENU1'+struct.pack('<II', 1, 64) and (len(data)-16) % 64 == 0, 'Invalid complete menu trace')
    rows = list(struct.iter_unpack('<16I', data[16:]))
    require(rows and len(rows) < 65536 and [r[0] for r in rows] == list(range(1, len(rows)+1)), 'Lost/exhausted menu history')
    natural = next(r for r in flow['states'] if r['event'] == 'natural-report')
    # Header state.sequence is a seqlock, not this event-file sequence.
    # Establish the original OK and no earlier Quit using callback order.
    callbacks = [r for r in rows if r[1] == 3]
    oks = [r for r in callbacks if r[3] == 6 and r[9] == 21]
    require(oks and oks[0][2] == natural['thread'], 'Original report OK callback absent')
    require(not any(r[3] == 17 and r[9] == 3 and r[0] < oks[0][0] for r in callbacks), 'Quit precedes the claimed natural outcome')
    require(any(r[1] == 28 and r[11] == 0x6db967 and r[0] < oks[0][0] for r in rows), 'Original World-to-report observation absent')
    require(callbacks[-1][3] == 3 and callbacks[-1][9] == 4, 'Original Main Quit missing')
    return dict(original_result_ok_sequence=oks[0][0], original_main_quit_sequence=callbacks[-1][0], verified=True)


def prepare_launch_manifest(game, manifest):
    """Required physical TSV input for run-game.sh, with immutable existing pins."""
    game, manifest = Path(game), Path(manifest)
    require('original' not in manifest.parts and not manifest.is_symlink(), 'Redirected installation manifest')
    if not manifest.exists():
        lines=['sha256\tsize\tpath\n']
        for path in sorted(game.rglob('*')):
            if not path.is_file():
                continue
            require(not path.is_symlink(), 'Redirected installation file')
            name=path.relative_to(game).as_posix()
            require(not any(c in name for c in '\t\r\n'), 'Unsafe installation filename')
            lines.append(f'{sha(path)}\t{path.stat().st_size}\t{name}\n')
        manifest.parent.mkdir(parents=True,exist_ok=True)
        with manifest.open('xb') as out:
            out.write(''.join(lines).encode())
    raw=manifest.read_bytes()
    rows=[line.split('\t') for line in raw.decode().splitlines()]
    require(rows and rows[0]==['sha256','size','path'] and all(len(r)==3 for r in rows), 'Invalid launcher manifest format')
    entries={r[2]:(r[0],int(r[1])) for r in rows[1:]}
    require(len(entries)==len(rows)-1, 'Duplicate launcher manifest identity')
    for name in ('Chaos.exe','jpeg.dll','CDROMDrive.cfg'):
        path=game/name
        require(entries.get(name)==(sha(path),path.stat().st_size), 'Launcher input differs from pinned manifest: '+name)
    return raw
