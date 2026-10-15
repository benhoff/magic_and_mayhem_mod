/* Pure admission for the owned facade's fresh VirtualAlloc zero policy.
 * identity is process-local, never a driver descriptor or shared pointer. */
static int mnm_owned_initial_identity(const u32* identity,u32 width,u32 height){
    return identity[0]&&width&&width<=2048&&height&&height<=2048&&
        identity[1]==width&&identity[2]==height&&identity[3]>=width*2&&
        identity[3]<=4096&&!(identity[3]&1)&&
        !(identity[4]|identity[5]|identity[6]|identity[7]);
}
static int mnm_owned_initial_zero(const u8* pixels,u32 width,u32 height,u32 pitch){
    for(u32 y=0;y<height;++y)for(u32 x=0;x<width*2;++x)if(pixels[y*pitch+x])return 0;
    return 1;
}
