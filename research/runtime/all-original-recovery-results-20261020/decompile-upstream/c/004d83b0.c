
void __fastcall FUN_004d83b0(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  undefined4 local_6c [26];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0x128) == 0) {
    puVar3 = local_6c;
    for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar3 = 0;
      puVar3 = puVar3 + 1;
    }
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[3] = 0x194;
    local_6c[2] = 0x50;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x1a66);
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    *(undefined4 *)(param_1 + 0x144) = 0x38;
    *(undefined4 *)(param_1 + 0x148) = 0;
  }
  puVar3 = *(undefined4 **)(param_1 + 0xc);
  for (uVar2 = *(uint *)(param_1 + 0x15a) & 0x3fffffff; uVar2 != 0; uVar2 = uVar2 - 1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  for (iVar1 = 0; iVar1 != 0; iVar1 = iVar1 + -1) {
    *(undefined1 *)puVar3 = 0;
    puVar3 = (undefined4 *)((int)puVar3 + 1);
  }
  return;
}

