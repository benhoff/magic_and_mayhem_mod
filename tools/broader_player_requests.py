"""Fail-closed cumulative start accounting across explicitly reviewed phases."""
from rendering_pilot import require


def cumulative_bound(reader, plan):
    require(plan['success'] is True and plan['previous_WP10_original_games'] == 8,
            'Unreviewed cumulative start basis')
    maximum = plan['maximum_additional_original_games']
    require(type(maximum) is int and maximum in (96, 112), 'Unreviewed cumulative start ceiling')
    if maximum == 112:
        ref = plan['source_continuation_review']
        require(reader.digest(ref['record']) == ref['sha256'], 'Changed source continuation review')
        review = reader.json(ref['record'])
        require(review['success'] is True and review['original_game_runs'] == 0
                and review['reserved_original_starts_retained'] == 12
                and review['previous_WP10_starts_retained'] == 8
                and review['old_cases_current_runtime_credit'] is False
                and plan['maximum_combined_original_games'] == 120,
                'Source continuation may not refund or promote old runtime execution')
    return maximum


def reviewed_predecessors(reader, rows, plan):
    """An exact reviewed negative permits continuation; it never becomes a pass."""
    exceptions = plan.get('preserved_negative_predecessors', {})
    for row in rows:
        if row['state'] == 'admitted':
            continue
        require(row['case_id'] in exceptions, 'Stop after every failed or unreviewed predecessor')
        ref = exceptions[row['case_id']]
        require(reader.digest(ref['record']) == ref['sha256'], 'Negative review identity changed')
        review = reader.json(ref['record'])
        require(review['kind'] == 'broader_player_negative_predecessor_review'
                and review['success'] is True and review['case_accepted'] is False
                and review['original_game_runs'] == 0
                and review['reserved_original_starts_retained'] == row['reserved_games']
                and review['acquisition'] == dict(record=row['report'], sha256=row['acquisition_sha256'])
                and review['source_continuation_review'] == plan['source_continuation_review'],
                'Negative review cannot grant acceptance, mix phases or erase charges')
        require(row['state'] == 'acquired' and row['acquisition_sha256'],
                'An incomplete or unknown original start needs its own explicit review')
        comparison = review['failed_comparison']
        require(reader.digest(comparison['record']) == comparison['sha256'], 'Failed comparison identity changed')
        result = reader.json(comparison['record'])
        require(result['success'] is False and result['comparison_processes_stopped'] is True
                and result['acquisition'] == review['acquisition'],
                'Only the exact stopped negative comparison is reviewed')
        require(all(reader.digest(n) == h for n,h in review['reviewer_sources'].items()),
                'Negative reviewer source changed')
    return sum(row['reserved_games'] for row in rows)
