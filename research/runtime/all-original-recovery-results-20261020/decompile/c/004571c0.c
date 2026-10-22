
/* WARNING: Type propagation algorithm not settling */

undefined4 __thiscall FUN_004571c0(int param_1,int ***param_2,undefined4 ****param_3,int param_4)

{
  ushort uVar1;
  char cVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  uint ***pppuVar6;
  int ***pppiVar7;
  int iVar8;
  uint uVar9;
  undefined4 ****extraout_ECX;
  uint ***extraout_ECX_00;
  uint ***extraout_ECX_01;
  undefined4 ****extraout_ECX_02;
  uint ***extraout_ECX_03;
  undefined4 ****ppppuVar10;
  uint ***pppuVar11;
  uint uVar12;
  undefined *puVar13;
  int ****ppppiStack_268;
  undefined4 ****ppppuStack_264;
  undefined4 ****ppppuStack_260;
  uint ***local_25c;
  int local_248;
  int ***local_240;
  int ***local_23c;
  char local_235;
  int ***local_234;
  uint ***local_230;
  int *local_22c;
  undefined4 ****local_228;
  uint ***local_224;
  undefined4 ***local_220;
  int ***local_21c;
  int ***local_218;
  undefined4 ****local_214;
  int **local_210;
  int ***local_20c;
  undefined4 ****local_208;
  int ***local_204;
  int local_200;
  undefined4 local_1fc;
  undefined4 ***local_1f8;
  undefined4 ***local_1f4;
  undefined4 ***local_1f0;
  undefined4 local_1ec;
  undefined1 local_1e8;
  undefined4 local_1e7;
  undefined4 local_1e3;
  undefined4 local_1df;
  undefined4 local_1db;
  undefined4 local_1d7;
  undefined4 local_1cf;
  char local_1c7;
  undefined4 local_1c3;
  undefined4 local_1bf;
  undefined4 local_1bb;
  undefined4 local_1b7;
  undefined4 local_1b3;
  undefined4 local_1af;
  undefined4 local_1ab;
  undefined4 local_1a7;
  undefined4 local_163;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005bf2ab;
  local_c = ExceptionList;
  if (*(char *)(param_1 + 0x149) != '\0') {
    local_220 = (undefined4 ***)0x0;
    local_25c = (uint ***)0x457205;
    ExceptionList = &local_c;
    local_200 = param_1;
    FUN_00461550();
    local_25c = (uint ***)0x457211;
    FUN_004df200();
    local_240 = (int ***)0x0;
    local_23c = (int ***)0x0;
    if (*(char *)(param_1 + 0x19e) != '\0') {
      local_25c = *(uint ****)(param_1 + 0x156);
      *(undefined1 *)(param_1 + 0x19e) = 0;
      *(undefined1 *)(param_1 + 0x8f) = 1;
      *(undefined4 *)(param_1 + 0x1ab) = 0x32;
      ppppuStack_260 = (undefined4 ****)0x45724d;
      uVar3 = FUN_004079f0();
      *(undefined4 *)(param_1 + 0x15a) = uVar3;
    }
    local_204 = (int ***)((uint)DAT_006b8ce0 / (uint)DAT_006b8ce4);
    if (9 < (int)local_204) {
      local_204 = (int ***)0x9;
    }
    if (*(int *)(param_1 + 0x15a) != 0) {
      while (param_4 <= *(int *)(param_1 + 0x16)) {
        local_25c = *(uint ****)(*(int *)(param_1 + 0x15a) + 8);
        ppppuStack_260 = (undefined4 ****)0x4572a2;
        local_240 = (int ***)FUN_0040e650();
        local_25c = *(uint ****)(*(int *)(param_1 + 0x15a) + 0xc);
        ppppuStack_260 = (undefined4 ****)0x4572b9;
        local_23c = (int ***)FUN_004132e0();
        iVar4 = *(int *)(param_1 + 0x15a);
        iVar5 = *(int *)(iVar4 + 0x10);
        uVar9 = *(int *)(*(int *)(param_1 + 0x14a) + 8) - *(int *)(iVar4 + 8);
        uVar12 = *(int *)(*(int *)(param_1 + 0x14a) + 0xc) - *(int *)(iVar4 + 0xc);
        if ((int)DAT_006c5494 / 2 < (int)((uVar9 ^ (int)uVar9 >> 0x1f) - ((int)uVar9 >> 0x1f))) {
          uVar9 = -uVar9;
        }
        if (DAT_006c5498 / 2 < (int)((uVar12 ^ (int)uVar12 >> 0x1f) - ((int)uVar12 >> 0x1f))) {
          uVar12 = -uVar12;
        }
        if ((int)uVar9 < 1) {
          if ((int)uVar9 < 0) {
            local_25c = (uint ***)((int)local_240 + -1);
            goto LAB_00457322;
          }
        }
        else {
          local_25c = (uint ***)((int)local_240 + 1);
LAB_00457322:
          ppppuStack_260 = (undefined4 ****)0x45732b;
          local_240 = (int ***)FUN_0040e650();
        }
        if ((int)uVar12 < 1) {
          if ((int)uVar12 < 0) {
            local_25c = (uint ***)((int)local_23c + -1);
            goto LAB_00457343;
          }
        }
        else {
          local_25c = (uint ***)((int)local_23c + 1);
LAB_00457343:
          ppppuStack_260 = (undefined4 ****)0x45734c;
          local_23c = (int ***)FUN_004132e0();
        }
        ppppuVar10 = (undefined4 ****)(iVar5 + -1);
        local_224 = (uint ***)0x0;
        puVar13 = &DAT_006c5490 + (int)ppppuVar10 * 4;
        do {
          if ((0x6c5490 < (int)puVar13) && ((int)ppppuVar10 < DAT_006c549c)) {
            iVar4 = (&DAT_006cb942)[(int)local_23c] + *(int *)(puVar13 + 0x6432) + (int)local_240;
            uVar1 = *(ushort *)(DAT_006c54dc + iVar4 * 0xc);
            local_25c = (uint ***)(DAT_006c54dc + iVar4 * 0xc);
            if (((*(byte *)((int)local_25c + 0xb) & 0x40) == 0) &&
               (((*(int *)(DAT_0065660c + (uint)uVar1 * 0x164 + 0x94) != 0x10 &&
                 (*(short *)(local_25c + 1) == -1)) && (uVar1 == 0)))) {
              ppppuStack_264 = (undefined4 ****)0x457404;
              ppppuStack_260 = ppppuVar10;
              local_214 = ppppuVar10;
              FUN_0040eeb0();
              ppppuStack_264 = &local_23c;
              ppppiStack_268 = (int ****)0x457415;
              FUN_0040e8d0();
              ppppiStack_268 = &local_240;
              ppppuStack_264 = extraout_ECX;
              FUN_0040e2c0();
              ppppiStack_268 = (int ****)extraout_ECX_00;
              FUN_0040eeb0();
              FUN_0040e8a0();
              FUN_0040e290();
              cVar2 = FUN_00419920();
              if (cVar2 != '\0') {
                *(int ****)(param_1 + 0x166) = local_240;
                local_25c = (uint ***)(param_1 + 0x1ab);
                ppppuStack_260 = param_3;
                ppppuStack_264 = param_3;
                ppppiStack_268 = (int ****)(param_1 + 0x16e);
                *(int ****)(param_1 + 0x16a) = local_23c;
                *ppppiStack_268 = (int ***)ppppuVar10;
                FUN_0040eec0();
                ppppiStack_268 = (int ****)extraout_ECX_01;
                FUN_0040e8d0();
                FUN_0040e2c0();
                FUN_0040eeb0();
                FUN_0040e8a0();
                FUN_0040e290(*(undefined4 *)(*(int *)(param_1 + 0x14a) + 8));
                local_235 = FUN_0040b280(*(undefined4 *)(param_1 + 0x14a),0x2b);
                local_1f8 = *param_3;
                local_1f4 = param_3[1];
                local_1f0 = param_3[2];
                local_25c = (uint ***)0x457547;
                iVar4 = FUN_00407600();
                if (iVar4 < 1) {
                  ExceptionList = local_c;
                  return 0;
                }
                if (local_235 != '\0') {
                  if ((*(int *)(param_1 + 0xd8) == 0) ||
                     ((*(int *)(param_1 + 0xdc) - *(int *)(param_1 + 0xd8) & 0xffffffe0U) == 0)) {
                    local_22c = (int *)0x0;
                  }
                  else {
                    local_22c = *(int **)(param_1 + 0xd8);
                  }
                  iVar4 = (*local_22c - DAT_006c54dc) / 0xc;
                  iVar5 = iVar4 / DAT_006c54a0;
                  *(int *)(param_1 + 0x17a) = iVar5;
                  iVar4 = iVar4 - (&DAT_006cb8c2)[iVar5];
                  local_25c = (uint ***)(iVar4 / (int)DAT_006c5494);
                  ppppuStack_260 = (undefined4 ****)0x4575da;
                  iVar5 = FUN_004132e0();
                  *(int *)(param_1 + 0x176) = iVar5;
                  uVar9 = iVar4 - (&DAT_006cb942)[iVar5];
                  if ((int)uVar9 < 0) {
                    do {
                      uVar9 = uVar9 + DAT_006c5494;
                    } while ((int)uVar9 < 0);
                  }
                  else {
                    for (; DAT_006c5494 <= uVar9; uVar9 = uVar9 - DAT_006c5494) {
                    }
                  }
                  *(uint *)(param_1 + 0x172) = uVar9;
                  local_25c = (uint ***)0x45760e;
                  uVar3 = __ftol();
                  pppiVar7 = local_204;
                  *(undefined4 *)(param_1 + 0x17e) = uVar3;
                  local_25c = (uint ***)local_204;
                  local_220 = (undefined4 ***)0x0;
                  *param_2 = (int **)0x0;
                  ppppiStack_268 = (int ****)0x45763b;
                  ppppuStack_264 = ppppuVar10;
                  FUN_0040eeb0();
                  ppppiStack_268 = &local_23c;
                  FUN_0040e8d0();
                  FUN_0040e2c0();
                  FUN_0044d110();
                  FUN_0040ff50();
                  local_25c = (uint ***)pppiVar7;
                  local_21c = (int ***)&ppppuStack_260;
                  ppppiStack_268 = (int ****)0x457684;
                  ppppuStack_264 = ppppuVar10;
                  FUN_0040eeb0();
                  local_21c = (int ***)&ppppuStack_264;
                  ppppiStack_268 = &local_23c;
                  ppppuStack_264 = extraout_ECX_02;
                  FUN_0040e8d0();
                  ppppiStack_268 = (int ****)extraout_ECX_03;
                  local_21c = (int ***)&ppppiStack_268;
                  FUN_0040e2c0();
                  FUN_0044d110();
                  local_1fc = FUN_00411ab0();
                  local_25c = (uint ***)0x4576c6;
                  iVar4 = FUN_004078a0();
                  if (iVar4 != 0) {
                    local_25c = (uint ***)0x2;
                    ppppuStack_260 = (undefined4 ****)0x0;
                    local_22c = (int *)0x0;
                    local_248 = 0;
                    ppppuStack_264 = DAT_006b8ce4;
                    ppppiStack_268 = (int ****)DAT_006b8ce0;
                    FUN_0049f610();
                    local_25c = (uint ***)0x457707;
                    local_25c = (uint ***)FUN_00407920();
                    ppppuStack_260 = (undefined4 ****)0x457713;
                    local_21c = (int ***)local_25c;
                    iVar4 = FUN_004079f0();
                    while (iVar4 != 0) {
                      local_224 = (uint ***)&local_25c;
                      local_230 = (uint ***)0x9;
                      local_228 = (undefined4 ****)0x9;
                      local_234 = (int ***)0x9;
                      ppppuStack_260 = (undefined4 ****)0x457743;
                      local_25c = (uint ***)local_240;
                      uVar9 = FUN_0040e7d0();
                      local_224 = (uint ***)&local_25c;
                      local_25c = (uint ***)local_23c;
                      ppppuStack_260 = (undefined4 ****)0x45775a;
                      uVar12 = FUN_0040eda0();
                      if ((int)DAT_006c5494 / 2 <
                          (int)((uVar9 ^ (int)uVar9 >> 0x1f) - ((int)uVar9 >> 0x1f))) {
                        if ((int)uVar9 < 1) {
                          uVar9 = uVar9 + DAT_006c5494;
                        }
                        else {
                          uVar9 = uVar9 - DAT_006c5494;
                        }
                      }
                      if (DAT_006c5498 / 2 <
                          (int)((uVar12 ^ (int)uVar12 >> 0x1f) - ((int)uVar12 >> 0x1f))) {
                        if ((int)uVar12 < 1) {
                          uVar12 = uVar12 + DAT_006c5498;
                        }
                        else {
                          uVar12 = uVar12 - DAT_006c5498;
                        }
                      }
                      pppuVar11 = (uint ***)((int)local_230 + uVar9);
                      ppppuStack_264 = (undefined4 ****)(uVar12 + (int)local_228);
                      ppppuStack_260 =
                           (undefined4 ****)
                           ((((*(uint *)(*(int *)(iVar4 + 0xac) + 0xc) >> 1) +
                             *(int *)(iVar4 + 0x10)) - (int)local_214) + (int)local_234);
                      local_25c = (uint ***)&local_210;
                      ppppiStack_268 = (int ****)pppuVar11;
                      local_224 = pppuVar11;
                      local_218 = (int ***)ppppuStack_260;
                      local_208 = ppppuStack_264;
                      FUN_004df6e0();
                      pppuVar6 = DAT_006b8ce0;
                      while (((local_230 != pppuVar11 || (local_228 != local_208)) ||
                             (local_234 != local_218))) {
                        if ((int)pppuVar6 < 1) goto LAB_00457a7b;
                        local_20c = local_234;
                        local_25c = (uint ***)&local_210;
                        ppppuStack_260 = &local_234;
                        ppppuStack_264 = &local_228;
                        ppppiStack_268 = (int ****)&local_230;
                        FUN_004df500();
                        ppppuStack_260 = (undefined4 ****)local_234;
                        ppppuStack_264 = local_228;
                        ppppiStack_268 = (int ****)local_230;
                        local_25c = pppuVar6;
                        pppuVar6 = (uint ***)FUN_0049f7e0();
                        pppuVar11 = local_224;
                        param_1 = local_200;
                      }
                      if (0 < (int)pppuVar6) {
                        iVar5 = DAT_006e1fa8 * 8;
                        if (0x20 < iVar5) {
                          iVar5 = 0x20;
                        }
                        local_22c = (int *)((int)local_22c +
                                           (uint)(*(int *)(iVar4 + 0xe4) * iVar5) / 100);
                        uVar9 = (uint)((int)pppuVar6 * 100) /
                                (uint)(*(int *)(iVar4 + 0x84) + (int)pppuVar6);
                        if (*(int *)(iVar4 + 0x90) != 0) {
                          local_248 = local_248 +
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006be2da &
                                            0xffffffffU) / 100);
                        }
                        if (*(int *)(iVar4 + 0x8c) != 0) {
                          local_248 = local_248 +
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006c01d5 &
                                            0xffffffffU) / 100);
                        }
                        if (*(int *)(iVar4 + 0x94) != 0) {
                          local_248 = local_248 +
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006b88ba &
                                            0xffffffffU) / 100);
                        }
                        if (*(int *)(iVar4 + 0x778) != 0) {
                          local_248 = local_248 +
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006bf962 &
                                            0xffffffffU) / 100);
                        }
                        local_25c = (uint ***)0x45796e;
                        iVar8 = FUN_0051fef0();
                        iVar5 = DAT_006bfc33;
                        if (iVar8 == 1) {
LAB_0045799d:
                          uVar12 = (uint)((ulonglong)(uVar9 * iVar5) * 0x51eb851f >> 0x20);
LAB_004579a4:
                          local_248 = local_248 + (uVar12 >> 5);
                        }
                        else {
                          if (iVar8 == 2) {
                            uVar12 = (uint)(((longlong)(int)uVar9 * (longlong)DAT_006bee1e &
                                            0xffffffffU) * 0x51eb851f >> 0x20);
                            goto LAB_004579a4;
                          }
                          iVar5 = DAT_006bff04;
                          if (iVar8 == 3) goto LAB_0045799d;
                        }
                        if (*(int *)(iVar4 + 0x77c) != 0) {
                          local_248 = local_248 -
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006bd1f4 &
                                            0xffffffffU) / 100);
                        }
                        if (*(int *)(iVar4 + 0x110) != 0) {
                          local_248 = local_248 -
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006b8e5c &
                                            0xffffffffU) / 100);
                        }
                        if (*(int *)(iVar4 + 0x780) != 0) {
                          local_248 = local_248 -
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006b9f42 &
                                            0xffffffffU) / 100);
                        }
                        if ((*(int *)(iVar4 + 0x9c) != 0) && (*(int *)(iVar4 + 0xa0) == 1)) {
                          local_248 = local_248 -
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006b93fe &
                                            0xffffffffU) / 100);
                        }
                        if (*(int *)(iVar4 + 0x798) != 0) {
                          local_248 = local_248 -
                                      (int)(((longlong)(int)uVar9 * (longlong)DAT_006bf3c0 &
                                            0xffffffffU) / 100);
                        }
                      }
