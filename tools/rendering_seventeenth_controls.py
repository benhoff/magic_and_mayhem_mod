"""Fixed acceptance of the five remaining bounded rendering contracts."""
import ast
from pathlib import Path
from coverage_evidence import evidence_digest

BEHAVIORS = ['NR.world-continuous-source', 'NR.world-continuous-normal-performance',
             'NR.public-resident-world', 'NP.native-output-ownership',
             'NP.native-only-launch-contract']
SCENARIO = 'NR.seventeenth-five.current-rendering-20261023'
STAGES = ('controls', 'verification', 'normal', 'impossible', 'consumer-death',
          'producer-death', 'recovery', 'owned-campaign', 'strict-campaign',
          'strict-restart', 'public-implementation')
PYTHON_TESTS = ['tests/test-rendering-seventeenth-controls.py',
                'tests/test-rendering-session-contracts.py',
                'tests/test-native-output-owner.py', 'tests/test-native-only-contract.py',
                'tests/test-native-campaign.py', 'tests/test-native-cpu-store-profile.py']
TOOLS = ['tools/test-native-world-continuous.py', 'tools/test-native-output-owner.py',
         'tools/test-native-backing-owner.py', 'tools/test-native-only.py',
         'tools/test-native-owned-campaign.py', 'tools/test-native-campaign.py',
         'tools/build-native-draw.py', 'tools/run-rendering-hardware-control.py',
         'tools/test-rendering-seventeenth-five.py', 'tools/rendering_seventeenth_controls.py']


def source_closure(root, declaration):
    """Prospective compiler/fixture/Python inputs; every spelling stays contained."""
    from rendering_session_contracts import declared_source
    todo = set(declaration['sources']) | set(PYTHON_TESTS) | set(TOOLS)
    for directory in ('runtime', 'assets', 'audio', 'renderer', 'compat/legacy',
                      'apps', 'protocols', 'reconstruction', 'game', 'tests'):
        todo.update(p.relative_to(root).as_posix() for p in (root/directory).rglob('*')
                    if p.is_file() and (p.suffix in {'.c', '.h', '.cpp', '.hpp', '.S', '.inc', '.cmake'}
                                       or p.name == 'CMakeLists.txt'))
    todo.update(p.relative_to(root).as_posix() for p in (root/'protocols').rglob('*.json'))
    todo.update(p.relative_to(root).as_posix() for p in (root/'tools/native-render-profile').rglob('*')
                if p.is_file() and p.suffix in {'.h', '.hpp', '.cpp', '.json'})
    seen = set()
    while todo:
        name = todo.pop()
        if name in seen:
            continue
        seen.add(name)
        path = declared_source(root, name)
        declaration['sources'][name] = evidence_digest(path, root=root)
        if path.suffix != '.py':
            continue
        for node in ast.walk(ast.parse(path.read_text())):
            names = []
            if isinstance(node, ast.Import):
                names = [alias.name for alias in node.names]
            elif isinstance(node, ast.ImportFrom) and node.module:
                names = [node.module] + [node.module+'.'+alias.name for alias in node.names]
            for imported in names:
                relative = Path(*imported.split('.'))
                for base in (root/'tools', root/'protocols/python'):
                    for candidate in (base/relative.with_suffix('.py'), base/relative/'__init__.py'):
                        if candidate.is_file():
                            todo.add(candidate.relative_to(root).as_posix())
            if isinstance(node, ast.Constant) and isinstance(node.value, str) and node.value.endswith('.py'):
                for candidate in (root/node.value, path.parent/node.value, root/'tools'/node.value, root/'tests'/node.value):
                    if candidate.is_file() and candidate.resolve().is_relative_to(root):
                        todo.add(candidate.resolve().relative_to(root).as_posix())
    return declaration


