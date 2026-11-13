"""Verify an explicit derived compiler input at the native-only startup boundary."""
import ast,hashlib,inspect,json,os,sys
from pathlib import Path
import native_only_contract as original
from coverage_evidence import Reader,write_report
from broader_secondary_scene_variant import verify_variant
from broader_secondary_subprocess import configuration as preparer_configuration,FLAG
from secondary_world_caller_policy import BASE
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
POLICY='explicit-derived-native-only-inputs-v1'
HELPER='tools/broader_derived_admission.py'
STARTUP='patches/secondary-derived-admission-startup/sitecustomize.py'
MIRRORS='working/broader-player/derived-compiler-inputs'
ORIGINAL_ADMIT=original.admit


def mirror_path(root,digest):
    require(len(digest)==64 and all(c in '0123456789abcdef'for c in digest),'Invalid compiled input identity')
    return Path(root)/MIRRORS/(digest+'.h')


def materialize_input(root,variant_ref):
    root=Path(root)
    with Reader(root,memoize=True)as reader:
        proof=verify_variant(reader,root,variant_ref,materialized=True)
        build=reader.json(variant_ref['record']);raw=reader.read_bytes(build['derived_header']['record'])
    path=mirror_path(root,proof['actual_compiled_header_sha256']);path.parent.mkdir(parents=True,exist_ok=True)
    require(not any(p.is_symlink()for p in [path.parent,*path.parent.parents]if p==root or root in p.parents),
            'Derived input mirror traverses an alias')
    if not path.exists():
        with path.open('xb')as stream:stream.write(raw)
    require(path.is_file()and not path.is_symlink()and path.stat().st_nlink==1 and path.read_bytes()==raw,
            'Preserve a differing derived input mirror')
    pin=path.parent/'.retain'
    if not pin.exists():pin.write_bytes(b'Exact materialized compiled header bytes. Retain while prospective/native admission consumes them; original physical header remains unchanged.\n')
    return dict(path=path.relative_to(root).as_posix(),sha256=hashlib.sha256(raw).hexdigest(),
                source=build['derived_header'],bytes=len(raw))


def checked_configuration(reader,root,phase_ref,report):
    require(reader.digest(phase_ref['record'])==phase_ref['sha256'],'Derived admission phase changed')
    phase=reader.json(phase_ref['record']);cfg=preparer_configuration(root,phase,report,reader.digest)
    selection=phase['derived_native_admission']
    require(selection['policy']==POLICY and selection['helper']==HELPER and selection['startup']==STARTUP
            and selection['source_waivers']==[] and all(reader.digest(n)==h for n,h in selection['sources'].items()),
            'Derived admission source/selection changed')
    proof=verify_variant(reader,root,phase['scene_runtime_variant'],materialized=True)
    build=reader.json(phase['scene_runtime_variant']['record']);mirror=mirror_path(root,proof['actual_compiled_header_sha256'])
    require(mirror.resolve()==mirror and mirror.is_file()and not mirror.is_symlink()and mirror.stat().st_nlink==1
            and mirror.read_bytes()==reader.read_bytes(build['derived_header']['record']),
            'Exact declared compiler input is not materialized')
    return phase,cfg,proof,build,mirror


def selected_source(source):
    tree=ast.parse(source);matches=[]
    wanted=ast.dump(ast.parse('source = (repository / path).resolve()').body[0],include_attributes=False)
    class Select(ast.NodeTransformer):
        def visit_Assign(self,node):
            if ast.dump(node,include_attributes=False)==wanted:
                matches.append(node);return ast.copy_location(ast.parse('source = resolve_compiled_input(root, repository, name, build, path, expected)').body[0],node)
            return self.generic_visit(node)
    tree=Select().visit(tree);require(len(matches)==1,'Original native-only source resolution changed')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'


def compiler_sources(name,build):
    declared={}
    for field in ('sources','compiled_sources','protocol_sources'):
        for path,expected in build.get(field,{}).items():
            if '/'not in path and name in('bridge','render-bridge'):
                prefix='protocols/include/mnm'if name=='bridge'and path.startswith('menu_v')else'runtime/menu'if name=='bridge'else'runtime/render'
                path=prefix+'/'+path
            require(path not in declared or declared[path]==expected,'Conflicting compiler source identity')
            declared[path]=expected
    return declared


