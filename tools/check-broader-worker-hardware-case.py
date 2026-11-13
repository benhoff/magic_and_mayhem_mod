#!/usr/bin/env python3
"""Require actual NVIDIA identity in both independently compared fault epochs."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

from coverage_evidence import Reader, write_report
from native_render_driver import require_nvidia
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/broader-player-acceptance-results-20261110/'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('acquisition', 'comparison', 'report'):
        parser.add_argument('--'+name, required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh immutable hardware validation required')
    spec = importlib.util.spec_from_file_location('broader_fault', ROOT/'tools/check-broader-worker-player-case.py')
    base = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(base)
    result = dict(schema=1, kind='broader_current_hardware_fault_case', success=False,
                  current_whole_player_proved=False, supported_player_accepted=False,
                  original_recovery_complete=False, physical_driver_reset_proved=False)
    with Reader(ROOT, memoize=True) as reader:
        try:
            result.update(base.assess(reader, args.acquisition, args.comparison))
            a = reader.json(args.acquisition)
            provider = a['provider_adaptation']
            drivers = [e['resident']['renderer'] for e in a['proof']['epochs']]
            require(a['configuration']['software_flag'] is False and provider['provider'] == 'nvidia'
                    and provider['actual_drivers'] == drivers and len(drivers) == 2,
                    'Hardware identity differs from actual resident epochs')
            result['hardware_identity'] = require_nvidia(drivers)
            receipts = provider['host_receipts']
            environments = provider['actual_qt_environments']
            require(len(receipts) == len(environments) == 1, 'Missing exact owned GPU host')
            receipt = receipts[0]
            require(receipt['physical_output_connected'] is False and receipt['physical_device_reset'] is False
                    and '-novtswitch' in receipt['command'] and '-noreset' in receipt['command'],
                    'Unreviewed physical display/reset scope')
            require(environments[0]['DISPLAY'] == receipt['display']
                    and environments[0]['XAUTHORITY'] == receipt['authority']
                    and environments[0]['LIBGL_ALWAYS_SOFTWARE'] is None
                    and environments[0]['GALLIUM_DRIVER'] == 'zink', 'Wrong actual Qt provider environment')
            from retained_artifacts import read_artifact
            name = str((Path(a['temporary_root'])/'host/owner.json').relative_to(ROOT))
            raw = read_artifact(reader, a, name, max_bytes=1024**2, label='NVIDIA host receipt')
            require(json.loads(raw) == receipt, 'Owned GPU receipt changed')
            names = ('tools/check-broader-worker-hardware-case.py', 'tools/native_render_driver.py',
                     'tools/broader_native_host.py', 'tools/run-broader-recovery.py')
            result['sources'].update({n: reader.digest(n) for n in names})
            require(all(reader.digest(n) == h for n, h in result['sources'].items()),
                    'Hardware assertion source drift')
            result.update(success=True, sources_stable=True, provider_host=receipt,
                          hardware_driver_equivalence_to_original=False)
        except (ValueError, KeyError, TypeError, OSError) as error:
            result['error'] = str(error)
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), sha256=sha)), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
