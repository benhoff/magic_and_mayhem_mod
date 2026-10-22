
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004970e0(int *param_1)

{
  ushort *puVar1;
  ushort uVar2;
  int *piVar3;
  int *piVar4;
  void **ppvVar5;
  void **ppvVar6;
  int iVar7;
  uint *puVar8;
  uint uVar9;
  uint uVar10;
  undefined4 *puVar11;
  undefined4 uVar12;
  int ***pppiVar13;
  int ***pppiVar14;
  undefined1 **ppuVar15;
  undefined1 **ppuVar16;
  int iVar17;
  uint uVar18;
  int *piVar19;
  int ***pppiStack_180;
  undefined1 ***pppuStack_17c;
  short *local_168;
  undefined1 **local_164;
  int *local_160;
  short *local_15c;
  undefined1 ***local_158;
  int *local_154;
  undefined1 ***local_150;
  undefined1 ***local_14c;
  undefined1 ***local_148;
  int *local_144;
  short *local_140;
  short *local_13c;
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  undefined4 local_12c;
  undefined1 local_128;
  undefined4 local_127;
  undefined1 local_123;
  undefined4 local_120;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined1 local_110;
  undefined4 local_10f;
  undefined1 local_10b;
  int aiStack_108 [7];
  undefined4 uStack_ec;
  int iStack_e8;
  undefined4 uStack_e4;
  undefined4 uStack_e0;
  undefined4 uStack_dc;
  undefined4 uStack_d8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c0344;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  ppvVar5 = &local_c;
  ppvVar6 = &local_c;
  switch(param_1[10]) {
  case 0:
  case 1:
    pppuStack_17c = (undefined1 ***)0x1;
    pppiStack_180 = (int ***)0x497290;
    ExceptionList = &local_c;
    iVar7 = FUN_004883f0();
    if ((iVar7 == 2) || (iVar7 == 4)) {
LAB_00499bcf:
      pppuStack_17c = (undefined1 ***)0x499bd6;
      FUN_004943f0();
    }
    else {
      if (iVar7 == 3) {
        local_168 = (short *)0x0;
        if (0 < DAT_005e1460) {
          do {
            pppuStack_17c = (undefined1 ***)0x45;
            pppiStack_180 = (int ***)0x4972ce;
            local_164 = (undefined1 **)FUN_00597850();
            local_4 = 1;
            if (local_164 == (undefined1 **)0x0) {
              puVar11 = (undefined4 *)0x0;
            }
            else {
              pppuStack_17c = (undefined1 ***)0x4972f0;
              uVar18 = FUN_0054e200();
              pppuStack_17c = (undefined1 ***)0x497302;
              uVar9 = FUN_0054e200();
              pppuStack_17c = (undefined1 ***)0x497314;
              uVar10 = FUN_0054e200();
              pppuStack_17c = (undefined1 ***)(((uVar18 & 0x7fffffff) % 3 - 1) + param_1[7]);
              pppiStack_180 = (int ***)(((uVar9 & 0x7fffffff) % 3 - 1) + param_1[6]);
              puVar11 = (undefined4 *)FUN_00546660(((uVar10 & 0x7fffffff) % 3 - 1) + param_1[5]);
            }
            local_4 = 0xffffffff;
            if (puVar11 != (undefined4 *)0x0) {
              iVar7 = *(int *)(&DAT_006b13cd + param_1[0x13] * 0x2d1);
              pppuStack_17c = *(undefined1 ****)(&DAT_005e1444 + iVar7 * 0xc);
              pppiStack_180 = *(int ****)(&DAT_005e1440 + iVar7 * 0xc);
              FUN_005467a0((&DAT_005e143c)[iVar7 * 3]);
              pppiStack_180 = (int ***)0x4973a6;
              pppuStack_17c = (undefined1 ***)puVar11;
              iVar7 = FUN_005457e0();
              if (iVar7 == 0) {
                pppuStack_17c = (undefined1 ***)0x1;
                pppiStack_180 = (int ***)0x4973b2;
                (**(code **)*puVar11)();
              }
            }
            local_168 = (short *)((int)local_168 + 1);
          } while ((int)local_168 < DAT_005e1460);
        }
      }
      else {
        pppuStack_17c = (undefined1 ***)0x4972ad;
        FUN_0048b0f0();
      }
      pppuStack_17c = (undefined1 ***)0x4973d6;
      iVar7 = FUN_00464ec0();
      while (iVar7 == 1) {
        pppuStack_17c = (undefined1 ***)0x4973e6;
        FUN_00464d70();
        pppuStack_17c = (undefined1 ***)0x4973ed;
        iVar7 = FUN_00464ec0();
      }
    }
    break;
  case 2:
    pppuStack_17c = (undefined1 ***)0x1;
    pppiStack_180 = (int ***)0x497126;
    ExceptionList = &local_c;
    iVar7 = FUN_004883f0();
    if ((iVar7 == 2) || (iVar7 == 4)) goto LAB_00499bcf;
    if (iVar7 == 3) {
      local_168 = (short *)0x0;
      if (0 < DAT_005e1460) {
        do {
          pppuStack_17c = (undefined1 ***)0x45;
          pppiStack_180 = (int ***)0x497164;
          local_164 = (undefined1 **)FUN_00597850();
          local_4 = 0;
          if (local_164 == (undefined1 **)0x0) {
            puVar11 = (undefined4 *)0x0;
          }
          else {
            pppuStack_17c = (undefined1 ***)0x497186;
            uVar18 = FUN_0054e200();
            pppuStack_17c = (undefined1 ***)0x497198;
            uVar9 = FUN_0054e200();
            pppuStack_17c = (undefined1 ***)0x4971aa;
            uVar10 = FUN_0054e200();
            pppuStack_17c = (undefined1 ***)(((uVar18 & 0x7fffffff) % 3 - 1) + param_1[7]);
            pppiStack_180 = (int ***)(((uVar9 & 0x7fffffff) % 3 - 1) + param_1[6]);
            puVar11 = (undefined4 *)FUN_00546660(((uVar10 & 0x7fffffff) % 3 - 1) + param_1[5]);
          }
          local_4 = 0xffffffff;
          if (puVar11 != (undefined4 *)0x0) {
            pppuStack_17c = (undefined1 ***)0x0;
            pppiStack_180 = (int ***)0xbf;
            FUN_005467a0(0xff);
            pppiStack_180 = (int ***)0x49721f;
            pppuStack_17c = (undefined1 ***)puVar11;
            iVar7 = FUN_005457e0();
            if (iVar7 == 0) {
              pppuStack_17c = (undefined1 ***)0x1;
              pppiStack_180 = (int ***)0x49722a;
              (**(code **)*puVar11)();
            }
          }
          local_168 = (short *)((int)local_168 + 1);
        } while ((int)local_168 < DAT_005e1460);
      }
    }
    else {
      pppuStack_17c = (undefined1 ***)0x497143;
      FUN_0048b0f0();
    }
    pppuStack_17c = (undefined1 ***)0x49724b;
    iVar7 = FUN_0049cdb0();
    if (iVar7 != 0) {
      pppuStack_17c = (undefined1 ***)(param_1 + 0xb);
      pppiStack_180 = (int ***)0x13;
      FUN_00493f20();
    }
    pppuStack_17c = (undefined1 ***)0x497269;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x497278;
      FUN_00464d70();
      pppuStack_17c = (undefined1 ***)0x49727f;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 3:
    pppuStack_17c = (undefined1 ***)0x1;
    pppiStack_180 = (int ***)0x4973ff;
    ExceptionList = &local_c;
    iVar7 = FUN_004883f0();
    iVar17 = *(int *)((int)param_1 + 0x1d2) + -1;
    *(int *)((int)param_1 + 0x1d2) = iVar17;
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x1;
      param_1[0x11] = -1;
      pppiStack_180 = (int ***)0x497421;
      FUN_0048ee40();
    }
    else if (iVar7 == 0) {
      if ((uint)param_1[0x12] < DAT_006def5c) {
        iVar7 = param_1[0x12] * 0xe4b + DAT_006def58;
      }
      else {
        iVar7 = 0;
      }
      param_1[0x66] = iVar7;
      uVar18 = *(uint *)(iVar7 + 0xe4);
      pppuStack_17c = *(undefined1 ****)(&DAT_006b13c5 + param_1[0x13] * 0x2d1);
      pppiStack_180 = (int ***)0x497472;
      FUN_0048ecf0();
      if (param_1[0x12] != -1) {
        iVar7 = param_1[0x66];
        *(int *)(iVar7 + 0x80d) = param_1[0x19] + 1;
        *(undefined4 *)(iVar7 + 0x811) = 100;
      }
      param_1[0x11] = param_1[0x12];
      if ((((*(uint *)(param_1[0x66] + 0xe4) < uVar18) && (-1 < DAT_00644520)) && (DAT_00644520 < 8)
          ) && (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) != 0)) {
        pppuStack_17c = (undefined1 ***)0x1;
        pppiStack_180 = (int ***)0x4974d2;
        FUN_0051fba0();
      }
    }
    else {
      if ((iVar7 == 2) || (iVar17 < 1)) goto LAB_00499bcf;
      if ((iVar7 == 3) && (iVar7 = 0, 0 < DAT_005e1460)) {
        do {
          pppuStack_17c = (undefined1 ***)0x45;
          pppiStack_180 = (int ***)0x49750a;
          local_164 = (undefined1 **)FUN_00597850();
          local_4 = 2;
          if (local_164 == (undefined1 **)0x0) {
            puVar11 = (undefined4 *)0x0;
          }
          else {
            pppuStack_17c = (undefined1 ***)param_1[7];
            pppiStack_180 = (int ***)param_1[6];
            puVar11 = (undefined4 *)FUN_00546660(param_1[5]);
          }
          local_4 = 0xffffffff;
          if (puVar11 != (undefined4 *)0x0) {
            iVar17 = *(int *)(&DAT_006b13cd + param_1[0x13] * 0x2d1);
            pppuStack_17c = *(undefined1 ****)(&DAT_005e1444 + iVar17 * 0xc);
            pppiStack_180 = *(int ****)(&DAT_005e1440 + iVar17 * 0xc);
            FUN_005467a0((&DAT_005e143c)[iVar17 * 3]);
            pppiStack_180 = (int ***)0x497584;
            pppuStack_17c = (undefined1 ***)puVar11;
            iVar17 = FUN_005457e0();
            if (iVar17 == 0) {
              pppuStack_17c = (undefined1 ***)0x1;
              pppiStack_180 = (int ***)0x497590;
              (**(code **)*puVar11)();
            }
          }
          iVar7 = iVar7 + 1;
        } while (iVar7 < DAT_005e1460);
      }
      pppuStack_17c = (undefined1 ***)0x4975ab;
      iVar7 = FUN_00464ec0();
      while (iVar7 == 1) {
        pppuStack_17c = (undefined1 ***)0x4975b7;
        FUN_00464d70();
        pppuStack_17c = (undefined1 ***)0x4975be;
        iVar7 = FUN_00464ec0();
      }
      if (param_1[0x13] == 0x49) {
        pppiStack_180 = (int ***)(param_1 + 0x67);
        if (param_1[0x67] != 0) goto LAB_00497b49;
        pppuStack_17c = (undefined1 ***)0x1;
        FUN_00571310(0x2c6,param_1[2],param_1[3],param_1[4],0);
      }
    }
    break;
  case 4:
  case 5:
    pppuStack_17c = (undefined1 ***)0x49760d;
    ExceptionList = &local_c;
    iVar17 = FUN_00489080();
    iVar7 = param_1[0x13];
    if (((iVar7 == 0x5d) || (iVar7 == 0x5f)) || (iVar7 == 0x62)) {
      pppuStack_17c = (undefined1 ***)0x497628;
      iVar7 = FUN_00489200();
      pppuStack_17c = (undefined1 ***)0x0;
      pppiStack_180 =
           (int ***)(*(int *)(*(int *)(param_1[9] + 0x438) + iVar7 * 0x10) + param_1[0x16] * 0x10 +
                    *(int *)((int)param_1 + 0x1a2));
      FUN_00464cb0();
    }
    if ((iVar17 == 2) || (iVar17 == 4)) goto LAB_00499bcf;
    if (iVar17 != 3) {
      pppuStack_17c = (undefined1 ***)0x497672;
      FUN_0048b0f0();
    }
    iVar7 = param_1[0x13];
    if (((iVar7 == 0x5e) || (iVar7 == 0x60)) ||
       ((iVar7 == 0x5d || ((iVar7 == 0x66 || (iVar7 == 0x62)))))) {
      pppuStack_17c = (undefined1 ***)0x49769c;
      iVar7 = FUN_0049cdb0();
      if (iVar7 != 0) {
        piVar19 = aiStack_108;
        for (iVar7 = 0x3f; iVar7 != 0; iVar7 = iVar7 + -1) {
          *piVar19 = 0;
          piVar19 = piVar19 + 1;
        }
        aiStack_108[0] = *(int *)((int)param_1 + 0x1ea);
        aiStack_108[5] = 0xffffffff;
        aiStack_108[4] = 0xffffffff;
        aiStack_108[3] = 0xffffffff;
        uStack_e4 = 0xffffffff;
        uStack_ec = 0xffffffff;
        uStack_dc = 0xffffffff;
        aiStack_108[1] = *(undefined4 *)((int)param_1 + 0x1ee);
        aiStack_108[2] = *(undefined4 *)((int)param_1 + 0x1f2);
        iStack_e8 = param_1[0x13];
        aiStack_108[6] = param_1[0x11];
        pppuStack_17c = (undefined1 ***)aiStack_108;
        pppiStack_180 = (int ***)0x13;
        uStack_d8 = 7;
        uStack_e0 = 1;
        FUN_00493f20();
      }
    }
    pppuStack_17c = (undefined1 ***)0x49773c;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x49774b;
      FUN_00464d70();
      pppuStack_17c = (undefined1 ***)0x497752;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 6:
    pppuStack_17c = (undefined1 ***)0x4978de;
    ExceptionList = &local_c;
    iVar7 = FUN_00489080();
    ppvVar6 = ExceptionList;
    if ((iVar7 == 1) || (iVar7 == 0)) goto switchD_00497117_caseD_b;
    if ((iVar7 == 2) || (iVar7 == 4)) goto LAB_00499bcf;
    if ((*(int *)((int)param_1 + 0x22a) != 0) && (local_168 = (short *)0x0, 0 < DAT_005e1460)) {
      do {
        pppuStack_17c = (undefined1 ***)0x45;
        pppiStack_180 = (int ***)0x497929;
        local_164 = (undefined1 **)FUN_00597850();
        local_4 = 4;
        if (local_164 == (undefined1 **)0x0) {
          puVar11 = (undefined4 *)0x0;
        }
        else {
          pppuStack_17c = (undefined1 ***)0x49794b;
          uVar18 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)0x49795d;
          uVar9 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)0x49796f;
          uVar10 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)(((uVar18 & 0x7fffffff) % 3 - 1) + param_1[7]);
          pppiStack_180 = (int ***)(((uVar9 & 0x7fffffff) % 3 - 1) + param_1[6]);
          puVar11 = (undefined4 *)FUN_00546660(((uVar10 & 0x7fffffff) % 3 - 1) + param_1[5]);
        }
        local_4 = 0xffffffff;
        if (puVar11 != (undefined4 *)0x0) {
          iVar7 = *(int *)(&DAT_006b13cd + param_1[0x13] * 0x2d1);
          pppuStack_17c = *(undefined1 ****)(&DAT_005e1444 + iVar7 * 0xc);
          pppiStack_180 = *(int ****)(&DAT_005e1440 + iVar7 * 0xc);
          FUN_005467a0((&DAT_005e143c)[iVar7 * 3]);
          pppiStack_180 = (int ***)0x497a01;
          pppuStack_17c = (undefined1 ***)puVar11;
          iVar7 = FUN_005457e0();
          if (iVar7 == 0) {
            pppuStack_17c = (undefined1 ***)0x1;
            pppiStack_180 = (int ***)0x497a0d;
            (**(code **)*puVar11)();
          }
        }
        local_168 = (short *)((int)local_168 + 1);
      } while ((int)local_168 < DAT_005e1460);
    }
    pppuStack_17c = (undefined1 ***)0x497a31;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x497a41;
      FUN_00464d70();
      pppuStack_17c = (undefined1 ***)0x497a48;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 7:
    pppuStack_17c = (undefined1 ***)0x1;
    pppiStack_180 = (int ***)0x497763;
    ExceptionList = &local_c;
    iVar7 = FUN_004883f0();
    ppvVar6 = ExceptionList;
    if ((iVar7 == 1) || (iVar7 == 0)) goto switchD_00497117_caseD_b;
    if ((iVar7 == 2) || (iVar7 == 4)) goto LAB_00499bcf;
    if ((*(int *)((int)param_1 + 0x22a) != 0) && (local_168 = (short *)0x0, 0 < DAT_005e1460)) {
      do {
        pppuStack_17c = (undefined1 ***)0x45;
        pppiStack_180 = (int ***)0x4977ae;
        local_164 = (undefined1 **)FUN_00597850();
        local_4 = 3;
        if (local_164 == (undefined1 **)0x0) {
          puVar11 = (undefined4 *)0x0;
        }
        else {
          pppuStack_17c = (undefined1 ***)0x4977d0;
          uVar18 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)0x4977e2;
          uVar9 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)0x4977f4;
          uVar10 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)(((uVar18 & 0x7fffffff) % 3 - 1) + param_1[7]);
          pppiStack_180 = (int ***)(((uVar9 & 0x7fffffff) % 3 - 1) + param_1[6]);
          puVar11 = (undefined4 *)FUN_00546660(((uVar10 & 0x7fffffff) % 3 - 1) + param_1[5]);
        }
        local_4 = 0xffffffff;
        if (puVar11 != (undefined4 *)0x0) {
          iVar7 = *(int *)(&DAT_006b13cd + param_1[0x13] * 0x2d1);
          pppuStack_17c = *(undefined1 ****)(&DAT_005e1444 + iVar7 * 0xc);
          pppiStack_180 = *(int ****)(&DAT_005e1440 + iVar7 * 0xc);
          FUN_005467a0((&DAT_005e143c)[iVar7 * 3]);
          pppiStack_180 = (int ***)0x497886;
          pppuStack_17c = (undefined1 ***)puVar11;
          iVar7 = FUN_005457e0();
          if (iVar7 == 0) {
            pppuStack_17c = (undefined1 ***)0x1;
            pppiStack_180 = (int ***)0x497892;
            (**(code **)*puVar11)();
          }
        }
        local_168 = (short *)((int)local_168 + 1);
      } while ((int)local_168 < DAT_005e1460);
    }
    pppuStack_17c = (undefined1 ***)0x4978b6;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x4978c6;
      FUN_00464d70();
      pppuStack_17c = (undefined1 ***)0x4978cd;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 8:
    pppuStack_17c = (undefined1 ***)0x497a5d;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x497a6c;
      FUN_0048b0f0();
    }
    break;
  case 9:
    if (*(int *)((int)param_1 + 0x1d2) < 1) {
      ExceptionList = &local_c;
      if (param_1[0x13] == 0x4b) {
        pppuStack_17c = (undefined1 ***)0x497a8c;
        ExceptionList = &local_c;
        FUN_00493010();
      }
      iVar7 = *(int *)((int)param_1 + 0x1da);
      if (param_1[0x46] <= iVar7) {
        pppuStack_17c = (undefined1 ***)*DAT_00689908;
        if (pppuStack_17c == (undefined1 ***)DAT_00689908) {
LAB_00497b0d:
          pppuStack_17c = (undefined1 ***)0x497b14;
          FUN_004943f0();
        }
        else {
          do {
            if (pppuStack_17c[2] == (undefined1 **)param_1) {
              pppiStack_180 = (int ***)&local_164;
              FUN_0049e2c0();
              goto LAB_00497b0d;
            }
            pppuStack_17c = (undefined1 ***)*pppuStack_17c;
          } while (pppuStack_17c != (undefined1 ***)DAT_00689908);
          pppuStack_17c = (undefined1 ***)0x497af8;
          FUN_004943f0();
        }
        break;
      }
      uVar18 = param_1[iVar7 + 0x18];
      *(int *)((int)param_1 + 0x1da) = iVar7 + 1;
      if (uVar18 < DAT_0068949c) {
        pppuStack_17c = (undefined1 ***)0x497acf;
        FUN_004943f0();
      }
      else {
        pppuStack_17c = (undefined1 ***)0x497ad8;
        FUN_004943f0();
      }
    }
    else {
      pppuStack_17c = (undefined1 ***)0x497b20;
      ExceptionList = &local_c;
      FUN_0048b0f0();
      iVar7 = *(int *)((int)param_1 + 0x1d2) + -1;
      *(int *)((int)param_1 + 0x1d2) = iVar7;
      if (iVar7 == 0) {
        *(undefined4 *)((int)param_1 + 0x1da) = 0;
      }
    }
    pppiStack_180 = (int ***)(param_1 + 0x67);
    if (param_1[0x67] == 0) {
      pppuStack_17c = (undefined1 ***)0x1;
      FUN_00571310(0x2d9,param_1[2],param_1[3],param_1[4],0);
    }
    else {
LAB_00497b49:
      pppuStack_17c = (undefined1 ***)param_1[4];
      pppiStack_180 = (int ***)param_1[3];
      FUN_00571460(param_1 + 0x67,param_1[2]);
    }
    break;
  case 10:
    iVar7 = *(int *)((int)param_1 + 0x1d2) + -1;
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar7;
    if (iVar7 == 0) {
      param_1[0x60] = 1;
    }
    pppuStack_17c = (undefined1 ***)0x497bab;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x497bba;
      FUN_004943f0();
      pppuStack_17c = (undefined1 ***)0x497bc1;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 0xc:
    pppuStack_17c = (undefined1 ***)0x497bd5;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x497be4;
      FUN_004943f0();
      if (*(short *)param_1[0x65] != 0) {
        pppuStack_17c = *(undefined1 ****)(&DAT_006b13c5 + param_1[0x13] * 0x2d1);
        pppiStack_180 = (int ***)0x497c0d;
        FUN_004eb040();
      }
    }
    break;
  case 0xd:
  case 0x13:
  case 0x14:
  case 0x28:
  case 0x2f:
  case 0x3a:
  case 0x3b:
  case 0x3f:
  case 0x4e:
  case 0x52:
  case 0x57:
