"""Owned native transport, COPY checkpoint and physical-paint controls."""
import hashlib,json,mmap,os,re,shutil,struct,subprocess,time
from pathlib import Path
from drawing_copy_phase import select,transpose
from drawing_physical_generation import validate_capture


def last_json(log):
    decoder=json.JSONDecoder();at=0;value=None
    while True:
        match=re.search(r'(?m)^[ \t]*(?=\{)',log[at:])
        if match is None:break
        start=at+match.end()
        try:value,end=decoder.raw_decode(log,start)
        except json.JSONDecodeError:at=start+1
        else:at=end
    if not isinstance(value,dict):raise ValueError('Native control has no JSON outcome')
    return value


def copy_cohorts(root,out,binary,run):
    """Actual native SHA output against independent authored source formulas."""
    import importlib.util
    spec=importlib.util.spec_from_file_location('twelfth_checkpoint_comparator',root/'tools/compare-drawing-copy-phase-hashes.py')
    comparator=importlib.util.module_from_spec(spec);spec.loader.exec_module(comparator)
    cohorts=[];physical=[]
    for index,colour in enumerate((0xf800,0x07e0,0x001f)):
        directory=out/('copy-cohort-'+str(index));directory.mkdir();assets=directory/'assets';assets.mkdir()
        # A valid authored 24-bit 1x1 bitmap is bound as a read-only asset.
        bmp=bytearray(58);bmp[:2]=b'BM';struct.pack_into('<I',bmp,2,58);struct.pack_into('<IIiiHHII',bmp,10,54,40,1,1,1,24,0,4)
        (assets/'owned.bmp').write_bytes(bmp)
        records=[]
        def row(op,canvas,width=0,height=0):
            r=[0]*24;r[:4]=[96,len(records)+1,op,canvas];r[5:7]=[width,height];records.append(r);return r
        for canvas,w,h,value in ((1,800,600,0x1234),(2,800,600,colour),(5,40,40,0x4210)):
            row(1,canvas,w,h);r=row(5,canvas,w,h);r[12:15]=[w,h,value]
        r=row(10,1,800,600);r[14]=1
        for dst,src,w,h,pos,box,caller in ((1,5,800,600,(400,300),(0,0,40,40),0x4a4b01),(5,1,40,40,(0,0),(400,300,440,340),0x4a4c26),(1,2,800,600,(0,0),(0,0,800,600),0x46ba01)):
            r=row(6,dst,w,h);r[4]=src;r[8:14]=[*pos,*box];r[14]=0xffffffff;r[22]=caller
        r=row(10,1,800,600);r[14]=2;r=row(10,5,40,40);r[14]=3
        returns=[];bindings=[]
        for request,sequence,lo in ((1,8,1),(2,10,3),(3,9,5)):
            r=records[sequence-1]
            returns.append(dict(request=request,owner=11,producer_return_sequence=sequence,destination=r[3],source=r[4],destination_extent_stride=list(r[5:8]),destination_position=list(r[8:10]),source_rectangle=list(r[10:14]),raw_key=r[14]))
            bindings.append(dict(request=request,owner=11,producer_return_sequence=sequence,producer_entry_prefix=7,commands=[dict(command=request,worker=request,worker_entry_record=lo,worker_return_record=lo+1)]))
        chosen=select([(tuple(r),b'') for r in records],dict(entries=[],returns=returns),dict(bindings=bindings))
        if len(chosen['selected'])!=1 or chosen['refused']:raise ValueError('Owned exact COPY conjunction refused')
        header=bytearray(64);header[:8]=b'MNMPRO03';struct.pack_into('<5I',header,8,3,64,0x40209ca7,131072,32)
        raw=bytes(header)+b''.join(struct.pack('<24I',*r) for r in records)
        scene=dict(schema=1,boundary=dict(sequence=12,queues=0,canvas=1,width=800,height=600),assets={'owned.bmp':hashlib.sha256(bmp).hexdigest()},retain_checkpoints=[])
        declaration=directory/'scene.json';declaration.write_text(json.dumps(scene)+'\n')
        expected=[dict(ordinal=n,sequence=sequence,canvas=canvas,width=w,height=h,original_sha256=hashlib.sha256(struct.pack('<H',value)*(w*h)).hexdigest()) for n,sequence,canvas,w,h,value in ((1,7,1,800,600,0x1234),(2,11,1,800,600,colour),(3,12,5,40,40,colour))]
        result={}
        for mode,data in [('default',raw),('phase',transpose(raw,chosen['selected']))]:
            source=directory/(mode+'.bin');source.write_bytes(data);native=directory/mode
            observation=last_json(run('copy-'+str(index)+'-'+mode,['xvfb-run','-a',binary,source,assets,declaration,native]))
            if not observation['success'] or observation['remaining_surfaces'] or observation['original_oracles_read'] or observation['original_pixels_used_as_native_inputs']:raise ValueError('Native COPY source closure changed')
            if list(native.glob('checkpoint-*.565')):raise ValueError('Duplicate checkpoint planes were dumped')
            result[mode]=comparator.compare_checkpoints(expected,observation['checkpoints'])
        if sum(not c['full_plane_hash_matches'] for c in result['default'])!=1 or not all(c['full_plane_hash_matches'] for c in result['phase']):raise ValueError('Owned default failure or phase checkpoint denominator changed')
        if any(c['different_pixels'] is not None for c in result['default'] if not c['full_plane_hash_matches']):raise ValueError('Hash mismatch was relabelled zero pixels')
        native=directory/'phase/scene.565';expected_plane=struct.pack('<H',colour)*(800*600)
        if native.read_bytes()!=expected_plane:raise ValueError('Independent full native plane differs')
        physical.append((native,colour));cohorts.append(dict(colour=colour,reference_kind='authored source-formula oracle, not original game',comparisons=result,selected=chosen['selected'],duplicate_checkpoint_planes=0))
    return cohorts,physical


