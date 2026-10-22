
void __fastcall FUN_004d9530(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  int *piVar4;
  bool bVar5;
  int local_6c [26];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0x5a4) == 0) {
    piVar4 = local_6c;
    for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
      *piVar4 = 0;
      piVar4 = piVar4 + 1;
    }
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[3] = 0x48;
    local_6c[2] = 0xb4;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x1c99);
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    bVar5 = DAT_006de6d5 != 0;
    *(undefined4 *)(param_1 + 0x5c4) = 100;
    *(uint *)(param_1 + 0x5c0) = ((-(uint)bVar5 & 0xa0) + 0x280) - local_6c[3];
    uVar2 = 0;
    if (*(int *)(param_1 + 0x594) != 0) {
      puVar3 = (undefined4 *)(param_1 + 8);
      do {
        *(int *)((int)puVar3 + 0x35) = param_1 + 0x5a0;
        *puVar3 = 0xffffffff;
        *(undefined4 *)((int)puVar3 + 0x7e) = 0;
        *(undefined4 *)((int)puVar3 + 0x86) = 3;
        uVar2 = uVar2 + 1;
        puVar3 = (undefined4 *)((int)puVar3 + 0x8a);
      } while (uVar2 < *(uint *)(param_1 + 0x594));
    }
    if (*(int *)(param_1 + 0x61f) == 0) {
      puVar3 = (undefined4 *)FUN_00597850(0xc);
      if (puVar3 != (undefined4 *)0x0) {
        puVar3[1] = FUN_004d96d0;
        puVar3[2] = param_1;
        *puVar3 = &PTR_LAB_005c6ed0;
        *(undefined4 **)(param_1 + 0x61f) = puVar3;
        *(undefined4 **)(param_1 + 0x5c8) = puVar3;
        return;
      }
      *(undefined4 *)(param_1 + 0x61f) = 0;
      *(undefined4 *)(param_1 + 0x5c8) = 0;
    }
  }
  else {
    FUN_0058bc10(0x1f);
    uVar2 = 0;
    if (*(int *)(param_1 + 0x594) != 0) {
      puVar3 = (undefined4 *)(param_1 + 0x86);
      do {
        *puVar3 = 0;
        *(undefined4 *)((int)puVar3 + -0x41) = 3;
        FUN_004a5000();
        uVar2 = uVar2 + 1;
        puVar3 = (undefined4 *)((int)puVar3 + 0x8a);
      } while (uVar2 < *(uint *)(param_1 + 0x594));
      return;
    }
  }
  return;
}