def admit_derived(reader,root,phase_ref,report,experiment,repository,metadata,environment):
    root,repository,experiment=Path(root).resolve(),Path(repository).resolve(),Path(experiment).resolve()
    require(repository==root and experiment.is_relative_to(root/'working/experiments/menu-observer'),
            'Foreign derived-admission root/session')
    phase,cfg,proof,built,mirror=checked_configuration(reader,root,phase_ref,report)
    raw=(experiment/'scene-bridge-build.json').read_bytes()
    require(raw==reader.read_bytes(built['compiled_manifest']['record'])
            and hashlib.sha256((experiment/'game/MnmScene.dll').read_bytes()).hexdigest()==proof['scene_sha256'],
            'Actual staged scene/manifest differs from declared compiled input')
    compiled=json.loads(raw)
    def resolve_compiled_input(stage,repo,name,build,path,expected):
        if name=='scene-bridge'and path==BASE:
            require(stage==experiment and repo==repository and build==compiled
                    and expected==proof['actual_compiled_header_sha256']
                    and compiled['physical_derivation_inputs'][BASE]==reader.digest(BASE)==proof['physical_header_sha256']
                    and mirror.read_bytes()==reader.read_bytes(built['derived_header']['record']),
                    'Derived compiler input relation changed during admission')
            return mirror.resolve()
        require((name=='scene-bridge'and build==compiled)or not build.get('source_overrides'),'Only the exact declared scene header may be overridden')
        return(repo/path).resolve()
    # Keep every original capability, binary, source-format/path and terminal
    # guard. The only selected AST node resolves the actual checked compiler
    # input bytes instead of assuming a virtual compiler path is physical.
    namespace=dict(ORIGINAL_ADMIT.__globals__,resolve_compiled_input=resolve_compiled_input)
    source=selected_source(inspect.getsource(ORIGINAL_ADMIT));exec(compile(source,str(root/HELPER),'exec'),namespace)
    result=namespace['admit'](experiment,repository,metadata,environment)
    actual_by_binary={}
    for name,(dll,key)in original.BUILDS.items():
        build=json.loads((experiment/(name+'-build.json')).read_bytes())
        actual_by_binary[dll]=compiler_sources(name,build)
    require(actual_by_binary['MnmScene.dll'][BASE]==proof['actual_compiled_header_sha256'],
            'Actual scene compiler closure omitted derived input')
    physical=dict(result['sources']);physical[BASE]=reader.digest(BASE)
    require(all(reader.digest(path)==expected for path,expected in physical.items()),
            'Physical compiler derivation inputs changed')
    # The legacy sources field remains a verified physical dependency map.
    # Per-binary actual compiled inputs are separate and authoritative: one
    # logical header can have different compiled versions in different DLLs.
    result['sources']=physical
    result.update(source_identity_policy=POLICY,
        sources_semantics='Verified physical compiler derivation inputs. Actual compiled inputs are recorded separately per binary.',
        actual_compiled_sources_by_binary=actual_by_binary,
        derived_compiler_input=dict(logical_path=BASE,physical_sha256=proof['physical_header_sha256'],
            compiled_sha256=proof['actual_compiled_header_sha256'],materialized_path=mirror.relative_to(root).as_posix(),
            exact_compiled_bytes=built['derived_header'],build=phase['scene_runtime_variant'],
            phase=phase_ref,acquisition_path=report,source_overrides=compiled['source_overrides'],
            compiled_manifest=built['compiled_manifest']),
        original_admission_predicates_preserved=True,source_hash_waivers=[],
        selected_source_resolution_sha256=hashlib.sha256(source.encode()).hexdigest())
    return result


def startup(root,config):
    root=Path(root)
    require(Path(sys.argv[0]).resolve()==root/'tools/owned-menu-game-runner.py','Only the owned pre-Wine runner may select derived admission')
    with Reader(root,memoize=True)as reader:
        phase,cfg,proof,built,mirror=checked_configuration(reader,root,config['phase'],config['acquisition'])
        require(config==cfg|{'phase':config['phase']},'Foreign derived startup configuration')
    def selected(experiment,repository,metadata,environment):
        with Reader(root,memoize=True)as reader:
            result=admit_derived(reader,root,config['phase'],config['acquisition'],experiment,repository,metadata,environment)
            cases=[c for c in phase['broader_cases']if c['report']==config['acquisition']];require(len(cases)==1,'Unknown derived-admission case')
            name=P+cases[0]['id']+'-actual-derived-admission-01.json';require(not(root/name).exists(),'Never repeat startup admission automatically')
            q=dict(schema=1,kind='actual_owned_runner_derived_compiler_input_admission',success=True,
                phase=config['phase'],acquisition_path=config['acquisition'],session=str(Path(experiment).resolve()),
                native_only=result,Original_started_by_admission=False,runner_pid=os.getpid(),
                sources={n:reader.digest(n)for n in [HELPER,STARTUP,'tools/native_only_contract.py']},sources_stable=True)
            write_report(root,name,(json.dumps(q,indent=2)+'\n').encode())
        return result
    original.admit=selected
