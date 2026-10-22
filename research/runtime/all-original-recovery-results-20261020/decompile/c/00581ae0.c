
uint FUN_00581ae0(int param_1,uint param_2,uint param_3,int *param_4,byte *param_5)

{
  undefined4 *puVar1;
  undefined2 uVar2;
  int iVar3;
  short *psVar4;
  int iVar5;
  byte *pbVar6;
  uint uVar7;
  uint uVar8;
  byte *pbVar9;
  uint uVar10;
  int iVar11;
  int iVar12;
  uint uStack_10;
  int iStack_4;
  
  uStack_10 = 0;
  uVar8 = *(uint *)(param_1 + 8);
  if ((uVar8 == 0) && (*(int *)(param_1 + 4) == 0)) {
LAB_00581eb5:
    return uVar8 & 0xffff0000;
  }
  puVar1 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (byte *)0x0; param_5 = (byte *)((int)param_5 + -1)) {
    puVar1 = (undefined4 *)*puVar1;
  }
  iVar11 = (int)param_4 >> ((byte)puVar1[1] & 0x1f);
  if ((iVar11 < 0) && (puVar1[1] != 0)) {
    iVar11 = iVar11 + 1;
  }
  iVar11 = (puVar1[2] + iVar11) * 0x200 + 0xc;
  uVar10 = *(uint *)(param_1 + 4);
  iVar12 = param_2 - *(int *)(param_1 + 0xc);
  iVar3 = param_3 - *(int *)(param_1 + 0x10);
  if (((((int)(uVar10 + iVar12) <= DAT_006e0008) || ((int)(uVar8 + iVar3) <= DAT_006cbb6c)) ||
      (DAT_006a49b8 <= iVar12)) || (DAT_00656618 <= iVar3)) goto LAB_00581eb5;
  uVar7 = 0;
  if (iVar3 < DAT_006cbb6c) {
    uVar7 = DAT_006cbb6c - iVar3;
  }
  param_5 = (byte *)uVar8;
  if (DAT_00656618 < (int)(uVar8 + iVar3)) {
    param_5 = (byte *)(DAT_00656618 - iVar3);
  }
  if (uVar8 <= uVar7) {
    uStack_10 = 4;
  }
  if (uVar8 < param_5) {
    uStack_10 = 5;
  }
  iVar5 = DAT_006a2dc8 - uVar10;
  psVar4 = (short *)(DAT_00658174 + ((uVar7 + iVar3) * DAT_006a2dc8 + iVar12) * 2);
  param_4 = (int *)(param_1 + 0x28 + uVar7 * 8);
  param_3 = 0;
  if (iVar12 < DAT_006e0008) {
    param_3 = DAT_006e0008 - iVar12;
  }
  if (DAT_006a49b8 < (int)(uVar10 + iVar12)) {
    param_2 = DAT_006a49b8 - iVar12;
  }
  else {
    param_2 = uVar10;
    if (iVar12 >= DAT_006e0008) {
      uVar2 = (undefined2)((uint)DAT_006a49b8 >> 0x10);
      if (uVar7 < param_5) {
        param_5 = (byte *)((int)param_5 - uVar7);
        do {
          uVar8 = 0;
          pbVar9 = (byte *)(param_1 + *param_4);
          pbVar6 = (byte *)(param_4[1] + param_1);
          if (uVar10 != 0) {
            do {
              uVar8 = uVar8 + *pbVar9;
              psVar4 = psVar4 + *pbVar9;
              if (uVar10 <= uVar8) break;
              uVar10 = (uint)pbVar9[1];
              pbVar9 = pbVar9 + 2;
              uVar8 = uVar8 + uVar10;
              for (; uVar10 != 0; uVar10 = uVar10 - 1) {
                if (*psVar4 == 0) {
                  *psVar4 = *(short *)((int)puVar1 + (uint)*pbVar6 * 2 + iVar11);
                }
                pbVar6 = pbVar6 + 1;
                psVar4 = psVar4 + 1;
              }
              uVar10 = *(uint *)(param_1 + 4);
            } while (uVar8 < uVar10);
          }
          psVar4 = psVar4 + iVar5;
          param_4 = param_4 + 2;
          param_5 = (byte *)((int)param_5 + -1);
          if (param_5 == (byte *)0x0) {
            return uStack_10;
          }
        } while( true );
      }
      goto LAB_00581ea8;
    }
  }
  if (DAT_006e1f68 != 0) {
    return CONCAT22((short)((uint)DAT_006e1f68 >> 0x10),(undefined2)uStack_10);
  }
  uVar2 = 0;
  if (uVar7 < param_5) {
    iStack_4 = (int)param_5 - uVar7;
    do {
      param_5 = (byte *)(param_1 + *param_4);
      pbVar6 = (byte *)(param_4[1] + param_1);
      uVar8 = 0;
      if (uVar10 != 0) {
        do {
          uVar8 = uVar8 + *param_5;
          pbVar9 = param_5 + 1;
          psVar4 = psVar4 + *param_5;
          if (uVar10 <= uVar8) break;
          param_5 = param_5 + 2;
          uVar10 = (uint)*pbVar9;
          if (uVar8 < param_3) {
            uVar7 = param_3 - uVar8;
            if (uVar10 < uVar7) {
              pbVar6 = pbVar6 + uVar10;
              psVar4 = psVar4 + uVar10;
            }
            else {
              pbVar6 = pbVar6 + uVar7;
              iVar3 = uVar10 - uVar7;
              psVar4 = psVar4 + uVar7;
              if (param_2 < uVar8 + uVar10) {
                iVar12 = (iVar3 - (uVar8 + uVar10)) + param_2;
                uVar7 = iVar3 - iVar12;
                for (; iVar12 != 0; iVar12 = iVar12 + -1) {
                  if (*psVar4 == 0) {
                    *psVar4 = *(short *)((int)puVar1 + (uint)*pbVar6 * 2 + iVar11);
                  }
                  pbVar6 = pbVar6 + 1;
                  psVar4 = psVar4 + 1;
                }
                goto LAB_00581e60;
              }
              for (; iVar3 != 0; iVar3 = iVar3 + -1) {
                if (*psVar4 == 0) {
                  *psVar4 = *(short *)((int)puVar1 + (uint)*pbVar6 * 2 + iVar11);
                }
                pbVar6 = pbVar6 + 1;
                psVar4 = psVar4 + 1;
              }
            }
          }
          else {
            uVar7 = uVar10;
            if (uVar8 < param_2) {
              if (uVar8 + uVar10 < param_2) {
                for (; uVar7 != 0; uVar7 = uVar7 - 1) {
                  if (*psVar4 == 0) {
                    *psVar4 = *(short *)((int)puVar1 + (uint)*pbVar6 * 2 + iVar11);
                  }
                  pbVar6 = pbVar6 + 1;
                  psVar4 = psVar4 + 1;
                }
                goto LAB_00581e65;
              }
              iVar3 = (uVar10 - (uVar8 + uVar10)) + param_2;
              uVar7 = uVar10 - iVar3;
              for (; iVar3 != 0; iVar3 = iVar3 + -1) {
                if (*psVar4 == 0) {
                  *psVar4 = *(short *)((int)puVar1 + (uint)*pbVar6 * 2 + iVar11);
                }
                pbVar6 = pbVar6 + 1;
                psVar4 = psVar4 + 1;
              }
            }
LAB_00581e60:
            psVar4 = psVar4 + uVar7;
            pbVar6 = pbVar6 + uVar7;
          }
LAB_00581e65:
          uVar8 = uVar8 + uVar10;
          uVar10 = *(uint *)(param_1 + 4);
        } while (uVar8 < uVar10);
      }
      uVar10 = *(uint *)(param_1 + 4);
      if (uVar8 != uVar10) {
        uStack_10 = 3;
      }
      param_4 = param_4 + 2;
      psVar4 = psVar4 + iVar5;
      iStack_4 = iStack_4 + -1;
      uVar2 = 0;
    } while (iStack_4 != 0);
  }
LAB_00581ea8:
  return CONCAT22(uVar2,(undefined2)uStack_10);
}

