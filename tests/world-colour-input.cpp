#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdexcept>
static void pause(int ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));}
static void find(Display* d,Window w,int depth,std::vector<Window>& found){
 if(depth>8)return;
 char* name=nullptr;
 if(XFetchName(d,w,&name)&&name){if(std::strncmp(name,"SceneObserver",13)==0)found.push_back(w);XFree(name);}
 Window root,parent,*children=nullptr;unsigned count=0;
 if(XQueryTree(d,w,&root,&parent,&children,&count)){for(unsigned i=0;i<count;++i)find(d,children[i],depth+1,found);if(children)XFree(children);}
}
int main()try{
 Display* d=XOpenDisplay(nullptr);if(!d)throw std::runtime_error("Private X11 missing");
 std::vector<Window> found;find(d,DefaultRootWindow(d),0,found);Window target=0;
 for(auto w:found){XWindowAttributes a;if(XGetWindowAttributes(d,w,&a)&&a.width>=800&&a.height>=600&&a.map_state==IsViewable){target=w;break;}}
 if(!target)throw std::runtime_error("SceneObserver window unavailable");
 XSetInputFocus(d,target,RevertToParent,CurrentTime);XSync(d,False);
 for(int pass=0;pass<1;++pass){
  const int x0=160,y0=80,x1=700,y1=450;
  XWarpPointer(d,None,target,0,0,0,0,x0,y0);XSync(d,False);pause(100);
  XTestFakeButtonEvent(d,1,True,CurrentTime);XSync(d,False);pause(150);
  for(int step=1;step<=12;++step){XWarpPointer(d,None,target,0,0,0,0,x0+(x1-x0)*step/12,y0+(y1-y0)*step/12);XSync(d,False);pause(45);}
  pause(500);XTestFakeButtonEvent(d,1,False,CurrentTime);XSync(d,False);pause(250);
 }
 XCloseDisplay(d);std::cout<<"{\"private_x11_drag_actions\":1,\"original_memory_written\":false,\"semantic_outcomes_assumed\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