switchD_00497117_caseD_d:
    ExceptionList = ppvVar5;
    pppuStack_17c = (undefined1 ***)0x4983e9;
    iVar7 = FUN_00464ec0();
LAB_004983e9:
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x4983f8;
      FUN_004943f0();
    }
    break;
  case 0xe:
    pppuStack_17c = (undefined1 ***)0x497e93;
    ExceptionList = &local_c;
    uVar18 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      pppuStack_17c = (undefined1 ***)0x2080;
      pppiStack_180 = (int ***)s_c__Projects_Chaos_effects_cpp_005e0138;
      FUN_00485b10();
    }
    iVar7 = *(int *)((int)param_1 + 0x1d2);
    *(int *)((int)param_1 + 0x1d2) = iVar7 + -1;
    if ((iVar7 == 0) ||
       ((uVar18 & 0x7fffffff) % 99 + 1 < *(uint *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1))) {
LAB_00498034:
      param_1[0x60] = 1;
    }
    else {
      iVar7 = *(int *)((int)param_1 + 0x1d6);
      *(int *)((int)param_1 + 0x1d6) = iVar7 + -1;
      if (iVar7 < 1) {
        *(undefined4 *)((int)param_1 + 0x1d6) =
             *(undefined4 *)(&DAT_006b1274 + param_1[0x13] * 0x2d1);
        pppuStack_17c = (undefined1 ***)0x497f17;
        FUN_0048a540();
      }
      iVar7 = *(int *)((int)param_1 + 0x1da);
      *(int *)((int)param_1 + 0x1da) = iVar7 + -1;
      if (iVar7 < 1) {
        pppuStack_17c = (undefined1 ***)0x497f31;
        FUN_0048b0f0();
      }
      pppuStack_17c = (undefined1 ***)0x497f3c;
      iVar7 = FUN_00464ec0();
      if (iVar7 == 1) {
        if (*(int *)(param_1[100] + 0x94) == 0x10) {
          if (((param_1[2] < 0) || ((int)DAT_006c5494 <= param_1[2])) ||
             ((param_1[3] < 0 ||
              ((((int)DAT_006c5498 <= param_1[3] || (param_1[4] + 1 < 0)) ||
               (DAT_006c549c <= param_1[4] + 1)))))) goto LAB_00497fe8;
          *(ushort *)(param_1[0x65] + 10) = *(ushort *)(param_1[0x65] + 10) & 0xffef;
          iVar7 = (&DAT_006cb942)[param_1[3]] + (&DAT_006cb8c6)[param_1[4]] + param_1[2];
          uVar2 = *(ushort *)(DAT_006c54dc + iVar7 * 0xc);
          if ((uVar2 == 0) || (*(int *)(DAT_0065660c + 0x94 + (uint)uVar2 * 0x164) != 0x10)) {
            puVar1 = (ushort *)(DAT_006c54dc + iVar7 * 0xc + 10);
            *puVar1 = *puVar1 & 0xffef;
          }
        }
        else {
LAB_00497fe8:
          *(ushort *)(param_1[0x65] + 10) = *(ushort *)(param_1[0x65] + 10) & 0xffef;
        }
        if (((*(byte *)(param_1[100] + 0xb0) & 0x20) == 0) && (*(int *)((int)param_1 + 0x1d2) < 1))
        {
          pppuStack_17c = (undefined1 ***)0xff;
          pppiStack_180 = (int ***)0x49801d;
          FUN_004eb040();
        }
        if (param_1[1] != 0) {
          pppuStack_17c = (undefined1 ***)0x49802f;
          FUN_004943f0();
        }
      }
    }
    break;
  case 0x11:
    ExceptionList = &local_c;
    if ((uint)*(ushort *)(param_1[0x65] + 4) != param_1[0x11]) {
      ExceptionList = &local_c;
      param_1[0x60] = 1;
    }
    uVar2 = *(ushort *)(param_1[0x65] + 4);
    if (uVar2 != 0xffff) {
      if (uVar2 < DAT_006def5c) {
        iVar7 = (uint)uVar2 * 0xe4b + DAT_006def58;
      }
      else {
        iVar7 = 0;
      }
      if ((((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
          (*(int *)(iVar7 + 0x24 + DAT_00644520 * 4) == 0)) && (DAT_005e1404 != 0)) {
        *(undefined4 *)((int)param_1 + 0x22a) = 0;
      }
      else {
        *(undefined4 *)((int)param_1 + 0x22a) = 1;
      }
    }
    pppuStack_17c = (undefined1 ***)0x499022;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x49902d;
      FUN_004943f0();
    }
    else {
      pppiStack_180 = (int ***)(param_1 + 0x67);
      if (param_1[0x67] == 0) {
        pppuStack_17c = (undefined1 ***)0x1;
        FUN_00571310(0x405,param_1[2],param_1[3],param_1[4],0);
      }
      else {
        pppuStack_17c = (undefined1 ***)param_1[4];
        pppiVar14 = pppiStack_180;
        pppiStack_180 = (int ***)param_1[3];
        FUN_00571460(pppiVar14,param_1[2]);
      }
    }
    break;
  case 0x12:
    pppuStack_17c = (undefined1 ***)0x4995e5;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x4995f0;
      FUN_004943f0();
    }
    else if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x0;
      pppiStack_180 = (int ***)0x0;
      FUN_00571310(0x2d4,param_1[2],param_1[3],param_1[4],0);
      pppuStack_17c = (undefined1 ***)0x499626;
      FUN_0048b0f0();
    }
    break;
  case 0x15:
    iVar17 = *(int *)((int)param_1 + 0x1d2);
    iVar7 = *(int *)((int)param_1 + 0x1da);
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar17 + -1;
    if ((iVar17 == 0) && (0 < iVar7)) {
      pppuStack_17c = (undefined1 ***)0x497c36;
      FUN_00491810();
LAB_00497c9f:
      *(undefined4 *)((int)param_1 + 0x1d2) = DAT_006bde91;
      *(int *)((int)param_1 + 0x1da) = *(int *)((int)param_1 + 0x1da) + -1;
    }
    else {
LAB_00497c3a:
      if (iVar7 == 0) {
        pppuStack_17c = (undefined1 ***)0x497c47;
        FUN_0048b0f0();
      }
    }
    break;
  case 0x16:
    pppuStack_17c = (undefined1 ***)0x4998cb;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x4998d6;
      FUN_004943f0();
    }
    else if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x4998eb;
      FUN_00493c30();
    }
    break;
  case 0x18:
    iVar7 = *(int *)((int)param_1 + 0x1d2) + -1;
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar7;
    if (iVar7 < 1) {
      pppuStack_17c = (undefined1 ***)0x0;
      *(undefined4 *)((int)param_1 + 0x1d2) = 3;
      pppiStack_180 = (int ***)0x498f0b;
      FUN_00492090();
    }
    if (*(int *)((int)param_1 + 0x1da) == 0) {
      pppuStack_17c = (undefined1 ***)0x498f20;
      FUN_004943f0();
    }
    break;
  case 0x19:
    pppuStack_17c = (undefined1 ***)0x497cc8;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x1a;
      pppiStack_180 = (int ***)0x497cd9;
      FUN_00494ab0();
    }
    break;
  case 0x1a:
    iVar7 = *(int *)((int)param_1 + 0x1d2);
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar7 + -1;
    if (iVar7 < 1) {
      pppuStack_17c = (undefined1 ***)0x1b;
      pppiStack_180 = (int ***)0x497cfa;
      FUN_00494ab0();
      *(undefined4 *)((int)param_1 + 0x1d2) = 999;
    }
    else {
      pppuStack_17c = (undefined1 ***)0x497d16;
      iVar7 = FUN_00464ec0();
      if (iVar7 == 1) {
        pppuStack_17c = (undefined1 ***)0x497d21;
        FUN_00464d70();
      }
      else if (iVar7 == 2) {
        pppuStack_17c = (undefined1 ***)0x497d32;
        uVar18 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          pppuStack_17c = (undefined1 ***)0x205a;
          pppiStack_180 = (int ***)s_c__Projects_Chaos_effects_cpp_005e0138;
          FUN_00485b10();
        }
        pppuStack_17c = (undefined1 ***)0x0;
        pppiStack_180 =
             (int ***)(*(int *)(*(int *)(param_1[9] + 0x438) + 0x2b0) + (uVar18 & 0x7fffffff) % 3);
        FUN_00464cb0();
      }
      iVar7 = *(int *)((int)param_1 + 0x1d6);
      *(int *)((int)param_1 + 0x1d6) = iVar7 + -1;
      if (iVar7 < 1) {
        pppuStack_17c = (undefined1 ***)0x497d93;
        FUN_0048b0f0();
      }
      uVar2 = *(ushort *)(param_1[0x65] + 4);
      if (uVar2 != 0xffff) {
        if (uVar2 < DAT_006def5c) {
          param_1[0x66] = (uint)uVar2 * 0xe4b + DAT_006def58;
          pppuStack_17c = (undefined1 ***)0x497dcf;
          FUN_0050cf30();
        }
        else {
          param_1[0x66] = 0;
          pppuStack_17c = (undefined1 ***)0x497de3;
          FUN_0050cf30();
        }
      }
    }
    break;
  case 0x1b:
    pppuStack_17c = (undefined1 ***)0x497df3;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      if (param_1[0x17] == 2) {
        uVar18 = 0;
        puVar8 = (uint *)(param_1 + 0x18);
        do {
          if (*puVar8 < DAT_0068949c) {
            piVar19 = (int *)(DAT_00689498 + *puVar8 * 0x22e);
          }
          else {
            piVar19 = (int *)0x0;
          }
          if ((*piVar19 != *param_1) && (piVar19[1] != 0)) {
            piVar19[0x17] = 1;
            pppuStack_17c = (undefined1 ***)0x1b;
            *(ushort *)(piVar19[0x65] + 10) = *(ushort *)(piVar19[0x65] + 10) & 0xffdf;
            pppiStack_180 = (int ***)0x497e59;
            FUN_00494ab0();
            *(undefined4 *)((int)piVar19 + 0x1d2) = 999;
          }
          uVar18 = uVar18 + 1;
          puVar8 = puVar8 + 1;
        } while (uVar18 <= (uint)param_1[0x47]);
      }
      *(ushort *)(param_1[0x65] + 10) = *(ushort *)(param_1[0x65] + 10) & 0xffdf;
      pppuStack_17c = (undefined1 ***)0x497e84;
      FUN_004943f0();
    }
    break;
  case 0x1c:
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar7 = param_1[0x11] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    ExceptionList = &local_c;
    param_1[0x66] = iVar7;
    if (*(int *)(iVar7 + 0xe4) != 0) {
      pppuStack_17c = (undefined1 ***)param_1[0x11];
      pppiStack_180 = (int ***)0x4983b0;
      FUN_00489500();
      if (((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
         (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) == 0)) {
        *(undefined4 *)((int)param_1 + 0x22a) = 0;
        ppvVar5 = ExceptionList;
      }
      else {
        *(undefined4 *)((int)param_1 + 0x22a) = 1;
        ppvVar5 = ExceptionList;
      }
      goto switchD_00497117_caseD_d;
    }
    pppuStack_17c = (undefined1 ***)0x4983a0;
    FUN_004943f0();
    break;
  case 0x1d:
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar7 = param_1[0x11] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    ExceptionList = &local_c;
    param_1[0x66] = iVar7;
    if (*(int *)(iVar7 + 0xe4) == 0) {
      pppuStack_17c = (undefined1 ***)0x498431;
      FUN_004943f0();
    }
    else {
      pppuStack_17c = (undefined1 ***)param_1[0x11];
      pppiStack_180 = (int ***)0x498441;
      FUN_00489500();
      if (((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
         (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) == 0)) {
        *(undefined4 *)((int)param_1 + 0x22a) = 0;
      }
      else {
        *(undefined4 *)((int)param_1 + 0x22a) = 1;
      }
      pppuStack_17c = (undefined1 ***)0x49847a;
      iVar7 = FUN_00464ec0();
      if (iVar7 == 1) {
        if (*(short *)(DAT_006c54dc + 6 +
                      ((&DAT_006cb942)[param_1[3]] + (&DAT_006cb8be)[param_1[4]] + param_1[2]) * 0xc
                      ) != -1) {
          if ((uint)param_1[0x11] < DAT_006def5c) {
            pppuStack_17c = (undefined1 ***)0x4984f3;
            FUN_00468470();
            pppuStack_17c = (undefined1 ***)0x4984fa;
            FUN_004943f0();
            break;
          }
          pppuStack_17c = (undefined1 ***)0x498508;
          FUN_00468470();
        }
        pppuStack_17c = (undefined1 ***)0x49850f;
        FUN_004943f0();
      }
    }
    break;
  case 0x1e:
    iVar7 = *(int *)((int)param_1 + 0x1d2);
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar7 + -1;
    if (iVar7 < 1) {
      pppuStack_17c = (undefined1 ***)0x498106;
      FUN_0048b0f0();
    }
    break;
  case 0x1f:
    pppuStack_17c = (undefined1 ***)0x498046;
    ExceptionList = &local_c;
    FUN_0048b0f0();
    iVar7 = *(int *)((int)param_1 + 0x1d2);
    *(int *)((int)param_1 + 0x1d2) = iVar7 + -1;
    if (iVar7 < 1) {
      if (param_1[0x13] == 0x58) {
        pppuStack_17c = (undefined1 ***)*DAT_006898f0;
        if (pppuStack_17c == (undefined1 ***)DAT_006898f0) {
LAB_004980dc:
          pppuStack_17c = (undefined1 ***)0x4980e3;
          FUN_004943f0();
        }
        else {
          do {
            if (pppuStack_17c[2] == (undefined1 **)param_1) {
              pppiStack_180 = (int ***)&local_164;
              FUN_0049e2c0();
              pppuStack_17c = (undefined1 ***)0x49809f;
              FUN_004943f0();
              goto switchD_00497117_caseD_f;
            }
            pppuStack_17c = (undefined1 ***)*pppuStack_17c;
          } while (pppuStack_17c != (undefined1 ***)DAT_006898f0);
          pppuStack_17c = (undefined1 ***)0x498083;
          FUN_004943f0();
        }
      }
      else {
        if ((param_1[0x13] != 0x22) ||
           (pppuStack_17c = (undefined1 ***)*DAT_006898fc,
           pppuStack_17c == (undefined1 ***)DAT_006898fc)) goto LAB_004980dc;
        do {
          if (pppuStack_17c[2] == (undefined1 **)param_1) {
            pppiStack_180 = (int ***)&local_164;
            FUN_0049e2c0();
            goto LAB_004980dc;
          }
          pppuStack_17c = (undefined1 ***)*pppuStack_17c;
        } while (pppuStack_17c != (undefined1 ***)DAT_006898fc);
        pppuStack_17c = (undefined1 ***)0x4980c7;
        FUN_004943f0();
      }
    }
    break;
  case 0x22:
    if ((uint)param_1[0x12] < DAT_006def5c) {
      iVar7 = param_1[0x12] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    pppuStack_17c = (undefined1 ***)0x0;
    pppiStack_180 = (int ***)0x0;
    ExceptionList = &local_c;
    param_1[0x66] = iVar7;
    FUN_004918b0(param_1[0x16],0,0,0);
    pppuStack_17c = (undefined1 ***)0x498f91;
    FUN_0048b0f0();
    pppuStack_17c = (undefined1 ***)0x498f98;
    FUN_004943f0();
    break;
  case 0x23:
    pppuStack_17c = (undefined1 ***)0x1;
    pppiStack_180 = (int ***)0x498146;
    ExceptionList = &local_c;
    iVar7 = FUN_004883f0();
    if (iVar7 == 1) {
      if ((*(byte *)(param_1[100] + 0xb0) & 1) == 0) {
        pppuStack_17c = (undefined1 ***)0x24;
        pppiStack_180 = (int ***)0x498170;
        FUN_00494ab0();
      }
      else {
        pppuStack_17c = (undefined1 ***)0x25;
        pppiStack_180 = (int ***)0x498162;
        FUN_00494ab0();
      }
    }
    else if (iVar7 == 0) {
      pppuStack_17c = *(undefined1 ****)(&DAT_006b13c5 + param_1[0x13] * 0x2d1);
      pppiStack_180 = (int ***)0x498194;
      FUN_0048ecf0();
      pppuStack_17c = (undefined1 ***)0x24;
      pppiStack_180 = (int ***)0x49819d;
      FUN_00494ab0();
      if ((param_1[0x11] != -1) && (param_1[0x12] != -1)) {
        iVar7 = param_1[0x66];
        *(int *)(iVar7 + 0x80d) = param_1[0x11] + 1;
        *(undefined4 *)(iVar7 + 0x811) = 100;
      }
    }
    else if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x4981db;
      FUN_004943f0();
    }
    else {
      pppuStack_17c = (undefined1 ***)0x4981ed;
      iVar7 = FUN_00464ec0();
      while (iVar7 == 1) {
        pppuStack_17c = (undefined1 ***)0x4981fc;
        FUN_00464d70();
        pppuStack_17c = (undefined1 ***)0x498203;
        iVar7 = FUN_00464ec0();
      }
    }
    break;
  case 0x24:
    pppuStack_17c = (undefined1 ***)0x498219;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x498228;
      FUN_004943f0();
      pppuStack_17c = (undefined1 ***)0x49822f;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 0x25:
    pppuStack_17c = (undefined1 ***)0x498245;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x498254;
      FUN_004943f0();
      pppuStack_17c = (undefined1 ***)0x49825b;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 0x26:
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar7 = param_1[0x11] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    ExceptionList = &local_c;
    param_1[0x66] = iVar7;
    if (*(int *)(iVar7 + 0xe4) == 0) {
      pppuStack_17c = (undefined1 ***)0x498298;
      FUN_004943f0();
    }
    else {
      pppuStack_17c = (undefined1 ***)param_1[0x11];
      pppiStack_180 = (int ***)0x4982a8;
      FUN_00489500();
      if (((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
         (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) == 0)) {
        *(undefined4 *)((int)param_1 + 0x22a) = 0;
      }
      else {
        *(undefined4 *)((int)param_1 + 0x22a) = 1;
      }
      pppuStack_17c = (undefined1 ***)0x4982e1;
      iVar7 = FUN_00464ec0();
      if (iVar7 == 1) {
        pppuStack_17c = (undefined1 ***)0x4982ec;
        FUN_004943f0();
      }
      else if (iVar7 == 2) {
        pppiStack_180 = (int ***)((int)param_1 + 0x216);
        pppuStack_17c = (undefined1 ***)0x0;
        FUN_005076f0(param_1[0x14],*(undefined4 *)((int)param_1 + 0x212));
        pppuStack_17c = (undefined1 ***)0x0;
        iVar7 = param_1[0x66];
        pppiStack_180 = (int ***)0x0;
        if (param_1[0x13] == 4) {
          FUN_00571310(0x404,*(undefined4 *)(iVar7 + 8),*(undefined4 *)(iVar7 + 0xc),
                       *(undefined4 *)(iVar7 + 0x10),0);
        }
        else {
          FUN_00571310(0x403,*(undefined4 *)(iVar7 + 8),*(undefined4 *)(iVar7 + 0xc),
                       *(undefined4 *)(iVar7 + 0x10),0);
        }
      }
    }
    break;
  case 0x27:
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar7 = param_1[0x11] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    ExceptionList = &local_c;
    param_1[0x66] = iVar7;
    if (*(int *)(iVar7 + 0xe4) == 0) {
      pppuStack_17c = (undefined1 ***)0x498548;
      FUN_004943f0();
    }
    else {
      pppuStack_17c = (undefined1 ***)param_1[0x11];
      pppiStack_180 = (int ***)0x498558;
      FUN_00489500();
      if (((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
         (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) == 0)) {
        *(undefined4 *)((int)param_1 + 0x22a) = 0;
      }
      else {
        *(undefined4 *)((int)param_1 + 0x22a) = 1;
      }
      pppuStack_17c = (undefined1 ***)0x498591;
      iVar7 = FUN_00464ec0();
      if (iVar7 == 1) {
        if ((uint)param_1[0x11] < DAT_006def5c) {
          iVar7 = param_1[0x11] * 0xe4b + DAT_006def58;
        }
        else {
          iVar7 = 0;
        }
        local_15c = *(short **)(iVar7 + 8);
        local_158 = *(undefined1 ****)(iVar7 + 0xc);
        local_160 = *(int **)(iVar7 + 0x10);
        pppuStack_17c = (undefined1 ***)&local_160;
        pppiStack_180 = (int ***)0x4985df;
        iVar17 = FUN_004694e0();
        if (iVar17 != 0) {
          pppuStack_17c = (undefined1 ***)local_160;
          pppiStack_180 = (int ***)local_158;
          iVar17 = FUN_00468ad0();
          *(int *)(iVar17 + 0x6a) = param_1[0x18];
          *(int *)(iVar17 + 0x6e) = param_1[0x19];
          *(int *)(iVar17 + 0x72) = param_1[0x1a];
          *(int *)(iVar17 + 0x76) = param_1[0x1b];
          if (((local_15c == *(short **)(iVar7 + 8)) &&
              ((int ***)local_158 == *(int ****)(iVar7 + 0xc))) &&
             ((undefined1 **)local_160 == *(undefined1 ***)(iVar7 + 0x10))) {
            if ((uint)param_1[0x11] < DAT_006def5c) {
              pppuStack_17c = (undefined1 ***)0x498656;
              FUN_00468470();
              pppuStack_17c = (undefined1 ***)0x49865d;
              FUN_004943f0();
              break;
            }
            pppuStack_17c = (undefined1 ***)0x49866b;
            FUN_00468470();
          }
        }
        pppuStack_17c = (undefined1 ***)0x498672;
        FUN_004943f0();
      }
      else if (iVar7 == 2) {
        pppuStack_17c = (undefined1 ***)0x49868a;
        iVar7 = FUN_0049cdb0();
        if (iVar7 == 0) goto LAB_00498034;
        pppuStack_17c = (undefined1 ***)(param_1 + 0xb);
        param_1[0x16] = *param_1;
        pppiStack_180 = (int ***)0x37;
        FUN_00493f20();
      }
    }
    break;
  case 0x2a:
    pppuStack_17c = (undefined1 ***)0x20;
    pppiStack_180 = (int ***)0x4;
    ExceptionList = &local_c;
    FUN_004918b0(3,3,0,0);
    uVar18 = param_1[0x12];
    if (uVar18 != 0xffffffff) {
      if (uVar18 < DAT_006def5c) {
        iVar7 = uVar18 * 0xe4b + DAT_006def58;
      }
      else {
        iVar7 = 0;
      }
      param_1[0x66] = iVar7;
      pppuStack_17c = (undefined1 ***)0x1;
      pppiStack_180 = (int ***)0x0;
      FUN_005076f0(*(undefined4 *)(&DAT_006b13c5 + param_1[0x13] * 0x2d1),0xffffffff);
      pppuStack_17c = (undefined1 ***)param_1[0x66];
      pppiStack_180 = (int ***)0x498b6d;
      FUN_0049a350();
      if (*(int *)(param_1[0x66] + 0x114) != 0) {
        pppuStack_17c = (undefined1 ***)0x498b87;
        uVar18 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          pppuStack_17c = (undefined1 ***)0x221d;
          pppiStack_180 = (int ***)s_c__Projects_Chaos_effects_cpp_005e0138;
          FUN_00485b10();
        }
        if ((uint)(DAT_006bc985 * 100) /
            (uint)(*(int *)(&DAT_006a5fac + *(int *)(param_1[0x66] + 0xa8) * 0x5c9) + DAT_006bc985)
            < (uVar18 & 0x7fffffff) % 100) {
          pppuStack_17c = (undefined1 ***)0x498bec;
          FUN_00516150();
        }
      }
    }
    pppuStack_17c = (undefined1 ***)0x498bf3;
    FUN_004943f0();
    break;
  case 0x2b:
  case 0x46:
    pppuStack_17c = (undefined1 ***)0x4986b6;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x4986c5;
      FUN_00464d70();
    }
    break;
  case 0x2c:
    iVar7 = *(int *)((int)param_1 + 0x1d2);
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar7 + -1;
    if (iVar7 < 1) {
      pppuStack_17c = (undefined1 ***)0x2d;
      pppiStack_180 = (int ***)0x498715;
      FUN_00494ab0();
    }
    break;
  case 0x2d:
    pppuStack_17c = (undefined1 ***)0x498725;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      local_15c = (short *)(DAT_006c54dc +
                           ((&DAT_006cb942)[param_1[3]] + (&DAT_006cb8c2)[param_1[4]] + param_1[2])
                           * 0xc);
      iVar7 = param_1[0x19];
      if (iVar7 == 0) {
        if (*local_15c != 0) {
          pppuStack_17c = (undefined1 ***)param_1[0x18];
          pppiStack_180 = (int ***)0x4988aa;
          FUN_004eb040();
        }
        piVar19 = &DAT_005e1784;
        local_160 = &DAT_005e178c;
        do {
          if (local_160 == &DAT_005e178c) {
LAB_0049893a:
            uVar2 = local_15c[2];
            if (uVar2 != 0xffff) {
              if (uVar2 < DAT_006def5c) {
                iVar7 = (uint)uVar2 * 0xe4b + DAT_006def58;
              }
              else {
                iVar7 = 0;
              }
              if (((*(int *)(iVar7 + 8) == param_1[2]) && (*(int *)(iVar7 + 0xc) == param_1[3])) &&
                 ((*(uint *)(*(int *)(iVar7 + 0xac) + 0xc) >> 1) + *(int *)(iVar7 + 0x10) ==
                  param_1[4])) {
                pppiStack_180 = *(int ****)((int)param_1 + 0x212);
                pppuStack_17c = (undefined1 ***)((int)param_1 + 0x216);
                FUN_00514860(param_1[0x18]);
                iVar17 = param_1[0x11];
                if (iVar17 != -1) {
                  *(undefined4 *)(iVar7 + 0x811) = 100;
                  *(int *)(iVar7 + 0x80d) = iVar17 + 1;
                }
                break;
              }
            }
          }
          else {
            ppuVar16 = (undefined1 **)(piVar19[-1] + param_1[2]);
            if ((int)ppuVar16 < 0) {
              do {
                ppuVar16 = (undefined1 **)((int)ppuVar16 + (int)DAT_006c5494);
              } while ((int)ppuVar16 < 0);
            }
            else {
              for (; DAT_006c5494 <= ppuVar16;
                  ppuVar16 = (undefined1 **)((int)ppuVar16 - (int)DAT_006c5494)) {
              }
            }
            pppiVar14 = (int ***)(*piVar19 + param_1[3]);
            if ((int)pppiVar14 < 0) {
              do {
                pppiVar14 = (int ***)((int)pppiVar14 + (int)DAT_006c5498);
              } while ((int)pppiVar14 < 0);
            }
            else {
              for (; DAT_006c5498 <= pppiVar14;
                  pppiVar14 = (int ***)((int)pppiVar14 - (int)DAT_006c5498)) {
              }
            }
            iVar7 = piVar19[1] + param_1[4];
            if ((-1 < iVar7) && (iVar7 < DAT_006c549c)) {
              local_15c = (short *)(DAT_006c54dc +
                                   ((int)ppuVar16 +
                                   (&DAT_006cb8c2)[iVar7] + (&DAT_006cb942)[(int)pppiVar14]) * 0xc);
              goto LAB_0049893a;
            }
          }
          piVar19 = piVar19 + 3;
          local_160 = local_160 + 3;
        } while ((int)piVar19 < 0x5e18c8);
        if ((((local_15c[5] & 0x30U) != 0) && ((local_15c[5] & 0x10U) == 0)) &&
           (uVar2 = local_15c[1], uVar2 != 0xffff)) {
          if (uVar2 < DAT_0068949c) {
            iVar7 = DAT_00689498 + (uint)uVar2 * 0x22e;
          }
          else {
            iVar7 = 0;
          }
          if (*(int *)(iVar7 + 0x28) == 0x1a) {
            iVar17 = *(int *)(iVar7 + 0x1d2) - param_1[0x18];
            *(int *)(iVar7 + 0x1d2) = iVar17;
            if (iVar17 < 0xb) {
              *(int *)(iVar7 + 0x1d2) = iVar17 + -10;
              pppuStack_17c = (undefined1 ***)0x498a69;
              FUN_004943f0();
              break;
            }
            if (*(int *)(iVar7 + 0x28) == 0x1a) {
              pppuStack_17c = (undefined1 ***)0x0;
              pppiStack_180 = *(int ****)(*(int *)(*(int *)(iVar7 + 0x24) + 0x438) + 0x2e0);
              FUN_00464cb0();
            }
          }
        }
        pppuStack_17c = (undefined1 ***)0x498a97;
        FUN_004943f0();
      }
      else if (iVar7 == 1) {
        local_160 = &DAT_005e178c;
        piVar19 = &DAT_005e1784;
        ppuVar16 = DAT_006c5494;
        pppiVar14 = DAT_006c5498;
        do {
          if (local_160 == &DAT_005e178c) {
LAB_0049880a:
            uVar2 = local_15c[2];
            if (uVar2 != 0xffff) {
              if (uVar2 < DAT_006def5c) {
                iVar7 = (uint)uVar2 * 0xe4b + DAT_006def58;
              }
              else {
                iVar7 = 0;
              }
              if (((*(int *)(iVar7 + 8) == param_1[2]) && (*(int *)(iVar7 + 0xc) == param_1[3])) &&
                 (*(int *)(iVar7 + 0x10) == param_1[4])) {
                pppuStack_17c = *(undefined1 ****)((int)param_1 + 0x212);
                pppiStack_180 = (int ***)0x498863;
                FUN_0050d9f0();
                ppuVar16 = DAT_006c5494;
                pppiVar14 = DAT_006c5498;
              }
            }
          }
          else {
            ppuVar15 = (undefined1 **)(piVar19[-1] + param_1[2]);
            if ((int)ppuVar15 < 0) {
              do {
                ppuVar15 = (undefined1 **)((int)ppuVar15 + (int)ppuVar16);
              } while ((int)ppuVar15 < 0);
            }
            else {
              for (; ppuVar16 <= ppuVar15; ppuVar15 = (undefined1 **)((int)ppuVar15 - (int)ppuVar16)
                  ) {
              }
            }
            pppiVar13 = (int ***)(*piVar19 + param_1[3]);
            if ((int)pppiVar13 < 0) {
              do {
                pppiVar13 = (int ***)((int)pppiVar13 + (int)pppiVar14);
              } while ((int)pppiVar13 < 0);
            }
            else {
              for (; pppiVar14 <= pppiVar13; pppiVar13 = (int ***)((int)pppiVar13 - (int)pppiVar14))
              {
              }
            }
            iVar7 = piVar19[1] + param_1[4];
            if ((-1 < iVar7) && (iVar7 < DAT_006c549c)) {
              local_15c = (short *)(DAT_006c54dc +
                                   ((int)ppuVar15 +
                                   (&DAT_006cb8c2)[iVar7] + (&DAT_006cb942)[(int)pppiVar13]) * 0xc);
              goto LAB_0049880a;
            }
          }
          piVar19 = piVar19 + 3;
          local_160 = local_160 + 3;
        } while ((int)piVar19 < 0x5e18c8);
        pppuStack_17c = (undefined1 ***)0xe;
        pppiStack_180 = (int ***)0x498892;
        FUN_00494ab0();
      }
      else if (iVar7 == 2) {
        pppuStack_17c = (undefined1 ***)0x498774;
        FUN_0048b0f0();
      }
    }
    break;
  case 0x2e:
    pppuStack_17c = (undefined1 ***)0x20;
    pppiStack_180 = (int ***)0x4;
    ExceptionList = &local_c;
    FUN_004918b0(2,3,0,0);
    uVar18 = param_1[0x12];
    if (uVar18 != 0xffffffff) {
      if (uVar18 < DAT_006def5c) {
        iVar7 = uVar18 * 0xe4b + DAT_006def58;
      }
      else {
        iVar7 = 0;
      }
      pppiStack_180 = *(int ****)((int)param_1 + 0x212);
      param_1[0x66] = iVar7;
      pppuStack_17c = (undefined1 ***)0x0;
      FUN_00514860(DAT_006bd8eb);
      *(undefined4 *)(param_1[0x66] + 0x815) = 1;
    }
    pppuStack_17c = (undefined1 ***)0x498afe;
    FUN_004943f0();
    break;
  case 0x30:
    iVar17 = *(int *)((int)param_1 + 0x1d2);
    iVar7 = *(int *)((int)param_1 + 0x1da);
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar17 + -1;
    if (iVar17 != 0) goto LAB_00497c3a;
    if (-1 < iVar7) {
      if (((param_1[0x66] != 0) && (-1 < DAT_00644520)) &&
         ((DAT_00644520 < 8 && (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) != 0)))) {
        pppuStack_17c = (undefined1 ***)0x20;
        pppiStack_180 = (int ***)0x4;
        FUN_004918b0(0,3,0,0);
      }
      goto LAB_00497c9f;
    }
    break;
  case 0x31:
    iVar7 = *(int *)((int)param_1 + 0x1d2);
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar7 + -1;
    if (iVar7 < 1) {
      pppuStack_17c = (undefined1 ***)0xff;
      pppiStack_180 = (int ***)0x498132;
      FUN_004eb040();
      pppuStack_17c = (undefined1 ***)0x498139;
      FUN_004943f0();
    }
    break;
  case 0x33:
    pppiStack_180 = (int ***)param_1[0x11];
    pppuStack_17c = (undefined1 ***)0x0;
    ExceptionList = &local_c;
    FUN_004896b0();
    pppuStack_17c = (undefined1 ***)0x498cb3;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x498cbf;
      FUN_004919d0();
    }
    else if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x34;
      pppiStack_180 = (int ***)0x498cd5;
      FUN_00494ab0();
    }
    break;
  case 0x34:
    pppiStack_180 = (int ***)param_1[0x11];
    pppuStack_17c = (undefined1 ***)0x0;
    ExceptionList = &local_c;
    FUN_004896b0();
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar7 = param_1[0x11] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    param_1[0x66] = iVar7;
    if (((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
       (*(int *)(iVar7 + 0x24 + DAT_00644520 * 4) == 0)) {
      *(undefined4 *)((int)param_1 + 0x22a) = 0;
    }
    else {
      *(undefined4 *)((int)param_1 + 0x22a) = 1;
    }
    pppuStack_17c = (undefined1 ***)0x498c5e;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x498c69;
      FUN_00464d70();
    }
    iVar7 = *(int *)((int)param_1 + 0x1d2) + -1;
    *(int *)((int)param_1 + 0x1d2) = iVar7;
    if ((0x1e < iVar7) || (iVar7 < 1)) {
      *(undefined4 *)((int)param_1 + 0x1d2) = 0x1e;
      pppuStack_17c = (undefined1 ***)0x498c96;
      FUN_00491b30();
    }
    break;
  case 0x36:
    pppuStack_17c = (undefined1 ***)0x498ce7;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)param_1[4];
      pppiStack_180 = (int ***)param_1[3];
      FUN_00468ad0();
      pppuStack_17c = (undefined1 ***)0x498d09;
      FUN_004943f0();
      pppuStack_17c = (undefined1 ***)0x498d10;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 0x37:
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar7 = param_1[0x11] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    ExceptionList = &local_c;
    param_1[0x66] = iVar7;
    if (*(int *)(iVar7 + 0xe4) == 0) {
      pppuStack_17c = (undefined1 ***)0x498d4d;
      FUN_004943f0();
    }
    else {
      pppuStack_17c = (undefined1 ***)param_1[0x11];
      pppiStack_180 = (int ***)0x498d5d;
      FUN_00489500();
      if (((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
         (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) == 0)) {
        *(undefined4 *)((int)param_1 + 0x22a) = 0;
      }
      else {
        *(undefined4 *)((int)param_1 + 0x22a) = 1;
      }
      pppuStack_17c = (undefined1 ***)0x498d96;
      iVar7 = FUN_00464ec0();
      if (iVar7 == 1) {
        if (((uint)param_1[0x16] < DAT_0068949c) &&
           (iVar7 = DAT_00689498 + param_1[0x16] * 0x22e, iVar7 != 0)) {
          pppuStack_17c = (undefined1 ***)param_1[0x11];
          *(undefined1 ****)(iVar7 + 0x44) = pppuStack_17c;
          switch(*(undefined4 *)(iVar7 + 0x28)) {
          default:
            pppuStack_17c = (undefined1 ***)0x498e1e;
            FUN_004943f0();
            goto LAB_00498e1e;
          case 0x1a:
            pppuStack_17c = (undefined1 ***)0x1b;
            pppiStack_180 = (int ***)0x498e0b;
            FUN_00494ab0();
            pppuStack_17c = (undefined1 ***)0x498e12;
            FUN_004943f0();
            break;
          case 0x27:
            pppiStack_180 = (int ***)0x498df0;
            FUN_004898a0();
          case 0xe:
          case 0x28:
          case 0x3a:
            *(undefined4 *)(iVar7 + 0x180) = 1;
            pppuStack_17c = (undefined1 ***)0x498dfd;
            FUN_004943f0();
          }
        }
        else {
LAB_00498e1e:
          pppuStack_17c = (undefined1 ***)0x498e25;
          FUN_004943f0();
        }
      }
      else if (iVar7 == 2) {
        uVar18 = param_1[0x11];
        uVar9 = param_1[0x12];
        if (uVar18 == uVar9) {
          if (uVar18 < DAT_006def5c) {
            iVar7 = uVar18 * 0xe4b + DAT_006def58;
          }
          else {
            iVar7 = 0;
          }
          param_1[0x66] = iVar7;
          if ((iVar7 != 0) && (*(int *)(iVar7 + 4) != 0)) {
            pppuStack_17c = *(undefined1 ****)(iVar7 + 0x174);
            if (pppuStack_17c == (undefined1 ***)0xffffffff) {
              puVar11 = (undefined4 *)0x0;
            }
            else {
              puVar11 = &DAT_00643a80 + (int)pppuStack_17c * 0x55;
            }
            if (puVar11[3] != -1) {
              pppiStack_180 = (int ***)0x498ea8;
              puVar8 = (uint *)FUN_00466870();
              uVar9 = *puVar8;
              goto LAB_00498eaa;
            }
          }
          goto LAB_00499bcf;
        }
LAB_00498eaa:
        param_1[0x11] = uVar9;
        if (uVar9 < DAT_006def5c) {
          pppuStack_17c = *(undefined1 ****)(uVar9 * 0xe4b + DAT_006def58);
          pppiStack_180 = (int ***)0x498ecf;
          FUN_004898a0();
        }
        else {
          pppuStack_17c = (undefined1 ***)uRam00000000;
          pppiStack_180 = (int ***)0x498ee0;
          FUN_004898a0();
        }
      }
    }
    break;
  case 0x38:
    iVar7 = *(int *)((int)param_1 + 0x1d2) + -1;
    ExceptionList = &local_c;
    *(int *)((int)param_1 + 0x1d2) = iVar7;
    if (iVar7 < 1) {
      pppuStack_17c = (undefined1 ***)0x498f43;
      FUN_0048b0f0();
      *(undefined4 *)((int)param_1 + 0x1d2) = 2;
    }
    break;
  case 0x39:
    pppuStack_17c = (undefined1 ***)0x1;
    pppiStack_180 = (int ***)0x49908a;
    ExceptionList = &local_c;
    iVar7 = FUN_004883f0();
    if (((iVar7 == 2) || (iVar7 == 4)) || (iVar7 == 1)) {
      local_160 = (int *)param_1[2];
      if ((int)local_160 < 0) {
        do {
          local_160 = (int *)((int)local_160 + (int)DAT_006c5494);
        } while ((int)local_160 < 0);
      }
      else {
        for (; DAT_006c5494 <= local_160; local_160 = (int *)((int)local_160 - (int)DAT_006c5494)) {
        }
      }
      local_158 = (undefined1 ***)param_1[3];
      if ((int)local_158 < 0) {
        do {
          local_158 = (undefined1 ***)((int)local_158 + (int)DAT_006c5498);
        } while ((int)local_158 < 0);
      }
      else {
        for (; DAT_006c5498 <= local_158;
            local_158 = (undefined1 ***)((int)local_158 - (int)DAT_006c5498)) {
        }
      }
      local_15c = (short *)param_1[4];
      local_118 = 0;
      local_10f = 0;
      pppuStack_17c = (undefined1 ***)&local_15c;
      pppiStack_180 = (int ***)&local_158;
      local_120 = 4;
      local_11c = 1;
      local_114 = 3;
      local_110 = 1;
      local_10b = 0;
      FUN_004f5230(0xffffffff,&local_120,&local_160);
      local_164 = (undefined1 **)local_160;
      pppuStack_17c = (undefined1 ***)&local_13c;
      local_13c = local_15c;
      local_14c = local_158;
      pppiStack_180 = (int ***)0x4992fd;
      iVar7 = FUN_004694e0();
      if (iVar7 != 0) {
        pppuStack_17c = (undefined1 ***)local_13c;
        pppiStack_180 = (int ***)local_14c;
LAB_0049930f:
        FUN_00468ad0();
      }
    }
    else {
      if (iVar7 != 0) {
        pppiStack_180 = (int ***)(param_1 + 0x67);
        if (param_1[0x67] == 0) {
          pppuStack_17c = (undefined1 ***)0x1;
          FUN_00571310(0x2c6,param_1[2],param_1[3],param_1[4],0);
        }
        else {
          pppuStack_17c = (undefined1 ***)param_1[4];
          pppiVar14 = pppiStack_180;
          pppiStack_180 = (int ***)param_1[3];
          FUN_00571460(pppiVar14,param_1[2]);
        }
        pppuStack_17c = (undefined1 ***)0x499224;
        iVar7 = FUN_00464ec0();
        while (iVar7 == 1) {
          pppuStack_17c = (undefined1 ***)0x499233;
          FUN_00464d70();
          pppuStack_17c = (undefined1 ***)0x49923a;
          iVar7 = FUN_00464ec0();
        }
        break;
      }
      if (DAT_006b1254 < 0xb) {
        puVar11 = (undefined4 *)0x0;
      }
      else {
        puVar11 = (undefined4 *)(DAT_006b1250 + 0x140);
      }
      pppuStack_17c = (undefined1 ***)*puVar11;
      pppiStack_180 = (int ***)0x4990ce;
      FUN_0048ecf0();
      uVar18 = param_1[0x12];
      if (uVar18 != 0xffffffff) {
        if (uVar18 < DAT_006def5c) {
          iVar7 = uVar18 * 0xe4b + DAT_006def58;
        }
        else {
          iVar7 = 0;
        }
        local_154 = *(int **)(iVar7 + 8);
        if ((int)local_154 < 0) {
          do {
            local_154 = (int *)((int)local_154 + (int)DAT_006c5494);
          } while ((int)local_154 < 0);
        }
        else {
          for (; DAT_006c5494 <= local_154; local_154 = (int *)((int)local_154 - (int)DAT_006c5494))
          {
          }
        }
        local_150 = *(undefined1 ****)(iVar7 + 0xc);
        if ((int)local_150 < 0) {
          do {
            local_150 = (undefined1 ***)((int)local_150 + (int)DAT_006c5498);
          } while ((int)local_150 < 0);
        }
        else {
          for (; DAT_006c5498 <= local_150;
              local_150 = (undefined1 ***)((int)local_150 - (int)DAT_006c5498)) {
          }
        }
        local_168 = *(short **)(iVar7 + 0x10);
        pppuStack_17c = (undefined1 ***)&local_168;
        pppiStack_180 = (int ***)&local_150;
        local_138 = 4;
        local_134 = 1;
        local_130 = 0;
        local_12c = 3;
        local_128 = 1;
        local_127 = 0;
        local_123 = 0;
        FUN_004f5230(0xffffffff,&local_138,&local_154);
        local_144 = local_154;
        pppuStack_17c = (undefined1 ***)&local_140;
        local_148 = local_150;
        local_140 = local_168;
        pppiStack_180 = (int ***)0x4991b5;
        iVar7 = FUN_004694e0();
        if (iVar7 != 0) {
          pppuStack_17c = (undefined1 ***)local_140;
          pppiStack_180 = (int ***)local_148;
          goto LAB_0049930f;
        }
      }
    }
    pppuStack_17c = (undefined1 ***)0x499320;
    FUN_004943f0();
    break;
  case 0x3c:
    pppuStack_17c = (undefined1 ***)0x499332;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x49933d;
      FUN_00464d70();
      pppuStack_17c = (undefined1 ***)0x499344;
      iVar7 = FUN_00464ec0();
    }
    if (*(ushort *)(param_1[0x65] + 6) < DAT_006b124c) {
      iVar7 = DAT_006b1248 + (uint)*(ushort *)(param_1[0x65] + 6) * 0x7e;
    }
    else {
      iVar7 = 0;
    }
    if (*(int *)(iVar7 + 0x2a) == 0x32) {
      pppuStack_17c = (undefined1 ***)0x499383;
      FUN_004943f0();
    }
    break;
  case 0x3e:
  case 0x41:
    ExceptionList = &local_c;
    if (*(int *)((int)param_1 + 0x1d2) != -1) {
      ExceptionList = &local_c;
      *(int *)((int)param_1 + 0x1d2) = *(int *)((int)param_1 + 0x1d2) + -1;
      if (DAT_006905ac == 0) {
        pppuStack_17c = (undefined1 ***)param_1[0x16];
        pppiStack_180 = (int ***)0x4993b3;
        FUN_004dde90();
        pppuStack_17c = (undefined1 ***)0x4993bd;
        FUN_004de610();
        DAT_006905ac = 1;
        pppuStack_17c = (undefined1 ***)0x4993cd;
        FUN_004de5c0();
        pppuStack_17c = (undefined1 ***)((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x158);
        pppiStack_180 = (int ***)((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x1e0);
        FUN_004de5a0();
      }
      pppuStack_17c = *(undefined1 ****)((int)param_1 + 0x1d2);
      pppiStack_180 = (int ***)0x49940c;
      FUN_004de490();
    }
    if (*(int *)((int)param_1 + 0x1d2) != 0) {
      pppuStack_17c = (undefined1 ***)0x4995d5;
      FUN_004933d0();
      break;
    }
    DAT_006905ac = 0;
    pppuStack_17c = (undefined1 ***)0x0;
    pppiStack_180 = (int ***)0x0;
    FUN_00571310(0x412,param_1[2],param_1[3],param_1[4],0);
    pppuStack_17c = (undefined1 ***)param_1[0x65];
    pppiStack_180 = (int ***)0x499451;
    FUN_004f3010();
    DAT_006dbc26 = 1;
    pppuStack_17c = (undefined1 ***)0x499462;
    pppuStack_17c = (undefined1 ***)FUN_00474b40();
    pppiStack_180 = (int ***)0x49946d;
    FUN_00474b10();
    if ((uint)param_1[0x19] < DAT_0068949c) {
      iVar7 = DAT_00689498 + param_1[0x19] * 0x22e;
    }
    else {
      iVar7 = 0;
    }
    switch(*(undefined4 *)(iVar7 + 0x28)) {
    default:
      pppuStack_17c = (undefined1 ***)0x4994d7;
      FUN_004943f0();
      break;
    case 0x1a:
      pppuStack_17c = (undefined1 ***)0x1b;
      pppiStack_180 = (int ***)0x4994ce;
      FUN_00494ab0();
      break;
    case 0x27:
      pppuStack_17c = *(undefined1 ****)(iVar7 + 0x44);
      pppiStack_180 = (int ***)0x4994bd;
      FUN_004898a0();
    case 0xe:
    case 0x28:
    case 0x3a:
      *(undefined4 *)(iVar7 + 0x180) = 1;
    }
    piVar19 = (int *)param_1[0x67];
    if (piVar19 != (int *)0x0) {
      if (((*(int *)(DAT_006f437c + 0x1c) != 0) && (*(int *)(DAT_006f437c + 0x18) != 0)) &&
         (*piVar19 != 0)) {
        _DAT_006fa794 = 0;
        pppiStack_180 = (int ***)*piVar19;
        pppuStack_17c = (undefined1 ***)&DAT_006fa794;
        iVar7 = (*(code *)(*pppiStack_180)[9])();
        if ((iVar7 != 0) || ((_DAT_006fa794 & 1) != 0)) {
          pppuStack_17c = (undefined1 ***)*piVar19;
          pppiStack_180 = (int ***)0x499520;
          DAT_006fa798 = (**(code **)((int)*pppuStack_17c + 0x48))();
          piVar3 = *(int **)(DAT_006f437c + 0x260);
          piVar4 = (int *)piVar3[4];
          pppuStack_17c = (undefined1 ***)piVar3;
          while (piVar4 != piVar19) {
            pppuStack_17c = (undefined1 ***)*pppuStack_17c;
            if (pppuStack_17c == (undefined1 ***)piVar3) {
              pppuStack_17c = (undefined1 ***)0x0;
              break;
            }
            piVar4 = (int *)pppuStack_17c[4];
          }
          pppiStack_180 = (int ***)0x499552;
          FUN_00571d20();
          if (DAT_006fa798 == 0) {
            pppiStack_180 = (int ***)*piVar19;
            pppuStack_17c = (undefined1 ***)0x0;
            DAT_006fa798 = (*(code *)(*pppiStack_180)[0xd])();
          }
        }
      }
      param_1[0x67] = 0;
    }
    pppuStack_17c = (undefined1 ***)param_1[3];
    if ((int)pppuStack_17c < 0) {
      do {
        pppuStack_17c = (undefined1 ***)((int)pppuStack_17c + (int)DAT_006c5498);
      } while ((int)pppuStack_17c < 0);
    }
    else {
      for (; DAT_006c5498 <= pppuStack_17c;
          pppuStack_17c = (undefined1 ***)((int)pppuStack_17c - (int)DAT_006c5498)) {
      }
    }
    local_164 = (undefined1 **)&pppiStack_180;
    pppiStack_180 = DAT_006c5498;
    FUN_0040e290(param_1[2]);
    FUN_00556a00();
    goto switchD_00499e05_caseD_2;
  case 0x42:
  case 0x48:
  case 0x49:
  case 0x4c:
  case 0x4d:
    pppiStack_180 = (int ***)param_1[0x12];
    pppuStack_17c = (undefined1 ***)0x0;
    ExceptionList = &local_c;
    FUN_004896b0();
    if ((uint)param_1[0x12] < DAT_006def5c) {
      iVar7 = param_1[0x12] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    param_1[0x66] = iVar7;
    if ((*(int *)(iVar7 + 4) == 0) || (*(int *)(iVar7 + 0xe4) == 0)) goto LAB_00499bcf;
    if ((DAT_00644520 < 0) ||
       ((7 < DAT_00644520 || (*(int *)(iVar7 + 0x24 + DAT_00644520 * 4) == 0)))) {
      *(undefined4 *)((int)param_1 + 0x22a) = 0;
    }
    else {
      *(undefined4 *)((int)param_1 + 0x22a) = 1;
    }
    pppuStack_17c = (undefined1 ***)0x49996d;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x49997c;
      FUN_0048b0f0();
      pppuStack_17c = (undefined1 ***)0x499983;
      FUN_004943f0();
    }
    break;
  case 0x43:
    pppiStack_180 = (int ***)param_1[0x12];
    pppuStack_17c = (undefined1 ***)0x0;
    ExceptionList = &local_c;
    FUN_004896b0();
    pppuStack_17c = (undefined1 ***)0x499643;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x49964e;
      FUN_004943f0();
    }
    else if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x499666;
      iVar7 = FUN_0049cdb0();
      if (iVar7 != 0) {
        pppuStack_17c = (undefined1 ***)(param_1 + 0xb);
        pppiStack_180 = (int ***)0x44;
        FUN_00493f20();
      }
    }
    break;
  case 0x44:
    pppiStack_180 = (int ***)param_1[0x12];
    pppuStack_17c = (undefined1 ***)0xfffffffe;
    ExceptionList = &local_c;
    FUN_004896b0();
    pppuStack_17c = (undefined1 ***)0x499698;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x4996a3;
      FUN_004943f0();
    }
    else if (iVar7 == 2) {
      if ((uint)param_1[0x12] < DAT_006def5c) {
        iVar7 = param_1[0x12] * 0xe4b + DAT_006def58;
      }
      else {
        iVar7 = 0;
      }
      param_1[0x66] = iVar7;
      if ((iVar7 != 0) && (*(int *)(iVar7 + 0xe4) != 0)) {
        pppuStack_17c = (undefined1 ***)0x0;
        pppiStack_180 = (int ***)0x1;
        FUN_0050d060(*(uint *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) / *(uint *)(iVar7 + 0x84));
        pppuStack_17c = (undefined1 ***)0x0;
        pppiStack_180 = (int ***)0x0;
        FUN_00571310(0x2df,param_1[2],param_1[3],param_1[4],0);
      }
    }
    break;
  case 0x45:
    pppuStack_17c = (undefined1 ***)0x49973e;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x499749;
      FUN_004943f0();
    }
    else if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x49975e;
      FUN_00493a20();
    }
    break;
  case 0x47:
    pppuStack_17c = (undefined1 ***)0x49976a;
    ExceptionList = &local_c;
    iVar7 = FUN_00489080();
    if ((((iVar7 == 1) || (iVar7 == 0)) || (iVar7 == 2)) || (iVar7 == 4)) goto LAB_00499bcf;
    local_164 = (undefined1 **)0x0;
    if (0 < DAT_005e1460) {
      do {
        pppuStack_17c = (undefined1 ***)0x45;
        pppiStack_180 = (int ***)0x4997a7;
        local_14c = (undefined1 ***)FUN_00597850();
        local_4 = 5;
        if ((int ***)local_14c == (int ***)0x0) {
          puVar11 = (undefined4 *)0x0;
        }
        else {
          pppuStack_17c = (undefined1 ***)0x4997c9;
          uVar18 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)0x4997db;
          uVar9 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)0x4997ed;
          uVar10 = FUN_0054e200();
          pppuStack_17c = (undefined1 ***)(((uVar18 & 0x7fffffff) % 3 - 1) + param_1[7]);
          pppiStack_180 = (int ***)(((uVar9 & 0x7fffffff) % 3 - 1) + param_1[6]);
          puVar11 = (undefined4 *)FUN_00546660(((uVar10 & 0x7fffffff) % 3 - 1) + param_1[5]);
        }
        local_4 = 0xffffffff;
        if (puVar11 != (undefined4 *)0x0) {
          iVar7 = param_1[0x16];
          pppuStack_17c = *(undefined1 ****)(&DAT_005e1444 + iVar7 * 0xc);
          pppiStack_180 = *(int ****)(&DAT_005e1440 + iVar7 * 0xc);
          FUN_005467a0((&DAT_005e143c)[iVar7 * 3]);
          pppiStack_180 = (int ***)0x49986f;
          pppuStack_17c = (undefined1 ***)puVar11;
          iVar7 = FUN_005457e0();
          if (iVar7 == 0) {
            pppuStack_17c = (undefined1 ***)0x1;
            pppiStack_180 = (int ***)0x49987b;
            (**(code **)*puVar11)();
          }
        }
        local_164 = (undefined1 **)((int)local_164 + 1);
      } while ((int)local_164 < DAT_005e1460);
    }
    pppuStack_17c = (undefined1 ***)0x49989f;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x4998af;
      FUN_00464d70();
      pppuStack_17c = (undefined1 ***)0x4998b6;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 0x4a:
    pppiStack_180 = (int ***)param_1[0x12];
    pppuStack_17c = (undefined1 ***)0x0;
    ExceptionList = &local_c;
    FUN_004896b0();
    if ((uint)param_1[0x12] < DAT_006def5c) {
      iVar7 = param_1[0x12] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    param_1[0x66] = iVar7;
    if ((*(int *)(iVar7 + 4) == 0) || (*(int *)(iVar7 + 0xe4) == 0)) goto LAB_00499bcf;
    if ((DAT_00644520 < 0) ||
       ((7 < DAT_00644520 || (*(int *)(iVar7 + 0x24 + DAT_00644520 * 4) == 0)))) {
      *(undefined4 *)((int)param_1 + 0x22a) = 0;
    }
    else {
      *(undefined4 *)((int)param_1 + 0x22a) = 1;
    }
    if ((*(int *)((int)param_1 + 0x1da) != 0) &&
       ((*(int *)(iVar7 + 0x110) < 1 || (*(int *)(iVar7 + 0xe4) == 0)))) {
      param_1[0x60] = 1;
    }
    pppuStack_17c = (undefined1 ***)0x499abb;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x499ac6;
      FUN_004943f0();
    }
    else if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x499adb;
      FUN_0048b0f0();
    }
    break;
  case 0x4b:
    pppuStack_17c = (undefined1 ***)0x499b32;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x499b41;
      FUN_00464d70();
      pppuStack_17c = (undefined1 ***)0x499b48;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 0x4f:
    pppuStack_17c = (undefined1 ***)0x1;
    pppiStack_180 = (int ***)0x499ae8;
    ExceptionList = &local_c;
    iVar7 = FUN_004883f0();
    if (((iVar7 == 2) || (iVar7 == 4)) || (iVar7 == 1)) goto LAB_00499bcf;
    pppuStack_17c = (undefined1 ***)0x499b0b;
    iVar7 = FUN_0049cdb0();
    if (iVar7 != 0) {
      pppuStack_17c = (undefined1 ***)(param_1 + 0xb);
      pppiStack_180 = (int ***)0x13;
      FUN_00493f20();
    }
    break;
  case 0x50:
    pppiStack_180 = (int ***)param_1[0x12];
    pppuStack_17c = (undefined1 ***)0x0;
    ExceptionList = &local_c;
    FUN_004896b0();
    if ((uint)param_1[0x12] < DAT_006def5c) {
      iVar7 = param_1[0x12] * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    param_1[0x66] = iVar7;
    if ((*(int *)(iVar7 + 4) == 0) || (*(int *)(iVar7 + 0xe4) == 0)) goto LAB_00499bcf;
    if ((DAT_00644520 < 0) ||
       ((7 < DAT_00644520 || (*(int *)(iVar7 + 0x24 + DAT_00644520 * 4) == 0)))) {
      *(undefined4 *)((int)param_1 + 0x22a) = 0;
    }
    else {
      *(undefined4 *)((int)param_1 + 0x22a) = 1;
    }
    pppuStack_17c = (undefined1 ***)0x499a05;
    iVar7 = FUN_00464ec0();
    ppvVar6 = ExceptionList;
    if (iVar7 != 2) goto LAB_004983e9;
  case 0xb:
  case 0x20:
  case 0x21:
switchD_00497117_caseD_b:
    ExceptionList = ppvVar6;
    pppuStack_17c = (undefined1 ***)0x499a15;
    FUN_0048b0f0();
    break;
  case 0x51:
    pppuStack_17c = (undefined1 ***)0x499bcb;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 1) goto LAB_00499bcf;
    if (iVar7 != 2) break;
    pppuStack_17c = (undefined1 ***)0x499bee;
    iVar7 = FUN_0049cdb0();
    if (iVar7 == 0) break;
    iVar7 = param_1[0xb];
    iVar17 = param_1[0xc];
    pppuStack_17c = (undefined1 ***)(param_1 + 0xb);
    param_1[0x10] = param_1[0xd];
    param_1[0xe] = iVar7;
    param_1[0xf] = iVar17;
    switch(*(undefined4 *)((int)param_1 + 0x1a6)) {
    case 0:
      iVar7 = *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
      goto LAB_00499d35;
    case 1:
      piVar19 = (int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
      param_1[0xe] = *piVar19 * 0x20 + iVar7;
      goto LAB_00499d2e;
    case 2:
      param_1[0xe] = *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) * 0x20 + iVar7;
      break;
    case 3:
      param_1[0xe] = *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) * 0x20 + iVar7;
      param_1[0xf] = *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) * 0x20 + iVar17;
      break;
    case 4:
      param_1[0xf] = *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) * 0x20 + iVar17;
      break;
    case 5:
      param_1[0xe] = iVar7 + *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) * -0x20;
      param_1[0xf] = *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) * 0x20 + iVar17;
      break;
    case 6:
      param_1[0xe] = iVar7 + *(int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1) * -0x20;
      break;
    case 7:
      piVar19 = (int *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
      param_1[0xe] = iVar7 + *piVar19 * -0x20;
LAB_00499d2e:
      iVar7 = *piVar19;
LAB_00499d35:
      param_1[0xf] = iVar17 + iVar7 * -0x20;
    }
    pppiStack_180 = (int ***)0x4f;
    param_1[0x14] = 0x19;
    FUN_00493f20();
    pppuStack_17c = (undefined1 ***)0x499d50;
    FUN_0048b0f0();
    break;
  case 0x54:
    pppuStack_17c = (undefined1 ***)0x499b5c;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    if (iVar7 != 2) goto LAB_004983e9;
    pppuStack_17c = (undefined1 ***)0x499b93;
    FUN_004943f0();
    pppuStack_17c = (undefined1 ***)0x499b9d;
    iVar7 = FUN_0049cdb0();
    if (iVar7 != 0) {
      pppuStack_17c = (undefined1 ***)(param_1 + 0xb);
      pppiStack_180 = (int ***)0x51;
      param_1[0x10] = param_1[7];
      param_1[0xd] = param_1[7];
      FUN_00493f20();
    }
    break;
  case 0x55:
    pppuStack_17c = (undefined1 ***)0x499d5f;
    ExceptionList = &local_c;
    iVar7 = FUN_00464ec0();
    while (iVar7 == 1) {
      pppuStack_17c = (undefined1 ***)0x499d6a;
      FUN_004943f0();
      pppuStack_17c = (undefined1 ***)0x499d71;
      iVar7 = FUN_00464ec0();
    }
    break;
  case 0x58:
    pppuStack_17c = (undefined1 ***)param_1[0x11];
    pppiStack_180 = (int ***)0x4986d5;
    ExceptionList = &local_c;
    FUN_00489500();
    pppuStack_17c = (undefined1 ***)0x4986e0;
    iVar7 = FUN_00464ec0();
    if (iVar7 == 2) {
      pppuStack_17c = (undefined1 ***)0x4986f0;
      FUN_004943f0();
    }
  }
