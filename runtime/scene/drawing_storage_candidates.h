/* Address/VirtualQuery metadata only. No source bytes or private driver reads. */
static u32 storage_candidate_rows;
static void storage_candidates(u32 query,u32* point,void* storage,u32 row_pitch){
 if(point[2]!=3||!storage)return; /* Live unknown only; never pending/ambiguous/stopped. */
 u32 input[7]={0},input_ok=VirtualQuery(storage,input,28)==28;
 u32 count=0;
 for(u32 i=0;i<MAX_SURFACES;++i){struct Surface* s=surfaces+i;
  if(!s->object||!s->pixels||!s->width||!s->height||s->width>2048||s->height>2048||s->stride<s->width||s->stride>4096||s->stride*2!=row_pitch)continue;
  if(++count>16||storage_transition_records>=131071)ExitProcess(99);
  u32 canvas[7]={0},canvas_ok=VirtualQuery((void*)s->pixels,canvas,28)==28;
  u32 address=(u32)storage,n=row_pitch*s->height,r[24]={0};
  r[0]=++storage_transition_records;r[1]=5;r[2]=query;r[3]=s->id;r[4]=count;r[5]=1;
  if(canvas_ok)r[5]|=2;if(input_ok)r[5]|=4;
  if(canvas_ok&&input_ok&&canvas[1]&&canvas[1]==input[1]&&(canvas[4]==0x1000||canvas[4]==0x2000)&&(input[4]==0x1000||input[4]==0x2000))r[5]|=8;
  if(address>=s->pixels&&address-s->pixels<n)r[5]|=16;
  if(s->pixels>=address&&s->pixels-address<n)r[5]|=32;
  r[6]=address>=s->pixels?address-s->pixels:s->pixels-address;r[7]=address>=s->pixels;
  r[8]=s->width;r[9]=s->height;r[10]=s->stride;r[11]=storage_address_id(s->pixels);
  r[12]=canvas_ok?canvas[4]:0;r[13]=canvas_ok?canvas[5]:0;r[14]=canvas_ok?canvas[6]:0;
  r[15]=input_ok?input[4]:0;r[16]=input_ok?input[5]:0;r[17]=input_ok?input[6]:0;
  r[18]=storage_address_id(address);r[19]=point[3];r[20]=point[4];r[21]=point[14];
  for(u32 j=0;j<128;++j)if(storage_transition_epochs[j].canvas==s->id){r[22]=storage_transition_epochs[j].epoch;break;}
  if(!write(storage_transition_file,r,96))ExitProcess(99);++storage_candidate_rows;
 }
}
