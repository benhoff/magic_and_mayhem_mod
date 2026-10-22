
undefined2 __fastcall FUN_0057e1b0(int param_1,int param_2,uint param_3,uint param_4,int *param_5)

{
  byte bVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined2 *puVar4;
  byte *pbVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  byte *pbVar12;
  undefined2 uStack_10;
  uint uStack_8;
  
  uVar9 = *(uint *)(param_1 + 8);
  uStack_10 = 0;
  if ((uVar9 == 0) && (*(int *)(param_1 + 4) == 0)) {
    return 0;
  }
  puVar2 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (int *)0x0; param_5 = (int *)((int)param_5 + -1)) {
    puVar2 = (undefined4 *)*puVar2;
  }
  iVar6 = (int)param_4 >> ((byte)puVar2[1] & 0x1f);
  if ((iVar6 < 0) && (puVar2[1] != 0)) {
    iVar6 = iVar6 + 1;
  }
  iVar6 = (puVar2[2] + iVar6) * 0x200 + 0xc;
  uVar10 = *(uint *)(param_1 + 4);
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  iVar3 = param_3 - *(int *)(param_1 + 0x10);
  if ((int)(uVar10 + param_2) <= DAT_006e0008) {
    return 0;
  }
  if ((int)(uVar9 + iVar3) <= DAT_006cbb6c) {
    return 0;
  }
  if (DAT_006a49b8 <= param_2) {
    return 0;
  }
  if (DAT_00656618 <= iVar3) {
    return 0;
  }
  uVar7 = 0;
  if (iVar3 < DAT_006cbb6c) {
    uVar7 = DAT_006cbb6c - iVar3;
  }
  param_4 = uVar9;
  if (DAT_00656618 < (int)(uVar9 + iVar3)) {
    param_4 = DAT_00656618 - iVar3;
  }
  if (uVar9 <= uVar7) {
    uStack_10 = 4;
  }
  if (uVar9 < param_4) {
    uStack_10 = 5;
  }
  iVar11 = DAT_006a2dc8 - uVar10;
  puVar4 = (undefined2 *)(DAT_00658174 + ((uVar7 + iVar3) * DAT_006a2dc8 + param_2) * 2);
  param_5 = (int *)(param_1 + 0x28 + uVar7 * 8);
  uStack_8 = 0;
  if (param_2 < DAT_006e0008) {
    uStack_8 = DAT_006e0008 - param_2;
  }
  if (DAT_006a49b8 < (int)(uVar10 + param_2)) {
    param_3 = DAT_006a49b8 - param_2;
  }
  else {
    param_3 = uVar10;
    if (param_2 >= DAT_006e0008) {
      if (param_4 <= uVar7) {
        return uStack_10;
      }
      param_4 = param_4 - uVar7;
      do {
        uVar9 = 0;
        pbVar12 = (byte *)(param_1 + *param_5);
        pbVar5 = (byte *)(param_5[1] + param_1);
        if (uVar10 != 0) {
          do {
            uVar9 = uVar9 + *pbVar12;
            puVar4 = puVar4 + *pbVar12;
            if (uVar10 <= uVar9) break;
            uVar10 = (uint)pbVar12[1];
            pbVar12 = pbVar12 + 2;
            uVar9 = uVar9 + uVar10;
            for (; uVar10 != 0; uVar10 = uVar10 - 1) {
              bVar1 = *pbVar5;
              pbVar5 = pbVar5 + 1;
              *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar6);
              puVar4 = puVar4 + 1;
            }
            uVar10 = *(uint *)(param_1 + 4);
          } while (uVar9 < uVar10);
        }
        param_5 = param_5 + 2;
        puVar4 = puVar4 + iVar11;
        param_4 = param_4 + -1;
        if (param_4 == 0) {
          return uStack_10;
        }
      } while( true );
    }
  }
  if (DAT_006e1f68 != 0) {
    return uStack_10;
  }
  if (uVar7 < param_4) {
    param_4 = param_4 - uVar7;
    do {
      pbVar5 = (byte *)(param_5[1] + param_1);
      pbVar12 = (byte *)(param_1 + *param_5);
      uVar9 = 0;
      if (uVar10 != 0) {
        do {
          uVar9 = uVar9 + *pbVar12;
          puVar4 = puVar4 + *pbVar12;
          if (uVar10 <= uVar9) break;
          uVar10 = (uint)pbVar12[1];
          pbVar12 = pbVar12 + 2;
          if (uVar9 < uStack_8) {
            uVar7 = uStack_8 - uVar9;
            if (uVar10 < uVar7) {
              pbVar5 = pbVar5 + uVar10;
              puVar4 = puVar4 + uVar10;
            }
            else {
              pbVar5 = pbVar5 + uVar7;
              iVar3 = uVar10 - uVar7;
              puVar4 = puVar4 + uVar7;
              if (param_3 < uVar9 + uVar10) {
                iVar8 = (iVar3 - (uVar9 + uVar10)) + param_3;
                uVar7 = iVar3 - iVar8;
                for (; iVar8 != 0; iVar8 = iVar8 + -1) {
                  bVar1 = *pbVar5;
                  pbVar5 = pbVar5 + 1;
                  *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar6);
                  puVar4 = puVar4 + 1;
                }
                goto LAB_0057e4d0;
              }
              for (; iVar3 != 0; iVar3 = iVar3 + -1) {
                bVar1 = *pbVar5;
                pbVar5 = pbVar5 + 1;
                *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar6);
                puVar4 = puVar4 + 1;
              }
            }
          }
          else {
            uVar7 = uVar10;
            if (uVar9 < param_3) {
              if (uVar9 + uVar10 < param_3) {
                for (; uVar7 != 0; uVar7 = uVar7 - 1) {
                  bVar1 = *pbVar5;
                  pbVar5 = pbVar5 + 1;
                  *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar6);
                  puVar4 = puVar4 + 1;
                }
                goto LAB_0057e4d5;
              }
              iVar3 = (uVar10 - (uVar9 + uVar10)) + param_3;
              uVar7 = uVar10 - iVar3;
              for (; iVar3 != 0; iVar3 = iVar3 + -1) {
                bVar1 = *pbVar5;
                pbVar5 = pbVar5 + 1;
                *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar6);
                puVar4 = puVar4 + 1;
              }
            }
LAB_0057e4d0:
            puVar4 = puVar4 + uVar7;
            pbVar5 = pbVar5 + uVar7;
          }
LAB_0057e4d5:
          uVar9 = uVar9 + uVar10;
          uVar10 = *(uint *)(param_1 + 4);
        } while (uVar9 < uVar10);
      }
      uVar10 = *(uint *)(param_1 + 4);
      if (uVar9 != uVar10) {
        uStack_10 = 3;
      }
      param_5 = param_5 + 2;
      puVar4 = puVar4 + iVar11;
      param_4 = param_4 + -1;
    } while (param_4 != 0);
  }
  return uStack_10;
}

