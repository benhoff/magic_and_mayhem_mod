#!/usr/bin/env python3
"""Bounded fixed-region, catalog and seeded selection controls; SQLite evidence."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
SELECTED = ['RE.terrain_region', 'RE.terrain_catalog', 'RE.terrain_selection']
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--asset-root', type=Path, required=True)
    args = parser.parse_args()
    out, report_path = args.output.resolve(), args.report.absolute()
    if (out.exists() or not out.is_relative_to(ROOT/'working/tests') or report_path.exists()
            or not report_path.is_relative_to(ROOT/'research/runtime')):
        parser.error('Require new checkout-local execution and research report paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain bounded terrain inputs, logs, binaries and failures.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = {b['id']:b for b in register['behaviors']}
    builds = {b['id']:b for b in register['builds']}
    model = lambda name:'reconstruction/rendering/'+name+'.cpp'
    region = [model('terrain_region'), model('terrain_sections')]
    catalog = [model(n) for n in ['terrain_catalog','terrain_specific','terrain_solver','terrain_constraints','terrain_selection']]
    selection = [model('terrain_selection')]
    units = {'region':['tests/terrain-region-test.cpp',*region],
             'catalog':['tests/terrain-catalog-build-test.cpp',*catalog],
             'selection':['tests/terrain-selection-test.cpp',*selection]}
    refs = {'region':['tests/terrain-region-reference.cpp',*region],
            'catalog':['tests/terrain-catalog-reference.cpp','assets/region_recipe.cpp',*catalog],
            'selection':['tests/terrain-selection-reference.cpp',*selection]}
    assembly = ['tests/terrain-region-assembly-reference.cpp','assets/terrain_catalog.cpp',*region]
    flags = ['g++','-std=c++17','-Wall','-Wextra','-Ireconstruction/rendering',
             '-ffunction-sections','-fdata-sections','-Wl,--gc-sections']
    paths = set().union(*(required_sources(behaviors[b]) for b in SELECTED))
    all_cpp = sorted(set(sum(units.values(),[])+sum(refs.values(),[])+assembly))
    deps = subprocess.check_output(flags+['-MM']+all_cpp,cwd=ROOT,env=env,text=True)
    (out/'dependencies.txt').write_text(deps)
    paths.update(all_cpp)
    for token in shlex.split(deps.replace('\\\n',' ')):
        p = ROOT/token
        if p.is_file(): paths.add(p.resolve().relative_to(ROOT).as_posix())
    sources = {p:sha(ROOT/p) for p in sorted(paths)}
    claims = [dict(behavior=b,contract_sha256=behavior_contract(behaviors[b],builds)) for b in SELECTED]
    report = dict(schema=1,success=False,sources=sources,claims=claims,original_claims=claims,
                  original_sha256=BUILD,inputs={},commands=[],controls={},comparisons={},live_replacement=False,
                  scope='Owned offline fixed-region planning/ordinary cell assembly, ordered descriptor catalog '
                  'construction and seeded single-block selection. Original comparisons use safe placement, '
                  'descriptor, RNG/table/choice helpers and the unchanged copied threshold instruction window. '
                  'Whole-cell assembly has independent native Python controls. Native malformed/capacity/overflow '
                  'refusal is separate policy. No original CFG/file/dialog/allocation recovery, whole-world/scene '
                  'production, GPU presentation, live integration/replacement or FPS credit.')

    def run(label, argv, structured=False):
        command = [str(a) for a in argv]
        r = subprocess.run(command,cwd=ROOT,env=env,capture_output=True,timeout=300)
        log = out/(label+'.log');log.write_bytes(r.stdout+b'\n'+r.stderr)
        report['commands'].append(dict(label=label,argv=command,exit_code=r.returncode,log_sha256=sha(log)))
        r.check_returncode()
        return json.loads(r.stdout) if structured else None

    def track(path):
        report['inputs'].setdefault(str(path),sha(path))

    verified_before = False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified_before=True
        exe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(exe)!=BUILD: raise ValueError('Unsupported original executable')
        track(exe)
        codec = module('terrain_batch_codec',ROOT/'tools/decode-cfg.py')
        independent = module('terrain_batch_recipe',ROOT/'tools/test-terrain-region.py')
        assets = args.asset_root.resolve()
        rows, cfgs, recipes, cache, missing = [], [], {}, {}, []

        def header(path):
            if path not in cache:
                track(path);cache[path]=codec.decode(path.read_bytes())[1]
            return cache[path]

        for realm in ['Celtic','Greek','Medieval']:
            config = assets/f'Realms/{realm}/{realm}.cfg';track(config)
            raw = config.read_bytes();raw = raw if raw.startswith(b';') else codec.decode(raw)[1]
            target = out/(realm+'.cfg');target.write_bytes(raw);cfgs.append(target)
            recipes[realm]=independent.recipe_rows(raw)
            for recipe in recipes[realm]:
                entries = [q+[1] for q in recipe['specific']]+[[q[0],-1,-1,-1,q[1]] for q in recipe['random']]
                row = struct.pack('<5I',recipe['id'],recipe['columns'],recipe['rows'],len(recipe['specific']),len(recipe['random']))
                for entry in entries:
                    path = assets/recipe['path'].replace('\\','/')/(recipe['prefix']+f'{entry[0]:02}.map')
                    if path.exists(): raw_header=header(path)[:76]
                    else:
                        missing.append(dict(realm=realm,region=recipe['id'],section=entry[0],path=str(path.relative_to(assets))))
                        raw_header=bytes(76)
                    row += struct.pack('<5i',*entry)+raw_header
                rows.append(row)
        catalogs=out/'catalogs.bin';catalogs.write_bytes(struct.pack('<I',len(rows))+b''.join(rows))
        singles=[]
        for path in sorted(assets.rglob('*.map')):
            raw=header(path)
            if struct.unpack_from('<2I',raw,24)==(1,1):singles.append(struct.pack('<I',len(singles)%100)+raw[:76])
        headers=out/'single-headers.bin';headers.write_bytes(struct.pack('<I',len(singles))+b''.join(singles))
        report['missing_headers']=missing
        report['installed_inputs']=dict(recipes=len(rows),single_headers=len(singles),map_files=len(cache),missing_headers=len(missing))
        fixed_fixtures=[];assembly_cases=[]

        def fixture(recipe, maps, ttd, label, installed=False):
            # Header-only fixture is consumed by the private original placement reference.
            if installed:
                p=out/(label+'-headers.bin')
                p.write_bytes(struct.pack('<3I',recipe['columns'],recipe['rows'],len(maps))+
                             b''.join(struct.pack('<4i',*q)+m[:76] for q,m in zip(recipe['specific'],maps)))
                fixed_fixtures.append(p)
            payload=struct.pack('<3I',recipe['columns'],recipe['rows'],len(maps))
            payload+=b''.join(struct.pack('<4iI',*q,len(m))+m for q,m in zip(recipe['specific'],maps))
            bundle=out/(label+'-bundle.bin');bundle.write_bytes(payload)
            expected,_=independent.assemble(recipe,[(struct.unpack_from('<19I',m),m) for m in maps],ttd.read_bytes())
            golden=out/(label+'-expected.map');golden.write_bytes(expected)
            assembly_cases.append((ttd,bundle,golden))

        for region_id in [6,9]:
            recipe=next(r for r in recipes['Celtic'] if r['id']==region_id)
            maps=[header(assets/recipe['path'].replace('\\','/')/(recipe['prefix']+f'{p[0]:02}.map')) for p in recipe['specific']]
            ttd=assets/recipe['sprite_path'].replace('\\','/')/'Terrain.ttd';track(ttd)
            fixture(recipe,maps,ttd,'installed-'+str(region_id),True)
        ttd=out/'synthetic.ttd';payload=bytearray(b'TTD\0'+struct.pack('<3I',16+16*356,4,16)+bytes(16*356))
        for i in range(16):
            for rotation in range(1,4):struct.pack_into('<H',payload,16+i*356+0x94-4*rotation,(i+rotation)%16)
        ttd.write_bytes(payload)
        raw=bytearray(76);struct.pack_into('<8I',raw,0,6,4,4,2,16,32,2,2)
        raw+=b''.join(struct.pack('<6H',i%16,0,65535,i,0xa5a5,i|8) for i in range(32))
        for rotation in range(4):
            for x,y in [(0,0),(1,0),(0,1),(1,1)]:
                recipe=dict(columns=2,rows=2,path='synthetic',prefix='s',specific=[[0,rotation,x,y]],random=[])
                fixture(recipe,[bytes(raw)],ttd,f'synthetic-{rotation}-{x}-{y}')
        for mode, options in [('release',['-O2','-DNDEBUG']),('sanitized',['-O1','-g',
                              '-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])]:
            for label, src in units.items():
                binary=out/(label+'-'+mode)
                run(label+'-'+mode+'-build',flags+['-Werror']+options+src+['-o',binary])
                run(label+'-'+mode,[binary]);report['controls'][label+'-'+mode]=dict(success=True,binary_sha256=sha(binary))
            binary=out/('assembly-'+mode)
            run('assembly-'+mode+'-build',flags+options+assembly+['-o',binary])
            values=[run(f'assembly-{mode}-{i}',[binary,*case],True) for i,case in enumerate(assembly_cases)]
            if not all(v['all_match'] for v in values):raise ValueError('Whole-cell mismatch')
            report['comparisons']['assembly-'+mode]=dict(cases=len(values),cells=sum(v['cells'] for v in values),all_match=True,results=values)
        reference_args={'region':fixed_fixtures,'catalog':[catalogs,*cfgs],'selection':[headers]}
        for label,src in refs.items():
            binary=out/(label+'-original')
            run(label+'-original-build',flags+['-m32','-O2','-fno-pie','-no-pie']+src+['-o',binary])
            value=run(label+'-original',[binary,exe,*reference_args[label]],True)
            if not value['all_match']:raise ValueError('Original comparison refused: '+label)
            report['comparisons'][label]=value
            binary=out/(label+'-sanitized-corpus')
            run(label+'-corpus-build',flags+['-DMNM_NATIVE_ONLY','-O1','-fsanitize=address,undefined',
                '-fno-sanitize-recover=all','-fno-pie','-no-pie']+src+['-o',binary])
            native=run(label+'-corpus',[binary,exe,*reference_args[label]],True)
            if native!=value:raise ValueError('Sanitized corpus result differs: '+label)
            report['comparisons'][label+'-sanitized']=native
        report['derived_inputs']={p.name:sha(p) for p in out.iterdir() if p.suffix in {'.bin','.map','.cfg','.ttd'}}
        report['native_all_passed']=len(report['controls'])==6
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items())
        report['inputs_stable']=all(sha(Path(p))==h for p,h in report['inputs'].items())
        if not report['sources_stable'] or not report['inputs_stable']:raise ValueError('Input changed during execution')
        report['success']=True
    except BaseException as error:
        report['failure']=dict(type=type(error).__name__,message=str(error))
        raise
    finally:
        try:
            run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after']=verified_before
        except BaseException:
            report['success']=False
            raise
        finally:write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'],controls=len(report['controls']),comparisons=report['comparisons']),indent=2))


if __name__=='__main__':main()
