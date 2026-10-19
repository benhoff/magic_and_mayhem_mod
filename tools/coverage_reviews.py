"""Lossless, hash-pinned shards for the append-only coverage review journal."""
import argparse
import hashlib
import json
from pathlib import Path

REVIEWS = 'research/runtime/coverage/change-reviews.json'
ARCHIVES = 'research/runtime/coverage/review-archives/'


def read_journal(read_bytes, path=REVIEWS):
    """Read current or historical Git bytes without executing historical code."""
    journal = json.loads(read_bytes(path))
    reviews = []
    paths = {path}
    for entry in journal.get('archives', []):
        name, expected = entry['path'], entry['sha256']
        if (name != ARCHIVES + expected + '.json' or len(expected) != 64
                or any(c not in '0123456789abcdef' for c in expected)):
            raise ValueError('Invalid review archive path: ' + name)
        if name in paths:
            raise ValueError('Duplicate review archive: ' + name)
        data = read_bytes(name)
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError('Review archive hash mismatch: ' + name)
        archive = json.loads(data)
        if archive.get('schema') != 1 or set(archive) != {'schema', 'reviews'}:
            raise ValueError('Invalid review archive: ' + name)
        reviews.extend(archive['reviews'])
        paths.add(name)
    reviews.extend(journal['reviews'])
    return dict(journal, reviews=reviews), paths


def load_journal(root, path=REVIEWS):
    root = Path(root).resolve()
    def read(name):
        target = (root/name).resolve()
        if Path(name).is_absolute() or not target.is_relative_to(root):
            raise ValueError('Escaping review path: ' + name)
        return target.read_bytes()
    return read_journal(read, path)


def split_journal(root, max_bytes=32 * 1024 * 1024):
    """Archive oldest receipts unchanged; leave the last shard appendable."""
    root = Path(root)
    path = root/REVIEWS
    journal, _ = load_journal(root)
    chunks, chunk, size = [], [], 32
    for review in journal['reviews']:
        length = len(json.dumps(review, ensure_ascii=True, separators=(',', ':')).encode()) + 1
        if length + 32 > max_bytes:
            raise ValueError('One review exceeds the shard size')
        if chunk and size + length > max_bytes:
            chunks.append(chunk)
            chunk, size = [], 32
        chunk.append(review)
        size += length
    archives = []
    for reviews in chunks:
        data = (json.dumps({'schema': 1, 'reviews': reviews}, separators=(',', ':')) + '\n').encode()
        sha = hashlib.sha256(data).hexdigest()
        name = ARCHIVES + sha + '.json'
        target = root/name
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists():
            if target.read_bytes() != data:
                raise ValueError('Refusing to overwrite review archive: ' + name)
        else:
            with target.open('xb') as out:
                out.write(data)
        archives.append({'path': name, 'sha256': sha})
    value = dict(journal, archives=archives, reviews=chunk)
    data = (json.dumps(value, separators=(',', ':')) + '\n').encode()
    if len(data) > max_bytes:
        raise ValueError('Active journal exceeds shard size including archive index')
    temporary = path.with_suffix('.json.tmp')
    with temporary.open('xb') as out:
        out.write(data)
    temporary.replace(path)
    assert load_journal(root)[0]['reviews'] == journal['reviews']


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    split_journal(args.root)
