"""Bounded original menu controller for a source-owned storage reference.

All UI work goes through recovered menu callbacks or ordinary XTest input.
The controller never writes simulation/serializer memory or assumes input success.
"""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import hashlib
import json
import struct
import subprocess
import time


def channel_bytes():
    raw=bytearray(524288);raw[:8]=b'MNMMCM22'
    struct.pack_into('<II',raw,8,22,len(raw));struct.pack_into('<III',raw,16,2,1,1)
    return raw


def storage_rows(channel):
    raw=bytes(channel[110592:110592+12960])
    if len(raw)!=12960:raise ValueError('Missing V13 storage lane')
    words=struct.unpack_from('<8I',raw)
    if words[0] not in (12,13) or words[6]>100 or words[7]>words[6]:raise ValueError('Invalid original storage lane')
    return [raw[160+i*128:288+i*128].split(b'\0',1)[0].decode('cp1252') for i in range(words[6])]


def wait_for_worlds(required,timeout,process,refresh,source_worlds,refocus):
    """Observe actual committed returns; a larger deadline never supplies records."""
    if type(required) is not int or not 3<=required<=16 or type(timeout) is not int or not 30<=timeout<=180:
        raise ValueError('Storage dwell requires3..16 returns and30..180 seconds')
    start=time.monotonic();base=source_worlds();count=base;focus_attempts=0
    while time.monotonic()-start<timeout:
        if process.poll() is not None:raise ValueError('Original exited during World capture')
        refresh();count=source_worlds();elapsed=time.monotonic()-start
        if count<base:raise ValueError('Original World source prefix regressed')
        if count>=base+required and elapsed>=2:
            return dict(world_returns=count,source_base=base,required_worlds=required,elapsed_seconds=elapsed,timeout_seconds=timeout)
        if count<base+required and elapsed>(focus_attempts+1)*3 and focus_attempts<3:
            focus_attempts+=1;refocus(focus_attempts)
        time.sleep(.05)
    raise ValueError(f'No fresh original World sources: observed {count-base}/{required} within {timeout}s')


