
uint __fastcall
FUN_00581200(int param_1,int param_2,uint param_3,int *param_4,uint param_5,uint param_6,
            uint param_7)

{
  byte bVar1;
  undefined2 uVar2;
  uint uVar3;
  ushort *puVar4;
  byte *pbVar5;
  ushort uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  byte *pbVar10;
  uint uVar11;
  ushort uVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int local_4;
  
  uVar11 = *(uint *)(param_1 + 8);
  uVar12 = (-(ushort)(DAT_006e1f88 != 0) & 0x200) - 0x411;
  uVar2 = 0;
  if ((uVar11 == 0) && (uVar3 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_005816eb:
    return uVar3 & 0xffff0000;
  }
  iVar13 = *(int *)(param_1 + 0x1c);
  iVar7 = (int)param_4 >> ((byte)*(int *)(iVar13 + 4) & 0x1f);
  if ((iVar7 < 0) && (*(int *)(iVar13 + 4) != 0)) {
    iVar7 = iVar7 + 1;
  }
  iVar14 = (int)param_4 + 0x7f;
  iVar13 = (*(int *)(iVar13 + 8) + iVar7) * 0x200 + 0xc + iVar13;
  uVar9 = (int)((param_7 & 0xff) * iVar14) / 0x7f;
  uVar3 = (int)((param_6 & 0xff) * iVar14) / 0x7f;
  uVar8 = (int)((param_5 & 0xff) * iVar14) / 0x7f;
  if (0xff < (ushort)uVar8) {
    uVar8 = 0xff;
  }
  if (0xff < (ushort)uVar3) {
    uVar3 = 0xff;
  }
  if (0xff < (ushort)uVar9) {
    uVar9 = 0xff;
  }
  if (DAT_006e1f88 == 0) {
    uVar6 = (ushort)(((uVar8 & 0xfff8) << 5 | uVar3 & 0xfc) << 3);
  }
  else {
    uVar6 = (ushort)(((uVar8 & 0xf8) << 5 | uVar3 & 0xf8) << 2);
  }
  uVar6 = uVar6 | (ushort)(uVar9 >> 3) & 0x1f;
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  uVar3 = param_3 - *(int *)(param_1 + 0x10);
  uVar9 = *(uint *)(param_1 + 4);
  if (((((int)(uVar9 + param_2) <= DAT_006e0008) || ((int)(uVar11 + uVar3) <= DAT_006cbb6c)) ||
      (DAT_006a49b8 <= param_2)) || (DAT_00656618 <= (int)uVar3)) goto LAB_005816eb;
  uVar8 = 0;
  if ((int)uVar3 < DAT_006cbb6c) {
    uVar8 = DAT_006cbb6c - uVar3;
  }
  param_6 = uVar11;
  if (DAT_00656618 < (int)(uVar11 + uVar3)) {
    param_6 = DAT_00656618 - uVar3;
  }
  if (uVar11 <= uVar8) {
    uVar2 = 4;
  }
  if (uVar11 < param_6) {
    uVar2 = 5;
  }
  iVar7 = DAT_006a2dc8 - uVar9;
  param_4 = (int *)(param_1 + 0x28 + uVar8 * 8);
  puVar4 = (ushort *)(DAT_00658174 + ((uVar3 + uVar8) * DAT_006a2dc8 + param_2) * 2);
  param_3 = 0;
  if (param_2 < DAT_006e0008) {
    param_3 = DAT_006e0008 - param_2;
  }
  if (DAT_006a49b8 < (int)(uVar9 + param_2)) {
    param_7 = DAT_006a49b8 - param_2;
  }
  else {
    param_7 = uVar9;
    if (param_2 >= DAT_006e0008) {
      if (uVar8 < param_6) {
        param_7 = param_6 - uVar8;
        do {
          pbVar5 = (byte *)(*param_4 + param_1);
          uVar11 = 0;
          if (uVar9 != 0) {
            do {
              uVar11 = uVar11 + *pbVar5;
              puVar4 = puVar4 + *pbVar5;
              if (uVar9 <= uVar11) break;
              bVar1 = pbVar5[1];
              puVar4[-1] = uVar6;
              *puVar4 = uVar6;
              pbVar5 = pbVar5 + 2;
              uVar11 = uVar11 + bVar1;
              iVar13 = bVar1 - 1;
              if (iVar13 != 0) {
                puVar4 = puVar4 + iVar13;
                puVar4[-1] = uVar6;
                *puVar4 = uVar6;
              }
              puVar4 = puVar4 + 1;
              uVar9 = *(uint *)(param_1 + 4);
            } while (uVar11 < uVar9);
          }
          uVar9 = *(uint *)(param_1 + 4);
          if (uVar11 != uVar9) {
            uVar2 = 3;
          }
          param_4 = param_4 + 2;
          puVar4 = puVar4 + iVar7;
          param_7 = param_7 + -1;
          if (param_7 == 0) {
            return CONCAT22((short)((uint)puVar4 >> 0x10),uVar2);
          }
        } while( true );
      }
      goto LAB_005816dc;
    }
  }
  if (uVar8 < param_6) {
    local_4 = param_6 - uVar8;
    do {
      pbVar10 = (byte *)(*param_4 + param_1);
      pbVar5 = (byte *)(param_4[1] + param_1);
      uVar11 = 0;
      if (uVar9 != 0) {
        do {
          uVar11 = uVar11 + *pbVar10;
          puVar4 = puVar4 + *pbVar10;
          if (uVar9 <= uVar11) break;
          uVar3 = (uint)pbVar10[1];
          pbVar10 = pbVar10 + 2;
          if (uVar11 < param_3) {
            uVar9 = param_3 - uVar11;
            if (uVar3 < uVar9) {
              pbVar5 = pbVar5 + uVar3;
              puVar4 = puVar4 + uVar3;
            }
            else {
              puVar4 = puVar4 + uVar9;
              pbVar5 = pbVar5 + uVar9;
              iVar15 = uVar3 - uVar9;
              if (param_7 < uVar11 + uVar3) {
                iVar14 = (iVar15 - (uVar11 + uVar3)) + param_7;
                iVar15 = iVar15 - iVar14;
                for (; iVar14 != 0; iVar14 = iVar14 + -1) {
                  bVar1 = *pbVar5;
                  pbVar5 = pbVar5 + 1;
                  *puVar4 = (*(ushort *)(iVar13 + (uint)bVar1 * 2) >> 1 & uVar12) +
                            (*puVar4 >> 1 & uVar12);
                  puVar4 = puVar4 + 1;
                }
LAB_00581690:
                puVar4 = puVar4 + iVar15;
                pbVar5 = pbVar5 + iVar15;
              }
              else {
                for (; iVar15 != 0; iVar15 = iVar15 + -1) {
                  bVar1 = *pbVar5;
                  pbVar5 = pbVar5 + 1;
                  *puVar4 = (*(ushort *)(iVar13 + (uint)bVar1 * 2) >> 1 & uVar12) +
                            (*puVar4 >> 1 & uVar12);
                  puVar4 = puVar4 + 1;
                }
              }
            }
          }
          else if (uVar11 < param_7) {
            uVar9 = uVar3;
            if (param_7 <= uVar11 + uVar3) {
              iVar14 = (uVar3 - (uVar11 + uVar3)) + param_7;
              iVar15 = uVar3 - iVar14;
              for (; iVar14 != 0; iVar14 = iVar14 + -1) {
                bVar1 = *pbVar5;
                pbVar5 = pbVar5 + 1;
                *puVar4 = (*(ushort *)(iVar13 + (uint)bVar1 * 2) >> 1 & uVar12) +
                          (*puVar4 >> 1 & uVar12);
                puVar4 = puVar4 + 1;
              }
              goto LAB_00581690;
            }
            for (; uVar9 != 0; uVar9 = uVar9 - 1) {
              bVar1 = *pbVar5;
              pbVar5 = pbVar5 + 1;
              *puVar4 = (*(ushort *)(iVar13 + (uint)bVar1 * 2) >> 1 & uVar12) +
                        (*puVar4 >> 1 & uVar12);
              puVar4 = puVar4 + 1;
            }
          }
          else {
            puVar4 = puVar4 + uVar3;
            pbVar5 = pbVar5 + uVar3;
          }
          uVar11 = uVar11 + uVar3;
          uVar9 = *(uint *)(param_1 + 4);
        } while (uVar11 < uVar9);
      }
      uVar9 = *(uint *)(param_1 + 4);
      if (uVar11 != uVar9) {
        uVar2 = 3;
      }
      puVar4 = puVar4 + iVar7;
      param_4 = param_4 + 2;
      local_4 = local_4 + -1;
    } while (local_4 != 0);
  }
LAB_005816dc:
  return CONCAT22((short)((uint)puVar4 >> 0x10),uVar2);
}

