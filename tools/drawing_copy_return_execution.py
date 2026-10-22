"""Distinguish producer return records from exact bound worker intervals."""
from drawing_observed_inputs import unique


def associate_return(record, returned, binding, current):
    descriptor = [returned['destination'], returned['source'],
                  *returned['destination_extent_stride'],
                  *returned['destination_position'],
                  *returned['source_rectangle'], returned['raw_key']]
    if (record[2] != 6 or record[1] != returned['producer_return_sequence'] or
            list(record[3:15]) != descriptor or
            binding['request'] != returned['request'] or
            binding['producer_return_sequence'] != record[1] or
            binding['owner'] != returned['owner']):
        raise ValueError('Changed copy return/request/API association')
    commands = binding['commands']
    unique(commands, 'worker')
    if not commands or any(c['worker_entry_record'] >= c['worker_return_record']
                           for c in [*commands, current]):
        raise ValueError('Incomplete worker execution interval')
    for command in [*commands, current]:
        if any(type(command[k]) is not int or command[k] <= 0 for k in ('worker', 'worker_entry_record', 'worker_return_record')):
            raise ValueError('Invalid worker interval identity')
        if command['worker'] == current['worker'] and any(command[k] != current[k] for k in ('worker_entry_record', 'worker_return_record')):
            raise ValueError('Same worker has changed execution interval')
    first = min(c['worker_entry_record'] for c in commands)
    last = max(c['worker_return_record'] for c in commands)
    if any(c['worker'] == current['worker'] for c in commands):
        relation = 'same-request-worker'
    elif last < current['worker_entry_record']:
        relation = 'commands-completed-before-worker-entry'
    elif first > current['worker_return_record']:
        relation = 'commands-started-after-worker-return'
    else:
        relation = 'interleaved-or-overlapping-command-envelope'
    return dict(sequence=record[1], request=returned['request'],
                api_entry=binding['api_entry'], owner=binding['owner'],
                first_worker_entry_record=first, last_worker_return_record=last,
                commands=commands, relation=relation,
                mutation_during_current_worker_proved=False,
                byte_or_backing_aliases_proved=False)