def drive(channel,run,capture,process,env,input_binary,wait,send,refresh,state,initial_worlds=3,phase_worlds=3,dwell_timeout=30):
    if type(initial_worlds) is not int or not 3<=initial_worlds<=16:
        raise ValueError('Storage dwell requires3..16 observed initial World returns')
    if type(phase_worlds) is not int or not 3<=phase_worlds<=16 or type(dwell_timeout) is not int or not 30<=dwell_timeout<=180:
        raise ValueError('Storage dwell requires3..16 returns and30..180 seconds')
    (run/'game/Save').mkdir(exist_ok=True)
    name='draw-storage-'+run.name.removeprefix('run-')
    name_bytes=name.encode('ascii')
    if len(name_bytes)>127:raise ValueError('Storage reference name too long')
    channel[106496:106624]=name_bytes+b'\0'*(128-len(name_bytes))
    actions=[];checkpoints=[];save_hash=None
    def physical(label,action):
        schedule=run/(label+'.json');schedule.write_text(json.dumps({'actions':[action]}))
        result=run/(label+'-result.json')
        with (run/(label+'.log')).open('x') as log:
            child=subprocess.Popen([str(input_binary),str(capture),str(schedule),str(result)],env=dict(env,MNM_DRAW_STORAGE_INPUT='1'),stdout=log,stderr=subprocess.STDOUT)
            try:
                bound=time.monotonic()+15
                while child.poll() is None and time.monotonic()<bound:refresh();time.sleep(.02)
                if child.poll() is None:raise ValueError('Storage physical input timed out')
                if child.returncode:raise ValueError('Storage physical input failed: '+label)
            finally:
                if child.poll() is None:child.terminate();child.wait(timeout=5)
        observed=json.loads(evidence_text(result))
        if observed['success'] is not True or observed['original_memory_written'] is not False:raise ValueError('Unadmitted storage input')
        actions.append(dict(kind='physical',label=label,path=result.name,sha256=hashlib.sha256(evidence_bytes(result)).hexdigest()))
    def source_worlds():
        p=capture/'canvas-producers.bin'
        # The recorder's committed file prefix is flushed at source boundaries.
        # Count validated World-return records through the ordinary decoder.
        if not p.exists():return 0
        raw=evidence_bytes(p)
        if len(raw)<64:return 0
        if raw[:8]!=b'MNMPRO05':raise ValueError('Require session source recorder')
        at=64;count=0;sequence=0
        while len(raw)-at>=96:
            size,ordinal,operation=struct.unpack_from('<3I',raw,at)
            if size<96 or size>16*1024*1024 or ordinal!=sequence+1:raise ValueError('Malformed committed source prefix')
            if size>len(raw)-at:break
            sequence=ordinal;count+=operation==12;at+=size
        return count
    def dwell(phase):
        required=initial_worlds if phase=='before-save' else phase_worlds
        physical(phase+'-focus',dict(kind='move',x=400,y=300,after_ms=300))
        def refocus(attempt):physical(phase+f'-focus-retry-{attempt}',dict(kind='move',x=400,y=300,after_ms=300))
        try:
            checkpoint=wait_for_worlds(required,dwell_timeout,process,refresh,source_worlds,refocus)
        except ValueError as error:
            diagnostic=dict(schema=1,success=False,phase=phase,required_worlds=required,timeout_seconds=dwell_timeout,error=str(error),completed_checkpoints=checkpoints)
            (run/'storage-dwell-failure.json').write_text(json.dumps(diagnostic,indent=2)+'\n')
            raise ValueError(str(error)+': '+phase) from error
        checkpoints.append(dict(phase=phase,**checkpoint))
        if phase=='before-save':
            from drawing_process_snapshot import snapshot_processes
            (run/'process-world.json').write_text(json.dumps(snapshot_processes(run),indent=2)+'\n')
    def mini(label):
        for attempt in range(6):
            physical(label+f'-{attempt}',dict(kind='tap',key='Escape',after_ms=350))
            try:wait(17,timeout=2);return
            except ValueError as error:
                if str(error)!='Original menu state timed out':raise
        raise ValueError('Original campaign Mini not reached')
    def load():
        nonlocal save_hash
        wait(12);rows=storage_rows(channel)
        if rows.count(name)!=1:raise ValueError('Original Load list lacks exact new save')
        send(12,31,rows.index(name)+1)
        saved=run/'game/Save'/(name+'.sav');current=hashlib.sha256(evidence_bytes(saved)).hexdigest()
        if current!=save_hash:raise ValueError('Original Load changed save bytes')
    def leave(label):
        mini(label);send(17,15)
        physical(label+'-confirm',dict(kind='click',button=1,x=265,y=395,after_ms=500))
        wait(6);send(6,26);wait(3)
    send(3,22);wait(18);send(18,23,1);send(18,25)
    dwell('before-save');mini('save-open');send(17,29);wait(13)
    if (run/'game/Save'/(name+'.sav')).exists():raise ValueError('Save name already exists')
    send(13,31)
    dwell('after-save');saved=run/'game/Save'/(name+'.sav')
    if not saved.is_file() or not saved.stat().st_size:raise ValueError('Original Save did not finish')
    save_hash=hashlib.sha256(evidence_bytes(saved)).hexdigest()
    mini('load-open');send(17,28);load();dwell('after-load');leave('leave-first')
    rebuilds=[]
    for resolution in (1,0):
        send(3,18);wait(10);payload=bytes(channel[44708:44756]);values=list(struct.unpack_from('<7I',payload,16));values[2]=resolution
        # Header host seqlock is held odd until refresh publishes the next request.
        seq=struct.unpack_from('<I',channel,16)[0];struct.pack_into('<I',channel,16,seq|1)
        struct.pack_into('<7I',channel,44756,*values);send(10,19);wait(3);rebuilds.append(resolution)
    send(3,27);load();dwell('after-recreate-load');leave('leave-reloaded')
    (run/'input.json').write_text(json.dumps(dict(schema=1,success=True,actions=actions,original_memory_written=False,semantic_outcomes_assumed=False),indent=2)+'\n')
    result=dict(schema=1,success=True,initial_worlds=initial_worlds,phase_worlds=phase_worlds,dwell_timeout_seconds=dwell_timeout,saved_name=name,saved_sha256=save_hash,save_bytes=saved.stat().st_size,checkpoints=checkpoints,display_rebuilds=rebuilds,original_callbacks_required=True,native_menu_layout_compared=False,scope='Original-active callback/input storage journey; observer and native comparisons establish their own presentation/source boundaries.')
    (run/'storage-flow.json').write_text(json.dumps(result,indent=2)+'\n');return result
