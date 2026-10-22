
void __fastcall FUN_004dadb0(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 local_6c [26];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0xc) == 0) {
    puVar1 = local_6c;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar1 = 0;
      puVar1 = puVar1 + 1;
    }
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[3] = 0x208;
    local_6c[2] = 0x16c;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x1fd2);
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0xff,0xff,0);
    }
    if (*(int *)(param_1 + 0x1f6) == 0) {
      puVar1 = (undefined4 *)FUN_00597850(0x10);
      if (puVar1 != (undefined4 *)0x0) {
        puVar1[1] = &LAB_004daa20;
        puVar1[2] = 0;
        puVar1[3] = param_1 + -0x21;
        *puVar1 = &PTR_LAB_005c6f1c;
        *(undefined4 **)(param_1 + 0x1f6) = puVar1;
        *(undefined4 **)(param_1 + 0x30) = puVar1;
        return;
      }
      *(undefined4 *)(param_1 + 0x1f6) = 0;
      *(undefined4 *)(param_1 + 0x30) = 0;
    }
  }
  return;
}

