
void __fastcall FUN_004de360(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  bool bVar3;
  undefined4 local_6c [26];
  undefined4 local_4;
  
  *(undefined4 *)(param_1 + 4) = 0;
  if (*(int *)(param_1 + 0xc) == 0) {
    puVar1 = local_6c;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar1 = 0;
      puVar1 = puVar1 + 1;
    }
    local_6c[3] = 0x28;
    local_6c[2] = 0x28;
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x2695);
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    bVar3 = DAT_006de6d5 != 0;
    *(uint *)(param_1 + 0x2c) = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x12e;
    *(uint *)(param_1 + 0x28) = (-(uint)bVar3 & 0xa0) + 0x220;
    if (*(int *)(param_1 + 0x53) == 0) {
      puVar1 = (undefined4 *)FUN_00597850(0xc);
      if (puVar1 == (undefined4 *)0x0) {
        puVar1 = (undefined4 *)0x0;
      }
      else {
        puVar1[1] = FUN_004ddcc0;
        puVar1[2] = param_1;
        *puVar1 = &PTR_LAB_005c6f78;
      }
      *(undefined4 **)(param_1 + 0x53) = puVar1;
      *(undefined4 **)(param_1 + 0x30) = puVar1;
    }
    iVar2 = FUN_0057d310(s_sprites_timer_spr_005e3280,2);
    if (iVar2 == 0) {
      MessageBoxA((HWND)0x0,s_Could_not_load_timer_spr_005e3264,s_INITIALISATION_ERROR_005da35c,
                  0x42030);
      DestroyWindow(DAT_00656614);
    }
  }
  return;
}