def physical_frames(out,binary,frames,run):
    directory=out/'physical-input';directory.mkdir();entries=[]
    for index,(source,colour) in enumerate(frames):
        path='owned-'+str(index)+'.565';shutil.copyfile(source,directory/path)
        entries.append(dict(path=path,width=800,height=600,sha256=hashlib.sha256(source.read_bytes()).hexdigest()))
    (directory/'presentation.json').write_text(json.dumps(dict(schema=1,frames=entries))+'\n')
    native=out/'physical-native';observed=last_json(run('physical-requested-frames',['xvfb-run','-a',binary,directory,native]))
    if not observed['success'] or observed['painted_before_swap_frames']!=3 or observed['physical_generation_frames']!=3 or observed['qimage_uploads'] or observed['remaining_surfaces']:raise ValueError('Requested frame paint/swap closure changed')
    pixels=0
    for index,(item,(_,colour)) in enumerate(zip(observed['frames'],frames),1):
        body=(native/item['path']).read_bytes();marker=(native/item['marker_path']).read_bytes()
        validate_capture(item,body,marker,800,600,index)
        rgb=(255 if colour==0xf800 else 0,255 if colour==0x07e0 else 0,255 if colour==0x001f else 0,255)
        if body!=bytes(rgb)*(800*600):raise ValueError('Independent physical RGB plane differs')
        pixels+=800*600
    refusals=[]
    for name in ('late-fractional','late-hash'):
        bad=out/name;bad.mkdir();changed=[dict(e) for e in entries]
        for entry in changed:shutil.copyfile(directory/entry['path'],bad/entry['path'])
        if name=='late-fractional':changed[-1]['width']=800.5
        else:changed[-1]['sha256']='0'*64
        (bad/'presentation.json').write_text(json.dumps(dict(schema=1,frames=changed))+'\n');target=out/(name+'-refused')
        observation=last_json(run('physical-'+name,['xvfb-run','-a',binary,bad,target],expected=1))
        if observation['success'] or target.exists():raise ValueError('Invalid later presentation row reached output/window admission')
        refusals.append(dict(name=name,outcome=observation,output_directory_absent=True))
    return dict(success=True,frames=3,pixels=pixels,same_image_markers=True,paint_before_swap=True,source='actual owned native COPY phase outputs',refusals=refusals,presentation=observed)