def require_continuous(value, stage):
    """A failed run earns credit only for its exact, independently checked control."""
    for key in ('sources_stable', 'frozen_sources_stable', 'generated_sources_stable',
                'supplied_declaration_stable', 'inputs_stable', 'original_manifest_verified_before_after'):
        if value.get(key) is not True:
            raise ValueError('Continuous provenance failed: '+key)
    if value.get('admission_policy', {}).get('functional_only') is not False:
        raise ValueError('Full continuous contracts require ordinary admission')
    if value.get('resident'):
        raise ValueError('Pixel verification/performance contracts require their pixel-reply path')
    if stage in ('consumer-death', 'producer-death'):
        if not (value.get('success') is True and value.get('mode') == stage
                and value.get('expected_terminal_failure') is True
                and value.get('completed_success_claimed') is False
                and value.get('fault_generation', 0) >= 34):
            raise ValueError('Missing actual post-prefix peer refusal')
        return
    native = value.get('completed', {}).get('native_continuous', {})
    count = 4096 if stage in ('verification', 'normal') else 128
    if not (value.get('requested_queues') == count and native.get('success') is True
            and int(native.get('original_returns', -1)) == count
            and int(native.get('native_completions', -1)) == count
            and native.get('remaining_surfaces') == 0
            and int(native.get('qt_paints', -1)) == count
            and int(native.get('coalesced_presentations', -1)) == 0):
        raise ValueError('Incomplete continuous prefix')
    verifying = stage == 'verification'
    if native.get('verify') != verifying or int(native.get('cpu_reference_queues', -1)) != (count if verifying else 0):
        raise ValueError('Wrong verification/normal execution path')
    if stage == 'impossible':
        budget = value.get('performance_budget', {})
        if not (value.get('success') is False and value.get('error') == 'Full-path performance budget exceeded'
                and budget.get('passed') is False and budget.get('impossible_negative_control') is True
                and budget.get('median_limit_ms') == .001
                and budget.get('checks', {}).get('warm_median') is False):
            raise ValueError('Impossible budget did not independently refuse valid completed rendering')
    elif value.get('success') is not True or (not verifying and value.get('performance_budget', {}).get('passed') is not True):
        raise ValueError('Continuous acceptance failed')


def public_portrait_pixels(inputs, flow, capture_root):
    """Check retained original/native face bytes independently of ordinary timing.

    Reexecute the unchanged ordinary portrait validator too. A failed timing
    result is retained; it cannot prevent an independent pixel comparison.
    """
    import hashlib,importlib.util,io,re
    from PIL import Image,ImageChops
    from coverage_evidence import read_bytes
    captures=[c for c in inputs.get('captures',[]) if c.get('name','').endswith('-portrait-stress')]
    if len(captures)!=1:raise ValueError('Missing unique portrait capture')
    capture=captures[0];name=capture['name'];metadata=capture['metadata']
    if not re.fullmatch(r'combat-\d{2}-[a-z-]+',name) or metadata.get('fallback') is not False or not all(metadata.get(k) is True for k in ('native_saved','original_saved')):
        raise ValueError('Unsafe or incomplete independent portrait capture')
    images=[];hashes={}
    for route in ('native','original'):
        raw=read_bytes(capture_root/(name+'-'+route+'.png'))
        hashes[route]=hashlib.sha256(raw).hexdigest()
        if hashes[route]!=capture['image_sha256'][route]:raise ValueError('Portrait image changed')
        with Image.open(io.BytesIO(raw)) as image:
            if image.width<759 or image.height<538:raise ValueError('Portrait image extent too small')
            images.append(image.convert('RGB').crop((716,507,759,538)))
    maximum=max(hi for lo,hi in ImageChops.difference(*images).getextrema())
    if maximum>1:raise ValueError('Native portrait face differs from independent original pixels')
    spec=importlib.util.spec_from_file_location('implementation_portrait_campaign',Path(__file__).with_name('test-native-campaign.py'))
    campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
    timing=None;timing_error=None
    try:timing=campaign.validate_portrait_stress(inputs,flow,capture_root,10,19.)
    except ValueError as error:
        if str(error)!='Portrait rendering too slow':raise
        timing_error=str(error)
    return dict(verified=True,maximum_channel_error=maximum,portrait_region=[716,507,759,538],
                image_sha256=hashes,ordinary_minimum_fps=19,ordinary_timing=timing,
                ordinary_timing_error=timing_error,
                scope='Independent retained stable-face pixel comparison only; all unchanged ordinary portrait timing checks retain their separate outcome. No full World/frame equivalence or performance credit.')


