"""Explicit observation cohorts; API-only evidence cannot claim core calls."""


def cohort(metadata):
    api=metadata.get('api_observation_enabled',False)
    core=metadata.get('call_census_enabled',True)
    only=metadata.get('api_only',False)
    if any(type(flag) is not bool for flag in (api,core,only)):
        raise ValueError('Observation cohort flags must be boolean')
    if only:
        if not api or core:raise ValueError('API-only cohort requires API enabled and core absent')
        return 'api_only'
    if api:
        if not core:raise ValueError('API observation without core requires explicit API-only cohort')
        return 'core_api'
    return 'core' if core else 'menu_control'


def require_bindings(bindings,selected):
    core=bindings['modules']['core']['base']
    if selected=='api_only':
        if core or bindings['core_leave']:raise ValueError('API-only cohort loaded unexpected core observer')
    elif selected=='core_api':
        if not core or not bindings['core_leave']:raise ValueError('Core/API cohort lacks exact loaded core leave identity')
    else:raise ValueError('Binding check requires an API cohort')


def call_role(metadata):
    """Wire reuse never makes a CPU target cohort graphics API evidence."""
    selected=cohort(metadata)
    api=metadata.get('api_observation_enabled',False)
    role=metadata.get('call_boundary_role','graphics_api' if api else 'none')
    if type(role) is not str or role not in {'none','graphics_api','cpu_backend_dispatch'}:
        raise ValueError('Unknown CALL boundary observation role')
    if (role!='none')!=api:
        raise ValueError('CALL boundary role differs from observer admission')
    if role=='cpu_backend_dispatch' and selected!='core_api':
        raise ValueError('CPU target role requires the original-active core cohort')
    return role
