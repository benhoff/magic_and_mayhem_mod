#!/usr/bin/env python3
"""Native Qt lifetime assertions and exact fresh-process checkpoint continuation."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    if len(sys.argv) != 2:
        raise ValueError('Supply world-session-lifecycle-test')
    binary = Path(sys.argv[1]).resolve()
    parent = os.environ.get('MNM_SCENE_LIFECYCLE_EVIDENCE')
    temporary = None
    if parent:
        Path(parent).mkdir(parents=True, exist_ok=True)
        out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    else:
        temporary = tempfile.TemporaryDirectory(prefix='mnm-scene-lifecycle-')
        out = Path(temporary.name)
    try:
        def run(label, arguments):
            result = subprocess.run([str(binary), *map(str, arguments)], capture_output=True,
                                    text=True, timeout=25)
            (out / (label + '.stdout')).write_text(result.stdout)
            (out / (label + '.stderr')).write_text(result.stderr)
            if result.returncode or 'AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
                raise RuntimeError((label, result.returncode, result.stdout, result.stderr))
            return result.stdout
        report = json.loads(run('lifecycle', [out / 'native']))
        assert report['success'] and all(value is True for value in report.values())
        original = (out / 'native/pending.mnw').read_bytes()
        run('resume', ['resume', out / 'native/pending.mnw', out / 'resumed.mnw', 3])
        assert (out / 'native/pending.mnw').read_bytes() == original
        assert (out / 'resumed.mnw').read_bytes() == (out / 'native/whole.mnw').read_bytes()
        report.update(fresh_process_continuation=True,
                      checkpoint_sha256=hashlib.sha256((out / 'resumed.mnw').read_bytes()).hexdigest())
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, sort_keys=True))
    finally:
        if temporary:
            temporary.cleanup()


if __name__ == '__main__':
    main()
