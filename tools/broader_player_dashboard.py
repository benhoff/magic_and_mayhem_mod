"""Validate a dashboard projection of the complete finite player decision."""
from broader_player_verdicts import issue
from rendering_pilot import require

KIND = 'native_rendering_required_boundaries_admission'
PROJECTION_KIND = 'broader_current_native_player_dashboard_projection'
READER = 'tools/coverage_rendering_artifacts.py'


def validate(projection, verified):
    """Preserve recorded source phases; the UI separately checks their freshness."""
    require(projection['kind'] == KIND and projection['projection_kind'] == PROJECTION_KIND
            and projection['success'] is True,
            'Incomplete current-player dashboard projection')
    current_ref = projection['current_acceptance']
    current = verified(current_ref)
    verdict = verified(projection['independent_decisions'])
    boundary = verified(current['required_boundary_impact'])
    parent_ref = boundary['retained_finite_master']
    parent = verified(parent_ref)
    prior = verified(parent['decision_record'])
    expected = issue(current, parent, prior)
    require(verdict['kind'] == 'broader_current_independent_rendering_verdicts'
            and verdict['success'] is True and verdict['sources_stable'] is True
            and verdict['current_acceptance'] == current_ref
            and verdict['finite_requirement_parent'] == parent_ref
            and verdict['finite_requirement_rows'] == parent['requirements']
            and verdict['all156parent_rows_preserved_exactly'] is True
            and all(verdict.get(k) == value for k, value in expected.items()),
            'Independent player/recovery decision or parent rows changed')
    require(parent['kind'] == 'native_rendering_required_boundaries_admission'
            and projection['finite_requirement_parent'] == parent_ref
            and projection['requirements'] == parent['requirements']
            and projection['proof'] == parent['proof']
            and projection['scope'] == current['scope']
            and projection['decisions'] == expected['decisions'],
            'Dashboard changed the accepted scope, requirements or boundary proof')
    sources = projection['sources']
    require(READER in sources and 'tools/broader_player_dashboard.py' in sources
            and all(sources.get(n) == h for values in (current['sources'], verdict['sources'])
                    for n, h in values.items()),
            'Dashboard omitted or rewrote current-player source bindings')
    return parent
