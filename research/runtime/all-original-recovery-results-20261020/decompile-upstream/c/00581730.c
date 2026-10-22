
uint FUN_00581730(int param_1,uint param_2,uint param_3,uint param_4,int *param_5)

{
  byte bVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined4 *puVar4;
  short *psVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  byte *pbVar9;
  int iVar10;
  int iVar11;
  byte *pbVar12;
  undefined2 uStack_c;
  
  uStack_c = 0;
  if (DAT_006f60cc == 0) {
    DAT_006f60cc = (-(ushort)(DAT_006e1f88 != 0) & 0xe800) + 0x30c6;
  }
  uVar3 = *(uint *)(param_1 + 8);
  if ((uVar3 == 0) && (uVar7 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_00581acb:
    return uVar7 & 0xffff0000;
  }
  puVar2 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (int *)0x0; param_5 = (int *)((int)param_5 + -1)) {
    puVar2 = (undefined4 *)*puVar2;
  }
  iVar10 = (int)param_4 >> ((byte)puVar2[1] & 0x1f);
  if ((iVar10 < 0) && (puVar2[1] != 0)) {
    iVar10 = iVar10 + 1;
  }
  puVar2 = puVar2 + (puVar2[2] + iVar10) * 0x80 + 3;
  uVar8 = *(uint *)(param_1 + 4);
  iVar11 = param_2 - *(int *)(param_1 + 0xc);
  iVar10 = param_3 - *(int *)(param_1 + 0x10);
  uVar7 = uVar8 + iVar11;
  if (((((int)(uVar8 + iVar11) <= DAT_006e0008) ||
       (uVar7 = DAT_006cbb6c, (int)(uVar3 + iVar10) <= (int)DAT_006cbb6c)) ||
      ((int)DAT_006a49b8 <= iVar11)) || (DAT_00656618 <= iVar10)) goto LAB_00581acb;
  uVar7 = 0;
  if (iVar10 < (int)DAT_006cbb6c) {
    uVar7 = DAT_006cbb6c - iVar10;
  }
  param_4 = uVar3;
  if (DAT_00656618 < (int)(uVar3 + iVar10)) {
    param_4 = DAT_00656618 - iVar10;
  }
  if (uVar3 <= uVar7) {
    uStack_c = 4;
  }
  if (uVar3 < param_4) {
    uStack_c = 5;
  }
  iVar6 = DAT_006a2dc8 - uVar8;
  psVar5 = (short *)(DAT_00658174 + ((iVar10 + uVar7) * DAT_006a2dc8 + iVar11) * 2);
  param_5 = (int *)(param_1 + 0x28 + uVar7 * 8);
  param_3 = 0;
  if (iVar11 < DAT_006e0008) {
    param_3 = DAT_006e0008 - iVar11;
  }
  if ((int)DAT_006a49b8 < (int)(uVar8 + iVar11)) {
    param_2 = (int)DAT_006a49b8 - iVar11;
  }
  else {
    param_2 = uVar8;
    if (iVar11 >= DAT_006e0008) {
      puVar4 = DAT_006a49b8;
      if (uVar7 < param_4) {
        iVar10 = param_4 - uVar7;
        do {
          pbVar12 = (byte *)(param_1 + *param_5);
          uVar3 = 0;
          if (uVar8 != 0) {
            do {
              uVar3 = uVar3 + *pbVar12;
              psVar5 = psVar5 + *pbVar12;
              if (uVar8 <= uVar3) break;
              uVar7 = (uint)pbVar12[1];
              pbVar12 = pbVar12 + 2;
              uVar3 = uVar3 + uVar7;
              for (; uVar7 != 0; uVar7 = uVar7 - 1) {
                if (*psVar5 != 0) {
                  DAT_006f60b0 = DAT_006f60b0 + 1;
                }
                *psVar5 = *psVar5 + DAT_006f60cc;
                psVar5 = psVar5 + 1;
              }
              uVar8 = *(uint *)(param_1 + 4);
            } while (uVar3 < uVar8);
          }
          param_5 = param_5 + 2;
          iVar10 = iVar10 + -1;
          psVar5 = psVar5 + iVar6;
          if (iVar10 == 0) {
            return CONCAT22((short)((uint)iVar6 >> 0x10),uStack_c);
          }
        } while( true );
      }
      goto LAB_00581abe;
    }
  }
  if (DAT_006e1f68 != 0) {
    return CONCAT22((short)((uint)DAT_006e1f68 >> 0x10),uStack_c);
  }
  puVar4 = (undefined4 *)0x0;
  if (uVar7 < param_4) {
    param_4 = param_4 - uVar7;
    do {
      pbVar12 = (byte *)(param_5[1] + param_1);
      pbVar9 = (byte *)(param_1 + *param_5);
      uVar3 = 0;
      if (uVar8 != 0) {
        do {
          uVar3 = uVar3 + *pbVar9;
          psVar5 = psVar5 + *pbVar9;
          if (uVar8 <= uVar3) break;
          uVar7 = (uint)pbVar9[1];
          pbVar9 = pbVar9 + 2;
          if (uVar3 < param_3) {
            uVar8 = param_3 - uVar3;
            if (uVar7 < uVar8) {
              pbVar12 = pbVar12 + uVar7;
              psVar5 = psVar5 + uVar7;
            }
            else {
              pbVar12 = pbVar12 + uVar8;
              iVar10 = uVar7 - uVar8;
              psVar5 = psVar5 + uVar8;
              if (param_2 < uVar3 + uVar7) {
                iVar11 = (iVar10 - (uVar3 + uVar7)) + param_2;
                uVar8 = iVar10 - iVar11;
                for (; iVar11 != 0; iVar11 = iVar11 + -1) {
                  bVar1 = *pbVar12;
                  pbVar12 = pbVar12 + 1;
                  *psVar5 = *(short *)((int)puVar2 + (uint)bVar1 * 2);
                  psVar5 = psVar5 + 1;
                }
                goto LAB_00581a76;
              }
              for (; iVar10 != 0; iVar10 = iVar10 + -1) {
                bVar1 = *pbVar12;
                pbVar12 = pbVar12 + 1;
                *psVar5 = *(short *)((int)puVar2 + (uint)bVar1 * 2);
                psVar5 = psVar5 + 1;
              }
            }
          }
          else {
            uVar8 = uVar7;
            if (uVar3 < param_2) {
              if (uVar3 + uVar7 < param_2) {
                for (; uVar8 != 0; uVar8 = uVar8 - 1) {
                  bVar1 = *pbVar12;
                  pbVar12 = pbVar12 + 1;
                  *psVar5 = *(short *)((int)puVar2 + (uint)bVar1 * 2);
                  psVar5 = psVar5 + 1;
                }
                goto LAB_00581a7b;
              }
              iVar10 = (uVar7 - (uVar3 + uVar7)) + param_2;
              uVar8 = uVar7 - iVar10;
              for (; iVar10 != 0; iVar10 = iVar10 + -1) {
                bVar1 = *pbVar12;
                pbVar12 = pbVar12 + 1;
                *psVar5 = *(short *)((int)puVar2 + (uint)bVar1 * 2);
                psVar5 = psVar5 + 1;
              }
            }
LAB_00581a76:
            psVar5 = psVar5 + uVar8;
            pbVar12 = pbVar12 + uVar8;
          }
LAB_00581a7b:
          uVar3 = uVar3 + uVar7;
          uVar8 = *(uint *)(param_1 + 4);
        } while (uVar3 < uVar8);
      }
      uVar8 = *(uint *)(param_1 + 4);
      if (uVar3 != uVar8) {
        uStack_c = 3;
      }
      param_5 = param_5 + 2;
      psVar5 = psVar5 + iVar6;
      param_4 = param_4 + -1;
      puVar4 = puVar2;
    } while (param_4 != 0);
  }
LAB_00581abe:
  return CONCAT22((short)((uint)puVar4 >> 0x10),uStack_c);
}

