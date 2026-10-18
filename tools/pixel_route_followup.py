"""Account for the fixed27-route cohort without converting controls into callers."""
from pixel_route_requirements import ALL_ROUTES, validate_routes
from pixel_route_matrix import comparison_matrix

TARGETS = (
    'pixel/rgb_add', 'pixel/pcx_words', 'pixel/brighten', 'pixel/row_copy',
    'minimap/creature', 'minimap/camera1', 'minimap/camera2', 'minimap/camera3',
    'colour_rect/rectangle',
    *(f'raster/splice{i}' for i in (0, 1, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22)),
)
ALTERNATE = tuple(f'raster/splice{i}' for i in (0, 1, 4, 5, 11, 12, 14))
UNRESOLVED = ('pixel/pcx_words', 'pixel/brighten', 'pixel/row_copy', 'raster/splice15', 'minimap/camera3')
PHYSICAL = tuple(r for r in TARGETS if r not in ALTERNATE + UNRESOLVED)


def campaign_routes(report, required):
    for key in ('success', 'sources_unchanged', 'inputs_stable', 'private_wine_stopped', 'original_manifest_verified_after'):
        if report.get(key) is not True:
            raise ValueError('Campaign lacks acceptance/provenance: ' + key)
    if report.get('normal_quit_required') is not True or report.get('cpu_census_only'):
        raise ValueError('Ordinary normal-Quit campaign required')
    rows = validate_routes(report['pixel_producers'], required)
    return {family + '/' + row['route']: row['native']
            for family, cohort in report['pixel_producers']['families'].items() for row in cohort}, rows


def summarize(physical, alternate, cpu, comparisons, caller_review):
    """Inputs must additionally be hash/source verified by the acquisition caller."""
    ordinary, ordinary_rows = campaign_routes(physical, PHYSICAL)
    authored, authored_rows = campaign_routes(alternate, ALTERNATE)
    if (cpu.get('verified') is not True or cpu.get('base_family') != 15 or
            cpu.get('executable_bytes_unchanged') is not True or
            cpu.get('dispatch_pointers_written_by_debugger') is not False or cpu.get('child_exit') != 0):
        raise ValueError('Missing unchanged original CPU-selection witness')
    if cpu.get('targets') != [0x5947b2, 0x595b47, 0x5968a4, 0x597086] or cpu.get('override') == 1:
        raise ValueError('Wrong original alternate raster dispatch')
    matrix = comparison_matrix(comparisons, TARGETS)
    compared = {r['route']: r for r in matrix['routes']}
    if caller_review.get('success') is not True or caller_review.get('global_unreachable_proved') is not False:
        raise ValueError('Caller review must retain unknown indirect/external flows')
    caller_rows = {row['route']: row for row in caller_review['routes']}
    if len(caller_review['routes']) != 5 or set(caller_rows) != set(UNRESOLVED):
        raise ValueError('Missing unresolved caller window')
    rows = []
    for route in TARGETS:
        if route not in compared:
            raise ValueError('Target lacks independent production-splice comparison: ' + route)
        live = bool(ordinary[route] or authored[route])
        rows.append(dict(route=route, physical_calls=ordinary[route], authored_cpu_calls=authored[route],
                         live_executed=live, independent_original=compared[route],
                         caller_boundary=caller_rows.get(route),
                         classification='original_game_caller' if live else 'controlled_original_only_caller_unresolved'))
    return dict(target_count=len(TARGETS), denominator=len(ALL_ROUTES), routes=rows,
                live_targets=[r['route'] for r in rows if r['live_executed']],
                remaining_live=[r['route'] for r in rows if not r['live_executed']],
                all_targets_live=all(r['live_executed'] for r in rows),
                all_targets_compared=True,
                all_routes_live_union=sorted(set(ordinary_rows['exercised']) | set(authored_rows['exercised'])),
                complete_replacement=False,
                scope='Original game callers across separate CPU contexts; independent fixture comparison and unresolved caller windows remain separate. No forced dispatch or global dead-code claim.')
