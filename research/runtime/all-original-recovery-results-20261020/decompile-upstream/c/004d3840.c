
/* WARNING: Type propagation algorithm not settling */

void __fastcall FUN_004d3840(int param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  int local_70 [27];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0xc) == 0) {
    piVar1 = local_70;
    for (iVar3 = 0x1b; piVar1 = (int *)((int)piVar1 + 4), iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar1 = 0;
    }
    local_70[1] = 0x6c;
    local_70[2] = 7;
    local_4 = 0x40;
    local_70[4] = 0xa4;
    local_70[3] = 0x50;
    FUN_0058ad90(local_70 + 1,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x11dd);
    FUN_0058bc10(0x1f);
    FUN_0058d0e0(0,0,0xff);
    if (((DAT_00689920 == 1) || (DAT_00689920 == 2)) || (DAT_00689920 == 4)) {
      if (DAT_00644520 != -1) {
        local_70[0] = 0;
        piVar1 = (int *)((int)&DAT_00658f25 + DAT_00644520 * 0x133);
        if (*piVar1 < 0) {
          piVar1 = local_70;
        }
        iVar3 = FUN_0058d1a0(s_bitmaps_Cornelius_Young_bmp_005e208c + *piVar1 * 0x100);
        if (iVar3 == 0) {
          iVar3 = FUN_0058d1a0(s_bitmaps_Cornelius_Young_bmp_005e208c);
          if (iVar3 == 0) {
            MessageBoxA((HWND)0x0,s_Could_not_load_wizard_face_bmp_005e2cfc,
                        s_INITIALISATION_ERROR_005da35c,0x42030);
            DestroyWindow(DAT_00656614);
          }
        }
      }
    }
    else {
      FUN_004d30e0();
    }
  }
  if (*(int *)(param_1 + 0x1d3) == 0) {
    puVar2 = (undefined4 *)FUN_00597850(0x10);
    if (puVar2 != (undefined4 *)0x0) {
      puVar2[1] = &LAB_004d3290;
      puVar2[2] = 0;
      puVar2[3] = param_1 + -0x21;
      *puVar2 = &PTR_LAB_005c6e44;
      *(undefined4 **)(param_1 + 0x1d3) = puVar2;
      *(undefined4 **)(param_1 + 0x30) = puVar2;
      return;
    }
    *(undefined4 *)(param_1 + 0x1d3) = 0;
    *(undefined4 *)(param_1 + 0x30) = 0;
  }
  return;
}

