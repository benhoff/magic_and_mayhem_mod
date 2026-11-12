"""Read-only No-CD report callback boundary; World pop is a later event."""
REPORT = 0x6db967


def report_callback(rows, owner=None, require_return=False):
    """Identify original report OK dispatch, separately from its actual return.

    The native driver separately checks natural outcome, verbatim strings and
    fresh parent ACK. This function neither infers those facts nor accepts an
    abandoned battle. Rows are passive original menu-observer records.
    """
    for begin in rows:
        if not (begin[1]==2 and begin[3]==6 and begin[9]==21 and begin[4]==REPORT
                and begin[11]==REPORT and begin[12]==5 and begin[6]==1
                and begin[5] and begin[2] and (owner is None or begin[2]==owner)):
            continue
        if any(r[1]==3 and r[3]==17 and r[9]==3 and r[0]<begin[0] for r in rows):
            return None
        returned=next((r for r in rows if r[0]>begin[0] and r[1]==3 and r[3]==6
                       and r[9]==21 and r[2]==begin[2] and r[4]==begin[4] and r[5]==begin[5]),None)
        if require_return and returned is None:
            continue
        return dict(begin_sequence=begin[0],return_sequence=returned[0] if returned else None,
                    thread=begin[2],begin=list(begin),returned=list(returned) if returned else None,
                    scope='Original report OK callback dispatch/return only; natural outcome and parent return ACK are separate')
    return None
