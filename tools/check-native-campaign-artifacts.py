#!/usr/bin/env python3
"""Validate saved gameplay artifacts independently of the retained FPS failure."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError('Output already exists')
    campaign = json.loads(args.report.read_text())
    out, experiment = args.report.parent, Path(campaign['experiment'])
    sources = {p: sha(ROOT / p) for p in [
        'tools/check-native-campaign-artifacts.py', 'tools/test-native-campaign.py',
        'tools/campaign_gameplay.py', 'tools/campaign_spell.py',
        'tools/campaign_movement.py', 'tools/campaign_crowded.py',
        'tools/native_audio_launch.py']}
    for p, h in sources.items():
        if p != 'tools/check-native-campaign-artifacts.py' and campaign['sources'].get(p) != h:
            raise ValueError('Gameplay validator differs from live execution: ' + p)
    inputs = {str(p): sha(p) for p in [args.report, out / 'flow.json', out / 'input-0.json',
              experiment / 'events.bin', experiment / 'channel.bin',
              experiment / 'gameplay-events.bin', experiment / 'spell-events.bin']}
    result = dict(success=False, scope=__doc__, sources=sources, inputs=inputs,
                  campaign_success=campaign['success'], campaign_error=campaign.get('error'),
                  performance_threshold_unchanged=True)
    try:
        spec = importlib.util.spec_from_file_location('campaign_validation', ROOT / 'tools/test-native-campaign.py')
        helper = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(helper)
        flow = json.loads((out / 'flow.json').read_text())
        submitted = json.loads((out / 'input-0.json').read_text())
        result['engine'] = helper.validate_events(experiment, campaign['menu_cycles'],
                                                  campaign['difficulty'], campaign['normal_quit_required'])
        result['dialogue'] = helper.validate_dialogue_readiness(submitted, out, resident_world=True)
        result['gameplay'] = helper.validate_casting_combat(experiment, submitted, out)
        result['spell_cases'] = helper.validate_spell_cases(
            helper.read_spell_rows(experiment / 'spell-events.bin', complete=True),
            helper.read_rows(experiment / 'gameplay-events.bin', complete=True), submitted)
        result['portrait'] = helper.validate_portrait_stress(submitted, flow, out,
            campaign['portrait_stress_seconds'], campaign['minimum_fps'])
        result.update(casting_verified=True, combat_verified=True,
                      movement_verified=result['gameplay']['movement']['verified'],
                      functional_flow_success=flow['success'], functional_checks_passed=True)
        helper.validate_flow(flow, campaign['menu_cycles'], campaign['stress_seconds_per_phase'],
                             campaign['minimum_fps'], campaign['normal_quit_required'])
        result.update(success=True, casting_verified=True, combat_verified=True,
                      movement_verified=result['gameplay']['movement']['verified'],
                      functional_flow_success=flow['success'])
    except (OSError, ValueError, RuntimeError) as error:
        result['error'] = str(error)
        if result.get('functional_checks_passed'):
            result['performance_error'] = str(error)
    finally:
        result['sources_stable'] = all(sha(ROOT / p) == h for p, h in sources.items())
        result['inputs_stable'] = all(sha(Path(p)) == h for p, h in inputs.items())
        result['success'] &= result['sources_stable'] and result['inputs_stable']
        args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(args.output)
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