LAB_00457a7b:
                      local_25c = (uint ***)local_21c;
                      ppppuStack_260 = (undefined4 ****)0x457a89;
                      iVar4 = FUN_004079f0();
                    }
                    local_25c = (uint ***)0x457a9e;
                    pppiVar7 = (int ***)FUN_00407920();
                    ppppuStack_260 = (undefined4 ****)0x457aaa;
                    local_25c = (uint ***)pppiVar7;
                    iVar4 = FUN_004079f0();
                    while (iVar4 != 0) {
                      local_21c = (int ***)&local_25c;
                      local_230 = (uint ***)0x9;
                      local_228 = (undefined4 ****)0x9;
                      local_234 = (int ***)0x9;
                      ppppuStack_260 = (undefined4 ****)0x457ada;
                      local_25c = (uint ***)local_240;
                      uVar9 = FUN_0040e7d0();
                      local_21c = (int ***)&local_25c;
                      local_25c = (uint ***)local_23c;
                      ppppuStack_260 = (undefined4 ****)0x457af1;
                      uVar12 = FUN_0040eda0();
                      if ((int)DAT_006c5494 / 2 <
                          (int)((uVar9 ^ (int)uVar9 >> 0x1f) - ((int)uVar9 >> 0x1f))) {
                        if ((int)uVar9 < 1) {
                          uVar9 = uVar9 + DAT_006c5494;
                        }
                        else {
                          uVar9 = uVar9 - DAT_006c5494;
                        }
                      }
                      if (DAT_006c5498 / 2 <
                          (int)((uVar12 ^ (int)uVar12 >> 0x1f) - ((int)uVar12 >> 0x1f))) {
                        if ((int)uVar12 < 1) {
                          uVar12 = uVar12 + DAT_006c5498;
                        }
                        else {
                          uVar12 = uVar12 - DAT_006c5498;
                        }
                      }
                      pppuVar11 = (uint ***)((int)local_230 + uVar9);
                      ppppuVar10 = (undefined4 ****)(uVar12 + (int)local_228);
                      ppppuStack_260 =
                           (undefined4 ****)
                           ((((*(uint *)(*(int *)(iVar4 + 0xac) + 0xc) >> 1) +
                             *(int *)(iVar4 + 0x10)) - (int)local_214) + (int)local_234);
                      local_25c = (uint ***)&local_210;
                      ppppiStack_268 = (int ****)pppuVar11;
                      ppppuStack_264 = ppppuVar10;
                      local_224 = pppuVar11;
                      local_218 = (int ***)ppppuStack_260;
                      local_208 = ppppuVar10;
                      FUN_004df6e0();
                      pppuVar6 = DAT_006b8ce0;
                      while (((local_230 != pppuVar11 || (local_228 != ppppuVar10)) ||
                             (local_234 != local_218))) {
                        if ((int)pppuVar6 < 1) goto LAB_00457df4;
                        local_20c = local_234;
                        local_25c = (uint ***)&local_210;
                        ppppuStack_260 = &local_234;
                        ppppuStack_264 = &local_228;
                        ppppiStack_268 = (int ****)&local_230;
                        FUN_004df500();
                        ppppuStack_260 = (undefined4 ****)local_234;
                        ppppuStack_264 = local_228;
                        ppppiStack_268 = (int ****)local_230;
                        local_25c = pppuVar6;
                        pppuVar6 = (uint ***)FUN_0049f7e0();
                        pppuVar11 = local_224;
                        ppppuVar10 = local_208;
                      }
                      if (0 < (int)pppuVar6) {
                        if (*(int *)(iVar4 + 0x7c) == 0) {
                          uVar9 = (uint)((int)pppuVar6 * 100) /
                                  (uint)(*(int *)(iVar4 + 0x84) + (int)pppuVar6);
                          if (*(int *)(iVar4 + 0x90) != 0) {
                            local_248 = local_248 -
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006be2da &
                                              0xffffffffU) / 100);
                          }
                          if (*(int *)(iVar4 + 0x8c) != 0) {
                            local_248 = local_248 -
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006c01d5 &
                                              0xffffffffU) / 100);
                          }
                          if (*(int *)(iVar4 + 0x94) != 0) {
                            local_248 = local_248 -
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006b88ba &
                                              0xffffffffU) / 100);
                          }
                          if (*(int *)(iVar4 + 0x778) != 0) {
                            local_248 = local_248 -
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006bf962 &
                                              0xffffffffU) / 100);
                          }
                          local_25c = (uint ***)0x457ce7;
                          iVar8 = FUN_0051fef0();
                          iVar5 = DAT_006bfc33;
                          if (iVar8 == 1) {
LAB_00457d16:
                            uVar12 = (uint)((ulonglong)(uVar9 * iVar5) * 0x51eb851f >> 0x20);
LAB_00457d1d:
                            local_248 = local_248 - (uVar12 >> 5);
                          }
                          else {
                            if (iVar8 == 2) {
                              uVar12 = (uint)(((longlong)(int)uVar9 * (longlong)DAT_006bee1e &
                                              0xffffffffU) * 0x51eb851f >> 0x20);
                              goto LAB_00457d1d;
                            }
                            iVar5 = DAT_006bff04;
                            if (iVar8 == 3) goto LAB_00457d16;
                          }
                          if (*(int *)(iVar4 + 0x77c) != 0) {
                            local_248 = local_248 +
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006bd1f4 &
                                              0xffffffffU) / 100);
                          }
                          if (*(int *)(iVar4 + 0x110) != 0) {
                            local_248 = local_248 +
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006b8e5c &
                                              0xffffffffU) / 100);
                          }
                          if (*(int *)(iVar4 + 0x780) != 0) {
                            local_248 = local_248 +
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006b9f42 &
                                              0xffffffffU) / 100);
                          }
                          if ((*(int *)(iVar4 + 0x9c) != 0) && (*(int *)(iVar4 + 0xa0) == 1)) {
                            local_248 = local_248 +
                                        (int)(((longlong)(int)uVar9 * (longlong)DAT_006b93fe &
                                              0xffffffffU) / 100);
                          }
                          if (*(int *)(iVar4 + 0x798) != 0) {
                            local_248 = local_248 + (uVar9 * DAT_006bf3c0) / 100;
                          }
                        }
                        else {
                          local_22c = (int *)((int)local_22c - *(int *)(iVar4 + 0xe4));
                        }
                      }
