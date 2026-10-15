"""Fail-closed admission for bounded live pixel-route scenarios."""
from native_pixel_counters import ROUTES

ALL_ROUTES = tuple(f'{family}/{route}' for family, routes in ROUTES.items() for route in routes)


def validate_routes(counters, required=()):
    required = list(required)
    if len(set(required)) != len(required) or any(route not in ALL_ROUTES for route in required):
        raise ValueError('Unknown or duplicate required pixel route')
    rows = {}
    for family, routes in ROUTES.items():
        actual = counters['families'][family]
        if [r['route'] for r in actual] != routes:
            raise ValueError('Missing or reordered pixel routes: '+family)
        for row in actual:
            if row['installed'] != 1 or any(type(row[k]) is not int or row[k] < 0 for k in ('native', 'forwarded')):
                raise ValueError('Invalid pixel route counters')
            rows[family+'/'+row['route']] = row
    native = sum(r['native'] for r in rows.values())
    forwarded = sum(r['forwarded'] for r in rows.values())
    if (set(counters['families']) != set(ROUTES) or
            native != counters['native'] or forwarded != counters['forwarded']):
        raise ValueError('Inconsistent pixel route totals')
    if counters['history_refused'] or forwarded or not native:
        raise ValueError('Pixel route scenario requires native calls, zero forwards and no history refusal')
    missing = [route for route in required if not rows[route]['native']]
    if missing:
        raise ValueError('Required pixel routes did not execute: '+', '.join(missing))
    return dict(required=required, exercised=[route for route in ALL_ROUTES if rows[route]['native']],
                unexercised=[route for route in ALL_ROUTES if not rows[route]['native']],
                native=native, forwarded=forwarded, complete_replacement=False,
                scope='Per-run route execution and zero forwarding; no pixel equivalence or whole renderer replacement')
