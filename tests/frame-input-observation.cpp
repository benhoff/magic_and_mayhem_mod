#include <cassert>
#include <cstdint>
using u32=std::uint32_t;
using i32=std::int32_t;
#define WIN
#define __declspec(x)
static u32 thread=77,tick=100;
static u32 GetCurrentThreadId(){return thread;}
static u32 GetTickCount(){return ++tick;}
static int input_desktop_coordinates=1;
#include "../runtime/scene/world_frame_observation.h"
#include "../runtime/render/cursor_observation.h"
int main(){
 u32 out[16]={};
 assert(!frame_observation_read(77,out,32));
 frame_observation_begin(77);assert(!frame_observation_read(77,out,32));
 frame_observation_returned(1,0,123,456,77);
 assert(!frame_observation_read(0,out,32));assert(!frame_observation_read(78,out,32));
 assert(!frame_observation_read(77,out,31));assert(!frame_observation_read(77,nullptr,32));
 assert(frame_observation_read(77,out,32));assert(out[3]==1&&out[5]==123&&out[6]==456&&out[7]==77);
 frame_observation_begin(77);assert(!frame_observation_read(77,out,32));
 frame_observation_returned(0,1,124,457,77);assert(frame_observation_read(77,out,32));assert(out[3]==0&&out[4]==1&&out[5]==124);
 i32 point[2]={-40,205};
 cursor_observation_complete(2,point,0x123);assert(!RenderCursorObservationV1(77,out,64));
 cursor_observation_enabled=1;cursor_observation_complete(4,point,0x123);
 assert(RenderCursorObservationV1(77,out,64));assert(out[5]==1&&out[6]==4&&out[8]==u32(-40)&&out[9]==205&&out[12]==0x123);
 assert(!RenderCursorObservationV1(78,out,64));assert(!RenderCursorObservationV1(77,out,63));assert(!RenderCursorObservationV1(77,nullptr,64));
 thread=78;assert(!RenderCursorObservationV1(77,out,64));assert(!RenderCursorObservationV1(78,out,64));
 cursor_observation_complete(6,point,0x456);assert(RenderCursorObservationV1(78,out,64));assert(out[5]==1&&out[12]==0x456);
 thread=77;assert(RenderCursorObservationV1(77,out,64));assert(out[12]==0x123);
 cursor_observation_invalidate();assert(!RenderCursorObservationV1(77,out,64));
 cursor_observation_complete(8,point,0x789);assert(RenderCursorObservationV1(77,out,64));assert(out[5]==2&&out[6]==8);
 for(thread=79;thread<93;++thread)cursor_observation_complete(thread*2,point,thread);
 thread=93;cursor_observation_complete(188,point,93);assert(!RenderCursorObservationV1(93,out,64));
 thread=77;assert(RenderCursorObservationV1(77,out,64));assert(out[12]==0x789);
}
