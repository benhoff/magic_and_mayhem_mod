/* Intentional native policy for a hidden original Spellbox redraw after a
 * High->Low rebuild. Never broaden COM rectangle admission or execute a
 * partially visible copy here. The original void wrapper retries INVALIDRECT;
 * this exact wholly invisible operation cannot affect either owned canvas. */
static int nd_hidden_spellbox_copy(u32 index,u32 caller,const u8* instruction,
 u32 sw,u32 sh,u32 sb,u32 dw,u32 dh,u32 db,const u32* r,u32 x,u32 y){
 static const u8 pinned[5]={0xe8,0xa6,0x53,0x01,0x00};
 if(index!=15 || caller!=0x5770fa || !instruction || !r ||
    sw!=640 || sh!=480 || dw!=640 || dh!=480 || sb!=16 || db!=16)return 0;
 for(u32 i=0;i<5;++i)if(instruction[i]!=pinned[i])return 0;
 if(r[0]>0x7fffffffu || r[1]>0x7fffffffu || r[2]>0x7fffffffu || r[3]>0x7fffffffu ||
    r[2]<=r[0] || r[3]<=r[1] || r[2]-r[0]!=40 || r[3]-r[1]!=40 ||
    x!=r[0] || y!=r[1])return 0;
 return r[0]>=sw || r[1]>=sh;
}
