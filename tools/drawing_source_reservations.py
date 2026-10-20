"""Read-reservation accounting; observed public resources are not physical backing."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data) < 192 or (len(data)-64) % 128 or len(data) > 64+131072*128:
        raise ValueError('Reservation envelope')
    h = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMSRG01' or h[2:8] != (1, 128, 0x40209ca7, 131072, 2048, 4) or any(h[8:]):
        raise ValueError('Reservation header')
    rows = list(struct.iter_unpack('<32I', data[64:]))
    pending, seen, commands, workers, links = {}, set(), set(), set(), []
    acquired = released = 0
    for ordinal, r in enumerate(rows, 1):
        if r[0] != ordinal or not r[2] or any(r[19:]):
            raise ValueError('Reservation row identity/reserved')
        phase = r[1]
        if phase == 7:
            if ordinal != len(rows) or pending or any(r[4:15]) or r[15] not in (1, 2) or r[16] or r[17:19] != (acquired, released) or acquired != released:
                raise ValueError('Reservation terminal/pending')
            break
        if phase not in range(1, 7) or r[15] or r[16] or r[14] != phase:
            raise ValueError('Reservation refused/unknown transition')
        if not r[4] or not r[6] or not r[7] or not 1 <= r[8] <= 2048 or not r[10]:
            raise ValueError('Reservation source identity')
        identity = r[4:11]
        if phase == 1:
            if r[4] in seen or any(r[11:14]):
                raise ValueError('Reservation duplicate acquisition')
            acquired += 1
            seen.add(r[4])
            pending[r[4]] = dict(identity=identity, state=1, acquisition_record=ordinal)
        else:
            s = pending.get(r[4])
            if s is None or s['identity'] != identity or s['state'] != phase-1:
                raise ValueError('Reservation identity/order changed')
            if not r[11] or (phase < 3 and any(r[12:14])) or (phase == 3 and r[13]) or (phase >= 3 and not r[12]) or (phase >= 4 and not r[13]):
                raise ValueError('Reservation consumer identity')
            if phase == 2:
                s['payload'] = r[11]
            elif r[11] != s['payload']:
                raise ValueError('Reservation payload changed')
            if phase == 3:
                if r[12] in commands:raise ValueError('Reservation duplicate command')
                commands.add(r[12]);s['command'] = r[12]
            elif phase >= 4 and r[12] != s['command']:
                raise ValueError('Reservation command changed')
            if phase == 4:
                if r[13] in workers:raise ValueError('Reservation duplicate worker')
                workers.add(r[13]);s.update(worker=r[13], worker_thread=r[2])
            elif phase >= 5 and (r[13], r[2]) != (s['worker'], s['worker_thread']):
                raise ValueError('Reservation completion worker changed')
            s['state'] = phase
            if phase == 6:
                released += 1
                links.append(dict(submission=r[4], api_entry=r[5], source_texture=r[6], source_generation=r[7], resource=r[8], subresource=r[9], epoch=r[10], payload_ordinal=r[11], command=r[12], worker=r[13], acquisition_record=s['acquisition_record'], release_record=ordinal))
                del pending[r[4]]
        if r[17:19] != (acquired, released):
            raise ValueError('Reservation counters')
    else:
        raise ValueError('Reservation missing terminal')
    return dict(schema=1, success=True, journal_sha256=sha(data), reservations=acquired, bindings=links,
                observed_resource_write_exclusion=True, current_context_fence_required=True,
                physical_backing_owned=False, separate_resource_aliases_closed=False,
                unobserved_writers_closed=False, native_replacement=False)


def analyze(directory, report):
    from drawing_source_version import decode as decode_versions
    from drawing_gpu_completion import decode as decode_fences
    raw = (directory/'wine-source-reservations.bin').read_bytes()
    result = decode(raw)
    # Independently validate both complete raw journals. A cached report or
    # plausible inner row cannot substitute for their headers and terminals.
    versions = decode_versions((directory/'wine-source-version.bin').read_bytes())
    fences = decode_fences((directory/'wine-gpu-completion.bin').read_bytes())
    for name, decoded in (('wine_source_version_observation', versions),
                          ('wine_gpu_completion_observation', fences)):
        if any(report[name].get(key) != value for key, value in decoded.items()):
            raise ValueError('Reservation dependency report/raw journal changed: '+name)
    worker_threads = {r[13]: r[2] for r in struct.iter_unpack('<32I', raw[64:])
                      if r[1] == 4}
    by_submission = {b['submission']: b for b in versions['bindings']}
    if set(by_submission) != {b['submission'] for b in result['bindings']}:
        raise ValueError('Reservation/source-version denominator')
    for link in result['bindings']:
        original = by_submission[link['submission']]
        if any(link[k] != original[k] for k in ('api_entry','source_texture','source_generation','resource','subresource','epoch','payload_ordinal','command','worker')):
            raise ValueError('Reservation/source-version identity')
        if original['worker_thread'] != worker_threads[link['worker']]:
            raise ValueError('Reservation/source-version worker thread changed')
    # Join the exact command, submission, worker and observed worker thread.
    # Aggregate fence counts cannot substitute for current-context identity.
    observed = {}
    for call in fences['calls']:
        if call['status'] != 2 or call['submission'] in observed:
            raise ValueError('Reservation signaled completion identity')
        observed[call['submission']] = (call['command'], call['worker'], call['thread'])
    expected = {b['submission']: (b['command'], b['worker'], worker_threads[b['worker']])
                for b in result['bindings']}
    if observed != expected:
        raise ValueError('Reservation/fence denominator')
    result.update(source_version_journal_sha256=versions['journal_sha256'], gpu_journal_sha256=fences['journal_sha256'])
    return result
