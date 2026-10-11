"""Shared bounded-file hashing and resident timing distribution reporting."""
import hashlib
import math
import statistics
import struct
import zlib

def normalize_capture(source,destination):
    """Bounded V1/V2 read; inflate once outside benchmark timing into new V1."""
    if source.stat().st_size>16*1024**3:raise ValueError('Resident compressed file bound')
    count=0;raw_size=8
    with source.open('rb') as incoming,destination.open('xb') as outgoing:
        magic=incoming.read(8)
        if magic not in (b'MNMPROF1',b'MNMPROF2'):raise ValueError('Resident capture magic')
        outgoing.write(b'MNMPROF1')
        while True:
            header=incoming.read(20 if magic==b'MNMPROF2' else 16)
            if not header:break
            if len(header)!=(20 if magic==b'MNMPROF2' else 16):raise ValueError('Truncated capture header')
            generation,source_bytes,request_bytes=struct.unpack('<QII',header[:16]);total=source_bytes+request_bytes
            if generation!=count+1 or generation>8192 or not 16<=source_bytes<=16777216 or not 64<=request_bytes<=197184:raise ValueError('Resident capture generation/extent')
            raw_size+=16+total
            if raw_size>16*1024**3:raise ValueError('Resident raw capture bound')
            if magic==b'MNMPROF2':
                compressed_bytes=struct.unpack('<I',header[16:])[0]
                if not 4<compressed_bytes<=total+total//1000+128:raise ValueError('Resident compression extent')
                packed=incoming.read(compressed_bytes)
                if len(packed)!=compressed_bytes or struct.unpack('>I',packed[:4])[0]!=total:raise ValueError('Resident compression length')
                decoder=zlib.decompressobj()
                try:payload=decoder.decompress(packed[4:],total+1)
                except zlib.error as error:raise ValueError('Resident compressed payload') from error
                if len(payload)!=total or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:raise ValueError('Resident compressed payload')
            else:
                payload=incoming.read(total)
                if len(payload)!=total:raise ValueError('Truncated resident payload')
            outgoing.write(header[:16]);outgoing.write(payload);count+=1
    if source.stat().st_size>16*1024**3:raise ValueError('Resident compressed file bound')
    return dict(worlds=count,raw_bytes=raw_size,stored_bytes=source.stat().st_size)

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
    keys=['source_acquire_ms','decode_ms','handoff_ms','native_consume_ms','native_cpu_producer_ms','native_prepare_ms',
          'native_submit_cpu_ms','gpu_timeline_ms','capture_io_cpu_ms','capture_enqueue_cpu_ms','producer_capture_ms',
          'producer_wait_ack_ms','producer_world_ms','producer_return_ms','original_frame_interval_ms',
          'engine_between_world_ms','engine_pacer_ms','engine_screen_copy_ms','world_rasters']
    result={k:distribution([r[k] for r in warm if k in r and (k!='engine_pacer_ms' or r.get('engine_pacer_observed'))]) for k in keys}
    result['native_decode_handoff_consume_ms']=distribution([r['decode_ms']+r['handoff_ms']+r['native_consume_ms'] for r in warm])
    result['native_acquire_decode_handoff_consume_ms']=distribution([r['source_acquire_ms']+r['decode_ms']+r['handoff_ms']+r['native_consume_ms'] for r in warm if 'source_acquire_ms' in r])
    observed=[r for r in warm if r.get('engine_pacer_observed') and 'engine_between_world_ms' in r]
    residual=[r['engine_between_world_ms']-r['engine_pacer_ms']-r['engine_screen_copy_ms'] for r in observed]
    result['between_world_residual_ms']=distribution(residual)
    result['pacer_observed_worlds']=len(observed)
    result['pacer_unobserved_worlds']=len(warm)-len(observed)
    result['negative_residuals']=sum(v<0 for v in residual)
    result['scope']='All retained generation64+ rows, including UI pauses and long tails. Original preceding pacing/screen intervals use uint32 millisecond ticks; residual is unclassified and may include simulation, UI, dispatch and wait work. Nested native producer/prepare/submit stages are not added to native consume. GPU timeline is not GPU busy time.'
    return result

