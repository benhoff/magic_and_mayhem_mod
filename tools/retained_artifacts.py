"""Verify retained bytes and select a recorded producer's session scope.

Readers supply SQLite-backed read_bytes; producer paths are lexical identities,
never paths to open in the current checkout. Excluded observations stay retained.
"""
import hashlib
from pathlib import Path


def _require(condition, message):
    if not condition:
        raise ValueError(message)


def read_artifact(reader, acquired, path, *, max_bytes=None, label='retained'):
    """Read one exact/suffix match, checking recorded length and SHA-256.

    An optional caller budget rejects the declared extent before reading. The
    returned bytes must still match that extent; a declaration is not proof.
    """
    candidates = [v for v in acquired['artifacts']
                  if v['path'] == path or v['path'].endswith('/' + path)]
    _require(len(candidates) == 1, f'Missing/ambiguous {label} artifact: ' + path)
    value = candidates[0]
    if max_bytes is not None:
        _require(type(max_bytes) is int and max_bytes >= 0, 'Invalid artifact byte budget')
        _require(type(value['bytes']) is int and 0 <= value['bytes'] <= max_bytes,
                 f'{label} artifact exceeds byte budget: ' + path)
    raw = reader.read_bytes(value['record'])
    _require(len(raw) == value['bytes'] and hashlib.sha256(raw).hexdigest() == value['sha256'],
             f'Changed {label} artifact: ' + path)
    return raw


def recorded_repository(acquired, *, label='retained'):
    """Recognize explicit --repo or a pinned absolute run-native.sh launcher."""
    command = acquired.get('command', [])
    _require(command and command.count('--repo') <= 1,
             f'Missing/ambiguous recorded {label} repository')
    if '--repo' in command:
        index = command.index('--repo') + 1
        _require(index < len(command), f'Missing recorded {label} repository value')
        return Path(command[index])
    launcher = Path(command[0])
    _require(launcher.is_absolute() and launcher.name == 'run-native.sh'
             and acquired.get('sources', {}).get('run-native.sh'),
             f'Missing recorded {label} repository or pinned launcher')
    return launcher.parent


def session_artifact_root(acquired, name, *, label='retained'):
    """Return a producer-relative root without resolving paths on this host."""
    root = recorded_repository(acquired, label=label)
    value = Path(acquired[name])
    _require(root.is_absolute() and value.is_absolute()
             and '..' not in root.parts and '..' not in value.parts,
             f'Invalid recorded {label} root')
    relative = value.relative_to(root)
    _require(relative.parts, f'{label.capitalize()} artifact root cannot be the whole repository')
    return relative.as_posix()


def scoped_artifacts(acquired, *, root_names=('temporary_root', 'session', 'owner'), label='retained'):
    """Project a case's exact named roots, returning excluded paths separately."""
    roots = [session_artifact_root(acquired, name, label=label) + '/'
             for name in root_names if acquired.get(name)]
    _require(roots, f'No exact {label} session/output roots')
    keep, excluded = [], []
    for value in acquired['artifacts']:
        (keep if any(value['path'].startswith(r) for r in roots) else excluded).append(value)
    _require(len({v['path'] for v in keep}) == len(keep), f'Duplicate exact {label} artifact path')
    return dict(acquired, artifacts=keep), [v['path'] for v in excluded]
