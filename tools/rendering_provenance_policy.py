"""Separate diagnostic drawing comparison from nominal input admission."""


def drawing_only(acquisition, requested):
    if type(requested) is not bool:
        raise ValueError('Drawing-only selection must be boolean')
    if not requested:
        return None
    configuration=acquisition.get('configuration')
    operations=configuration.get('operations_provenance') if type(configuration) is dict else None
    if (acquisition.get('kind')!='public_native_watchdog_diagnostic_acquisition'
            or acquisition.get('scenario')!='normal-quick'
            or acquisition.get('player_accepted') is not False
            or type(configuration) is not dict or configuration.get('diagnostic_only') is not True
            or type(operations) is not dict or set(operations)!={'copy_scope','cursor_state_version','qt_cursor_exercise'}
            or operations['copy_scope']!='all' or type(operations['cursor_state_version']) is not int
            or operations['cursor_state_version']!=1 or operations['qt_cursor_exercise'] is not True):
        raise ValueError('Drawing-only comparison requires explicit cursor diagnostic acquisition')
    return dict(verified=False,admitted=False,
                scope='Original Right-key poll acknowledgements are not admitted by this diagnostic drawing-only comparison. The strict default input guard remains unchanged.')