LAB_00457df4:
                      ppppuStack_260 = (undefined4 ****)0x457e02;
                      local_25c = (uint ***)pppiVar7;
                      iVar4 = FUN_004079f0();
                      param_1 = local_200;
                    }
                    if (0 < (int)local_22c + local_248 * 2) {
                      ppppuStack_260 = &local_220;
                      local_25c = (uint ***)param_2;
                      ppppuStack_264 = (undefined4 ****)0x40a00000;
                      ppppiStack_268 = (int ****)0x457e89;
                      FUN_0040ea00();
                      local_1b7 = *(undefined4 *)(param_1 + 0x15a);
                      local_1ab = 0x1010101;
                      local_1ec = 0xffffffff;
                      local_1e8 = 0;
                      local_1df = 0;
                      local_1db = 0;
                      local_1d7 = 0;
                      local_1c7 = local_235;
                      local_1c3 = 0;
                      local_1bf = 0;
                      local_1bb = 0;
                      local_1b3 = 0;
                      local_1af = 0;
                      local_1e7 = 0;
                      local_163 = 3;
                      local_1e3 = 0xffffffff;
                      local_1cf = 0;
                      local_1a7 = 0x1010101;
                      local_25c = (uint ***)0x46;
                      ppppuStack_260 = (undefined4 ****)0x40080000;
                      ppppuStack_264 = (undefined4 ****)0x0;
                      ppppiStack_268 = (int ****)0xffffffbe;
                      local_4 = 0;
                      FUN_0040eba0(*(undefined4 *)(*(int *)(param_1 + 0x14a) + 0x678),&local_1ec,
                                   &local_220,param_2,7,0,0x3fe00000,0xfffffffa,0,0x3fe00000,
                                   0xfffffff9,0,0x3fe00000,0x40,0,0x3fe00000,0x41,0);
                      local_4 = 0xffffffff;
                      local_25c = (uint ***)0x457f87;
                      FUN_00407650();
                      local_25c = (uint ***)0x457f92;
                      iVar4 = FUN_004078a0();
                      local_25c = (uint ***)0x41;
                      ppppuStack_260 = (undefined4 ****)0x3fe00000;
                      ppppuStack_264 = (undefined4 ****)0x0;
                      ppppiStack_268 = (int ****)0x40;
                      if (iVar4 < 1) {
                        FUN_0040eba0(*(undefined4 *)(*(int *)(param_1 + 0x14a) + 0x678),
                                     *(undefined4 *)(param_1 + 0x15e),&local_220,param_2,4,0,
                                     0x3fe00000,0x3b,0);
                      }
                      else {
                        FUN_0040eba0(*(undefined4 *)(param_1 + 0x162),
                                     *(undefined4 *)(param_1 + 0x15e),&local_220,param_2,8,0,
                                     0x3fe00000,0xffffffff,0,0x3fe00000,0xfffffffe,0,0x3fe00000,
                                     0xfffffffa,0,0x3fe00000,0xfffffff9);
                      }
                      local_25c = (uint ***)0x1;
                      ppppuStack_264 = &local_220;
                      ppppuStack_260 = (undefined4 ****)param_2;
                      ppppiStack_268 = (int ****)0x2b;
                      FUN_0040ec70();
                      *(undefined1 *)(param_1 + 0x19e) = 1;
                      ExceptionList = local_c;
                      return 1;
                    }
                  }
                }
                *(undefined1 *)(param_1 + 0x8f) = 1;
                *(undefined4 *)(param_1 + 0x1ab) = 0x32;
              }
              break;
            }
          }
          puVar13 = puVar13 + 4;
          local_224 = (uint ***)((int)local_224 + 1);
          ppppuVar10 = (undefined4 ****)((int)ppppuVar10 + 1);
          local_214 = ppppuVar10;
        } while ((int)local_224 < 3);
        local_25c = *(uint ****)(param_1 + 0x156);
        ppppuStack_260 = (undefined4 ****)0x457e42;
        iVar4 = FUN_004079f0();
        *(int *)(param_1 + 0x15a) = iVar4;
        if (iVar4 == 0) {
          ExceptionList = local_c;
          return 0;
        }
      }
    }
  }
  ExceptionList = local_c;
  return 0;
}

