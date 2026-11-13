"""Keep zero-game setup refusals and their conservative reservations explicit."""
from broader_player_requests import reviewed_predecessors
from rendering_pilot import require

def reviewed_with_setup_refusals(reader,rows,plan):
    reviews=plan.get('preserved_setup_refusals',{});ordinary=[];setup_charge=0
    for row in rows:
        if row['case_id']not in reviews:ordinary.append(row);continue
        ref=reviews[row['case_id']];require(reader.digest(ref['record'])==ref['sha256'],'Changed setup-refusal review')
        review=reader.json(ref['record'])
        require(row['state']=='failed'and row['acquisition_sha256'] and review['kind']=='broader_zero_game_setup_refusal_review'
                and review['success']is True and review['case_accepted']is False and review['original_game_runs']==0
                and review['reserved_original_starts_retained']==row['reserved_games']
                and review['acquisition']==dict(record=row['report'],sha256=row['acquisition_sha256'])
                and review['source_continuation_review']==plan['source_continuation_review'],'Setup review cannot accept a case or refund reservations')
        acquisition=review['acquisition'];require(reader.digest(acquisition['record'])==acquisition['sha256'],'Changed setup-refusal acquisition')
        a=reader.json(acquisition['record']);provider=a['provider_adaptation']
        require(a['success']is False and a['delegate_success']is False and a['sources_stable']is True
                and a['original_game_runs']==a['charged_original_starts']==0 and a['actual_start_count_complete']is True
                and a['observed_session_roots']==[] and provider['host_receipts']==provider['actual_drivers']==provider['actual_qt_environments']==[]
                and provider['capture_storage']['entries']==[] and a['error']=='Stop after unreviewed surface-loss acquisition/comparison',
                'Only the exact completely observed prelaunch refusal may continue')
        require(all(reader.digest(n)==h for n,h in review['reviewer_sources'].items()),'Setup reviewer sources changed')
        setup_charge+=row['reserved_games']
    return setup_charge+reviewed_predecessors(reader,ordinary,plan)
