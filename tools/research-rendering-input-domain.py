#!/usr/bin/env python3
"""Read installed recipes/saves and prepare one explicit audible-music control.

Research only. No production safety limit, source installation or original is
changed. Recipe identity does not establish the generated World or RNG state.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess

from coverage_evidence import Reader, write_report

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-execution-research-results-20261031/'


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('phase', choices=['save', 'music', 'catalog'])
    parser.add_argument('--report', required=True)
    parser.add_argument('--save', type=Path)
    args = parser.parse_args()
    if not args.report.startswith(PREFIX) or (ROOT/args.report).exists():
        parser.error('Require a new owned research report')
    source = ROOT/'working/game-nocd'
    names = ['tools/research-rendering-input-domain.py', 'tools/decode-cfg.py',
             'tools/test-terrain-region.py', 'tests/save-world-domain-research.cpp',
             'assets/save_world.cpp', 'assets/save_world.hpp', 'assets/save_loader.cpp',
             'assets/persistence.hpp', 'runtime/audio/music.h']
    with Reader(ROOT) as reader:
        sources = {n: reader.digest(n) for n in names}
    report = dict(schema=1, phase=args.phase, success=False, sources=sources,
        original_game_executed=False, production_policy_modified=False,
        player_acceptance_assessed=False, generated_world_identity_established=False,
        status_promotions=[], scope=__doc__, commands=[])
    def retain(label, data):
        name = str(Path(args.report).with_suffix(''))+'-'+label+'.bin'
        return dict(record=name, sha256=write_report(ROOT, name, data), bytes=len(data))
    def run(command, label, **settings):
        result = subprocess.run(list(map(str, command)), capture_output=True, timeout=180, **settings)
        report['commands'].append(dict(command=list(map(str,command)), returncode=result.returncode,
            stdout=retain(label+'-stdout',result.stdout), stderr=retain(label+'-stderr',result.stderr)))
        if result.returncode:
            raise ValueError(label+' failed: '+result.stderr.decode(errors='replace')[-1200:])
        return result.stdout
    try:
        run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
        if args.phase == 'save':
            save = args.save.resolve()
            if not save.is_relative_to(ROOT/'working'):
                raise ValueError('Save must be an owned working input')
            report['input'] = retain('save', save.read_bytes())
            output = ROOT/'working/save-domain-research-02'
            output.mkdir(exist_ok=False)
            flags = subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True).split()
            command = ['g++','-std=c++20','-O2','-fPIC','-pie',ROOT/names[3],
                ROOT/'working/build/qt-shell/assets/libmnm-persistence-loader.a',
                ROOT/'working/build/qt-shell/assets/libmnm-assets.a',*flags,'-o',output/'probe']
            report['linked_libraries'] = {str(p.relative_to(ROOT)):digest(p) for p in command if isinstance(p,Path) and p.suffix=='.a'}
            run(command,'build')
            report['binary'] = retain('probe',(output/'probe').read_bytes())
            report['result'] = json.loads(run([output/'probe',save.parent.parent,save.relative_to(save.parent.parent)],'probe'))
        elif args.phase == 'music':
            target = ROOT/'working/game-music-research'
            if target.exists():
                raise ValueError('Refusing to overwrite control installation')
            shutil.copytree(source,target)
            prefs = target/'CFG/prefs.cfg'
            before = prefs.read_bytes()
            if before.count(b'MusicVolume=0') != 1 or b'MusicVolume=00' in before:
                raise ValueError('Expected exact single zero-volume input')
            after = before.replace(b'MusicVolume=0',b'MusicVolume=15')
            prefs.write_bytes(after)
            if (source/'CFG/prefs.cfg').read_bytes() != before:
                raise ValueError('Source preferences changed')
            report.update(control_source=str(target), single_setting_delta=dict(
                key='SOUND/MusicVolume',before=0,after=15,source_modified=False,
                original_preferences=retain('prefs-before',before), control_preferences=retain('prefs-after',after)),
                files={str(p.relative_to(target)):dict(sha256=digest(p),bytes=p.stat().st_size)
                    for p in sorted(target.rglob('*')) if p.is_file()})
        else:
            codec = module('domain_codec','tools/decode-cfg.py')
            recipes = module('domain_recipes','tools/test-terrain-region.py')
            catalog = []
            for realm in ['Celtic','Greek','Medieval','Multiplayer']:
                path=source/f'Realms/{realm}/{realm}.cfg'
                if not path.exists():
                    raise ValueError('Expected installed realm '+realm)
                data=path.read_bytes(); raw=data if data.startswith(b';') else codec.decode(data)[1]
                rows=recipes.recipe_rows(raw)
                catalog.append(dict(realm=realm,path=str(path.relative_to(source)),
                    input=retain('cfg-'+realm,data),decoded_sha256=hashlib.sha256(raw).hexdigest(),recipes=rows))
            report['catalog']=catalog
            report['selection_join_policy']='Name/realm/recipe and actual ordered placed MAP inputs must all join; UI row+1 is not a filename or seed. Random choices, rotations, assembled grid and restored runtime state still require capture.'
        report['success']=True
    except Exception as error:
        report['error']=str(error)
    finally:
        try:
            run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
            with Reader(ROOT) as reader:
                report['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
            report['success'] &= report['sources_stable']
        except Exception as error:
            report.update(success=False,closeout_error=str(error))
        sha=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],sha256=sha,error=report.get('error'),result=report.get('result'))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
