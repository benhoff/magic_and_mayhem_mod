"""Strict external comparison of caller-visible API outcomes, independent of pixels."""
FIELDS=('ordinal','entry','method','object','related','caller','entry_error','flags')
def compare_calls(actual,original):
    if not isinstance(actual,list) or not isinstance(original,list) or not original:raise ValueError('Expected nonempty original/candidate caller-result histories')
    if len(actual)!=len(original):raise ValueError('Incomplete caller-result history')
    mismatches=[]
    for index,(a,o) in enumerate(zip(actual,original),1):
        for k in (*FIELDS,'hresult','last_error'):
            if type(a.get(k)) is not int or type(o.get(k)) is not int:raise ValueError('Unknown/untyped caller-visible result: '+k)
        if any(a[k]!=o[k] for k in FIELDS):raise ValueError('Caller-result method/order/generation/input identity mismatch')
        if any(not -2**31<=v['hresult']<2**31 or not 0<=v['last_error']<2**32 for v in (a,o)):raise ValueError('Caller-result range failure')
        if (a['hresult'],a['last_error'])!=(o['hresult'],o['last_error']):mismatches.append({'ordinal':index,'original':[o['hresult'],o['last_error']],'native':[a['hresult'],a['last_error']]})
    return {'matching':not mismatches,'calls_compared':len(original),'mismatches':mismatches,'pixel_equality_used_to_accept_results':False,'complete_call_input_identity':False}
def compare_fault_traces(actual,original,budget=None):
    if len(actual)!=len(original) or not original:raise ValueError('Incomplete original fault comparison')
    exhausted=0
    for row,out in zip(original,actual):
        expected=row['original']
        if type(out['id']) is not int or type(out['draws_consumed']) is not int or out['draws_consumed']<0 or out['id']!=expected['id'] or type(out['returned']) is not bool or type(out['budget_exhausted']) is not bool:raise ValueError('Fault case/result identity changed')
        if not isinstance(out['events'],list) or any(len(e)!=6 or any(type(v) is not int or not 0<=v<2**32 for v in e) for e in out['events']):raise ValueError('Unknown/untyped fault event outcome')
        if budget is None:
            if not out['returned'] or out['budget_exhausted'] or {k:out[k] for k in ('id','draws_consumed','events')}!=expected:raise ValueError('Original/native fault result or recovery side-effect mismatch')
        else:
            if type(out['draws_consumed']) is not int or not 0<=out['draws_consumed']<=budget or out['events']!=expected['events'][:len(out['events'])]:raise ValueError('Native bounded failure changed original prefix')
            if out['budget_exhausted']:
                if out['returned'] or out['draws_consumed']!=budget:raise ValueError('Failure exhaustion was relabeled successful return')
                exhausted+=1
            elif not out['returned'] or {k:out[k] for k in ('id','draws_consumed','events')}!=expected:raise ValueError('Partial fault schedule relabeled completed')
    return {'matching':True,'cases':len(actual),'budget_exhaustions':exhausted,'real_driver_loss_restore_equivalence':False}
