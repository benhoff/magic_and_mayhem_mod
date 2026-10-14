"""Observed lifecycle classification; absence of Lock is not absence of backing."""
from collections import Counter


def classify(traces, observation):
    events=observation['events'];unknown=[];identities={};counts=Counter()
    for trace in traces:
        for canvas in trace['canvases']:
            if canvas['layout_known']:continue
            ident=canvas['canvas'];history=[e for e in events if e['canvas']==ident]
            locks=[e for e in history if e['tag']==2 and e['phase']==2]
            before=[e for e in locks if e['producer_prefix']<trace['sequence']]
            equal=[e for e in locks if e['producer_prefix']==trace['sequence']]
            after=[e for e in locks if e['producer_prefix']>trace['sequence']]
            label='no-lock-return-in-captured-window' if not locks else 'earlier-lock-return-observed' if before else 'equal-prefix-lock-order-unknown' if equal else 'later-lock-return-observed'
            counts[label]+=1
            item=dict(lease=trace['lease'],canvas=ident,sequence=trace['sequence'],classification=label,has_recorded_pixels=canvas['has_pixels'],earlier_lock_returns=len(before),equal_prefix_lock_returns=len(equal),later_lock_returns=len(after))
            unknown.append(item)
            identities[ident]=dict(canvas=ident,create_returns=sum(e['tag']==0 and e['phase']==2 for e in history),release_returns=sum(e['tag']==1 and e['phase']==2 for e in history),lock_returns=len(locks),nonzero_returned_address_ids=sorted({e['returned_address'] for e in locks if e['returned_address']}),wrapper_interface_address_ids=sorted({a for e in history for a in e['interface_addresses'] if a}),events_observed=len(history))
    return dict(schema=1,success=True,unknown_entries=len(unknown),unknown_canvas_identities=len(identities),classifications=dict(sorted(counts.items())),canvases=[identities[k] for k in sorted(identities)],entries=unknown,physical_backing_owned=False,all_lock_calls_observed=False,equal_prefix_event_order_known=False,all_virtual_mappings_accounted=False)
