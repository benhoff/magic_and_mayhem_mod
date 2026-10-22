/* Owned Xvfb fixture peer. Root bytes are external oracles, never native input. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <stdio.h>
int main(int argc,char** argv){
 if(argc!=2)return 1;
 Display* display=XOpenDisplay(0);if(!display)return 2;
 XImage* image=XGetImage(display,DefaultRootWindow(display),0,0,800,600,AllPlanes,ZPixmap);
 if(!image)return 3;
 FILE* output=fopen(argv[1],"wb");if(!output)return 4;
 for(int y=0;y<600;++y)for(int x=0;x<800;++x){
  unsigned long pixel=XGetPixel(image,x,y);
  if(image->red_mask!=0xff0000||image->green_mask!=0xff00||image->blue_mask!=0xff)return 5;
  unsigned char rgba[4]={(unsigned char)(pixel>>16),(unsigned char)(pixel>>8),(unsigned char)pixel,255};
  if(fwrite(rgba,1,4,output)!=4)return 6;
 }
 if(fclose(output))return 7;
 XDestroyImage(image);XCloseDisplay(display);return 0;
}