def bind_case(root,out,exe,dll,prefix,env,width,case,probe,deferred):
    """Host mmap peer for actual PE32 publication, with no original artifacts."""
    import sys
    if str(root/'protocols/python') not in sys.path:sys.path.insert(0,str(root/'protocols/python'))
    from mnm_protocols import frame_v1,render_commands_v2 as ring
    label='bind-'+str(width)+'-'+case+('-deferred' if deferred else '')
    directory=out/label;directory.mkdir();channel=directory/'commands.bin';frame=directory/'frame.bin';capture=directory/'capture';capture.mkdir()
    header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,123)
    for path,initial,size in ((channel,header,ring.SIZE),(frame,frame_v1.initial_header(),frame_v1.SIZE)):
        with path.open('xb') as stream:stream.write(initial);stream.truncate(size)
    win=lambda p:'Z:'+str(p).replace('/','\\')
    environment=dict(env,WINEPREFIX=str(prefix),MNM_BIND_WIDTH=str(width),MNM_BIND_CASE=case,MNM_RENDER_COMMAND_CHANNEL=win(channel),MNM_RENDER_STREAM=win(frame),MNM_RENDER_LOCK_CAPTURE_DIR=win(capture),MNM_RENDER_CONTINUOUS='1',MNM_RENDER_ORDERED_COPIES='1',MNM_PUBLIC_RESIDENT_WORLD='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_AUTO_SHUTDOWN='0')
    if probe:environment['MNM_BIND_PUBLICATION_PROBE']='1'
    child=None;collected=bytearray();consumed=0;log=directory/'wine.log'
    try:
        with channel.open('r+b') as file,mmap.mmap(file.fileno(),0) as mapping,log.open('x') as output:
            child=subprocess.Popen(['wine',str(exe)],cwd=directory,env=environment,stdout=output,stderr=output)
            deadline=time.monotonic()+45
            if deferred:child.wait(timeout=45)
            while True:
                exited=child.poll() is not None
                if not exited and time.monotonic()>=deadline:raise ValueError('Owned bind fixture deadline')
                published=struct.unpack_from('<I',mapping,20)[0]
                if published<consumed or published-consumed>ring.CAPACITY:raise ValueError('Invalid ring publication')
                count=published-consumed;offset=consumed%ring.CAPACITY;first=min(count,ring.CAPACITY-offset)
                collected.extend(mapping[64+offset:64+offset+first]);collected.extend(mapping[64:64+count-first]);consumed=published
                struct.pack_into('<I',mapping,36,consumed)
                if exited:break
                time.sleep(.002)
            if child.returncode:raise ValueError('PE32 bind failed '+str(child.returncode)+': '+log.read_text(errors='replace'))
            if struct.unpack_from('<II',mapping,24)!=(2,0) or struct.unpack_from('<I',mapping,20)[0]!=consumed:raise ValueError('Unclosed publication terminal')
        result=struct.unpack('<13I',(directory/'bind-result.bin').read_bytes());bound=case=='baseline'
        if result[:6]!=(int(bound),0x77,1,1,1,1) or result[10]:raise ValueError('Original call/error or callback isolation differs')
        archive=(capture/'session-00000001.bin').read_bytes()
        if archive!=collected or archive[:16]!=b'MNMCMD05'+struct.pack('<II',5,16):raise ValueError('Archive/FIFO diverged')
        at=16;ops=[];bodies=[]
        while at<len(archive):
            op,sequence,size=struct.unpack_from('<III',archive,at)
            if sequence!=len(ops)+1 or size>len(archive)-at-12:raise ValueError('Archive record identity')
            ops.append(op);bodies.append(archive[at+12:at+12+size]);at+=12+size
        if at!=len(archive) or ops!=([1,19,6,7,8] if bound else [1,6,7,8]):raise ValueError('Bound World/FIFO/END changed')
        if len(bodies[0])!=28+width*(480 if width==640 else 600)*2 or any(bodies[0][28:]):raise ValueError('Borrowed initial pixels were not owned before poisoning')
        if bound and struct.unpack('<5I',bodies[1])!=(1,1,0,17,34):raise ValueError('Resident World scalar identity')
        trace=None
        if probe:
            trace=struct.unpack('<5I',(directory/'publication-trace.bin').read_bytes())
            if not trace[0] or trace[0]==trace[4] or not trace[1] or trace[2:4]!=(0,0):raise ValueError('Publication was not isolated on the existing worker')
        frequency=result[8]|result[9]<<32
        if not frequency:raise ValueError('Missing fixture QPC frequency')
        return dict(success=True,label=label,width=width,case=case,deferred_peer=deferred,publication_probe=probe,publication_trace=trace,bind_succeeded=bound,original_locks=1,original_unlocks=1,original_last_error=0x77,callback_pump_calls=0,complete_shutdown=True,fifo_archive_equal=True,source_owned_before_poison=True,opcodes=ops,unlock_ms=(result[11]|result[12]<<32)*1000/frequency,log=str(log.relative_to(root)),log_sha256=hashlib.sha256(log.read_bytes()).hexdigest())
    finally:
        if child is not None and child.poll() is None:child.terminate();child.wait(timeout=5)
