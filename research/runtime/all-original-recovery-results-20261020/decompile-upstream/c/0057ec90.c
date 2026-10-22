
undefined2 __fastcall FUN_0057ec90(int param_1,int param_2,uint param_3,byte *param_4,int *param_5)

{
  byte bVar1;
  undefined4 *puVar2;
  undefined2 uVar3;
  int iVar4;
  ushort *puVar5;
  byte *pbVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  byte *pbVar10;
  ushort uVar11;
  uint uVar12;
  uint uVar13;
  int iVar14;
  uint local_10;
  int local_c;
  uint local_8;
  
  uVar9 = *(uint *)(param_1 + 8);
  uVar11 = (-(ushort)(DAT_006e1f88 != 0) & 0x200) - 0x411;
  uVar3 = 0;
  if ((uVar9 == 0) && (*(int *)(param_1 + 4) == 0)) {
    return 0;
  }
  puVar2 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (int *)0x0; param_5 = (int *)((int)param_5 + -1)) {
    puVar2 = (undefined4 *)*puVar2;
  }
  iVar7 = (int)param_4 >> ((byte)puVar2[1] & 0x1f);
  if ((iVar7 < 0) && (puVar2[1] != 0)) {
    iVar7 = iVar7 + 1;
  }
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  iVar7 = (puVar2[2] + iVar7) * 0x200 + 0xc;
  iVar4 = param_3 - *(int *)(param_1 + 0x10);
  uVar13 = *(uint *)(param_1 + 4);
  if ((int)(uVar13 + param_2) <= DAT_006e0008) {
    return 0;
  }
  if ((int)(uVar9 + iVar4) <= DAT_006cbb6c) {
    return 0;
  }
  if (DAT_006a49b8 <= param_2) {
    return 0;
  }
  if (DAT_00656618 <= iVar4) {
    return 0;
  }
  param_4 = (byte *)0x0;
  if (iVar4 < DAT_006cbb6c) {
    param_4 = (byte *)(DAT_006cbb6c - iVar4);
  }
  param_3 = uVar9;
  if (DAT_00656618 < (int)(uVar9 + iVar4)) {
    param_3 = DAT_00656618 - iVar4;
  }
  if (uVar9 <= param_4) {
    uVar3 = 4;
  }
  if (uVar9 < param_3) {
    uVar3 = 5;
  }
  iVar8 = DAT_006a2dc8 - uVar13;
  puVar5 = (ushort *)(DAT_00658174 + ((iVar4 + (int)param_4) * DAT_006a2dc8 + param_2) * 2);
  param_5 = (int *)(param_1 + 0x28 + (int)param_4 * 8);
  local_8 = 0;
  if (param_2 < DAT_006e0008) {
    local_8 = DAT_006e0008 - param_2;
  }
  if (DAT_006a49b8 < (int)(uVar13 + param_2)) {
    local_10 = DAT_006a49b8 - param_2;
  }
  else {
    local_10 = uVar13;
    if (param_2 >= DAT_006e0008) {
      if (param_3 <= param_4) {
        return uVar3;
      }
      param_3 = param_3 - (int)param_4;
      do {
        pbVar10 = (byte *)(*param_5 + param_1);
        pbVar6 = (byte *)(param_5[1] + param_1);
        uVar9 = 0;
        if (uVar13 != 0) {
          do {
            uVar9 = uVar9 + *pbVar10;
            puVar5 = puVar5 + *pbVar10;
            if (uVar13 <= uVar9) break;
            uVar13 = (uint)pbVar10[1];
            pbVar10 = pbVar10 + 2;
            uVar9 = uVar9 + uVar13;
            for (; uVar13 != 0; uVar13 = uVar13 - 1) {
              bVar1 = *pbVar6;
              pbVar6 = pbVar6 + 1;
              *puVar5 = (*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar7) >> 1 & uVar11) +
                        (*puVar5 >> 1 & uVar11);
              puVar5 = puVar5 + 1;
            }
            uVar13 = *(uint *)(param_1 + 4);
          } while (uVar9 < uVar13);
        }
        uVar13 = *(uint *)(param_1 + 4);
        if (uVar9 != uVar13) {
          uVar3 = 3;
        }
        param_5 = param_5 + 2;
        puVar5 = puVar5 + iVar8;
        param_3 = param_3 + -1;
        if (param_3 == 0) {
          return uVar3;
        }
      } while( true );
    }
  }
  if (param_4 < param_3) {
    local_c = param_3 - (int)param_4;
    do {
      param_4 = (byte *)(*param_5 + param_1);
      pbVar6 = (byte *)(param_5[1] + param_1);
      uVar9 = 0;
      if (uVar13 != 0) {
        do {
          uVar9 = uVar9 + *param_4;
          puVar5 = puVar5 + *param_4;
          if (uVar13 <= uVar9) break;
          uVar13 = (uint)param_4[1];
          param_4 = param_4 + 2;
          if (uVar9 < local_8) {
            uVar12 = local_8 - uVar9;
            if (uVar13 < uVar12) {
              pbVar6 = pbVar6 + uVar13;
              puVar5 = puVar5 + uVar13;
            }
            else {
              pbVar6 = pbVar6 + uVar12;
              iVar4 = uVar13 - uVar12;
              puVar5 = puVar5 + uVar12;
              if (local_10 < uVar9 + uVar13) {
                iVar14 = (iVar4 - (uVar9 + uVar13)) + local_10;
                uVar12 = iVar4 - iVar14;
                for (; iVar14 != 0; iVar14 = iVar14 + -1) {
                  *puVar5 = (*(ushort *)((int)puVar2 + (uint)*pbVar6 * 2 + iVar7) >> 1 & uVar11) +
                            (*puVar5 >> 1 & uVar11);
                  pbVar6 = pbVar6 + 1;
                  puVar5 = puVar5 + 1;
                }
                goto LAB_0057f087;
              }
              for (; iVar4 != 0; iVar4 = iVar4 + -1) {
                bVar1 = *pbVar6;
                pbVar6 = pbVar6 + 1;
                *puVar5 = (*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar7) >> 1 & uVar11) +
                          (*puVar5 >> 1 & uVar11);
                puVar5 = puVar5 + 1;
              }
            }
          }
          else {
            uVar12 = uVar13;
            if (uVar9 < local_10) {
              if (uVar9 + uVar13 < local_10) {
                for (; uVar12 != 0; uVar12 = uVar12 - 1) {
                  bVar1 = *pbVar6;
                  pbVar6 = pbVar6 + 1;
                  *puVar5 = (*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar7) >> 1 & uVar11) +
                            (*puVar5 >> 1 & uVar11);
                  puVar5 = puVar5 + 1;
                }
                goto LAB_0057f08c;
              }
              iVar4 = (uVar13 - (uVar9 + uVar13)) + local_10;
              uVar12 = uVar13 - iVar4;
              for (; iVar4 != 0; iVar4 = iVar4 + -1) {
                *puVar5 = (*(ushort *)((int)puVar2 + (uint)*pbVar6 * 2 + iVar7) >> 1 & uVar11) +
                          (*puVar5 >> 1 & uVar11);
                pbVar6 = pbVar6 + 1;
                puVar5 = puVar5 + 1;
              }
            }
LAB_0057f087:
            puVar5 = puVar5 + uVar12;
            pbVar6 = pbVar6 + uVar12;
          }
LAB_0057f08c:
          uVar9 = uVar9 + uVar13;
          uVar13 = *(uint *)(param_1 + 4);
        } while (uVar9 < uVar13);
      }
      uVar13 = *(uint *)(param_1 + 4);
      if (uVar9 != uVar13) {
        uVar3 = 3;
      }
      param_5 = param_5 + 2;
      puVar5 = puVar5 + iVar8;
      local_c = local_c + -1;
    } while (local_c != 0);
  }
  return uVar3;
}

