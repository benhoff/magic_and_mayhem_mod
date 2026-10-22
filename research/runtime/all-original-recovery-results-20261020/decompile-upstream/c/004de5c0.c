
void __fastcall FUN_004de5c0(int param_1)

{
  int iVar1;
  
  *(undefined4 *)(param_1 + 0x42) = 0xffffffff;
  iVar1 = FUN_0058d320(s_bitmaps_timer_background_jpg_005e3244);
  if (iVar1 == 0) {
    MessageBoxA((HWND)0x0,s_Could_not_load_timer_background_005e3224,s_INITIALISATION_ERROR_005da35c
                ,0x42030);
  }
  FUN_0058db80(0,0,0xff,0x14);
  FUN_004ddd60();
  return;
}

