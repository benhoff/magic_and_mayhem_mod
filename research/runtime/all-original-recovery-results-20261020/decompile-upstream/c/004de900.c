
void __fastcall FUN_004de900(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  bool bVar3;
  undefined4 local_6c [26];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0x14) == 0) {
    puVar1 = local_6c;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar1 = 0;
      puVar1 = puVar1 + 1;
    }
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[3] = 0x30;
    local_6c[2] = 0x16;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x2772);
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    bVar3 = DAT_006de6d5 != 0;
    *(uint *)(param_1 + 0x34) = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x142;
    *(uint *)(param_1 + 0x30) = (-(uint)bVar3 & 0xa0) + 0x22a;
    if (*(int *)(param_1 + 0x42) == 0) {
      puVar1 = (undefined4 *)FUN_00597850(0xc);
      if (puVar1 == (undefined4 *)0x0) {
        puVar1 = (undefined4 *)0x0;
      }
      else {
        puVar1[1] = &LAB_004de640;
        puVar1[2] = param_1;
        *puVar1 = &PTR_LAB_005c6f90;
      }
      *(undefined4 **)(param_1 + 0x42) = puVar1;
      *(undefined4 **)(param_1 + 0x38) = puVar1;
    }
    *(undefined4 *)(param_1 + 0xc) = 0;
    *(undefined4 *)(param_1 + 8) = 0;
  }
  return;
}

