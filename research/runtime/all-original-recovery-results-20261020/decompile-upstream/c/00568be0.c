
void __thiscall FUN_00568be0(int *param_1,int *param_2,int param_3)

{
  byte bVar1;
  char cVar2;
  bool bVar3;
  int iVar4;
  undefined4 *puVar5;
  uint uVar6;
  undefined4 uVar7;
  int *piVar8;
  byte *pbVar9;
  int iVar10;
  byte *pbVar11;
  int iVar12;
  byte *pbVar13;
  bool bVar14;
  int local_108 [63];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2b7c;
  pvStack_c = ExceptionList;
  piVar8 = local_108;
  for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
    *piVar8 = 0;
    piVar8 = piVar8 + 1;
  }
  local_108[5] = -1;
  local_108[4] = -1;
  local_108[3] = -1;
  local_108[2] = -1;
  local_108[1] = -1;
  local_108[0] = -1;
  local_108[9] = 0xffffffff;
  local_108[8] = 0xffffffff;
  local_108[7] = 0xffffffff;
  local_108[6] = -1;
  local_108[0xb] = 0xffffffff;
  local_108[0xc] = 7;
  local_108[10] = 1;
  if ((param_3 < param_1[1]) && (-1 < param_3)) {
    iVar12 = *param_1 + param_3 * 0x47c;
  }
  else {
    iVar12 = 0;
  }
  ExceptionList = &pvStack_c;
  if (*(int *)(iVar12 + 0x364) != 0) {
    ExceptionList = &pvStack_c;
    if (*(int *)(iVar12 + 0x364) != 1) {
      ExceptionList = &pvStack_c;
      FUN_00514e40();
    }
    FUN_00514e40();
  }
  iVar4 = *(int *)(iVar12 + 0x368);
  if (iVar4 != 0) {
    if (iVar4 == 1) {
LAB_00568dc6:
      FUN_0051ff00();
    }
    else if (iVar4 == 2) {
      iVar4 = FUN_0049cdb0();
      if (iVar4 != 0) {
        local_108[6] = *param_2;
        iVar4 = param_2[0x2b];
        local_108[8] = 0x4e;
        local_108[0xc] = 1;
        local_108[7] = 0xffffffff;
        local_108[0] = param_2[5] + *(int *)(iVar4 + 0x198);
        local_108[1] = param_2[6] + *(int *)(iVar4 + 0x198);
        local_108[2] = param_2[7] + *(int *)(iVar4 + 0x19c);
        local_108[3] = local_108[0];
        local_108[4] = local_108[1];
        local_108[5] = local_108[2];
        FUN_00493f20();
        FUN_00494ab0();
        iVar4 = FUN_0051ff00();
        if (iVar4 == 0) {
          FUN_004943f0();
        }
      }
    }
    else if (iVar4 == 3) goto LAB_00568dc6;
  }
  if (*(int *)(iVar12 + 0x36c) != 0) {
    FUN_005183e0();
  }
  if ((*(int *)(iVar12 + 0x370) != 0) && (param_2[0x3e] = 1, param_2[0x26] == 0)) {
    FUN_0054dcb0();
  }
  if (*(int *)(iVar12 + 0x374) != 0) {
    FUN_00518080();
  }
  if (*(int *)(iVar12 + 0x378) != 0) {
    FUN_00518100();
  }
  if (*(int *)(iVar12 + 0x37c) != 0) {
    FUN_005180c0();
  }
  if (*(int *)(iVar12 + 0x380) != 0) {
    FUN_0051fb10();
  }
  if ((*(int *)(iVar12 + 900) != 0) && (FUN_00520c70(), 0 < DAT_005e14ac)) {
    iVar4 = FUN_00597850();
    local_4 = 0;
    if (iVar4 == 0) {
      puVar5 = (undefined4 *)0x0;
    }
    else {
      puVar5 = (undefined4 *)FUN_00547210();
    }
    local_4 = 0xffffffff;
    if (puVar5 != (undefined4 *)0x0) {
      uVar6 = *(uint *)(param_2[0x2b] + 8);
      puVar5[0x11] = param_2;
      puVar5[0x12] = 1;
      puVar5[0x13] = 1;
      puVar5[0x14] = 1;
      puVar5[4] = (int)(DAT_005e14ac * ((uVar6 & 0x7ffffff) * 0x10 + -4)) / 200;
    }
    iVar4 = FUN_005457e0();
    if ((iVar4 == 0) && (puVar5 != (undefined4 *)0x0)) {
      (**(code **)*puVar5)();
    }
  }
  if (*(int *)(iVar12 + 0x388) != 0) {
    FUN_0050d060();
  }
  if (*(int *)(iVar12 + 0x38c) != 0) {
    FUN_00520fe0();
    FUN_0051ef60();
    if (0 < DAT_005e1490) {
      iVar4 = FUN_00597850();
      local_4 = 1;
      if (iVar4 == 0) {
        puVar5 = (undefined4 *)0x0;
      }
      else {
        puVar5 = (undefined4 *)FUN_005472c0();
      }
      local_4 = 0xffffffff;
      if (puVar5 != (undefined4 *)0x0) {
        uVar6 = *(uint *)(param_2[0x2b] + 8);
        puVar5[0x11] = param_2;
        puVar5[0x12] = 1;
        puVar5[0x13] = 1;
        puVar5[0x14] = 1;
        puVar5[4] = (int)(((uVar6 & 0x7ffffff) * 0x10 + -4) * DAT_005e1490) / 200;
      }
      iVar4 = FUN_005457e0();
      if ((iVar4 == 0) && (puVar5 != (undefined4 *)0x0)) {
        (**(code **)*puVar5)();
      }
    }
  }
  if (-1 < *(int *)(iVar12 + 0x394)) {
    FUN_00507f00();
    FUN_00507f00();
  }
  if (*(int *)(iVar12 + 0x398) != 0) {
    FUN_00522f30();
  }
  if (*(int *)(iVar12 + 0x39c) != 0) {
    *(int *)((int)param_2 + 0x8cd) = *(int *)(iVar12 + 0x39c);
  }
  piVar8 = (int *)(iVar12 + 0x3a0);
  iVar4 = 3;
  do {
    if (*piVar8 != -1) {
      FUN_005097b0();
    }
    piVar8 = piVar8 + 1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  if (*(int *)(iVar12 + 0x3ac) != 0) {
    param_2[0x1c1] = 1;
  }
  iVar4 = FUN_0051e790();
  if ((iVar4 != 0) || (param_2[0x1c1] != 0)) {
    uVar6 = param_2[2];
    if ((int)uVar6 < 0) {
      do {
        uVar6 = uVar6 + DAT_006c5494;
      } while ((int)uVar6 < 0);
    }
    else {
      for (; DAT_006c5494 <= uVar6; uVar6 = uVar6 - DAT_006c5494) {
      }
    }
    *(uint *)((int)param_2 + 0xbcf) = uVar6;
    uVar6 = param_2[3];
    if ((int)uVar6 < 0) {
      do {
        uVar6 = uVar6 + DAT_006c5498;
      } while ((int)uVar6 < 0);
    }
    else {
      for (; DAT_006c5498 <= uVar6; uVar6 = uVar6 - DAT_006c5498) {
      }
    }
    *(uint *)((int)param_2 + 0xbd3) = uVar6;
    *(int *)((int)param_2 + 0xbd7) = param_2[4];
    *(undefined4 *)((int)param_2 + 0xbdb) = 0;
    *(undefined4 *)((int)param_2 + 0xbdf) = 0;
    *(undefined4 *)((int)param_2 + 0xbe3) = *(undefined4 *)(&DAT_006a6501 + param_2[0x2a] * 0x5c9);
    *(undefined4 *)((int)param_2 + 0xbe7) = *(undefined4 *)(&DAT_006a6505 + param_2[0x2a] * 0x5c9);
    *(undefined4 *)((int)param_2 + 0xbeb) = 0;
    *(undefined4 *)((int)param_2 + 0xbef) = 0;
    if (param_2[0x1c1] != 0) {
      *(undefined4 *)((int)param_2 + 0xbef) = 5;
    }
    if (*(int *)(iVar12 + 0x3b0) != 0) {
      *(int *)((int)param_2 + 0xbdb) = *(int *)(iVar12 + 0x3b0);
    }
    if (*(int *)(iVar12 + 0x3b4) != 0) {
      *(int *)((int)param_2 + 0xbdf) = *(int *)(iVar12 + 0x3b4);
    }
    if (*(int *)(iVar12 + 0x3b8) != 0) {
      *(int *)((int)param_2 + 0xbe3) = *(int *)(iVar12 + 0x3b8);
    }
    if (*(int *)(iVar12 + 0x3bc) != 0) {
      *(int *)((int)param_2 + 0xbe7) = *(int *)(iVar12 + 0x3bc);
    }
    if (*(int *)(iVar12 + 0x3c0) != 0) {
      *(int *)((int)param_2 + 0xbef) = *(int *)(iVar12 + 0x3c0);
    }
  }
  if (*(int *)(iVar12 + 0x3c4) != 0) {
    *(undefined4 *)((int)param_2 + 0x95a) = 1;
  }
  if (*(int *)((int)param_2 + 0xde7) == -1) {
    FUN_00441560();
    FUN_0040eeb0();
    FUN_0040e8a0();
    FUN_0040e290(param_2[2]);
    uVar7 = FUN_0053d7d0();
    *(undefined4 *)((int)param_2 + 0xde7) = uVar7;
  }
  if (-1 < *(int *)(iVar12 + 0x390)) {
    iVar4 = FUN_005075f0();
    param_2[0x39] = (iVar4 * *(int *)(iVar12 + 0x390)) / 100;
    FUN_0058f830();
    FUN_0058f890();
    piVar8 = (int *)FUN_00466870();
    if (param_2 == piVar8) {
      FUN_004d3a30();
    }
    if (param_2[0x39] == 0) {
      FUN_0050cdc0();
      goto LAB_0056956b;
    }
  }
  bVar3 = false;
  if (param_2[0x5d] == -1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = &DAT_00643a80 + param_2[0x5d] * 0x55;
  }
  if (((puVar5[3] != -1) || (*(int *)((int)param_2 + 0x8e5) != 0)) && (puVar5[1] == 0)) {
    bVar3 = true;
  }
  iVar4 = 0;
  if (0 < DAT_006f37fc) {
    pbVar11 = (byte *)(DAT_006f37f8 + 1);
    piVar8 = DAT_006f37f8;
    do {
      if (*piVar8 != 0) {
        pbVar13 = (byte *)(iVar12 + 0x2cc);
        pbVar9 = pbVar11;
        do {
          bVar1 = *pbVar9;
          bVar14 = bVar1 < *pbVar13;
          if (bVar1 != *pbVar13) {
LAB_005693de:
            iVar10 = (1 - (uint)bVar14) - (uint)(bVar14 != 0);
            goto LAB_005693e3;
          }
          if (bVar1 == 0) break;
          bVar1 = pbVar9[1];
          bVar14 = bVar1 < pbVar13[1];
          if (bVar1 != pbVar13[1]) goto LAB_005693de;
          pbVar9 = pbVar9 + 2;
          pbVar13 = pbVar13 + 2;
        } while (bVar1 != 0);
        iVar10 = 0;
LAB_005693e3:
        if (iVar10 == 0) {
          if ((iVar4 < DAT_006f37fc) && (piVar8 != (int *)0x0)) {
            if (piVar8[10] == -1) {
              piVar8[10] = *param_2;
              piVar8[9] = 1;
              piVar8[0xb] = param_2[2];
              piVar8[0xc] = param_2[3];
              piVar8[0xd] = param_2[4];
              piVar8[0xe] = *(int *)(iVar12 + 0x2ec);
              piVar8[0xf] = *(int *)(iVar12 + 0x3c8);
              FUN_00564a40();
            }
            else if (piVar8[9] == 2) {
              piVar8[10] = *param_2;
              piVar8[9] = 1;
              piVar8[0xb] = param_2[2];
              piVar8[0xc] = param_2[3];
              piVar8[0xd] = param_2[4];
              piVar8[0xe] = *(int *)(iVar12 + 0x2ec);
              FUN_00564a40();
              piVar8[0xf] = *(int *)(iVar12 + 0x3c8);
            }
            if ((((DAT_00644520 != -1) && (piVar8[0xf] != 0)) &&
                (((cVar2 = *(char *)(DAT_00644520 + 0x18 + (int)puVar5), cVar2 != '\0' &&
                  ((cVar2 != '\0' && (*(char *)(DAT_00644520 + 0x20 + (int)puVar5) != '\0')))) ||
                 (param_2[0x5d] == DAT_00644520)))) && (piVar8[10] != -1)) {
              FUN_00463cb0();
              FUN_00463eb0();
            }
          }
          break;
        }
      }
      iVar4 = iVar4 + 1;
      piVar8 = piVar8 + 0x10;
      pbVar11 = pbVar11 + 0x40;
    } while (iVar4 < DAT_006f37fc);
  }
  if ((!bVar3) && (*(int *)((int)param_2 + 0x8e5) != 0)) {
    FUN_00416590();
    iVar4 = *(int *)param_2[0x2b];
    if (((iVar4 == 0) || (((iVar4 == 0x18 || (iVar4 == 0x19)) || (iVar4 == 0x1a)))) &&
       (param_2[0x1f] == 0)) {
      FUN_00416590();
    }
  }
LAB_0056956b:
  if ((*(int *)(iVar12 + 0x2c) != -1) && (*(int *)(iVar12 + 0x30) == 0)) {
    FUN_00571310();
  }
  FUN_00564d60();
  ExceptionList = pvStack_c;
  return;
}

