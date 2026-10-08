#!/usr/bin/env python3
"""Run continuous World ownership/session and native renderer regression checks."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET

ROOT=Path(__file__).resolve().parents[1]


def sources():
    paths=set()
    for directory in ['assets','renderer','compat/legacy','runtime/scene']:
        paths.update(p for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ['.cpp','.hpp','.c','.h','.S'] or p.name in ['CMakeLists.txt','README.md']))
    paths.update(ROOT/p for p in ['apps/qt-shell/live_world_session.cpp','apps/qt-shell/live_world_session.hpp','apps/qt-shell/world_live_main.cpp',
        'apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp','protocols/include/mnm/world_channel_v1.h','protocols/include/mnm/world_frame_v1.h',
        'protocols/include/mnm/scene_snapshot_v1.h','tests/world-live-session-test.cpp','tests/world-channel-test.cpp','tests/world-frame-test.cpp',
        'tests/resource-fixtures.hpp','tests/scene-renderer-test.cpp','tests/scene-history-test.cpp','tools/test-native-world.py','tools/run-native-world.py','tools/capture-scene-game.py',
        'tools/build-scene-observer.py','tools/prepare-scene-observer.py','tools/scene-game-runner.py','research/formats/world-channel-v1.md',
        'research/runtime/native-world-live-rendering.md'])
    return {str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)}


def main():
    parent=ROOT/'working/tests/native-world-live';parent.mkdir(parents=True,exist_ok=True)
    output=Path(tempfile.mkdtemp(prefix='run-',dir=parent));build=ROOT/'working/build/world-frame'
    fingerprints=sources();report={'success':False,'sources':fingerprints,'scope':'Synthetic native ownership, Qt session, atomic drawing and renderer regression checks; no original/live equivalence'}
    try:
        with (output/'execution.log').open('x') as log:
            for command in [
                ['cmake','-S',str(ROOT/'compat/legacy'),'-B',str(build),'-DCMAKE_BUILD_TYPE=Debug'],
                ['cmake','--build',str(build),'-j4'],
                ['ctest','--test-dir',str(build),'--output-on-failure','--output-junit',str(output/'ctest.xml')]]:
                subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
        tree=ET.parse(output/'ctest.xml');cases=tree.findall('.//testcase')
        if not cases or any(c.find('failure') is not None or c.find('skipped') is not None for c in cases):raise RuntimeError('Native World regression failure')
        if sources()!=fingerprints:raise RuntimeError('Sources changed during tests')
        report.update(success=True,sources_stable=True,ctests_passed=len(cases),tests=[c.attrib['name'] for c in cases])
    finally:
        (output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(output/'report.json',flush=True)


if __name__=='__main__':main()
