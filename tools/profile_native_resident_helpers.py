"""Shared bounded-file hashing and resident timing distribution reporting."""
import hashlib
import math
import statistics

def sha(path):
    digest=hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):digest.update(block)
    return digest.hexdigest()

def distribution(values):
    if not values:return None
    values=sorted(values)
    return dict(samples=len(values),mean=statistics.mean(values),median=statistics.median(values),
                p95=values[math.ceil(len(values)*.95)-1],maximum=values[-1],minimum=values[0])

def summarize(rows):
    warm=[r for r in rows if int(r['generation'])>=64]
    keys=['decode_ms','handoff_ms','native_consume_ms','native_cpu_producer_ms','native_prepare_ms',
          'native_submit_cpu_ms','gpu_timeline_ms','capture_io_cpu_ms','producer_capture_ms',
          'producer_wait_ack_ms','producer_world_ms','producer_return_ms','original_frame_interval_ms',
          'engine_between_world_ms','engine_pacer_ms','engine_screen_copy_ms','world_rasters']
    result={k:distribution([r[k] for r in warm if k in r and (k!='engine_pacer_ms' or r.get('engine_pacer_observed'))]) for k in keys}
    result['native_decode_handoff_consume_ms']=distribution([r['decode_ms']+r['handoff_ms']+r['native_consume_ms'] for r in warm])
    observed=[r for r in warm if r.get('engine_pacer_observed') and 'engine_between_world_ms' in r]
    residual=[r['engine_between_world_ms']-r['engine_pacer_ms']-r['engine_screen_copy_ms'] for r in observed]
    result['between_world_residual_ms']=distribution(residual)
    result['pacer_observed_worlds']=len(observed)
    result['pacer_unobserved_worlds']=len(warm)-len(observed)
    result['negative_residuals']=sum(v<0 for v in residual)
    result['scope']='All retained generation64+ rows, including UI pauses and long tails. Original preceding pacing/screen intervals use uint32 millisecond ticks; residual is unclassified and may include simulation, UI, dispatch and wait work. Nested native producer/prepare/submit stages are not added to native consume. GPU timeline is not GPU busy time.'
    return result

