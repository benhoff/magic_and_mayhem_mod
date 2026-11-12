#ifndef MNM_MINIMAP_MARKER_ROW_H
#define MNM_MINIMAP_MARKER_ROW_H
/* 0x553850 reads rows[(u+v)/2] once, then addresses the thirteen
 * stores with the object's pixel stride. Unused neighbouring table entries
 * need not be canonical or readable. Destination store bounds are checked by
 * the native minimap raster before it changes the owned plane. */
typedef int (*MnmMinimapReadRow)(void*,unsigned,unsigned*);
static int mnm_minimap_marker_row(unsigned rows,int row,unsigned pixels,
 unsigned stride,unsigned object_stride,int ox,int oy,MnmMinimapReadRow read,
 void* context,long long* entry,long long* expected){
 *entry=(long long)rows+(long long)row*4;
 *expected=(long long)pixels+(((long long)oy+row)*stride+ox)*2;
 if(!stride||stride>4096||object_stride!=stride||!read||
    *entry<=0||*entry>0xfffffffcll||*expected<0||*expected>0xfffffffell)return 0;
 unsigned actual=0;
 return read(context,(unsigned)*entry,&actual)&&actual==(unsigned)*expected;
}
#endif
