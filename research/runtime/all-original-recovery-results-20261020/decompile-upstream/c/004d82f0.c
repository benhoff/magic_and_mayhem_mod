
void __fastcall FUN_004d82f0(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 local_6c [26];
  undefined4 local_4;
  
  puVar2 = local_6c;
  for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  local_6c[0] = 0x6c;
  local_6c[1] = 7;
  local_4 = 0x40;
  local_6c[3] = 0x194;
  local_6c[2] = 0x50;
  FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x1a41);
  if (DAT_006de6dd != 0) {
    FUN_0058bc10(0x1f);
    FUN_0058d0e0(0,0,0xff);
    *(undefined4 *)(param_1 + 0x144) = 0x38;
    *(undefined4 *)(param_1 + 0x148) = 0;
    return;
  }
  FUN_0058bc10(0);
  *(undefined4 *)(param_1 + 0x144) = 0x38;
  *(undefined4 *)(param_1 + 0x148) = 0;
  return;
}

