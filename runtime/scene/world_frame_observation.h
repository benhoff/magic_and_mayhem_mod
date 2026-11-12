/* Optional observer identity. Only a completed queue on its actual owner may
 * be named. No engine pointers or shared channel writes cross this interface. */
static u32 frame_observation_owner,frame_observation_phase;
static u32 frame_observation_identity[8];
static void frame_observation_begin(u32 owner){
 __atomic_store_n(&frame_observation_phase,1,__ATOMIC_RELEASE);
 __atomic_store_n(&frame_observation_owner,owner,__ATOMIC_RELEASE);
}
static void frame_observation_returned(u32 low,u32 high,u32 nonce_low,u32 nonce_high,u32 owner){
 frame_observation_identity[0]=1;frame_observation_identity[1]=32;
 frame_observation_identity[2]=0x40209ca7;
 frame_observation_identity[3]=low;frame_observation_identity[4]=high;
 frame_observation_identity[5]=nonce_low;frame_observation_identity[6]=nonce_high;
 frame_observation_identity[7]=owner;
 __atomic_store_n(&frame_observation_phase,2,__ATOMIC_RELEASE);
}
static u32 frame_observation_read(u32 owner,u32* out,u32 bytes){
 /* Foreign readers never touch mutable identity words. The single owner
  * cannot begin another queue concurrently with its own read. */
 if(!owner||owner!=__atomic_load_n(&frame_observation_owner,__ATOMIC_ACQUIRE)||bytes!=32||!out||
    __atomic_load_n(&frame_observation_phase,__ATOMIC_ACQUIRE)!=2)return 0;
 for(u32 i=0;i<8;++i)out[i]=frame_observation_identity[i];
 return 1;
}
