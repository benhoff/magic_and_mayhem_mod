"""Compare original GL source oracles externally; never build native inputs."""
from hashlib import sha256
from pathlib import Path
import numpy as np

def compare(original,capture,ordinal,native,width,height):
    rows=original.get('egl_swap_completion_observation',{}).get('frames',[])
    if not rows or 'gpu_source' not in rows[ordinal-1]:return None
    row=rows[ordinal-1];blit=row['last_blit'];source=row['gpu_source']
    if (row['ordinal']!=ordinal or (row['width'],row['height'])!=(width,height) or
        not blit['count'] or blit['mask']!=0x4000 or blit['filter']!=0x2600 or
        blit['source_box']!=[0,0,width,height] or blit['destination_box']!=[0,height,width,0] or
        (blit['read_width'],blit['read_height'])!=(width,height) or not blit['read_fbo'] or blit['draw_fbo'] or
        source['original_oracle_only'] is not True or source['bytes']!=width*height*2):
        raise ValueError('Unsupported original GL source projection')
    name=Path(source['path']);path=Path(capture)/name
    if name.is_absolute() or len(name.parts)!=1 or name.name!=f"egl-gl-source-{row['call']:06d}.565" or path.is_symlink():raise ValueError('Unsafe GL source oracle path')
    data=path.read_bytes()
    if len(data)!=width*height*2 or sha256(data).hexdigest()!=source['sha256'] or len(native)!=len(data):raise ValueError('GL source oracle changed')
    bad=int(np.count_nonzero(np.frombuffer(data,dtype='<u2')!=np.frombuffer(native,dtype='<u2')))
    return dict(pixels=width*height,mismatching_pixels=bad,original_sha256=source['sha256'],native_sha256=sha256(native).hexdigest(),rgb565_exact=True,original_pixels_used_as_native_inputs=False)
