#!/usr/bin/env python3
"""Execute ten bounded offline rendering contracts with immutable SQLite evidence."""
import argparse
import ast
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import shlex
import struct
from drawing_fixture_prefix import prepare
from coverage_evidence import write_report, evidence_digest

ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.drawing-engine-update-windows','NR.drawing-diagnostic-cadence-control','NR.drawing-campaign-guidance-handshake','NR.drawing-hud-cursor-binding','NR.drawing-actor-latch-binding','NR.drawing-region-click-latch-observation','NR.drawing-selection-toggle-observation','NR.drawing-pause-key-observation','NR.drawing-copy-phase-transposition','NR.drawing-mutator-alias-exposure']
SCENARIO='NR.eighth-ten.observation-policies-20261022'
TESTS=['tests/test-drawing-'+n+'.py' for n in ['engine-windows','diagnostic-cadence','diagnostic-schedule','campaign-guidance','hud-input-binding','hud-state','actor-latch-binding','actor-latches','region-selection','documented-semantics','copy-phase','surface-lifetimes','call-intervals','input-point','region-input']]+['tests/test-rendering-eighth-ten.py']


def sha(path):return evidence_digest(Path(path),root=ROOT)


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--wine-prefix',type=Path,required=True)
    parser.add_argument('--prefix-owner-root',type=Path,required=True)
    args=parser.parse_args()
    report_path=args.report if args.report.is_absolute() else ROOT/args.report
    if report_path.exists():parser.error('Refusing evidence overwrite')
    parent=ROOT/'working/tests/rendering-eighth-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Eighth rendering batch raw controls and failed attempts. Generated evidence is in SQLite.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('eighth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    todo=set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-eighth-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_observed_inputs.py'}
    todo.update({'tools/build-menu-observer.py','tools/drawing_fixture_prefix.py','tests/drawing-hud-input-binding-test.c','tests/drawing-actor-latch-region-test.c','tests/drawing-hud-state-test.c'})
    todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/menu').iterdir() if p.suffix in {'.c','.h','.S'})
    visited=set()
    while todo:
        name=todo.pop()
        if name in visited:continue
        visited.add(name);path=ROOT/name;declaration['sources'][name]=sha(path)
        if path.suffix=='.py':
            for node in ast.walk(ast.parse(path.read_text())):
                if isinstance(node,ast.Constant) and isinstance(node.value,str) and node.value.endswith('.py'):
                    for candidate in (ROOT/node.value,path.parent/node.value,ROOT/'tools'/node.value,ROOT/'tests'/node.value):
                        if candidate.is_file() and candidate.resolve().is_relative_to(ROOT):todo.add(candidate.resolve().relative_to(ROOT).as_posix())
                names=([alias.name for alias in node.names] if isinstance(node,ast.Import) else
                  [node.module]+[node.module+'.'+alias.name for alias in node.names] if isinstance(node,ast.ImportFrom) and node.module else [])
                for name in names:
                    relative=Path(*name.split('.'))
                    for base in (ROOT/'tools',ROOT/'protocols/python'):
                        for candidate in (base/relative.with_suffix('.py'),base/relative/'__init__.py'):
                            if candidate.is_file():todo.add(candidate.relative_to(ROOT).as_posix())
        if path.suffix in {'.c','.cpp','.h','.hpp','.S','.inc'}:
            for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
                child=(path.parent/include).resolve()
                if child.is_file() and child.is_relative_to(ROOT):todo.add(child.relative_to(ROOT).as_posix())
    report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],commands=[],tests_run=0,
        scope=__doc__,original_game_executed=False,original_artifacts_consumed=False,original_comparison_claimed=False,live_replacement=False,
        reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix())
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    (out/'tmp').mkdir();env.update(TMPDIR=str(out/'tmp'),PYTHONDONTWRITEBYTECODE='1',WINEPREFIX=str(args.wine_prefix.resolve()),WINEDEBUG='-all')
    lock=None
    def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=120):
        command=[str(x) for x in command];print(label,flush=True)
        process=subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},capture_output=True,text=True,timeout=timeout)
        log=process.stdout+process.stderr;(out/(label+'.log')).write_text(log)
        report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=process.returncode,log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()))
        if process.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(label+': '+log)
        return log
    try:
        owner=args.prefix_owner_root.resolve();prefix=args.wine_prefix.resolve()
        if owner!=ROOT and (owner.parent!=ROOT.parent or not (owner/'.git').is_file()):raise ValueError('Prefix owner must be this worktree or a registered sibling')
        prefix,lock=prepare(owner,out,prefix,prefix)
        report['fixture_prefix']=dict(owner=str(owner),path=str(prefix),exclusive_lock=True,idle_before_use=True)
        report['wine_version']=run('wine-version',['wine','--version']).strip()
        report['clang_version']=run('clang-version',['clang','--version']).strip()
        run('wineboot',['wineboot','-u'])
        run('menu-production',[sys.executable,'-B',ROOT/'tools/build-menu-observer.py'])
        manifest=json.loads((ROOT/'working/build/menu-observer/manifest.json').read_text())
        for name,digest in manifest['compiled_sources'].items():
            if report['sources'].get(name)!=digest:raise ValueError('Undeclared production compiler input: '+name)
        imports=dict(module('eighth_menu','tools/build-menu-observer.py').IMPORTS,CreateThread=24,WaitForSingleObject=8)
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
        run('kernel-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
        for name in ('drawing-hud-input-binding','drawing-actor-latch-region','drawing-hud-state'):
            obj=out/(name+'.obj');dep=out/(name+'.d');binary=out/(name+'.exe')
            run(name+'-compile',[*flags,'-MD','-MF',dep,'-c',ROOT/('tests/'+name+'-test.c'),'-o',obj])
            for value in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(value).resolve()
                if path.is_file() and path.is_relative_to(ROOT):
                    key=path.relative_to(ROOT).as_posix()
                    if report['sources'].get(key)!=sha(path):raise ValueError('Undeclared fixture compiler input: '+key)
            run(name+'-link',['lld-link','/machine:x86','/base:0x18000000','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),obj,out/'kernel32.lib'])
        hud=out/'native-hud';hud.mkdir()
        def win(path):return 'Z:'+str(path).replace('/','\\')
        values=dict(MNM_DRAW_HUD_STATE_FILE=win(hud/'hud.bin'),MNM_DRAW_HUD_INPUT_POINT='1',MNM_FIXTURE_DIRECTORY=win(hud))
        run('native-hud',['wine',out/'drawing-hud-input-binding.exe'],extra=values,cwd=hud)
        from drawing_hud_state import decode as hud_decode
        from drawing_hud_input_binding import bind as hud_bind
        from coverage_evidence import read_json
        observed=hud_decode((hud/'hud.bin').read_bytes());joined=hud_bind(observed,(hud/'session.bin').read_bytes())
        if len(observed['samples'])!=4 or joined['linked_samples']!=3 or joined['refused_points']!=1 or joined['stopped_linked_samples']!=1 or not observed['samples'][0]['input_point']['timely_cursor']:raise ValueError('Native HUD V2 source/refusal/retirement counts')
        report['native_hud']=dict(observation=observed,binding=joined,binary_sha256=sha(out/'drawing-hud-input-binding.exe'))
        # Actual analyzer entry with an explicitly synthetic protocol reference.
        with tempfile.TemporaryDirectory(dir=ROOT/'research/runtime',prefix='.eighth-native-') as reports:
            reports=Path(reports);reference=reports/'reference.json';destination=reports/'analysis.json'
            value=dict(success=True,source_sha256=__import__('drawing_api_session').BUILD,normal_original_quit_verified=True,synthetic_protocol_fixture=True,experiment=str(hud.parent/'native-hud-experiment'),staging=dict(hud_input_point=True),hud_state_observation=observed,observation=dict(journal_sha256=hashlib.sha256((hud/'session.bin').read_bytes()).hexdigest()))
            # Native raw input stays pinned in this run; only fixture pointers are temporary.
            experiment=hud.parent/'native-hud-experiment';experiment.mkdir();(experiment/'capture').symlink_to(hud,target_is_directory=True)
            (hud/'hud-state.bin').write_bytes((hud/'hud.bin').read_bytes());(hud/'session.done').write_bytes((hud/'session.bin').read_bytes()[-128:])
            write_report(ROOT,reference,(json.dumps(value)+'\n').encode())
            run('native-hud-cli',[sys.executable,'-B',ROOT/'tools/analyze-drawing-hud-input.py','--reference',reference,'--report',destination])
            report['native_hud']['cli_analysis']=read_json(destination,root=ROOT)
        for mode in range(1,5):
            target=out/('hud-refusal-'+str(mode));target.mkdir()
            run('native-hud-refusal-'+str(mode),['wine',out/'drawing-hud-input-binding.exe'],expected=99,extra={**values,'MNM_HUD_BINDING_CASE':str(mode),'MNM_DRAW_HUD_STATE_FILE':win(target/'hud.bin'),'MNM_FIXTURE_DIRECTORY':win(target)},cwd=target)
        legacy=out/'native-legacy-hud';legacy.mkdir()
        run('native-legacy-hud',['wine',out/'drawing-hud-state.exe'],extra=dict(MNM_DRAW_HUD_STATE_FILE=win(legacy/'hud.bin')),cwd=legacy)
        legacy_observation=hud_decode((legacy/'hud.bin').read_bytes())
        if len(legacy_observation['samples'])!=4:raise ValueError('Legacy HUD owner/read-only/cadence control counts')
        report['native_legacy_hud']=legacy_observation
        target=out/'native-region-latches';target.mkdir()
        run('native-region-latches',['wine',out/'drawing-actor-latch-region.exe'],extra=dict(MNM_DRAW_REGION_INPUT_FILE=win(target/'region.bin'),MNM_DRAW_ACTOR_LATCH_FILE=win(target/'actor-latches.bin')),cwd=target)
        from drawing_actor_latches import decode as latch_decode
        from drawing_region_input import decode as region_decode
        region=region_decode((target/'region.bin').read_bytes());latches=latch_decode((target/'actor-latches.bin').read_bytes())
        header=[0]*16;header[:2]=struct.unpack('<2I',b'MNMDAPI1');header[2:9]=[1,128,0x40209ca7,262144,1024,1073741824,2]
        terminal=[0]*32;terminal[:3]=[1,4,1];terminal[6]=region['owner']
        api=struct.pack('<16I',*header)+struct.pack('<32I',*terminal)
        bound=module('eighth_latch_analysis','tools/analyze-drawing-actor-latches.py').analyze(latches,region,api)
        if len(bound['callback_binding']['phases'])!=64:raise ValueError('Native Region/latch phase counts')
        report['native_region_latches']=dict(analysis=bound,binary_sha256=sha(out/'drawing-actor-latch-region.exe'),raw_region_sha256=hashlib.sha256((target/'region.bin').read_bytes()).hexdigest(),raw_latches_sha256=hashlib.sha256((target/'actor-latches.bin').read_bytes()).hexdigest())

        for path in TESTS:
            print(path,flush=True)
            command=[sys.executable,'-B',str(ROOT/path)]
            execution=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,text=True,timeout=120)
            log=execution.stdout+execution.stderr
            (out/(Path(path).stem+'.log')).write_text(log)
            report['commands'].append(dict(argv=command,actual_exit=execution.returncode,expected_exit=0,log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()))
            if execution.returncode:raise ValueError(path+': '+log)
            match=re.search(r'Ran (\d+) tests?',log)
            if not match:raise ValueError('Missing executed-test count: '+path)
            report['tests_run']+=int(match[1])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        fresh=module('eighth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])
        if fresh['claims']!=declaration['claims']:raise ValueError('Contract drift during execution')
        report['success']=True
    except Exception as error:report['error']=repr(error)
    finally:
        if lock is not None:
            try:
                run('owned-prefix-stop',['wineserver','-k'],expected=(0,1))
                run('owned-prefix-retired',['wineserver','-w'],timeout=20)
            except Exception as error:report.update(success=False,retirement_error=repr(error))
            finally:lock.close()
        report['source_inputs']=len(report['sources'])
        digest=write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=str(report_path),sha256=digest,tests_run=report['tests_run'],source_inputs=report['source_inputs'],error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
