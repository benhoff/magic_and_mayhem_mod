/* The provider supplies opaque identities only after physical retirement.
 * Called under provider -> copy admission, then ordinary tracker admission.
 * Never query a retired COM interface or manufacture a successful retirement
 * while accepted queued copies or CPU/DC writers still retain its backing. */
static u32 (WIN *game_native_retirements)(void**,u32);
static void game_native_lifetime_drain(void){
 if(!game_native_retirements)return;
 void* objects[32];u32 count;
 while((count=game_native_retirements(objects,32))!=0){
  for(u32 i=0;i<count;++i){
   game_lock_retire(objects[i]);
   int token=history_enter();if(token)history_release(history_find(objects[i]),0);history_leave(token);
  }
 }
}
