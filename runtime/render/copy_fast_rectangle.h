/* Diagnostic argument geometry, including refused Fast coordinates. Unsigned
 * arithmetic preserves raw PE32 words without signed-overflow UB. Admission
 * still checks the original x/y and full source/target bounds independently. */
static void game_copy_fast_rectangle(const i32* source,u32 x,u32 y,i32* target){
    target[0]=(i32)x;target[1]=(i32)y;
    target[2]=(i32)(x+(u32)source[2]-(u32)source[0]);
    target[3]=(i32)(y+(u32)source[3]-(u32)source[1]);
}
