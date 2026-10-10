#!/usr/bin/env python3
"""Bounded complete movie pixel production, independent decoder and skip checks."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes
import ctypes.util
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile
import numpy as np

ROOT=Path(__file__).resolve().parents[1]


def sha(path):
    h=hashlib.sha256()
    with path.open('rb') as source:
        for block in iter(lambda:source.read(1024*1024),b''):
            h.update(block)
    return h.hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();claims=json.loads(args.claims.read_text())
    for name,digest in claims['sources'].items():
        if sha(ROOT/name)!=digest:
            raise ValueError('Prospective source changed: '+name)
    parent=ROOT/'working/tests/native-movie-frames';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report={'success':False,'claims':claims['claims'],'sources':claims['sources'],
            'scope':'Owned opaque RGBA AVI movie production, all decoded frames, complete playback and bounded cancellation. Exact synthetic RGBA and independent FFmpeg-normalized YUV420P decoded planes with independently calculated limited BT.601 RGB policy for installed Indeo movies. Not original DirectShow/placement/control or live gameplay return equivalence.',
            'original_pixels_used_as_native_inputs':False,'live_replacement':False,'cases':[]}
    inputs={}

    def run(label,command,env=None):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(v) for v in command],cwd=ROOT,env=env,stdout=log,
                           stderr=subprocess.STDOUT,timeout=260,check=True)

    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        build=out/'build'
        run('configure',['cmake','-S',ROOT/'tests/native-movie-frames','-B',build,'-DCMAKE_BUILD_TYPE=Release'])
        run('build',['cmake','--build',build,'-j4'])
        dependencies={'tests/native-movie-frames/CMakeLists.txt','tools/test-native-movie-frames.py'}
        for path in build.rglob('*.o.d'):
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                p=Path(name).resolve() if Path(name).is_absolute() else Path(name)
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    dependencies.add(str(p.relative_to(ROOT)))
        if dependencies-set(report['sources']):
            raise ValueError('Undeclared dependencies: '+repr(sorted(dependencies-set(report['sources']))))
        report['native_binary_sha256']=sha(build/'native-movie-frame-test')
        # Every frame changes, with varying input alpha to exercise owned opaque
        # output even when the decoder exposes an alpha-capable pixel format.
        w,h,count=64,48,12
        raw=b''.join(bytes(((x*5+n*11)&255,(y*7+n*17)&255,(x+y+n*23)&255,(x*3+n)&255))
                     for n in range(count) for y in range(h) for x in range(w))
        (out/'synthetic.rgba').write_bytes(raw)
        run('synthetic-encode',['ffmpeg','-v','error','-f','rawvideo','-pixel_format','rgba','-video_size','64x48',
            '-framerate','12','-i',out/'synthetic.rgba','-c:v','ffv1','-pix_fmt','bgra',out/'synthetic.avi'])
        movies=[out/'synthetic.avi',*sorted((ROOT/'working/game-nocd/FMV').glob('*.avi'))]
        if len(movies)!=6:
            raise ValueError('Expected five installed movies plus synthetic')
        env={**os.environ,'QT_QPA_PLATFORM':'offscreen','QT_MEDIA_BACKEND':'ffmpeg'}

        def check(movie):
            name=movie.stem;inputs[str(movie.relative_to(ROOT))]=sha(movie)
            probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-select_streams','v:0',
                '-show_entries','stream=codec_name,pix_fmt,width,height,nb_frames','-of','json',movie],text=True))['streams'][0]
            output=out/(name+'-native.rgba');reference=out/(name+'-reference.rgba')
            width,height=probe['width'],probe['height']
            if movie==out/'synthetic.avi':
                run(name+'-reference',['ffmpeg','-v','error','-i',movie,'-map','0:v:0','-fps_mode','passthrough',
                    '-vf','format=rgba,lut=a=255','-f','rawvideo','-pix_fmt','rgba',reference])
            else:
                if probe['codec_name']!='indeo4' or probe['pix_fmt']!='yuv410p':
                    raise ValueError('Installed movie outside declared Indeo/YUV410P scope')
                planes=out/(name+'-reference.yuv')
                run(name+'-reference',['ffmpeg','-v','error','-i',movie,'-map','0:v:0','-fps_mode','passthrough',
                    '-f','rawvideo','-pix_fmt','yuv410p',planes])
                plane_bytes=width*height*9//8
                if width%4 or height%4 or planes.stat().st_size%plane_bytes:
                    raise ValueError('Unexpected installed YUV plane extent')
                # Qt's documented normalizer calls the low-level bicubic
                # sws_getContext path. FFmpeg's scale filter also applies chroma
                # location defaults, yielding a different source plane. Decode
                # raw YUV410P independently, then bind the declared normalizer
                # directly without using any Qt/native decoded pixels.
                sws=ctypes.CDLL(ctypes.util.find_library('swscale'))
                av=ctypes.CDLL(ctypes.util.find_library('avutil'))
                av.av_get_pix_fmt.argtypes=[ctypes.c_char_p];av.av_get_pix_fmt.restype=ctypes.c_int
                sws.sws_getContext.argtypes=[ctypes.c_int]*7+[ctypes.c_void_p]*3;sws.sws_getContext.restype=ctypes.c_void_p
                pointer=ctypes.POINTER(ctypes.c_uint8);pointers=pointer*4;strides=ctypes.c_int*4
                sws.sws_scale.argtypes=[ctypes.c_void_p,ctypes.POINTER(pointer),ctypes.POINTER(ctypes.c_int),
                    ctypes.c_int,ctypes.c_int,ctypes.POINTER(pointer),ctypes.POINTER(ctypes.c_int)]
                sws.sws_scale.restype=ctypes.c_int
                sws.sws_freeContext.argtypes=[ctypes.c_void_p]
                context=sws.sws_getContext(width,height,av.av_get_pix_fmt(b'yuv410p'),width,height,
                    av.av_get_pix_fmt(b'yuv420p'),4,None,None,None)
                if not context:
                    raise ValueError('Reference normalizer allocation failed')
                yuv=np.memmap(planes,dtype=np.uint8,mode='r')
                try:
                    with reference.open('xb') as pixels:
                        for at in range(0,len(yuv),plane_bytes):
                            source=ctypes.create_string_buffer(yuv[at:at+plane_bytes].tobytes())
                            dest=ctypes.create_string_buffer(width*height*3//2)
                            sp=pointers(*[ctypes.cast(ctypes.addressof(source)+off,pointer) for off in (0,width*height,width*height*17//16,0)])
                            dp=pointers(*[ctypes.cast(ctypes.addressof(dest)+off,pointer) for off in (0,width*height,width*height*5//4,0)])
                            if sws.sws_scale(context,sp,strides(width,width//4,width//4,0),0,height,
                                             dp,strides(width,width//2,width//2,0))!=height:
                                raise ValueError('Reference YUV normalization incomplete')
                            normalized=np.frombuffer(dest,dtype=np.uint8)
                            y=normalized[:width*height].reshape(height,width).astype(np.int32)
                            u=normalized[width*height:width*height*5//4].reshape(height//2,width//2).astype(np.int32).repeat(2,axis=0).repeat(2,axis=1)-128
                            v=normalized[width*height*5//4:width*height*3//2].reshape(height//2,width//2).astype(np.int32).repeat(2,axis=0).repeat(2,axis=1)-128
                            # Independent vector calculation of the declared
                            # fixed BT.601 policy, green bias and opaque alpha.
                            rgba=np.empty((height,width,4),dtype=np.uint8);luma=(y-16)*298
                            rgba[:,:,0]=np.clip((luma+409*v+128)//256,0,255)
                            rgba[:,:,1]=np.clip((luma-100*u-208*v-128)//256,0,255)
                            rgba[:,:,2]=np.clip((luma+516*u+128)//256,0,255);rgba[:,:,3]=255
                            pixels.write(rgba.tobytes())
                finally:
                    sws.sws_freeContext(context)
            run(name+'-native',[build/'native-movie-frame-test',movie,'0',out/(name+'.json'),output],env)
            data=json.loads((out/(name+'.json')).read_text())
            expected=reference.stat().st_size//(width*height*4)
            size=expected*width*height*4
            if not data['success'] or data['video_frames']!=expected or output.stat().st_size!=size or reference.stat().st_size!=size:
                raise ValueError('Dropped/extra frames or incomplete movie: '+name)
            old=np.memmap(reference,dtype=np.uint8,mode='r');new=np.memmap(output,dtype=np.uint8,mode='r')
            maximum=0;different=0
            for at in range(0,size,4*1024*1024):
                difference=np.abs(old[at:at+4*1024*1024].astype(np.int16)-new[at:at+4*1024*1024].astype(np.int16))
                maximum=max(maximum,int(difference.max()));different+=int(np.count_nonzero(difference))
            tolerance=0
            if maximum>tolerance:
                raise ValueError(f'{name} exceeds declared RGB tolerance: {maximum}>{tolerance}')
            native_hash=sha(output)
            if native_hash!=data['video_rgba_sha256']:
                raise ValueError('Movie delivered pixels do not match report hash')
            return {'movie':str(movie.relative_to(ROOT)),'frames':expected,'width':width,'height':height,
                    'pixels_compared':expected*width*height,'max_channel_difference':maximum,
                    'different_channels':different,'tolerance':tolerance,'native_rgba_sha256':native_hash,
                    'container_frame_slots':int(probe['nb_frames']),
                    'reference_rgba_sha256':sha(reference),'playback':data}

        # Installed clips run to EOF; two independent decoders at a time keeps
        # callback delivery bounded without making the suite wait for each clip.
        with ThreadPoolExecutor(max_workers=2) as pool:
            for case in pool.map(check,movies):
                report['cases'].append(case)
        run('cancel',[build/'native-movie-frame-test',out/'synthetic.avi','3',out/'cancel.json',out/'cancel.rgba'],env)
        cancelled=json.loads((out/'cancel.json').read_text())
        if not cancelled['success'] or cancelled['video_frames']!=3 or cancelled['result']!=4:
            raise ValueError('Bounded movie cancellation failed')
        report.update(cancel=cancelled,pixels_compared=sum(c['pixels_compared'] for c in report['cases']),
                      video_frames=sum(c['frames'] for c in report['cases']),success=True)
    except Exception as exc:
        report['error']=str(exc)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['inputs']=inputs;report['inputs_stable']=all(sha(ROOT/n)==h for n,h in inputs.items())
        report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
