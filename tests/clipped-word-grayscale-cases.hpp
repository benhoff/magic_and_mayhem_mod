#pragma once
// Authored input geometry shared by independent original/native executables.
// This table contains no clipping or pixel-conversion implementation.
namespace grayscale_fixture {
struct Case {int width,height,x,y,originX,originY,left,top,right,bottom;bool transparent;};
constexpr Case cases[]={
 {256,256,0,0,0,0,0,0,256,256,false},
 {37,41,0,96,0,0,0,96,37,137,true},
 {37,41,-8,-12,2,3,1,1,21,25,true},
 {37,41,12,8,-3,-2,17,13,34,32,true},
 {37,41,400,400,0,0,0,0,37,41,true},
 {37,41,0,96,0,0,0,96,35,137,true},
 {37,41,0,96,0,0,2,96,37,137,true},
 {37,41,0,96,0,0,0,100,37,130,true},
 {1,1,255,255,0,0,0,0,256,256,false},
 {1,1,256,255,0,0,0,0,256,256,false},
 {7,1,5,8,-1,0,0,0,256,256,false},
 {1,7,8,5,0,2,0,4,256,256,true},
 {37,41,-32768,-32768,0,0,0,0,256,256,true},
 {37,41,32767,32767,0,0,0,0,256,256,true},
 {3,3,0,0,1,1,0,0,256,256,true},
 {37,41,220,225,0,0,0,0,256,256,true}
};
constexpr unsigned count=sizeof(cases)/sizeof(cases[0]);
}
