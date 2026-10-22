#!/usr/bin/env python3
"""Validate saved gameplay artifacts independently of retained performance failures."""
import argparse,ast,math
import importlib.util
import json
from pathlib import Path
from coverage_evidence import evidence_digest, read_json, write_report

ROOT = Path(__file__).resolve().parents[1]
VALIDATORS = ['tools/test-native-campaign.py', 'tools/campaign_gameplay.py',
              'tools/campaign_spell.py', 'tools/campaign_movement.py',
              'tools/campaign_crowded.py', 'tools/native_audio_launch.py']


def sha(path):
    path=Path(path)
    return evidence_digest(path, root=ROOT if path.resolve().is_relative_to(ROOT) else None)


def validator_sources():
    """Pin local Python imports used by the saved validators, including helpers."""
    pending=set(VALIDATORS);seen=set()
    while pending:
        name=pending.pop()
        if name in seen:continue
        seen.add(name)
        for node in ast.walk(ast.parse((ROOT/name).read_text())):
            imports=([a.name for a in node.names] if isinstance(node,ast.Import) else [node.module] if isinstance(node,ast.ImportFrom) and node.module else [])
            for module in imports:
                candidate=ROOT/'tools'/Path(*module.split('.')).with_suffix('.py')
                if candidate.is_file():pending.add(candidate.relative_to(ROOT).as_posix())
    return sorted(seen)


def validate_saved(campaign, flow, submitted, out, experiment, helper):
    """Retain independent successes and all failures, including both rate gates."""
    floor=campaign['minimum_fps']
    if type(floor) not in (int,float) or not math.isfinite(floor) or not 1<=floor<=120:
        raise ValueError('Invalid retained performance threshold')
    result = dict(success=False, campaign_success=campaign['success'],
                  campaign_error=campaign.get('error'), errors={},
                  performance_threshold_unchanged=True, functional_flow_success=flow['success'])
    checks = {
        'engine': lambda: helper.validate_events(experiment, campaign['menu_cycles'], campaign['difficulty'], campaign['normal_quit_required']),
        'dialogue': lambda: helper.validate_dialogue_readiness(submitted, out, resident_world=True),
        'gameplay': lambda: helper.validate_casting_combat(experiment, submitted, out),
        'spell_cases': lambda: helper.validate_spell_cases(helper.read_spell_rows(experiment/'spell-events.bin', complete=True), helper.read_rows(experiment/'gameplay-events.bin', complete=True), submitted),
        'portrait': lambda: helper.validate_portrait_stress(submitted, flow, out, campaign['portrait_stress_seconds'], campaign['minimum_fps']),
        'flow': lambda: helper.validate_flow(flow, campaign['menu_cycles'], campaign['stress_seconds_per_phase'], campaign['minimum_fps'], campaign['normal_quit_required']),
    }
    for name, check in checks.items():
        try:
            result[name] = check()
        except (OSError, ValueError, RuntimeError, KeyError, TypeError) as error:
            result['errors'][name] = str(error)
    gates = campaign.get('ordinary_resident_gates')
    if gates is not None:
        result['retained_resident_gates'] = gates
        if gates.get('fps_passed') is not True or gates.get('worlds_passed') is not True or gates.get('minimum_fps') != campaign['minimum_fps']:
            result['errors']['resident_performance'] = 'Retained resident engine floor/completion gate failed or changed'
    result.update(casting_verified='gameplay' not in result['errors'],
                  combat_verified='gameplay' not in result['errors'],
                  movement_verified=result.get('gameplay', {}).get('movement', {}).get('verified') is True,
                  functional_checks_passed=not any(k in result['errors'] for k in ('engine','dialogue','gameplay','spell_cases')))
    result['success'] = campaign['success'] is True and not result['errors'] and flow['success'] is True
    if not result['success']:
        result['error']=campaign.get('error') or next(iter(result['errors'].values()),'Retained capture failed')
    performance=[result['errors'][k] for k in ('portrait','flow','resident_performance') if k in result['errors']]
    if performance:result['performance_error']='; '.join(performance)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError('Output already exists')
    campaign = read_json(args.report, root=ROOT)
    out, experiment = args.report.parent, Path(campaign['experiment'])
    validators=validator_sources()
    sources = {p: sha(ROOT/p) for p in ['tools/check-native-campaign-artifacts.py', *validators]}
    for p in validators:
        if campaign['sources'].get(p) != sources[p]:
            raise ValueError('Gameplay validator differs from live execution: '+p)
    inputs = {str(p): sha(p) for p in [args.report, out/'flow.json', out/'input-0.json', experiment/'events.bin', experiment/'channel.bin', experiment/'gameplay-events.bin', experiment/'spell-events.bin']}
    spec = importlib.util.spec_from_file_location('campaign_validation', ROOT/'tools/test-native-campaign.py')
    helper = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(helper)
    result = validate_saved(campaign, read_json(out/'flow.json', root=ROOT), read_json(out/'input-0.json', root=ROOT), out, experiment, helper)
    result.update(scope=__doc__, sources=sources, inputs=inputs,
                  sources_stable=all(sha(ROOT/p)==h for p,h in sources.items()),
                  inputs_stable=all(sha(Path(p))==h for p,h in inputs.items()))
    result['success'] &= result['sources_stable'] and result['inputs_stable']
    write_report(ROOT,args.output,(json.dumps(result,indent=2)+'\n').encode())
    print(args.output)
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
