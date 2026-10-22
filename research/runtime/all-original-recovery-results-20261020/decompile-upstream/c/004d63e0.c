
void __fastcall FUN_004d63e0(int param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  bool bVar5;
  undefined4 local_6c [26];
  undefined4 local_4;
  
  if ((*(int *)(param_1 + 0x1876) == 0) && (*(int *)(param_1 + 0x1844) == 0)) {
    puVar2 = local_6c;
    for (iVar3 = 0x1b; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    iVar3 = param_1 + 0x1872;
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[3] = 0xcc;
    local_6c[2] = 0x8c;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x16e7);
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    bVar5 = DAT_006de6d5 != 0;
    *(uint *)(param_1 + 0x1896) = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x154;
    *(uint *)(param_1 + 0x1892) = (-(uint)bVar5 & 0xa0) + 0x1b4;
    uVar1 = *(undefined4 *)(param_1 + 0x1892);
    *(undefined4 *)(param_1 + 0x1914) = uVar1;
    if (*(int *)(param_1 + 0x1844) == 0) {
      puVar2 = local_6c;
      for (iVar4 = 0x1b; iVar4 != 0; iVar4 = iVar4 + -1) {
        *puVar2 = 0;
        puVar2 = puVar2 + 1;
      }
      local_6c[0] = 0x6c;
      local_6c[1] = 7;
      local_4 = 0x40;
      local_6c[2] = 0x8c;
      local_6c[3] = uVar1;
      FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x16fe);
    }
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    iVar4 = DAT_006de6d5;
    *(undefined4 *)(param_1 + 0x1860) = 0;
    *(uint *)(param_1 + 0x1864) = (-(uint)(iVar4 != 0) & 0x78) + 0x154;
    *(int *)(param_1 + 0x15c7) = iVar3;
    *(int *)(param_1 + 0x1753) = iVar3;
    *(int *)(param_1 + 0x168d) = iVar3;
    *(int *)(param_1 + 0x154d) = iVar3;
    FUN_004d47f0();
    FUN_004d47f0();
    FUN_004d47f0();
    if (*(int *)(param_1 + 0x1925) == 0) {
      puVar2 = (undefined4 *)FUN_00597850(0xc);
      if (puVar2 == (undefined4 *)0x0) {
        puVar2 = (undefined4 *)0x0;
      }
      else {
        puVar2[1] = &LAB_004d4940;
        puVar2[2] = param_1;
        *puVar2 = &PTR_LAB_005c6e98;
      }
      *(undefined4 **)(param_1 + 0x1925) = puVar2;
      *(undefined4 **)(param_1 + 0x189a) = puVar2;
    }
    iVar3 = param_1 + 0xfc;
    iVar4 = 0x1a;
    do {
      *(int *)(iVar3 + 0x31) = param_1 + 0x1840;
      FUN_004d47f0();
      iVar3 = iVar3 + 0xc6;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    *(undefined4 *)(param_1 + 0x1555) = 0;
    *(undefined4 *)(param_1 + 0x1559) = 0;
    FUN_004d4970();
    return;
  }
  FUN_004d4970();
  return;
}

