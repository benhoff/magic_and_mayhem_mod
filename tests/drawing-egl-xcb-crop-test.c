#define _GNU_SOURCE
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../runtime/display/egl_xcb_resize_crop.h"
typedef struct {xcb_shm_seg_t xid;uint32_t* data;unsigned width,height;} Image;
static xcb_connection_t* connection;static xcb_screen_t* screen;static xcb_window_t window;static xcb_gcontext_t gc;
static void check(xcb_void_cookie_t cookie){xcb_generic_error_t* error=xcb_request_check(connection,cookie);if(error){fprintf(stderr,"XCB error %u\n",error->error_code);free(error);exit(3);}}
static Image image(unsigned w,unsigned h){
 int id=shmget(IPC_PRIVATE,(size_t)w*h*4,IPC_CREAT|0600);if(id<0)exit(4);void* data=shmat(id,0,0);if(data==(void*)-1)exit(5);Image result={xcb_generate_id(connection),data,w,h};check(xcb_shm_attach_checked(connection,result.xid,(uint32_t)id,0));if(shmctl(id,IPC_RMID,0))exit(6);return result;
}
static uint32_t pattern(unsigned x,unsigned y){unsigned word=((y%31)<<11)|((x%63)<<5)|((y+x)%31);return ((word>>11)*255/31<<16)|(((word>>5)&63)*255/63<<8)|((word&31)*255/31);}
static void fill(Image image,unsigned w,unsigned h,unsigned yoffset){memset(image.data,0,(size_t)image.width*image.height*4);for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)image.data[(y+yoffset)*image.width+x]=pattern(x,y);}
static unsigned upload(Image image,unsigned w,unsigned h,const uint32_t args[16]){
 uint32_t size[2]={w,h};check(xcb_configure_window_checked(connection,window,XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT,size));
 check(xcb_shm_put_image_checked(connection,window,gc,(uint16_t)args[1],(uint16_t)args[2],(uint16_t)args[3],(uint16_t)args[4],(uint16_t)args[5],(uint16_t)args[6],(int16_t)args[7],(int16_t)args[8],24,XCB_IMAGE_FORMAT_Z_PIXMAP,0,image.xid,0));
 xcb_generic_error_t* error=0;xcb_get_image_reply_t* reply=xcb_get_image_reply(connection,xcb_get_image(connection,XCB_IMAGE_FORMAT_Z_PIXMAP,window,0,0,(uint16_t)w,(uint16_t)h,0xffffffffu),&error);if(error||!reply||xcb_get_image_data_length(reply)!=(int)(w*h*4))exit(7);uint32_t* data=(uint32_t*)xcb_get_image_data(reply);unsigned bad=0;for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)bad+=(data[y*w+x]&0xffffffu)!=pattern(x,y);free(reply);return bad;
}
static void packet(Image image,uint32_t args[16]){uint32_t value[16]={window,image.width,image.height,0,0,image.width,image.height,0,0,24,2,0,0,0,image.xid,0};memcpy(args,value,sizeof(value));}
int main(void){
 connection=xcb_connect(0,0);if(xcb_connection_has_error(connection))return 1;const xcb_setup_t* setup=xcb_get_setup(connection);screen=xcb_setup_roots_iterator(setup).data;if(!screen||screen->root_depth!=24||setup->image_byte_order!=XCB_IMAGE_ORDER_LSB_FIRST)return 2;
 window=xcb_generate_id(connection);check(xcb_create_window_checked(connection,24,window,screen->root,0,0,800,600,0,XCB_WINDOW_CLASS_INPUT_OUTPUT,screen->root_visual,0,0));check(xcb_map_window_checked(connection,window));gc=xcb_generate_id(connection);check(xcb_create_gc_checked(connection,gc,window,0,0));Image old=image(832,640),fresh=image(640,512);uint32_t before[16],after[16];
 MnmEglStaleImage scope={800,600,640,480,window,1,old.xid,0,832,640,1};packet(old,before);fill(old,640,480,120);unsigned uncorrected=upload(old,640,480,before);if(!uncorrected||!mnm_egl_crop_stale_image(&scope,before,after))return 8;unsigned corrected=upload(old,640,480,after);if(corrected)return 9;
 unsigned cases=1;scope.width=800;scope.height=600;fill(old,800,600,0);if(mnm_egl_crop_stale_image(&scope,before,after)||memcmp(before,after,sizeof(before))||upload(old,800,600,after))return 10;++cases;
 scope.width=640;scope.height=480;packet(fresh,before);fill(fresh,640,480,0);if(mnm_egl_crop_stale_image(&scope,before,after)||memcmp(before,after,sizeof(before))||upload(fresh,640,480,after))return 11;++cases;
 scope.old_width=640;scope.old_height=480;scope.width=800;scope.height=600;scope.shm_segment=fresh.xid;scope.total_width=640;scope.total_height=512;packet(old,before);fill(old,800,600,0);if(mnm_egl_crop_stale_image(&scope,before,after)||memcmp(before,after,sizeof(before))||upload(old,800,600,after))return 12;++cases;
 scope.width=640;scope.height=480;packet(fresh,before);if(mnm_egl_crop_stale_image(&scope,before,after)||memcmp(before,after,sizeof(before))||upload(fresh,640,480,after))return 13;++cases;
 scope=(MnmEglStaleImage){800,600,640,480,window,1,old.xid,0,832,640,1};packet(old,before);
 for(unsigned test=0;test<6;++test){MnmEglStaleImage changed=scope;uint32_t args[16];memcpy(args,before,sizeof(args));if(test==0)changed.full_gl_blit=0;if(test==1)changed.window++;if(test==2)changed.generation=0;if(test==3)changed.shm_segment++;if(test==4)args[4]=1;if(test==5)args[6]=480;if(mnm_egl_crop_stale_image(&changed,args,after)||memcmp(args,after,sizeof(args)))return 14;}
 check(xcb_shm_detach_checked(connection,old.xid));check(xcb_shm_detach_checked(connection,fresh.xid));shmdt(old.data);shmdt(fresh.data);xcb_disconnect(connection);printf("{\"success\":true,\"cases\":%u,\"stale_uncorrected\":%u,\"stale_corrected\":%u,\"refusals\":6}\n",cases,uncorrected,corrected);return 0;
}