switchD_00497117_caseD_f:
  if ((*(int *)((int)param_1 + 0x222) != 0) && (param_1[1] != 0)) {
    pppuStack_17c = (undefined1 ***)0x499d95;
    uVar12 = FUN_00488bd0();
    iVar7 = DAT_006c5c80;
    if (DAT_006c5c80 + -2 <= param_1[4]) {
      uVar18 = *(uint *)((int)param_1 + 0x1f2) >> 4;
      if (((param_1[4] < DAT_006c5c80) && (DAT_006c5c80 <= (int)uVar18)) && (param_1[10] == 0xe)) {
        pppuStack_17c = (undefined1 ***)0x499dc5;
        FUN_004943f0();
      }
      if ((param_1[4] < iVar7 + -1) && (iVar7 + -1 <= (int)uVar18)) {
        pppuStack_17c = (undefined1 ***)0x0;
        pppiStack_180 = (int ***)0x0;
        FUN_00571310(0x417,param_1[2],param_1[3],param_1[4],1);
      }
    }
    if (param_1[1] != 0) {
      switch(uVar12) {
      case 0:
        pppuStack_17c = (undefined1 ***)0x499e1f;
        FUN_0049a240();
        break;
      case 1:
        pppuStack_17c = (undefined1 ***)0x499e13;
        FUN_0049a0b0();
        break;
      case 2:
      case 4:
switchD_00499e05_caseD_2:
        pppuStack_17c = (undefined1 ***)0x4995b5;
        FUN_004943f0();
      }
    }
  }
  ExceptionList = local_c;
  return;
}

