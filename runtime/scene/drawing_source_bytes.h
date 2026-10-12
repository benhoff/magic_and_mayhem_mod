static void source_pending_refusal(struct Surface*,const u32*,u32);
/* External comparison oracle only. Metadata consistency is not byte ownership. */
u32 WIN SceneDrawingSourceBytesV1(void* com,u32* before,u32* after,u8* pixels,u32 capacity){
 u32 error=GetLastError();
 if(!producer_trace_mode||!bypass_writable((u32)before,64)||!bypass_writable((u32)after,64)||!pixels){SetLastError(error);return 1;}
 PRODUCER_TRACE_SCOPE;
 if(!SceneDrawingSourcePointV1(com,before,64)){SetLastError(error);return 1;}
 if((before[2]&31)!=19||before[5]||before[13]){copy(after,before,64);SetLastError(error);return 2;}
 struct Surface* source=0;for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].object&&surfaces[i].id==before[6]){source=surfaces+i;break;}
 if(!source||!source->width||!source->height||source->width>2048||source->height>2048||source->stride<source->width||source->stride>4096){copy(after,before,64);SetLastError(error);return 3;}
 if(before[12]){u32 permitted=0;
  for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp){if(frames[i].sp==0xffffffffu||frames[i].tag!=7||frames[i].owner!=GetCurrentThreadId()||frames[i].ecx!=source->object||frames[i].primitive){source_pending_refusal(source,before,1);copy(after,before,64);SetLastError(error);return 2;}++permitted;}
  if(permitted!=1){source_pending_refusal(source,before,2);copy(after,before,64);SetLastError(error);return 2;}
 }
 u32 bytes=source->width*source->height*2;
 if(bytes>capacity||!bypass_writable((u32)pixels,bytes)||!readable(source->pixels,source->stride*source->height*2)){copy(after,before,64);SetLastError(error);return 3;}
 for(u32 y=0;y<source->height;++y)copy(pixels+y*source->width*2,(void*)(source->pixels+y*source->stride*2),source->width*2);
 if(!SceneDrawingSourcePointV1(com,after,64)){SetLastError(error);return 4;}
 for(u32 i=0;i<16;++i)if(before[i]!=after[i]){SetLastError(error);return 4;}
 SetLastError(error);return 0;
}
