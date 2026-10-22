
void __fastcall FUN_00491b30(int param_1)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 *puVar9;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_ECX_07;
  undefined4 *puVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 uVar18;
  undefined4 uVar19;
  undefined4 uVar20;
  uint local_10c;
  undefined4 *local_108;
  int local_100;
  undefined4 local_fc [63];
  
  puVar10 = (undefined4 *)0x0;
  local_100 = -1;
  bVar2 = false;
  if (*(uint *)(param_1 + 0x44) < DAT_006def5c) {
    iVar3 = *(uint *)(param_1 + 0x44) * 0xe4b + DAT_006def58;
  }
  else {
    iVar3 = 0;
  }
  puVar4 = local_fc;
  for (iVar8 = 0x3f; iVar8 != 0; iVar8 = iVar8 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  local_fc[5] = 0xffffffff;
  local_fc[4] = 0xffffffff;
  local_fc[3] = 0xffffffff;
  local_fc[2] = 0xffffffff;
  local_fc[1] = 0xffffffff;
  local_fc[0] = 0xffffffff;
  local_fc[9] = 0xffffffff;
  local_fc[8] = 0xffffffff;
  local_fc[7] = 0xffffffff;
  local_fc[6] = 0xffffffff;
  local_fc[0xc] = 7;
  local_fc[10] = 1;
  local_fc[0xb] = 0xffffffff;
  bVar1 = false;
  if ((*(int *)(iVar3 + 0x64c) != -1) && (*(int *)(iVar3 + 0x5ec) == 3)) {
    *(int *)(param_1 + 0x48) = *(int *)(iVar3 + 0x64c);
    bVar1 = true;
  }
  if (*(int *)(iVar3 + 0x784) != 0) {
    bVar1 = true;
    *(undefined4 *)(param_1 + 0x48) = *(undefined4 *)(iVar3 + 0x64c);
  }
  uVar7 = *(uint *)(param_1 + 0x48);
  if (uVar7 != 0xffffffff) {
    if (uVar7 < DAT_006def5c) {
      puVar10 = (undefined4 *)(uVar7 * 0xe4b + DAT_006def58);
    }
    local_108 = puVar10;
    if ((((puVar10[0x39] != 0) && (puVar10[1] != 0)) &&
        (iVar3 = *(int *)(param_1 + 0x212), -1 < iVar3)) &&
       ((iVar3 < 8 && (puVar10[iVar3 + 9] != 0)))) {
      if ((puVar10[0x5d] != 0) || (bVar1)) {
        iVar8 = 1;
      }
      else {
        iVar8 = 0;
      }
      if (iVar3 == -1) {
        puVar4 = (undefined4 *)0x0;
      }
      else {
        puVar4 = &DAT_00643a80 + iVar3 * 0x55;
      }
      if ((*(char *)((int)puVar4 + iVar8 + 0x18) == '\0') &&
         (iVar3 = *(int *)(puVar10[0x2b] + 0x198),
         iVar3 = FUN_004e1400(*(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x30),
                              *(undefined4 *)(param_1 + 0x34),puVar10[5] + iVar3,puVar10[6] + iVar3,
                              puVar10[7] + *(int *)(puVar10[0x2b] + 0x19c),DAT_006bef73 << 5),
         iVar3 != 0)) {
        iVar3 = *(int *)(puVar10[0x2b] + 0x198);
        iVar3 = FUN_004e1400(*(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x30),
                             *(undefined4 *)(param_1 + 0x34),puVar10[5] + iVar3,puVar10[6] + iVar3,
                             puVar10[7] + *(int *)(puVar10[0x2b] + 0x19c),
                             (DAT_006bef73 & 0x7ffffff) << 4);
        local_10c = (uint)(iVar3 != 0);
        uVar18 = *puVar10;
        puVar4 = local_fc;
        uVar17 = *(undefined4 *)(param_1 + 0x44);
        uVar19 = 0x62;
        uVar14 = DAT_006c27bf;
        iVar8 = FUN_0044cd00(DAT_006c27bf,uVar17,uVar18,0x62,local_10c,puVar4);
        iVar3 = puVar10[7];
        FUN_00411530(iVar8 + iVar3);
        iVar5 = FUN_0044ccf0(iVar3);
        iVar8 = puVar10[6];
        FUN_00411500(iVar5 + iVar8);
        iVar6 = FUN_0044ccf0(iVar8);
        iVar5 = puVar10[5];
        FUN_004114c0(iVar6 + iVar5);
        uVar13 = extraout_ECX;
        FUN_00411530(*(undefined4 *)(param_1 + 0x34));
        uVar12 = extraout_ECX_00;
        FUN_00411500(*(undefined4 *)(param_1 + 0x30));
        uVar11 = extraout_ECX_01;
        FUN_004114c0(*(undefined4 *)(param_1 + 0x2c));
        iVar3 = FUN_004e1db0(uVar11,uVar12,uVar13,iVar5,iVar8,iVar3,uVar14,uVar17,uVar18,uVar19,
                             local_10c,puVar4);
        if (iVar3 != 0) goto LAB_00492032;
      }
    }
  }
  puVar10 = local_108;
  *(undefined4 *)(param_1 + 0x48) = 0xffffffff;
  DAT_006def60 = 0;
  DAT_006def64 = DAT_006def58;
  puVar4 = (undefined4 *)FUN_0052a670();
  if (puVar4 == (undefined4 *)0x0) {
    return;
  }
  do {
    if (puVar4[1] != 0) {
      iVar3 = *(int *)(param_1 + 0x212);
      if (iVar3 == -1) {
        puVar9 = (undefined4 *)0x0;
      }
      else {
        puVar9 = &DAT_00643a80 + iVar3 * 0x55;
      }
      if ((((*(char *)((int)puVar9 + puVar4[0x5d] + 0x18) == '\0') && (puVar4[0x39] != 0)) &&
          (-1 < iVar3)) &&
         (((iVar3 < 8 && (puVar4[iVar3 + 9] != 0)) &&
          (iVar3 = *(int *)(puVar4[0x2b] + 0x198),
          iVar3 = FUN_004e1400(*(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x30),
                               *(undefined4 *)(param_1 + 0x34),puVar4[5] + iVar3,puVar4[6] + iVar3,
                               puVar4[7] + *(int *)(puVar4[0x2b] + 0x19c),DAT_006bef73 << 5),
          iVar3 != 0)))) {
        iVar3 = *(int *)(puVar4[0x2b] + 0x198);
        iVar3 = FUN_004e1400(*(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x30),
                             *(undefined4 *)(param_1 + 0x34),puVar4[5] + iVar3,puVar4[6] + iVar3,
                             puVar4[7] + *(int *)(puVar4[0x2b] + 0x19c),
                             (DAT_006bef73 & 0x7ffffff) << 4);
        local_10c = (uint)(iVar3 != 0);
        iVar3 = puVar4[0x39];
        iVar8 = FUN_004f4b30(*(undefined4 *)(param_1 + 0x10),puVar4[2],puVar4[3],puVar4[4]);
        iVar3 = (0x23 - iVar8) * 0x14 - iVar3;
        if (iVar3 < 0) {
          iVar3 = 0;
        }
        iVar8 = *(int *)puVar4[0x2b];
        if (((iVar8 == 0) || (iVar8 == 0x18)) || ((iVar8 == 0x19 || (iVar8 == 0x1a)))) {
          iVar3 = iVar3 + 100;
        }
        uVar7 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x1489);
        }
        iVar3 = iVar3 + (uVar7 & 0x7fffffff) % 0x32;
        if (local_100 < iVar3) {
          uVar18 = *puVar4;
          puVar9 = local_fc;
          uVar17 = *(undefined4 *)(param_1 + 0x44);
          uVar20 = 0x62;
          uVar16 = DAT_006c27bf;
          iVar8 = FUN_0044cd00(DAT_006c27bf,uVar17,uVar18,0x62,local_10c,puVar9);
          uVar15 = extraout_ECX_02;
          FUN_00411530(iVar8 + puVar4[7]);
          iVar8 = FUN_0044ccf0(uVar15);
          uVar19 = extraout_ECX_03;
          FUN_00411500(iVar8 + puVar4[6]);
          iVar8 = FUN_0044ccf0(uVar19);
          uVar14 = extraout_ECX_04;
          FUN_004114c0(iVar8 + puVar4[5]);
          uVar13 = extraout_ECX_05;
          FUN_00411530(*(undefined4 *)(param_1 + 0x34));
          uVar12 = extraout_ECX_06;
          FUN_00411500(*(undefined4 *)(param_1 + 0x30));
          uVar11 = extraout_ECX_07;
          FUN_004114c0(*(undefined4 *)(param_1 + 0x2c));
          iVar8 = FUN_004e1db0(uVar11,uVar12,uVar13,uVar14,uVar19,uVar15,uVar16,uVar17,uVar18,uVar20
                               ,local_10c,puVar9);
          if (iVar8 != 0) {
            bVar2 = true;
            puVar10 = puVar4;
            local_100 = iVar3;
          }
        }
      }
    }
    puVar4 = (undefined4 *)FUN_0052a670();
  } while (puVar4 != (undefined4 *)0x0);
  if (!bVar2) {
    return;
  }
LAB_00492032:
  *(int *)(param_1 + 0x38) = *(int *)(puVar10[0x2b] + 0x198) + puVar10[5];
  *(int *)(param_1 + 0x3c) = *(int *)(puVar10[0x2b] + 0x198) + puVar10[6];
  *(int *)(param_1 + 0x40) = *(int *)(puVar10[0x2b] + 0x19c) + puVar10[7];
  *(undefined4 *)(param_1 + 0x48) = *puVar10;
  FUN_00494ab0(0x33);
  return;
}

