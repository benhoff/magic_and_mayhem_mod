"""Independent Indeo/YUV normalization and exact movie viewport comparison."""
import ctypes
import ctypes.util
import hashlib
import subprocess
import numpy as np
from rendering_pilot import require


def decoded_rgba(movie, width, height):
    """Independent FFmpeg planes and the established limited BT.601 policy.

    Mirrors the pinned reference policy in test-native-movie-frames, using no
    Qt/native output as input. The complete result must also match its old digest.
    """
    sws = ctypes.CDLL(ctypes.util.find_library('swscale'))
    av = ctypes.CDLL(ctypes.util.find_library('avutil'))
    av.av_get_pix_fmt.argtypes = [ctypes.c_char_p]; av.av_get_pix_fmt.restype = ctypes.c_int
    sws.sws_getContext.argtypes = [ctypes.c_int]*7+[ctypes.c_void_p]*3
    sws.sws_getContext.restype = ctypes.c_void_p
    pointer = ctypes.POINTER(ctypes.c_uint8); pointers = pointer*4; strides = ctypes.c_int*4
    sws.sws_scale.argtypes = [ctypes.c_void_p,ctypes.POINTER(pointer),ctypes.POINTER(ctypes.c_int),ctypes.c_int,ctypes.c_int,ctypes.POINTER(pointer),ctypes.POINTER(ctypes.c_int)]
    sws.sws_scale.restype = ctypes.c_int; sws.sws_freeContext.argtypes = [ctypes.c_void_p]
    context = sws.sws_getContext(width,height,av.av_get_pix_fmt(b'yuv410p'),width,height,av.av_get_pix_fmt(b'yuv420p'),4,None,None,None)
    require(context, 'Reference normalizer allocation')
    child = subprocess.Popen(['ffmpeg','-v','error','-threads','2','-i',str(movie),'-map','0:v:0','-fps_mode','passthrough','-f','rawvideo','-pix_fmt','yuv410p','-'],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    size = width*height*9//8
    try:
        while True:
            raw = child.stdout.read(size)
            if not raw: break
            require(len(raw)==size, 'Truncated independent YUV frame')
            source = ctypes.create_string_buffer(raw); dest = ctypes.create_string_buffer(width*height*3//2)
            sp = pointers(*[ctypes.cast(ctypes.addressof(source)+off,pointer) for off in (0,width*height,width*height*17//16,0)])
            dp = pointers(*[ctypes.cast(ctypes.addressof(dest)+off,pointer) for off in (0,width*height,width*height*5//4,0)])
            require(sws.sws_scale(context,sp,strides(width,width//4,width//4,0),0,height,dp,strides(width,width//2,width//2,0))==height,'Incomplete YUV normalization')
            values=np.frombuffer(dest,dtype=np.uint8)
            y=values[:width*height].reshape(height,width).astype(np.int32)
            u=values[width*height:width*height*5//4].reshape(height//2,width//2).astype(np.int32).repeat(2,axis=0).repeat(2,axis=1)-128
            v=values[width*height*5//4:].reshape(height//2,width//2).astype(np.int32).repeat(2,axis=0).repeat(2,axis=1)-128
            rgba=np.empty((height,width,4),dtype=np.uint8); luma=(y-16)*298
            rgba[:,:,0]=np.clip((luma+409*v+128)//256,0,255)
            rgba[:,:,1]=np.clip((luma-100*u-208*v-128)//256,0,255)
            rgba[:,:,2]=np.clip((luma+516*u+128)//256,0,255); rgba[:,:,3]=255
            yield rgba.tobytes()
        error=child.stderr.read(); require(child.wait(timeout=20)==0,'Independent decode failed: '+error.decode())
    finally:
        sws.sws_freeContext(context)
        if child.poll() is None: child.kill(); child.wait()
        child.stdout.close(); child.stderr.close()


def compare_paint(row, body, frame, width, height):
    require(row['transport']=='opengl' and row['gpu_read_complete'] is True and row['pixels_captured'] is True
            and row['native_gpu_frame'] is False and row['lease']=='0' and row['world']=='0'
            and row['scaling'] in (0,2) and (row['frame_width'],row['frame_height'])==(width,height), 'Wrong movie paint source/transport')
    w,h=row['width'],row['height']; left,top,iw,ih=[row[k] for k in ('left','top','image_width','image_height')]
    require(all(type(v) is int for v in (w,h,left,top,iw,ih)) and 0<w<=4096 and 0<h<=4096 and min(iw,ih)>0
            and 0<=left<=w-iw and 0<=top<=h-ih and len(body)==w*h*4 and len(frame)==width*height*4, 'Movie paint extent')
    scale=min(w/width,h/height)
    if row['scaling']==2 and scale>=1: scale=int(scale)
    # Qt positive qRound is floor(x+0.5).
    ew,eh=int(width*scale+.5),int(height*scale+.5)
    require((left,top,iw,ih)==((w-ew)//2,(h-eh+1)//2,ew,eh),'Movie letterbox geometry differs')
    x=((2*np.arange(iw,dtype=np.int64)+1)*width)//(2*iw)
    y=((2*np.arange(ih,dtype=np.int64)+1)*height)//(2*ih)
    expected=np.zeros((h,w,4),dtype=np.uint8); expected[:,:,3]=255
    expected[top:top+ih,left:left+iw]=np.frombuffer(frame,dtype=np.uint8).reshape(height,width,4)[y[:,None],x]
    actual=np.frombuffer(body,dtype=np.uint8).reshape(h,w,4)
    differences=int(np.count_nonzero(np.any(actual!=expected,axis=2)))
    require(differences==0,'Movie viewport pixels differ: '+str(differences))
    return w*h
