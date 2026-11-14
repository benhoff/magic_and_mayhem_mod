"""Publish a complete admission receipt under an exclusive final journal name."""
import json
import os
from pathlib import Path
import tempfile


def publish(path, value):
    """Keep partial JSON invisible and retain exclusive-create semantics.

    A same-directory hard link publishes the fully flushed temporary inode
    without replacing an existing receipt. Readers enumerate only *.json.
    """
    path = Path(path)
    body = (json.dumps(value, indent=2) + '\n').encode('utf-8')
    fd, temporary = tempfile.mkstemp(prefix='.' + path.name + '.', suffix='.pending', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as output:
            output.write(body)
            output.flush()
            os.fsync(output.fileno())
        os.link(temporary, path)
        directory = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
        try:
            os.fsync(directory)
        finally:
            os.close(directory)
    finally:
        os.unlink(temporary)
