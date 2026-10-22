
void FUN_004d30e0(void)

{
  int iVar1;
  char *pcVar2;
  
  if (DAT_00644520 == -1) {
    return;
  }
  iVar1 = FUN_00593a80();
  if (iVar1 == 0) {
    pcVar2 = s_bitmaps_Cornelius_Young_bmp_005e2d1c;
  }
  else if (iVar1 == 1) {
    pcVar2 = s_bitmaps_Cornelius_Middle_bmp_005e2d38;
  }
  else {
    if (iVar1 != 2) goto LAB_004d3146;
    pcVar2 = s_bitmaps_Cornelius_Old_bmp_005e2d58;
  }
  iVar1 = FUN_0058d1a0(pcVar2);
  if (iVar1 != 0) {
    return;
  }
LAB_004d3146:
  MessageBoxA((HWND)0x0,s_Could_not_load_wizard_face_bmp_005e2cfc,s_INITIALISATION_ERROR_005da35c,
              0x42030);
  DestroyWindow(DAT_00656614);
  return;
}

