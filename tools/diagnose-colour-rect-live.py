#!/usr/bin/env python3
"""Run the ordinary campaign validator with bounded colour refusal diagnostics.

All arguments belong to test-native-campaign.py. Diagnostics do not alter its
route admission, forwarding policy or successful-journey requirements.
"""
import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    spec = importlib.util.spec_from_file_location('colour_rect_live_campaign', ROOT / 'tools/test-native-campaign.py')
    campaign = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(campaign)
    campaign.SOURCES.append('tools/diagnose-colour-rect-live.py')
    ordinary = campaign.clean_environment

    def environment(out):
        env = ordinary(out)
        env['MNM_COLOUR_RECT_DIAGNOSTICS'] = '1'
        return env

    campaign.clean_environment = environment
    return campaign.main()


if __name__ == '__main__':
    raise SystemExit(main())
