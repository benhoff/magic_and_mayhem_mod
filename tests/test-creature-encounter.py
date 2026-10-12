#!/usr/bin/env python3
"""Build and check native autonomous control, complete resumes and Qt controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def closure(build,target):
    pending=[build/'CMakeFiles'/f'{target}.dir/link.txt'];seen=set();sources=set()
    while pending:
        link=pending.pop().resolve()
        if link in seen:continue
        seen.add(link);base=link.parents[2]
        for token in shlex.split(link.read_text()):
            p=Path(token)
            if p.suffix not in ('.a','.o'):continue
            p=(base/p).resolve()
            if not p.is_relative_to(build):continue
            if p.suffix=='.a':pending.append(p.parent/'CMakeFiles'/f'{p.stem.removeprefix("lib")}.dir/link.txt');continue
            for name in shlex.split(Path(str(p)+'.d').read_text().replace('\\\n',' ')):
                dep=(base/name).resolve()
                if dep.is_relative_to(ROOT) and not dep.is_relative_to(ROOT/'working'):sources.add(dep.relative_to(ROOT).as_posix())
    return sources

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,required=True);p.add_argument('--installation',type=Path,default=ROOT/'working/game-clean');p.add_argument('--claims',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--sanitized',action='store_true');p.add_argument('--window',action='store_true');a=p.parse_args()
    if a.output.exists():p.error('new output required')
    a.output=a.output.resolve();a.build=a.build.resolve();a.output.mkdir(parents=True)
    c=json.loads(a.claims.read_text());assert all(sha(ROOT/n)==v for n,v in c['sources'].items())
    report=dict(success=False,claims=c['claims'],source_sha256=c['sources'],commands=[],wine_required=False,live_replacement=False,scope='Recovered ordinary control behind native flat-map movement and authored animation/knockback/scheduling policies; complete original AI and live replacement excluded.')
    def run(name,cmd,good=True,timeout=300):
        report['commands'].append([str(x) for x in cmd]);r=subprocess.run([str(x) for x in cmd],cwd=ROOT,capture_output=True,text=True,timeout=timeout)
        (a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
        if (r.returncode==0)!=good or 'runtime error:' in r.stderr or 'ERROR: AddressSanitizer' in r.stderr:raise RuntimeError(f'{name}: {r.returncode}: {r.stdout[-1000:]}{r.stderr[-2000:]}')
        return r.stdout
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        inputs=['CFG/Encrypted/creature.cfg','CFG/Encrypted/HTH.cfg','Creatures/crocodile.ani'];hashes={n:sha(a.installation/n) for n in inputs};report['input_sha256']=hashes
        flags=['-DCMAKE_BUILD_TYPE=Debug','-DBUILD_TESTING=ON']
        if a.sanitized:flags+=['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie']
        run('configure',['cmake','-S','apps/creature-lab','-B',a.build,*flags])
        targets=['native-creature-encounter-test','mnm-creature-cli']+(['creature-window-test','mnm-creature-lab'] if a.window else [])
        run('build',['cmake','--build',a.build,'--target',*targets,'-j','4'])
        deps=set().union(*(closure(a.build,t) for t in targets));report['compiler_dependencies']=sorted(deps)
        assert not deps-c['sources'].keys(),f'Unbound dependencies: {deps-c["sources"].keys()}'
        report['contract']=json.loads(run('contract',[a.build/'native-creature-encounter-test',a.installation]))
        cli=a.build/'mnm-creature-cli'
        rows=lambda data:[json.loads(line) for line in data.splitlines()]
        full=rows(run('full',[cli,'new',a.installation,8192,1,a.output/'full.mnm']))
        assert full[-1]['tick']<8192
        wanted={'move','attack-started','hit','stunned','stun-ended','knockback','died'}
        assert wanted<={e[1] for row in full for e in row['events']}
        points=set(range(0,min(20,len(full))))|{len(full)-1}
        for kind in wanted:
            tick=next(row['tick'] for row in full if any(e[1]==kind for e in row['events']));points|={tick-1,tick,min(tick+1,len(full)-1)}
        for tick in sorted(points):
            checkpoint=a.output/f'save-{tick}.mnm';run(f'capture-{tick}',[cli,'new',a.installation,tick,1,checkpoint]);resumed=rows(run(f'resume-{tick}',[cli,'resume',checkpoint,8192,0,a.output/f'resumed-{tick}.mnm']))
            assert resumed[1:]==full[tick+1:];assert (a.output/f'resumed-{tick}.mnm').read_bytes()==(a.output/'full.mnm').read_bytes()
        (a.output/'truncated.mnm').write_bytes((a.output/'full.mnm').read_bytes()[:-1]);run('refuse-truncated',[cli,'resume',a.output/'truncated.mnm',1,0,a.output/'bad.mnm'],False)
        run('refuse-existing',[cli,'new',a.installation,1,1,a.output/'full.mnm'],False)
        window=None
        if a.window:
            run('window',['xvfb-run','-a',a.build/'creature-window-test','--installation',a.installation,'--test-output',a.output/'window'])
            window=json.loads((a.output/'window/result.json').read_text());assert window['success'];assert (a.output/'window/final.mnm').read_bytes()==(a.output/'full.mnm').read_bytes()
            resumed=rows(run('window-resume',[cli,'resume',a.output/'window/mid-attack.mnm',8192,0,a.output/'window-resumed.mnm']));assert resumed[1:]==full[14:]
        report.update(success=True,process_continuations=len(points),final=full[-1],window=window,sanitizer_passed=a.sanitized,binaries={t:sha(a.build/t) for t in targets},input_unchanged=all(sha(a.installation/n)==v for n,v in hashes.items()))
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['sources_unchanged']=all(sha(ROOT/n)==v for n,v in c['sources'].items());(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_unchanged'] and report['input_unchanged'];print(json.dumps({k:report[k] for k in ['success','contract','process_continuations','final']}))
if __name__=='__main__':main()
