"""Correct-output acceptance, separate from the retained Wine resize negative.

This gate consumes complete externally compared cohorts. It does not change
pixels, tolerances, frame ordinals, reference files, or engine milestones.
"""
from native_render_driver import require_nvidia

POLICY = 'critical-rendering-correct-output-v1'
PROFILES = {'legacy_storage': 238, 'corrected_storage': 222, 'quick_battle': 323}
BACKENDS = ('software', 'hardware')


def assess(reports):
    """Require correct output on both backends and retain the old diagnostic."""
    if set(reports) != set(BACKENDS):
        raise ValueError('Both software and hardware cohorts are required')
    accepted, diagnostics = {}, {}
    for backend in BACKENDS:
        cohort = reports[backend]
        if set(cohort) != set(PROFILES):
            raise ValueError('Missing or additional presentation profile')
        accepted[backend] = {}
        for name, count in PROFILES.items():
            report = cohort[name]
            if (report.get('sources_stable') is not True or
                    report.get('original_reference_admitted') is not True or
                    report.get('original_pixels_used_as_native_inputs') is not False or
                    report.get('original_oracles_read_by_native') is not False or
                    report.get('new_original_execution') is not False or
                    report.get('channel_tolerance') != 1 or
                    report.get('alpha_compared') is not False or
                    report.get('frames') != count or
                    report.get('physical_generation_frames') != count or
                    report.get('painted_before_swap_frames') != count or
                    report.get('hardware_requested') is not (backend == 'hardware')):
                raise ValueError('Incomplete presentation provenance: '+backend+'/'+name)
            drivers = report.get('renderer_identities')
            if not isinstance(drivers, list) or len(drivers) != 2:
                raise ValueError('Both executed renderer identities are required')
            if backend == 'hardware':
                require_nvidia(drivers)
                if report.get('hardware_admission', {}).get('verified') is not True:
                    raise ValueError('Hardware execution was not admitted')
            elif any(not isinstance(d, str) or 'llvmpipe' not in d.casefold() for d in drivers):
                raise ValueError('Software execution did not retain llvmpipe identity')
            rows = report.get('comparisons', [])
            if len(rows) != count or [r['ordinal'] for r in rows] != list(range(1, count+1)):
                raise ValueError('Presentation denominator or ordinals changed')
            total = 0
            pixels = 0
            failures = []
            for row in rows:
                gpu, physical = row['gpu'], row['physical']
                if (type(gpu['pixels']) is not int or gpu['pixels'] <= 0 or
                        physical['pixels'] != gpu['pixels']):
                    raise ValueError('GPU/physical complete-plane denominators differ')
                for plane in (gpu, physical):
                    mismatch = plane['mismatching_pixels']
                    if type(mismatch) is not int or not 0 <= mismatch <= plane['pixels']:
                        raise ValueError('Invalid complete-plane mismatch count')
                total += gpu['mismatching_pixels']+physical['mismatching_pixels']
                pixels += 2*gpu['pixels']
                if gpu['mismatching_pixels'] or physical['mismatching_pixels']:
                    failures.append((row['ordinal'], gpu['mismatching_pixels'],
                                     physical['mismatching_pixels']))
            if report.get('mismatches') != total or report.get('pixels_compared') != pixels:
                raise ValueError('Presentation result summary differs from complete frames')
            if name == 'legacy_storage':
                if (report.get('success') is not False or report.get('outcome') != 'mismatch' or
                        failures != [(170, 126, 126)] or report.get('witness_mismatches') != 0):
                    raise ValueError('Retained legacy resize diagnostic changed')
                diagnostics[backend] = dict(outcome='EXPECTED_DIFFERENCE', frames=count,
                                            mismatches=total, blocking=False,
                                            reason='Unmodified Wine upload artifact; correct output is the selected target.')
                continue
            if (report.get('success') is not True or report.get('outcome') != 'matching' or
                    total or report.get('witness_mismatches') != 0):
                raise ValueError('Correct-output acceptance failed: '+backend+'/'+name)
            if name == 'corrected_storage':
                profile = report.get('reference_profile', {})
                if (profile.get('name') != 'stale_upload_crop' or
                        profile.get('policies', {}).get('egl_correct_stale_upload_policy') is not True or
                        report.get('witness_frames') != count):
                    raise ValueError('Corrected reference lacks its explicit policy and witnesses')
                for row in rows:
                    if (row.get('source', {}).get('mismatching_pixels') != 0 or
                            any(row.get('destination', {}).get(key) != 0 for key in
                                ('native_destination_mismatching_words',
                                 'source_destination_mismatching_words',
                                 'destination_physical_mismatching_pixels'))):
                        raise ValueError('Correct-output source/destination witness failed')
            accepted[backend][name] = dict(outcome='PASS', frames=count,
                                           mismatches=0, pixels_compared=pixels)
    return dict(schema=1, success=True, policy=POLICY,
                acceptance_target='Correct native output against the explicit corrected-driver profile',
                accepted_profiles=accepted, legacy_diagnostics=diagnostics,
                whole_renderer_accepted=False, all_critical_behaviors_accepted=False,
                limits=['Recorded cohorts only; no new original hardware execution.',
                        'Whole-game route/writer/alias, context-loss, performance and replacement acceptance remain separate.'])