def require_public_implementation(value, *, flow=None, portrait_pixels=None):
    """Implementation credit only; never converts a failed19FPS harness to a pass.

    Accept an ordinary pass, or its independently reproduced timing refusal after every
    earlier functional, portrait, flow, native ownership and cleanup check ran.
    Other failures cannot supply this implementation milestone. Ordinary timing
    checks are rerun unchanged; complete functional flow is checked separately.
    """
    import math
    try:
        for key in ('casting_verified','combat_verified','movement_verified','normal_quit_verified',
                    'sources_unchanged','inputs_stable','original_manifest_verified_after','private_wine_stopped'):
            if value.get(key) is not True:raise ValueError('Missing '+key)
        if value.get('changed_sources')!=[] or any(value.get(k) for k in ('cleanup_error','source_stability_error','profile_error')):
            raise ValueError('Changed/unclean execution')
        if value.get('spell_cases',{}).get('verified') is not True:raise ValueError('Missing original spell cases')
        portrait=value.get('portrait')
        if portrait_pixels is not None:
            if portrait_pixels.get('verified') is not True or portrait_pixels.get('maximum_channel_error',256)>1 or portrait_pixels.get('ordinary_minimum_fps')!=19:
                raise ValueError('Independent portrait pixels incomplete')
        elif not isinstance(portrait,dict) or portrait.get('maximum_channel_error',256)>1:
            raise ValueError('Portrait pixels incomplete')
        resident=value['resident_world'];counts=[int(resident[k]) for k in ('completed','imports','verified_returns')]
        if min(counts)<64 or len(set(counts))!=1 or not resident['normal_quit_retired'] or resident['pending'] or resident['error']:
            raise ValueError('Unbalanced resident retirement')
        if any(int(resident[k]) for k in ('world_readbacks','native_readbacks','rgba_readbacks','world_pixel_reply_bytes')):
            raise ValueError('Resident pixel/ownership boundary changed')
        fps=float(resident['engine_fps']);gates=value['ordinary_resident_gates']
        if not math.isfinite(fps) or fps<=0 or gates['minimum_fps']!=19 or not gates['worlds_passed'] or gates['fps_passed']!=(fps>=19):
            raise ValueError('Ordinary performance gate changed')
        if value.get('success') is True:
            if value.get('ordinary_campaign_accepted') is not True or fps<19:raise ValueError('False ordinary acceptance')
        elif value.get('success') is False and value.get('ordinary_campaign_accepted') is False:
            if value.get('error')=='Resident World engine FPS below target: '+str(fps) and fps<19:
                pass
            elif value.get('error')=='Portrait rendering too slow':
                if not isinstance(flow,dict) or portrait_pixels is None or portrait_pixels.get('ordinary_timing_error')!=value['error']:
                    raise ValueError('Portrait timing failure lacks independent pixels and reproduced timing')
                import importlib.util
                spec=importlib.util.spec_from_file_location('implementation_portrait_flow',Path(__file__).with_name('test-native-campaign.py'))
                campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
                campaign.validate_flow(flow,cycles=1,stress_seconds=0,normal_quit=True)
            else:
                import importlib.util,re
                if not isinstance(flow,dict) or not re.fullmatch(r'Gameplay too slow in cycle [01]: native [0-9]+\.[0-9], paints [0-9]+\.[0-9] FPS; minimum 19\.0',str(value.get('error'))):
                    raise ValueError('Failure outside verified ordinary timing')
                spec=importlib.util.spec_from_file_location('implementation_campaign_flow',Path(__file__).with_name('test-native-campaign.py'))
                campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
                campaign.validate_flow(flow,cycles=1,stress_seconds=0,normal_quit=True)
                try:campaign.validate_flow(flow,cycles=1,stress_seconds=10,min_fps=19.,normal_quit=True)
                except ValueError as error:
                    if str(error)!=value['error']:raise ValueError('Ordinary timing refusal does not match flow') from error
                else:raise ValueError('Ordinary timing did not refuse retained flow')
        else:raise ValueError('Invalid ordinary outcome')
    except (KeyError,TypeError,ValueError) as error:
        raise ValueError('Public implementation evidence refused: '+str(error)) from error
    return dict(stages=['implementation'],implementation_verified=True,
                ordinary_campaign_accepted=value['ordinary_campaign_accepted'],minimum_fps=19,
                observed_engine_fps=fps,aggregate_fps_passed=fps>=19,
                performance_gate_passed=value['ordinary_campaign_accepted'],
                current_live_performance_accepted=value['ordinary_campaign_accepted'],
                scope='Bounded source-bound native implementation only. The complete ordinary harness result is retained unchanged; failed portrait/aggregate/window19FPS acceptance stays pending. No current original/frame/driver/live equivalence or performance promotion.')
