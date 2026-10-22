
void __thiscall FUN_004dc4a0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 local_6c [26];
  undefined4 local_4;
  
  if (param_1[8] == 0) {
    puVar1 = local_6c;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar1 = 0;
      puVar1 = puVar1 + 1;
    }
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[3] = 0x256;
    local_6c[2] = 0x24;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x227c);
    param_1[0xf] = param_2;
    param_1[0x10] = param_3;
  }
  if (*(int *)((int)param_1 + 0x52) == 0) {
    puVar1 = local_6c;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar1 = 0;
      puVar1 = puVar1 + 1;
    }
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[3] = 0x256;
    local_6c[2] = 0x24;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x2288);
  }
  FUN_004dc160();
  param_1[3] = 0x24;
  param_1[2] = 0;
  *param_1 = 0;
  param_1[1] = 0;
  param_1[5] = param_4;
  param_1[6] = 0;
  param_1[4] = (int)(0x256 / (longlong)param_4);
  if (param_1[0x20] == 0) {
    puVar1 = (undefined4 *)FUN_00597850(0xc);
    if (puVar1 != (undefined4 *)0x0) {
      puVar1[1] = &LAB_004dc150;
      puVar1[2] = param_1;
      *puVar1 = &PTR_LAB_005c6f3c;
      param_1[0x20] = puVar1;
      param_1[0x11] = puVar1;
      return;
    }
    param_1[0x20] = 0;
    param_1[0x11] = 0;
  }
  return;
}

