"""Exact recorded witness cohorts and separate functional acknowledgement admission."""


def witness_cohort(catalog, original):
    rows = original['egl_swap_completion_observation']['frames']
    kept = catalog['witnesses']
    physical = original['egl_physical_observation']['frames']
    count = len(catalog['frames'])
    if not count or any(len(values) != count for values in (rows, kept, physical)):
        raise ValueError('Retained witness cohort incomplete')
    for ordinal, triple in enumerate(zip(rows, kept, physical), 1):
        if any(type(row.get('ordinal')) is not int or row['ordinal'] != ordinal for row in triple):
            raise ValueError('Retained witness cohort reordered or duplicated')
    return rows, kept, physical


def functional_admission(*, functional_only, resident, impossible_budget):
    if functional_only and (not resident or impossible_budget):
        raise ValueError('Functional-only admission requires resident delivery and no performance negative control')
    return dict(functional_only=functional_only, performance_required=not functional_only,
                scope='Functional GPU acknowledgement checks retain all transport/pixel/resource guards. '
                      'Performance observations remain recorded independently; no performance completion claim.')


def declared_source(root, name):
    """Compiler-relative source spellings may use parents within the checkout.

    Corpus data paths retain their stricter portable spelling rule. Source
    dependencies preserve their declared spelling while refusing unsafe roots.
    """
    from pathlib import Path
    root = Path(root).resolve(); value = Path(name); target = (root/value).resolve()
    if (value.is_absolute() or not target.is_relative_to(root) or
            any(target.is_relative_to(root/part) for part in ('original','working','.git'))):
        raise ValueError('Unsafe declared source path')
    return target


def delivery_admission(checks, *, functional_only):
    """Separate timing acceptance from mandatory memory and frame accounting."""
    resources = ('rss', 'caches', 'no_coalescing', 'all_presentations')
    required = resources if functional_only else tuple(checks)
    if not required or any(type(checks.get(key)) is not bool for key in required):
        raise ValueError('Missing delivery admission checks')
    return dict(passed=all(checks[key] for key in required), required_checks=list(required),
                performance_required=not functional_only)
