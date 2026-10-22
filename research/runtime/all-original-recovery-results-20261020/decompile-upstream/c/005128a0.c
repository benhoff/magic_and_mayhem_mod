
undefined1 __thiscall
FUN_005128a0(int param_1,uint param_2,uint param_3,int param_4,int param_5,int param_6)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint extraout_ECX;
  uint extraout_ECX_00;
  uint extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_ECX_07;
  undefined4 extraout_ECX_08;
  undefined4 extraout_ECX_09;
  undefined4 extraout_ECX_10;
  undefined4 extraout_ECX_11;
  undefined4 extraout_ECX_12;
  uint extraout_ECX_13;
  uint extraout_ECX_14;
  uint extraout_ECX_15;
  uint extraout_ECX_16;
  undefined4 extraout_ECX_17;
  undefined4 extraout_ECX_18;
  undefined4 extraout_ECX_19;
  undefined4 extraout_ECX_20;
  uint extraout_ECX_21;
  uint uVar4;
  undefined4 *puVar5;
  uint *puVar6;
  int iVar7;
  undefined4 *puVar8;
  uint *puVar9;
  undefined1 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 *puVar14;
  int *piVar15;
  uint local_2ac;
  int local_2a8;
  uint local_2a4;
  int local_2a0;
  uint local_29c;
  uint local_298;
  int local_294;
  undefined4 local_290 [4];
  undefined1 local_280;
  undefined4 local_27f;
  undefined1 local_27b;
  undefined4 local_278;
  int local_274;
  undefined4 local_26c;
  undefined4 local_268;
  undefined4 local_264;
  undefined4 local_260;
  undefined4 local_25c;
  undefined4 local_258;
  undefined4 local_254;
  undefined1 local_250;
  undefined4 local_24f;
  undefined1 local_24b;
  undefined4 local_24a;
  undefined4 local_246;
  undefined4 local_242;
  undefined4 local_23e;
  undefined4 local_232;
  undefined4 local_22e;
  undefined4 local_22a;
  undefined4 local_224 [6];
  undefined4 local_20c [131];
  
  iVar7 = 0x10;
  do {
    FUN_0042fd00();
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  FUN_00418cd0();
  iVar7 = param_5;
  if ((((DAT_006df184 != 0) && (*(int *)(param_1 + 0xd03) == 0)) && (DAT_006df190 == param_2)) &&
     (((DAT_006df194 == param_3 && (DAT_006df198 == param_4)) && (DAT_006df19c == param_5)))) {
    param_6 = 0;
  }
  uVar3 = param_3;
  if ((DAT_006e1f3c != 0) && (param_6 != 0)) {
    local_29c = param_2;
    local_298 = param_3;
    local_294 = param_4;
    if ((*(int *)(param_1 + 0xd03) == 0) ||
       (((param_2 != *(uint *)(param_1 + 0xdd3) || (param_3 != *(uint *)(param_1 + 0xdd7))) ||
        (param_4 != *(int *)(param_1 + 0xddb))))) {
      local_232 = *(undefined4 *)(param_1 + 0x174);
      local_22e = *(undefined4 *)(param_1 + 0x698);
      local_22a = *(undefined4 *)(param_1 + 0x69c);
      local_26c = 0;
      local_268 = 0;
      local_264 = 0;
      local_260 = 0;
      local_25c = 0;
      local_258 = 0;
      local_254 = 0;
      local_250 = 0;
      local_24f = 0;
      local_24b = 0;
      local_24a = 0;
      local_246 = 0;
      local_242 = 0;
      local_23e = 0;
      *(undefined4 *)(param_1 + 0xd03) = 0;
      local_278 = 0;
      local_274 = param_1;
      FUN_00441560(&local_278);
      uVar13 = extraout_ECX_02;
      FUN_0040eeb0(*(undefined4 *)(param_1 + 0x10));
      uVar12 = extraout_ECX_03;
      FUN_0040e8a0(*(undefined4 *)(param_1 + 0xc));
      uVar11 = extraout_ECX_04;
      FUN_0040e290(*(undefined4 *)(param_1 + 8));
      iVar2 = FUN_0053d7d0(uVar11,uVar12,uVar13);
      if ((*(char *)(param_1 + 0x723) == '\0') && (*(int *)(param_1 + 0x5ec) != 0xe)) {
        local_27b = 0;
      }
      else {
        local_27b = 1;
      }
      iVar7 = *(int *)(param_1 + 0xac);
      local_290[0] = *(undefined4 *)(iVar7 + 0xc);
      local_290[1] = *(undefined4 *)(iVar7 + 8);
      local_290[3] = *(undefined4 *)(iVar7 + 0x44);
      local_290[2] = *(undefined4 *)(param_1 + 0x104);
      local_280 = 0;
      local_27f = *(undefined4 *)(param_1 + 0xa8);
      puVar5 = local_290;
      puVar8 = local_224;
      for (iVar7 = 5; iVar7 != 0; iVar7 = iVar7 + -1) {
        *puVar8 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar8 = puVar8 + 1;
      }
      puVar14 = local_224;
      *(undefined2 *)puVar8 = *(undefined2 *)puVar5;
      FUN_0040eeb0(param_4);
      uVar12 = extraout_ECX_05;
      FUN_0040e8a0(param_3);
      uVar11 = extraout_ECX_06;
      FUN_0040e290(param_2);
      iVar7 = FUN_004f4900(uVar11,uVar12,iVar7,puVar14);
      if (iVar7 == 0) {
        local_2a4 = param_2;
        if ((int)param_2 < 0) {
          do {
            local_2a4 = local_2a4 + DAT_006c5494;
          } while ((int)local_2a4 < 0);
        }
        else {
          for (; DAT_006c5494 <= local_2a4; local_2a4 = local_2a4 - DAT_006c5494) {
          }
        }
        local_2ac = param_3;
        if ((int)param_3 < 0) {
          do {
            local_2ac = local_2ac + DAT_006c5498;
          } while ((int)local_2ac < 0);
        }
        else {
          for (; DAT_006c5498 <= local_2ac; local_2ac = local_2ac - DAT_006c5498) {
          }
        }
        local_2a8 = param_4;
        if ((*(char *)(param_1 + 0x723) == '\0') && (*(int *)(param_1 + 0x5ec) != 0xe)) {
          local_27b = 0;
        }
        else {
          local_27b = 1;
        }
        iVar7 = *(int *)(param_1 + 0xac);
        local_290[0] = *(undefined4 *)(iVar7 + 0xc);
        local_290[1] = *(undefined4 *)(iVar7 + 8);
        local_290[3] = *(undefined4 *)(iVar7 + 0x44);
        local_290[2] = *(undefined4 *)(param_1 + 0x104);
        local_280 = 0;
        local_27f = *(undefined4 *)(param_1 + 0xa8);
        puVar5 = local_290;
        puVar8 = local_224;
        for (iVar7 = 5; iVar7 != 0; iVar7 = iVar7 + -1) {
          *puVar8 = *puVar5;
          puVar5 = puVar5 + 1;
          puVar8 = puVar8 + 1;
        }
        *(undefined2 *)puVar8 = *(undefined2 *)puVar5;
        FUN_004f4f90(local_224,&local_2a4,&local_2ac,&local_2a8);
        FUN_00441560(&local_278);
        uVar13 = extraout_ECX_10;
        FUN_0040eec0(&local_2a8);
        uVar12 = extraout_ECX_11;
        FUN_0040e8d0(&local_2ac);
        uVar11 = extraout_ECX_12;
        FUN_0040e2c0(&local_2a4);
      }
      else {
        FUN_00441560(&local_278);
        uVar13 = extraout_ECX_07;
        FUN_0040eeb0(param_4);
        uVar12 = extraout_ECX_08;
        FUN_0040e8a0(param_3);
        uVar11 = extraout_ECX_09;
        FUN_0040e290(param_2);
      }
      iVar1 = FUN_0053d7d0(uVar11,uVar12,uVar13);
      uVar3 = extraout_ECX_13;
      iVar7 = param_5;
      if ((iVar2 != -1) && (iVar1 != -1)) {
        local_2a0 = 700;
        FUN_00441560(&local_278);
        iVar2 = FUN_0053e5e0(iVar2,iVar1,uVar13);
        uVar3 = extraout_ECX_14;
        iVar7 = param_5;
        if ((iVar2 != 0) && ((*(int *)(param_1 + 0xd0b) != -1 && (*(int *)(param_1 + 0xd0f) != -1)))
           ) {
          *(undefined4 *)(param_1 + 0xd03) = 1;
          *(uint *)(param_1 + 0xdd3) = param_2;
          *(uint *)(param_1 + 0xdd7) = param_3;
          *(int *)(param_1 + 0xddb) = param_4;
          *(undefined4 *)(param_1 + 0xdcf) = 1;
          uVar3 = param_3;
          if (*(int *)(param_1 + 0xd0f) != -1) {
            iVar7 = FUN_0053ea50(*(undefined4 *)(param_1 + 0xd07),*(undefined4 *)(param_1 + 0xd0b));
            iVar2 = FUN_0053ea50(*(undefined4 *)(param_1 + *(int *)(param_1 + 0xdcf) * 4 + 0xd07),
                                 *(undefined4 *)(param_1 + 0xd0b + *(int *)(param_1 + 0xdcf) * 4));
            uVar3 = extraout_ECX_15;
            if (iVar2 + iVar7 < 0xb4) {
              if (*(int *)(param_1 + 0xd0f + *(int *)(param_1 + 0xdcf) * 4) == -1) {
                *(undefined4 *)(param_1 + 0xd03) = 0;
              }
              else {
                *(undefined4 *)(param_1 + 0xdcf) = 2;
              }
            }
          }
          iVar7 = param_5;
          if (*(int *)(param_1 + 0xd03) != 0) {
            iVar7 = *(int *)(param_1 + 0xd07 + *(int *)(param_1 + 0xdcf) * 4);
            if ((iVar7 < DAT_006eaf0c) && (-1 < iVar7)) {
              iVar7 = DAT_006eaf08 + iVar7 * 0x294;
            }
            else {
              iVar7 = 0;
            }
            param_2 = *(uint *)(iVar7 + 4);
            param_3 = *(uint *)(iVar7 + 8);
            param_4 = *(int *)(iVar7 + 0xc);
            FUN_00513cf0(&param_2,&param_3,&param_4,*(undefined4 *)(iVar7 + 0x20c),local_29c,
                         local_298,local_294);
            uVar3 = extraout_ECX_16;
            iVar7 = param_5;
          }
        }
      }
    }
    else {
      if (DAT_006df184 != 0) {
        if ((*(int *)(param_1 + 0xdcf) < 0x32) &&
           (uVar3 = *(uint *)(param_1 + 0xd07 + *(int *)(param_1 + 0xdcf) * 4), uVar3 != 0xffffffff)
           ) {
          if (((int)uVar3 < DAT_006eaf0c) && (-1 < (int)uVar3)) {
            iVar2 = DAT_006eaf08 + uVar3 * 0x294;
          }
          else {
            iVar2 = 0;
          }
          local_2a4 = *(uint *)(iVar2 + 4);
          local_2ac = *(uint *)(iVar2 + 8);
          local_2a8 = *(int *)(iVar2 + 0xc);
          FUN_00513cf0(&local_2a4,&local_2ac,&local_2a8,*(undefined4 *)(iVar2 + 0x20c),param_2,
                       param_3,param_4);
          uVar3 = extraout_ECX;
          if (((local_2a4 == DAT_006df190) && (uVar3 = local_2ac, local_2ac == DAT_006df194)) &&
             (local_2a8 == DAT_006df198)) {
            param_2 = local_2a4;
            param_3 = local_2ac;
            param_4 = local_2a8;
            goto LAB_005130d4;
          }
        }
        else {
          DAT_006df184 = 0;
        }
      }
      iVar2 = *(int *)(param_1 + 0xdcf);
      if (((iVar2 < 0x32) && (uVar3 = *(uint *)(param_1 + 0xd07 + iVar2 * 4), uVar3 != 0xffffffff))
         && ((0x30 < iVar2 ||
             ((*(int *)(param_1 + 0xd0b + iVar2 * 4) != -1 ||
              (iVar2 = FUN_0053ea50(*(undefined4 *)(param_1 + 0xd03 + iVar2 * 4),uVar3),
              uVar3 = extraout_ECX_00, 0xb3 < iVar2)))))) {
        iVar2 = param_1 + *(int *)(param_1 + 0xdcf) * 4;
        if ((*(int *)(param_1 + 0xd0b + *(int *)(param_1 + 0xdcf) * 4) != -1) &&
           (*(int *)(iVar2 + 0xd0f) != -1)) {
          iVar2 = FUN_0053ea50(*(undefined4 *)(iVar2 + 0xd03),*(undefined4 *)(iVar2 + 0xd07));
          iVar1 = FUN_0053ea50(*(undefined4 *)(param_1 + 0xd07 + *(int *)(param_1 + 0xdcf) * 4),
                               *(undefined4 *)(param_1 + 0xd0b + *(int *)(param_1 + 0xdcf) * 4));
          if (iVar1 + iVar2 < 0xb4) {
            *(int *)(param_1 + 0xdcf) = *(int *)(param_1 + 0xdcf) + 1;
          }
        }
        iVar2 = *(int *)(param_1 + 0xd07 + *(int *)(param_1 + 0xdcf) * 4);
        if ((iVar2 < DAT_006eaf0c) && (-1 < iVar2)) {
          iVar2 = DAT_006eaf08 + iVar2 * 0x294;
        }
        else {
          iVar2 = 0;
        }
        param_2 = *(uint *)(iVar2 + 4);
        param_3 = *(uint *)(iVar2 + 8);
        param_4 = *(int *)(iVar2 + 0xc);
        FUN_00513cf0(&param_2,&param_3,&param_4,*(undefined4 *)(iVar2 + 0x20c),local_29c,local_298,
                     local_294);
        uVar3 = extraout_ECX_01;
      }
      else {
        *(undefined4 *)(param_1 + 0xd03) = 0;
      }
    }
  }
LAB_005130d4:
  if (((((DAT_006df184 == 0) || (DAT_006df190 != param_2)) ||
       (uVar3 = param_3, DAT_006df194 != param_3)) ||
      ((DAT_006df198 != param_4 || (DAT_006df19c != iVar7)))) ||
     ((uVar3 = *(uint *)(param_1 + 8), DAT_006df1a4 != uVar3 ||
      ((uVar3 = DAT_006df1a8, DAT_006df1a8 != *(uint *)(param_1 + 0xc) ||
       (DAT_006df1ac != *(int *)(param_1 + 0x10))))))) {
    DAT_006c4ddc = 1;
    DAT_006df18c = 0xd2;
    if (iVar7 == 0) {
      if (param_6 == 0) {
        DAT_006df18c = 0x46;
      }
    }
    else {
      puVar5 = local_20c;
      FUN_0040eeb0(param_4);
      uVar12 = extraout_ECX_19;
      FUN_0040e8a0(param_3);
      uVar11 = extraout_ECX_20;
      FUN_0040e290(param_2);
      iVar2 = FUN_00513370(uVar11,uVar12,uVar3,puVar5);
      uVar3 = extraout_ECX_21;
      if (iVar2 != 0) {
        puVar5 = local_20c;
        puVar8 = &DAT_006c4bd0;
        for (iVar2 = 0x83; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar8 = *puVar5;
          puVar5 = puVar5 + 1;
          puVar8 = puVar8 + 1;
        }
        goto LAB_005131d8;
      }
    }
  }
  iVar2 = DAT_006df188;
  if (DAT_006df18c < DAT_006df188) {
    iVar2 = DAT_006df18c;
  }
  piVar15 = &local_2a0;
  local_2a0 = iVar2;
  FUN_0040eeb0(param_4);
  uVar12 = extraout_ECX_17;
  FUN_0040e8a0(param_3);
  uVar11 = extraout_ECX_18;
  FUN_0040e290(param_2);
  FUN_0054b800(param_1,iVar7,uVar11,uVar12,uVar3,piVar15);
  DAT_006df18c = DAT_006df18c - (iVar2 - local_2a0);
  DAT_006df188 = DAT_006df188 - (iVar2 - local_2a0);
LAB_005131d8:
  if ((DAT_006df188 < 1) && (0 < DAT_006df18c)) {
    DAT_006df184 = 1;
    DAT_006df190 = param_2;
    DAT_006df194 = param_3;
    DAT_006df198 = param_4;
    DAT_006df1a4 = *(uint *)(param_1 + 8);
    DAT_006df1a8 = *(uint *)(param_1 + 0xc);
    DAT_006df1ac = *(int *)(param_1 + 0x10);
    uVar10 = 2;
    DAT_006df19c = iVar7;
  }
  else {
    puVar6 = &DAT_006c4bd0;
    puVar9 = (uint *)(param_1 + 0x96b);
    for (iVar7 = 0x83; iVar7 != 0; iVar7 = iVar7 + -1) {
      *puVar9 = *puVar6;
      puVar6 = puVar6 + 1;
      puVar9 = puVar9 + 1;
    }
    if (((DAT_006e1f3c != 0) && (param_6 != 0)) && (*(int *)(param_1 + 0xd03) != 0)) {
      *(uint *)(param_1 + 0x96b) = local_29c;
      *(uint *)(param_1 + 0x96f) = local_298;
      *(int *)(param_1 + 0x973) = local_294;
    }
    uVar10 = *(int *)(param_1 + 0x97b) != 0;
    *(undefined4 *)(param_1 + 0xb8b) = 1;
    if (!(bool)uVar10) {
      *(undefined4 *)(param_1 + 0xb8b) = 0;
    }
    DAT_006df184 = 0;
    if (((*(int *)(param_1 + 0xd03) != 0) && (param_6 != 0)) &&
       ((3 < *(uint *)(param_1 + 0x97b) &&
        (uVar3 = *(int *)(param_1 + 0x10) - param_4, uVar4 = (int)uVar3 >> 0x1f,
        (int)((uVar3 ^ uVar4) - uVar4) < 5)))) {
      *(uint *)(param_1 + 0x97b) = *(uint *)(param_1 + 0x97b) - 3;
    }
  }
  *(undefined4 *)(param_1 + 0x69c) = 0;
  return uVar10;
}

