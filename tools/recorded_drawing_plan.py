"""Input-only GPU transfer plan for an independently bound recorded interval.

Producer source snapshots are native outputs. Original image paths, pixel hashes
and expected results are deliberately not serialized into this backend input.
"""
import hashlib
import json
from pathlib import Path
import shutil


def sha(data):return hashlib.sha256(data).hexdigest()


def prepare(directory, native_directory, native_frames, bindings):
    directory=Path(directory);native_directory=Path(native_directory).resolve()
    if directory.exists():raise ValueError('Recorded composition input must be new')
    if not 1<=len(bindings)<=8192 or [f['ordinal'] for f in bindings]!=list(range(1,len(bindings)+1)):
        raise ValueError('Recorded presentation history has gaps or reordering')
    directory.mkdir(parents=True);(directory/'sources').mkdir()
    byseq={(f['sequence'],f['canvas']):f for f in native_frames}
    if len(byseq)!=len(native_frames):raise ValueError('Ambiguous native source snapshot identity')
    sources=[];source_ids={};frames=[]
    def snapshot(sequence,canvas):
        identity=(sequence,canvas)
        if identity in source_ids:return source_ids[identity]
        native=byseq[identity];name=Path(native['path'])
        if name.is_absolute() or '..' in name.parts or (native_directory/name).is_symlink():raise ValueError('Unsafe native snapshot path')
        source=(native_directory/name).resolve()
        if not source.is_relative_to(native_directory):raise ValueError('Native snapshot escapes output')
        data=source.read_bytes()
        if sha(data)!=native['sha256'] or len(data)!=native['width']*native['height']*2:raise ValueError('Native snapshot changed')
        index=len(sources)+1;target=directory/'sources'/f'snapshot-{index:04}.565';shutil.copyfile(source,target)
        sources.append(dict(id=index,path=target.relative_to(directory).as_posix(),sha256=sha(data),width=native['width'],height=native['height']))
        source_ids[identity]=index;return index
    for f in bindings:
        if f.get('refresh'):raise ValueError('Unbound refresh cannot supply a new presentation')
        if f['fill'] is not None:
            frames.append(dict(index=f['ordinal'],kind='fill',colour=f['fill']));continue
        background=f.get('background')
        if background:
            view=background['source_box']
            if 'from_presentation' in background:
                kind='history';source=background['from_presentation']
                if type(source) is not int or not 0<source<f['ordinal']:raise ValueError('Future/unknown saved history')
            else:kind='snapshot';source=snapshot(background['sequence'],background['canvas'])
        else:
            kind='snapshot';source=snapshot(f['sequence'],f['canvas']);view=[0,0,f['width'],f['height']]
        transfer=f['transfer']
        if transfer is None:
            if (f['width'],f['height'])!=(800,600):raise ValueError('Partial source cannot replace whole presentation')
            src=view;dst=[0,0,800,600];key=None
        else:
            sx,sy,right,bottom=transfer['source_box'];vx,vy,vr,vb=view
            if (vr-vx,vb-vy)!=(f['width'],f['height']):raise ValueError('Saved view/source extent mismatch')
            src=[vx+sx,vy+sy,vx+right,vy+bottom];dst=transfer['destination_box'];key=transfer['key']
        frames.append(dict(index=f['ordinal'],kind='copy',source_kind=kind,source=source,view_rect=view,source_rect=src,destination_rect=dst,key=key))
    result=dict(schema=1,width=800,height=600,sources=sources,frames=frames)
    (directory/'presentation.json').write_text(json.dumps(result,indent=2)+'\n');return result
