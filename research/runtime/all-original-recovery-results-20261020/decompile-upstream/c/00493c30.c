
void __fastcall FUN_00493c30(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  int *piVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  int local_fc [63];
  
  iVar1 = FUN_0049cdb0();
  piVar6 = local_fc;
  for (iVar5 = 0x3f; iVar5 != 0; iVar5 = iVar5 + -1) {
    *piVar6 = 0;
    piVar6 = piVar6 + 1;
  }
  local_fc[5] = 0xffffffff;
  local_fc[4] = 0xffffffff;
  local_fc[3] = 0xffffffff;
  local_fc[2] = 0xffffffff;
  local_fc[1] = 0xffffffff;
  local_fc[0] = -1;
  local_fc[9] = 0xffffffff;
  local_fc[8] = 0xffffffff;
  local_fc[7] = 0xffffffff;
  local_fc[6] = 0xffffffff;
  local_fc[0xc] = 7;
  local_fc[10] = 1;
  local_fc[0xb] = 0xffffffff;
  iVar5 = FUN_0040e650(*(undefined4 *)(param_1 + 8));
  iVar2 = FUN_004132e0(*(undefined4 *)(param_1 + 0xc));
  if (iVar1 != 0) {
    local_fc[1] = *(int *)(param_1 + 0xc) << 5;
    local_fc[0] = *(int *)(param_1 + 8) << 5;
    uVar3 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x1824);
    }
    local_fc[3] = (((uVar3 & 0x7fffffff) % 0xc - 6) + iVar5) * 0x20;
    uVar3 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x1826);
    }
    uVar4 = *(int *)(param_1 + 0x10) + 5;
    local_fc[2] = *(int *)(param_1 + 0x10) << 4;
    local_fc[4] = (((uVar3 & 0x7fffffff) % 0xc - 6) + iVar2) * 0x20;
    if (DAT_006c549c - 1U <= uVar4) {
      uVar4 = DAT_006c549c - 1U;
    }
    uVar12 = *(undefined4 *)(param_1 + 0x44);
    piVar6 = local_fc;
    uVar17 = 1;
    uVar16 = 0xffffffff;
    uVar15 = 0xffffffff;
    uVar14 = 0xffffffff;
    uVar13 = 0x14;
    local_fc[5] = uVar4 << 4;
    local_fc[8] = 0xffffffff;
    local_fc[0xc] = 1;
    local_fc[6] = uVar12;
    FUN_00411530(local_fc[5]);
    uVar11 = extraout_ECX;
    FUN_00411500(local_fc[4]);
    uVar10 = extraout_ECX_00;
    FUN_004114c0(local_fc[3]);
    uVar9 = extraout_ECX_01;
    FUN_00411530(local_fc[2]);
    uVar8 = extraout_ECX_02;
    FUN_00411500(local_fc[1]);
    uVar7 = extraout_ECX_03;
    FUN_004114c0(local_fc[0]);
    iVar1 = FUN_004e1db0(uVar7,uVar8,uVar9,uVar10,uVar11,uVar12,uVar13,uVar14,uVar15,uVar16,uVar17,
                         piVar6);
    if (iVar1 != 0) {
      FUN_00493f20(0x47,local_fc);
    }
  }
  return;
}

