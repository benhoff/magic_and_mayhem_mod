
undefined4 FUN_004e40d0(void)

{
  int iVar1;
  int *piVar2;
  int local_6c [26];
  undefined4 local_4;
  
  piVar2 = local_6c;
  for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
  }
  local_6c[0] = 0x6c;
  if (DAT_006a2920 == 0) {
    FUN_0058a9b0((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,DAT_005e13fc);
  }
  else {
    FUN_0058aa90();
  }
  local_6c[1] = 1;
  local_4 = 0x200;
  FUN_0058ad90(local_6c,s_c__Projects_Chaos_Main_cpp_005e3328,0x171);
  if (DAT_006e1f78 != 0) {
    DAT_006e1f70 = 0x60;
    DAT_006e1f74 = 0;
  }
  piVar2 = local_6c;
  for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
  }
  local_6c[0] = 0x6c;
  local_6c[3] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280 + DAT_006e1f70;
  local_6c[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0 + DAT_006e1f74;
  local_6c[1] = 7;
  local_4 = 0x840;
  FUN_0058ad90(local_6c,s_c__Projects_Chaos_Main_cpp_005e3328,0x184);
  FUN_0058bc10(0);
  piVar2 = local_6c;
  for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
  }
  local_6c[0] = 0x6c;
  local_6c[1] = 7;
  local_4 = 0x40;
  local_6c[3] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
  local_6c[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
  DAT_006e1f88 = FUN_0058d540();
  return 1;
}

