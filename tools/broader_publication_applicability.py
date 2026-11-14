"""Exact retained-output applicability for one Python setup-publication change."""
from broader_player_acceptance import checked, NATIVE_ROOTS
from coverage_claims import fingerprint
from rendering_pilot import require
PUBLISHER = 'tools/owned-menu-game-runner.py'


def reviewed_sources(reader, sources, identity, reference, *, purpose):
    """Return a new analysis map; never rewrite a historical source dictionary.

    Only three pinned completed records may use this publication-only review.
    Native provenance is separately checked in full by the caller, and the
    review itself rechecks the exact compiler/runtime build inputs.
    """
    changed = [n for n, h in sources.items() if reader.digest(n) != h]
    if not changed:
        return dict(sources), None
    require(changed == [PUBLISHER] and not PUBLISHER.startswith(NATIVE_ROOTS),
            'Unreviewed source drift; native files cannot be waived')
    require(purpose in ('validation', 'coordinator'), 'Unknown publication applicability purpose')
    q = checked(reader, reference)
    require(q['kind'] == 'broader_completed_journal_publication_applicability'
            and q['retained_current_cases'] == 11 and len(q['case_bindings']) == 3
            and q['native_files_waived'] == [] and q['old_source_bytes_preserved'] is True
            and q['all_retained_complete_journals_republish_identically'] is True
            and q['game_response_deadlines_changed'] is False,
            'Incomplete publication applicability review')
    require(all(reader.digest(n) == h for n, h in q['sources'].items()), 'Publication review source changed')
    require(reader.digest(q['old_publisher']['record']) == q['old_publisher']['sha256']
            and sources[PUBLISHER] == q['old_publisher']['sha256']
            and reader.digest(PUBLISHER) == q['current_publisher_sha256'],
            'Different publisher preimage or current source')
    build = checked(reader, q['current_build'])
    require(build['sources_stable'] and all(reader.digest(n) == h
            for n, h in (build['compiler_sources'] | build['runtime_sources']).items()),
            'Native/compiler/runtime changed across publication review')
    controls = checked(reader, q['publication_controls'])
    require(controls['sources_stable'] and controls['publication_controls']['tests'] == 8
            and all(reader.digest(n) == h for n, h in controls['sources'].items()),
            'Atomic publication controls no longer current')
    matches = [v for v in q['case_bindings'] if v['case_id'] == identity['case_id']]
    require(len(matches) == 1, 'Foreign or unreviewed historical publication case')
    binding = matches[0]
    require(binding['acquisition'] == identity['acquisition'] and binding['validation'] == identity['validation']
            and binding[purpose + '_sources_sha256'] == fingerprint(sources),
            'Publication review belongs to a different exact record/source map')
    result = dict(sources)
    result[PUBLISHER] = q['current_publisher_sha256']
    return result, dict(review=reference, purpose=purpose, old_publisher=q['old_publisher'],
                        current_publisher_sha256=q['current_publisher_sha256'],
                        historical_sources_preserved=True, native_files_waived=[])
