"""Join actual pre-Wine derived admission to a complete current acquisition."""
import hashlib,inspect
from pathlib import Path
from broader_derived_admission import checked_configuration,selected_source,ORIGINAL_ADMIT,P,POLICY,BASE
from native_only_contract import BUILDS
from rendering_pilot import require


def verify_admission(reader,root,phase_ref,acquisition_ref):
    require(reader.digest(acquisition_ref['record'])==acquisition_ref['sha256'],'Derived admission acquisition changed')
    acquired=reader.json(acquisition_ref['record'])
    require(acquired['success']and acquired['sources_stable']and acquired['actual_provider_verified']
            and acquired['original_game_runs']==1 and acquired['broader_phase']==phase_ref,
            'Derived admission needs a complete actual current acquisition')
    phase,cfg,proof,built,mirror=checked_configuration(reader,root,phase_ref,acquisition_ref['record'])
    case=next(c for c in phase['broader_cases']if c['report']==acquisition_ref['record'])
    name=P+case['id']+'-actual-derived-admission-01.json';actual=reader.json(name)
    require(actual['kind']=='actual_owned_runner_derived_compiler_input_admission'and actual['success']
            and actual['phase']==phase_ref and actual['acquisition_path']==acquisition_ref['record']
            and actual['session']==acquired['session'] and actual['Original_started_by_admission']is False
            and all(reader.digest(n)==h for n,h in actual['sources'].items()),'Actual pre-Wine admission receipt differs')
    startup=acquired['native_admission'];native=startup['native_only']
    require(startup['session']==acquired['session'] and startup['runner_identity']['pid']==actual['runner_pid']
            and native==actual['native_only'] and native['source_identity_policy']==POLICY
            and native['original_admission_predicates_preserved']is True and native['source_hash_waivers']==[],
            'Executed native-only admission differs from pre-Wine checked result')
    require(native['selected_source_resolution_sha256']==hashlib.sha256(selected_source(inspect.getsource(ORIGINAL_ADMIT)).encode()).hexdigest(),
            'Native-only selected source resolver changed')
    binding=native['derived_compiler_input'];compiled=reader.json(built['compiled_manifest']['record'])
    require(binding==dict(logical_path=BASE,physical_sha256=proof['physical_header_sha256'],
            compiled_sha256=proof['actual_compiled_header_sha256'],materialized_path=mirror.relative_to(Path(root).resolve()).as_posix(),
            exact_compiled_bytes=built['derived_header'],build=phase['scene_runtime_variant'],phase=phase_ref,
            acquisition_path=acquisition_ref['record'],source_overrides=compiled['source_overrides'],
            compiled_manifest=built['compiled_manifest']), 'Compiled versus physical input relation changed')
    require(native['sources'][BASE]==proof['physical_header_sha256']
            and all(reader.digest(n)==h for n,h in native['sources'].items()),'Physical derivation source changed')
    actual_inputs=native['actual_compiled_sources_by_binary']
    require(set(actual_inputs)=={v[0]for v in BUILDS.values()} and set(native['binaries'])==set(actual_inputs)
            and native['binaries']['MnmScene.dll']==proof['scene_sha256']
            and actual_inputs['MnmScene.dll'][BASE]==proof['actual_compiled_header_sha256'],
            'Actual compiled binary/input closure missing')
    overrides=[]
    for dll,sources in actual_inputs.items():
        require(sources,'Empty actual compiler input closure')
        for path,expected in sources.items():
            if dll=='MnmScene.dll'and path==BASE:
                overrides.append((dll,path));require(expected==proof['actual_compiled_header_sha256'],'Derived compiled header changed')
            else:require(reader.digest(path)==expected,'Other actual compiler input changed: '+dll+':'+path)
    require(overrides==[('MnmScene.dll',BASE)],'Changed compiled override domain')
    reference=dict(record=name,sha256=reader.digest(name))
    require(acquired['provider_adaptation']['native_only_derived_admission']==reference,
            'Acquisition omitted actual derived startup admission')
    return dict(verified=True,phase=phase_ref,acquisition=acquisition_ref,actual_owned_runner_admission=reference,
        actual_scene_sha256=proof['scene_sha256'],physical_header_sha256=proof['physical_header_sha256'],
        actual_compiled_header_sha256=proof['actual_compiled_header_sha256'],compiled_override_count=1,
        actual_compiler_inputs_by_binary_verified=True,physical_derivation_inputs_verified=True,
        all_base_admission_guards_preserved_except_checked_input_resolution=True,source_hash_waivers=[],
        original_game_runs=0,new_native_executions=0)
