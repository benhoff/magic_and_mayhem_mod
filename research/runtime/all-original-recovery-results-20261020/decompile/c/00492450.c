
/* WARNING: Removing unreachable block (ram,0x00492520) */

void __fastcall FUN_00492450(int param_1)

{
  ushort *puVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  uint uVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  int local_128;
  int local_124;
  uint local_120;
  int local_11c;
  int local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  undefined4 local_108;
  undefined4 local_104;
  undefined4 local_100;
  int local_fc [4];
  int local_ec;
  int local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  
  local_118 = DAT_006b7507;
  local_11c = *(int *)(param_1 + 0x50);
  iVar3 = *(int *)(param_1 + 0x1da);
  uVar2 = *(int *)(param_1 + 0x10) + iVar3;
  local_120 = 0;
  local_128 = 0;
  local_124 = 0;
  if (uVar2 < DAT_006c549c) {
    puVar1 = (ushort *)
             (DAT_006c54dc +
             ((&DAT_006cb942)[*(int *)(param_1 + 0xc)] + (&DAT_006cb8c2)[uVar2] +
             *(int *)(param_1 + 8)) * 0xc);
    if ((0 < iVar3) && (*puVar1 != 0)) goto LAB_00492848;
    if (puVar1[2] == 0xffff) {
      if (iVar3 == 0) {
        if ((*puVar1 == 0) || (*(uint *)(DAT_0065660c + 0x94 + (uint)*puVar1 * 0x164) < 2)) {
          uVar2 = FUN_0054e200();
          iVar3 = (uVar2 & 1) + 0x81;
          if (DAT_006e1f30 != 0) {
            uVar9 = 0x1529;
            goto LAB_0049259d;
          }
        }
        else {
          iVar3 = 0x89;
        }
      }
      else if ((iVar3 == DAT_006b7507) || (DAT_006c549c - 1 <= uVar2)) {
        uVar2 = FUN_0054e200();
        iVar3 = (uVar2 & 0x7fffffff) % 5 + 0x83;
        if (DAT_006e1f30 != 0) {
          uVar9 = 0x1535;
          goto LAB_0049259d;
        }
      }
      else {
        uVar2 = FUN_0054e200();
        iVar3 = (uVar2 & 0x7fffffff) % 0xc + 0x75;
        if (DAT_006e1f30 != 0) {
          uVar9 = 0x1531;
LAB_0049259d:
          FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,uVar9);
        }
      }
      iVar4 = ((int)puVar1 - DAT_006c54dc) / 0xc;
      local_124 = iVar4 / DAT_006c54a0;
      iVar4 = iVar4 - (&DAT_006cb8c2)[local_124];
      local_128 = FUN_004132e0(iVar4 / (int)DAT_006c5494);
      local_120 = iVar4 - (&DAT_006cb942)[local_128];
      if ((int)local_120 < 0) {
        do {
          local_120 = local_120 + DAT_006c5494;
        } while ((int)local_120 < 0);
      }
      else {
        for (; DAT_006c5494 <= local_120; local_120 = local_120 - DAT_006c5494) {
        }
      }
      puVar7 = &local_108;
      puVar10 = puVar7;
      FUN_0040eec0(&local_124);
      uVar6 = extraout_ECX;
      FUN_0040e8d0(&local_128);
      uVar9 = extraout_ECX_00;
      FUN_0040e2c0(&local_120);
      FUN_004f49e0(uVar9,uVar6,puVar7,puVar10);
      FUN_004eb710(iVar3);
      puVar1[4] = puVar1[4] & 0xfffc | 0x200;
      if (DAT_006e1f3c != 0) {
        puVar7 = &local_114;
        puVar10 = puVar7;
        FUN_0040eec0(&local_124);
        uVar6 = extraout_ECX_01;
        FUN_0040e8d0(&local_128);
        uVar9 = extraout_ECX_02;
        FUN_0040e2c0(&local_120);
        FUN_004f49e0(uVar9,uVar6,puVar7,puVar10);
        iVar3 = FUN_00540430(local_108,local_104,local_100,local_114,local_110,local_10c);
        if (iVar3 != -1) {
          uVar8 = extraout_ECX_03;
          FUN_0040eec0(&local_124);
          uVar6 = extraout_ECX_04;
          FUN_0040e8d0(&local_128);
          uVar9 = extraout_ECX_05;
          FUN_0040e2c0(&local_120);
          FUN_00540140(uVar9,uVar6,uVar8,iVar3);
        }
      }
      iVar3 = *(int *)(param_1 + 0x1da);
      if ((((1 < iVar3) &&
           (iVar4 = DAT_006c54dc +
                    ((&DAT_006cb8be)[iVar3 + *(int *)(param_1 + 0x10)] +
                     (&DAT_006cb942)[*(int *)(param_1 + 0xc)] + *(int *)(param_1 + 8)) * 0xc,
           iVar4 != 0)) && (*(short *)(iVar4 + 4) != -1)) || (iVar3 == 1)) {
        iVar3 = FUN_0049cdb0();
        if (iVar3 == 0) {
          return;
        }
        iVar3 = *(int *)(param_1 + 0x10);
        piVar5 = local_fc;
        for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
          *piVar5 = 0;
          piVar5 = piVar5 + 1;
        }
        local_fc[0] = *(int *)(param_1 + 8) << 5;
        local_d8 = 0xffffffff;
        local_fc[1] = *(int *)(param_1 + 0xc) << 5;
        local_dc = 0xffffffff;
        local_e0 = 0xffffffff;
        local_fc[2] = (*(int *)(param_1 + 0x1da) + iVar3) * 0x10;
        local_e4 = 0xffffffff;
        local_cc = 7;
        local_d4 = 1;
        local_d0 = 0xffffffff;
        local_fc[3] = local_fc[0];
        local_ec = local_fc[1];
        local_e8 = local_fc[2];
        FUN_00493f20(0x31,local_fc);
      }
    }
  }
  if ((*(int *)(param_1 + 0x58) == -1) && (*(int *)(param_1 + 0x1da) == 1)) {
    FUN_00492360();
  }
  else if ((local_11c != 0) && (*(int *)(param_1 + 0x1da) == 1)) {
    FUN_00492890(*(int *)(param_1 + 0x58));
  }
  iVar3 = *(int *)(param_1 + 0x1da);
  *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + -1;
  *(int *)(param_1 + 0x1da) = iVar3 + 1;
  if (iVar3 != local_118) {
    return;
  }
LAB_00492848:
  FUN_004943f0();
  return;
}

