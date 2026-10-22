
void __fastcall FUN_004da260(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  int local_94;
  undefined4 local_88;
  undefined4 local_84;
  int local_80;
  int local_7c;
  undefined4 local_78 [26];
  undefined4 local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c20cb;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00486950();
  local_4 = 0;
  FUN_0058a0a0();
  iVar2 = FUN_0058a0d0(s_cfg_messages_cfg_005e2fc4);
  if (iVar2 == 0) {
    MessageBoxA((HWND)0x0,s_Unable_to_load_messages_cfg_005e2fa8,s_LOAD_ERROR_005da224,0x42030);
    DestroyWindow(DAT_00656614);
  }
  FUN_00486ae0(s_bitmaps_smallscroll_bmp_005e3030);
  if (*(int *)(param_1 + 0x16d) == 0) {
    puVar3 = local_78;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = 0;
      puVar3 = puVar3 + 1;
    }
    local_78[0] = 0x6c;
    local_78[1] = 7;
    local_10 = 0x40;
    local_78[3] = *(undefined4 *)(local_94 + 4);
    local_78[2] = *(undefined4 *)(local_94 + 8);
    FUN_0058ad90(local_78,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x1e65);
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    *(undefined4 *)(param_1 + 0x189) = 10;
    *(undefined4 *)(param_1 + 0x18d) = 10;
  }
  if (*(int *)(param_1 + 0x1bf) == 0) {
    puVar3 = (undefined4 *)FUN_00597850(0x10);
    if (puVar3 == (undefined4 *)0x0) {
      puVar3 = (undefined4 *)0x0;
    }
    else {
      puVar3[1] = FUN_004d9df0;
      puVar3[2] = 0;
      puVar3[3] = param_1;
      *puVar3 = &PTR_LAB_005c6ef4;
    }
    *(undefined4 **)(param_1 + 0x1bf) = puVar3;
    *(undefined4 **)(param_1 + 0x191) = puVar3;
  }
  iVar2 = *(int *)(local_94 + 4) + -0x18;
  *(int *)(param_1 + 0x1bb) = iVar2;
  *(int *)(param_1 + 0x19b) = iVar2;
  uVar1 = *(undefined4 *)(local_94 + 4);
  *(undefined4 *)(param_1 + 0x19f) = 0;
  *(undefined4 *)(param_1 + 0x1a3) = uVar1;
  *(undefined4 *)(param_1 + 0x1a7) = *(undefined4 *)(local_94 + 8);
  *(undefined4 *)(param_1 + 0x1ab) = 0;
  *(undefined4 *)(param_1 + 0x1b3) = 0x18;
  *(undefined4 *)(param_1 + 0x1af) = 0;
  local_88 = 10;
  *(undefined4 *)(param_1 + 0x1b7) = *(undefined4 *)(local_94 + 8);
  local_84 = 10;
  local_80 = *(int *)(local_94 + 4) + 10;
  local_7c = *(int *)(local_94 + 8) + 10;
  FUN_004a4e20(&local_88,0);
  local_4 = 0xffffffff;
  FUN_00486960();
  ExceptionList = local_c;
  return;
}

