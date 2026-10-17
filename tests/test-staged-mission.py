#!/usr/bin/env python3
"""Bounded source-declared native mission and actual Qt/save-restart acceptance."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--assets', type=Path, required=True)
    p.add_argument('--build', type=Path, required=True)
    p.add_argument('--sanitized', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    a.output = a.output.resolve()
    a.output.mkdir(parents=True, exist_ok=False)
    (a.output/".retain").write_text("Source-bound native staged mission validation and restart evidence.\n")
    a.assets, a.build, a.sanitized = (x.resolve() for x in (a.assets, a.build, a.sanitized))
    declaration = json.loads(a.claims.read_text())
    sources = declaration['sources']
    assert all(sha(ROOT/k) == v for k, v in sources.items())
    report = dict(success=False, claims=declaration['claims'], sources=sources, commands=[],
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  scope='Native authored three-stage installed Plains mission, deferred atomic reinforcement, wizard melee, campaign Cure unlock, Qt objectives and deterministic fresh-process continuation. No original mission/rules equivalence or live replacement.')
    env = dict(os.environ, QT_SCALE_FACTOR='1', QT_AUTO_SCREEN_SCALE_FACTOR='0',
               XDG_CONFIG_HOME=str(a.output/'qt-config'), QT_STYLE_OVERRIDE='Fusion',
               ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(name, args, ok=True):
        args = list(map(str, args)); report['commands'].append(args)
        r = subprocess.run(args, cwd=ROOT, env=env, capture_output=True, text=True, timeout=900)
        (a.output/(name+'.stdout')).write_text(r.stdout)
        (a.output/(name+'.stderr')).write_text(r.stderr)
        assert (r.returncode == 0) == ok, (name, r.stdout[-1500:], r.stderr[-1500:])
        assert 'runtime error:' not in r.stderr and 'ERROR: AddressSanitizer' not in r.stderr
        return r.stdout

    def window(name, actions, load=None):
        script = a.output/(name+'.json'); script.write_text(json.dumps(actions, indent=2)+'\n')
        args = ['xvfb-run', '-a', '-s', '-screen 0 1400x1500x24', a.build/'campaign-window-test',
                '--installation', a.assets, '--test-script', script, '--test-output', a.output/name]
        if load: args += ['--load-campaign', load]
        run(name, args)
        assert json.loads((a.output/name/'report.json').read_text())['success']
        return a.output/name/'final.mnmc'

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    installed = ['Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd', 'Realms/Celtic/Plains/Terrain.spr',
                 'CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani', 'CFG/Encrypted/effectani.cfg']
    installed += [p.relative_to(a.assets).as_posix() for p in sorted((a.assets/'Sprites').glob('*')) if p.name.lower().startswith('effects') and p.suffix.lower()=='.ani']
    report['installed'] = {p: sha(a.assets/p) for p in installed}
    try:
        run('build-normal', ['cmake', '--build', a.build, '-j4'])
        (a.output/'normal-CMakeCache.txt').write_bytes((a.build/'CMakeCache.txt').read_bytes())
        report['normal_cache_sha256']=sha(a.build/'CMakeCache.txt')
        report['normal'] = json.loads(run('normal-contract', [a.build/'staged-mission-test', a.assets]))
        run('sanitized-configure', ['cmake', '-S', ROOT/'apps/skirmish', '-B', a.sanitized, '-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_CXX_FLAGS_DEBUG=-g0',
                                   '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])
        run('sanitized-build', ['cmake', '--build', a.sanitized, '--target', 'staged-mission-test', '-j4'])
        report['sanitized'] = json.loads(run('sanitized-contract', [a.sanitized/'staged-mission-test', a.assets]))
        assert report['normal']==report['sanitized']
        report['ctest'] = run('ctest', ['ctest', '--test-dir', a.build, '--output-on-failure'])
        cli = a.build/'mnm-map-campaign-cli'
        seed, wave = a.output/'seed.mnmc', a.output/'wave.mnmc'
        run('seed', [a.build/'staged-mission-test', a.assets, 'seed', seed])
        run('wave', [a.build/'staged-mission-test', a.assets, 'wave', wave])
        idle = a.output/'idle.mnmc'; run('create', [cli, a.assets, 'create-mission', idle])
        fresh = window('new-mission', [dict(kind='button', name='newMissionCampaignButton'),
                                     dict(kind='expect', mission_stage=0, cure_enabled=False, terrain=True, mission_text='Defeat the Plains guard.'),
                                     dict(kind='save', name='started.mnmc'), dict(kind='pause'),
                                     dict(kind='cell',wizard=0),dict(kind='button',name='summonButton'),
                                     dict(kind='cell',x=7,y=1,z=7),dict(kind='save',name='pending-summon.mnmc')])
        started = a.output/'started.mnmc'; run('start', [cli, a.assets, 'start', idle, started])
        assert (a.output/'new-mission/started.mnmc').read_bytes()==started.read_bytes()
        pending=a.output/'pending-summon-cli.mnmc';run('pending-summon',[cli,a.assets,'cast',started,pending,'summon',7,1,7])
        assert fresh.read_bytes()==pending.read_bytes()
        summoned=a.output/'summoned.mnmc';run('summon',[cli,a.assets,'resume',pending,summoned,3])
        attack=a.output/'attack-wizard.mnmc';run('attack-wizard',[cli,a.assets,'attack',summoned,attack,3,1])
        attack_copy=a.output/'attack-wizard-copy.mnmc';run('attack-roundtrip',[cli,a.assets,'resume',attack,attack_copy,0])
        assert attack.read_bytes()==attack_copy.read_bytes()
        run('locked-cure', [cli, a.assets, 'cast', started, a.output/'locked.mnmc', 'cure', '2'], ok=False)
        assert not (a.output/'locked.mnmc').exists()
        cut = window('guard-cut', [dict(kind='expect', mission_stage=0, cure_enabled=False), dict(kind='steps', count=2),
                                   dict(kind='save', name='attack.mnmc')], seed)
        reference = a.output/'reference.mnmc'; run('resume-guard', [cli, a.assets, 'resume', cut, reference, 8])
        resumed = window('guard-restart', [dict(kind='steps', count=8)], cut)
        assert resumed.read_bytes()==reference.read_bytes()
        completed = window('wave-victory', [dict(kind='expect', mission_stage=1, reinforcement=True, cure_enabled=False),
                                           dict(kind='steps', count=160), dict(kind='expect', battle=False, transition=2, mission_text='Cure unlocked'),
                                           dict(kind='save', name='reward.mnmc')], wave)
        reference = a.output/'victory-reference.mnmc';run('resume-wave', [cli, a.assets, 'resume', wave, reference, 160])
        assert completed.read_bytes()==reference.read_bytes()
        idle_copy = a.output/'reward-reopened.mnmc';run('idle-roundtrip', [cli, a.assets, 'resume', completed, idle_copy, 0])
        assert completed.read_bytes()==idle_copy.read_bytes()
        finale = window('reward-restart', [dict(kind='expect', battle=False, mission_text='Cure unlocked'),
                                          dict(kind='button', name='continueCampaignButton'),
                                          dict(kind='expect', battle=True, cure_enabled=True, terrain=False),
                                          dict(kind='save', name='finale.mnmc')], completed)
        finale_cli=a.output/'finale-cli.mnmc';run('start-finale',[cli,a.assets,'start',completed,finale_cli])
        assert finale.read_bytes()==finale_cli.read_bytes()
        report['qt_journeys']=5
        # Retained pre-change campaigns retain both exact bytes and old continuation.
        legacy_root=ROOT/'working/tests/terrain-campaign-bound-20261016/execution'
        report['legacy']={}
        for name in ['terrain-window','legacy-creature-window','legacy-map-window']:
            candidates=sorted((legacy_root/name).rglob('*.mnmc'))
            assert candidates, name
            source=candidates[0]; out=a.output/('legacy-'+name+'.mnmc')
            legacy_cli=cli if name!='legacy-creature-window' else a.build/'mnm-campaign-cli'
            args=[legacy_cli]
            args += [a.assets] if legacy_cli==cli else ['--installation',a.assets]
            run('legacy-'+name,[*args,'resume',source,out,0])
            assert source.read_bytes()==out.read_bytes()
            report['legacy'][str(source.relative_to(ROOT))]=sha(source)
        dependencies=set()
        for dep in a.build.rglob('*.o.d'):
            for token in dep.read_text().replace('\\\n',' ').split()[1:]:
                path=Path(token);path=(path if path.is_absolute() else a.build/path).resolve()
                if path.is_file() and path.is_relative_to(ROOT):dependencies.add(path.relative_to(ROOT).as_posix())
        report['compiler_dependencies']=sorted(dependencies)
        report['undeclared_dependencies']=sorted(dependencies-set(sources))
        assert not report['undeclared_dependencies'],report['undeclared_dependencies']
        report['sources_unchanged']=all(sha(ROOT/k)==v for k,v in sources.items())
        report['inputs_unchanged']=all(sha(a.assets/k)==v for k,v in report['installed'].items())
        assert report['sources_unchanged'] and report['inputs_unchanged']
        report['success']=True
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        (a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'success':report['success'],'report':str(a.output/'report.json')}))


if __name__=='__main__':
    main()
