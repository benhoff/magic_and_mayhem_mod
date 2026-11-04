"""Strict acceptance of the bounded actual-game failure/restart journey."""


def validate_flow(flow, *, expected_owner=None, expected_stream_version=None):
    if flow.get('success') is not True or flow.get('error'):
        raise ValueError('Restart journey failed: '+str(flow.get('error')))
    events = flow['events']
    steps = [e.get('step') for e in events]
    required = ['main', 'handoff', 'live-world', 'producer-failure-injected', 'terminal-cleanup',
                'terminal-stable', 'hard-restart-click', 'main', 'handoff', 'live-world',
                'fresh-session-confirmed', 'normal-main-quit', 'producer-finished']
    if steps != required:
        raise ValueError('Missing, duplicated or reordered restart transition')
    if any(a['milliseconds'] > b['milliseconds'] for a, b in zip(events, events[1:])):
        raise ValueError('Restart timeline moved backwards')
    worlds = [e for e in events if e['step'] == 'live-world']
    if [e['cycle'] for e in worlds] != [0, 1] or any(not e['identity'] or not e['renderer'] or e['source_generation'] < 3 or e['frames'] < 3 or e['paints'] < 3 for e in worlds):
        raise ValueError('Both sessions must present actual resident World progress')
    if worlds[0]['root'] == worlds[1]['root'] or worlds[0]['identity'] == worlds[1]['identity']:
        raise ValueError('Restart reused failed generation')
    if expected_owner is not None:
        if expected_owner not in ('gui-resident','dedicated-thread') or any(e.get('render_owner')!=expected_owner for e in worlds):
            raise ValueError('Restart render owner differs from the requested owner')
    if expected_stream_version is not None:
        if type(expected_stream_version) is not int or expected_stream_version not in range(1,6):
            raise ValueError('Unsupported expected command stream version')
        for e in worlds:
            if (type(e.get('command_stream_version')) is not int or e['command_stream_version']!=expected_stream_version
                    or type(e.get('transport_version')) is not int or e['transport_version']!=3
                    or type(e.get('cpu_materializations')) is not int or e['cpu_materializations']<0):
                raise ValueError('Restart command stream/transport/read observations are incomplete')
    if any(e['launcher_pid'] <= 0 for e in worlds) or worlds[0]['launcher_pid'] == worlds[1]['launcher_pid'] or events[10].get('old_launcher_gone') is not True:
        raise ValueError('Fresh launcher or old process retirement missing')
    cleanup = events[4]
    keys = {'terminal', 'owners_retired', 'timers_stopped', 'frame_cleared', 'input_detached',
            'input_suspended', 'restart_available', 'retry_disabled', 'fallback_disabled'}
    if set(cleanup['state']) != keys or any(v is not True for v in cleanup['state'].values()) or not cleanup['error']:
        raise ValueError('Incomplete terminal cleanup witness')
    if events[5]['milliseconds_observed'] < 500 or events[3]['reason'] != 5:
        raise ValueError('Missing controlled fault or terminal observation interval')
    if events[-1]['normal_quit_accepted'] is not True or events[-1]['stage'] != 11:
        raise ValueError('Fresh session did not acknowledge ordinary Quit')
    result=dict(success=True, live_sessions=2, injected_failures=1, terminal_stable_ms=events[5]['milliseconds_observed'],
                fresh_channel_identity=True, normal_quit=True, physical_context_loss=False, in_place_recovery=False)
    if expected_owner is not None:result['observed_render_owner']=expected_owner
    if expected_stream_version is not None:
        result.update(command_stream_version=expected_stream_version,transport_version=3,
                      cpu_materializations_at_world=[e['cpu_materializations'] for e in worlds],live_pending_cpu_read_fault_proved=False)
    return result
