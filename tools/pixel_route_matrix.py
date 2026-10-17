"""Validate route-specific original comparisons without granting live coverage."""
from native_pixel_counters import ROUTES

FAMILY_ROUTES = {
    'pixel': tuple('pixel/' + r for r in ROUTES['pixel'] if r != 'dib'),
    'minimap': tuple('minimap/' + r for r in ROUTES['minimap']),
    'colour_rect': ('colour_rect/rectangle',),
    'raster': tuple('raster/' + r for r in ROUTES['raster']),
}


def comparison_matrix(comparisons, pending):
    if set(comparisons) != set(FAMILY_ROUTES):
        raise ValueError('Missing or unknown comparison family')
    known = {r for routes in FAMILY_ROUTES.values() for r in routes}
    if len(set(pending)) != len(pending) or not set(pending) <= known:
        raise ValueError('Unknown or duplicate pending route')
    rows = {}
    words = 0
    for family, expected in FAMILY_ROUTES.items():
        comparison = comparisons[family]
        if comparison.get('success') is not True or type(comparison.get('physical_words')) is not int or comparison['physical_words'] <= 0:
            raise ValueError('Comparison did not complete physical output checks: ' + family)
        words += comparison['physical_words']
        cohort = comparison.get('route_coverage', [])
        if len(cohort) != len(expected):
            raise ValueError('Incomplete route cohort: ' + family)
        seen = set()
        for row in cohort:
            route = row['route']
            if route not in expected or route in seen:
                raise ValueError('Wrong or duplicate compared route: ' + route)
            seen.add(route)
            for name in ('native_by_format', 'changed_by_format'):
                counts = row[name]
                if len(counts) != 2 or any(type(n) is not int or n <= 0 for n in counts):
                    raise ValueError('Missing matched native drawing in both formats: ' + route)
            if any(c > n for c, n in zip(row['changed_by_format'], row['native_by_format'])):
                raise ValueError('Changed count exceeds admitted native calls: ' + route)
            rows[route] = dict(row, boundary='isolated_original_production_splice', live_exercised=False)
    return dict(routes=[rows[r] for r in sorted(rows)],
                compared_routes=len(rows), pending_routes_compared=sorted(pending),
                physical_words=words, live_routes_added=[],
                remaining_live_routes=sorted(pending), complete_replacement=False)
