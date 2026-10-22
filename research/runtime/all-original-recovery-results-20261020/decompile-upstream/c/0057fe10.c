
undefined2 __fastcall FUN_0057fe10(int param_1,int param_2,uint param_3,int param_4,byte *param_5)

{
  int iVar1;
  byte bVar2;
  undefined4 *puVar3;
  ushort uVar4;
  undefined2 uVar5;
  uint uVar6;
  int iVar7;
  ushort *puVar8;
  byte *pbVar9;
  int iVar10;
  uint uVar11;
  int *piVar12;
  uint uVar13;
  int iVar14;
  byte *pbVar15;
  byte *pbVar16;
  uint local_18;
  int local_10;
  uint local_c;
  
  uVar11 = *(uint *)(param_1 + 8);
  uVar5 = 0;
  uVar4 = (-(ushort)(DAT_006e1f88 != 0) & 0x200) - 0x411;
  if ((uVar11 == 0) && (*(int *)(param_1 + 4) == 0)) {
    return 0;
  }
  puVar3 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (byte *)0x0; param_5 = (byte *)((int)param_5 + -1)) {
    puVar3 = (undefined4 *)*puVar3;
  }
  param_4 = param_4 >> ((byte)puVar3[1] & 0x1f);
  if ((param_4 < 0) && (puVar3[1] != 0)) {
    param_4 = param_4 + 1;
  }
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  iVar1 = (puVar3[2] + param_4) * 0x200 + 0xc;
  iVar7 = param_3 - *(int *)(param_1 + 0x10);
  uVar6 = *(uint *)(param_1 + 4);
  if ((int)(uVar6 + param_2) <= DAT_006e0008) {
    return 0;
  }
  if ((int)(uVar11 + iVar7) <= DAT_006cbb6c) {
    return 0;
  }
  if (DAT_006a49b8 <= param_2) {
    return 0;
  }
  if (DAT_00656618 <= iVar7) {
    return 0;
  }
  param_5 = (byte *)0x0;
  if (iVar7 < DAT_006cbb6c) {
    param_5 = (byte *)(DAT_006cbb6c - iVar7);
  }
  param_3 = uVar11;
  if (DAT_00656618 < (int)(uVar11 + iVar7)) {
    param_3 = DAT_00656618 - iVar7;
  }
  if (uVar11 <= param_5) {
    uVar5 = 4;
  }
  if (uVar11 < param_3) {
    uVar5 = 5;
  }
  iVar10 = DAT_006a2dc8 - uVar6;
  puVar8 = (ushort *)(DAT_00658174 + ((iVar7 + (int)param_5) * DAT_006a2dc8 + param_2) * 2);
  piVar12 = (int *)(param_1 + 0x28 + (int)param_5 * 8);
  local_c = 0;
  if (param_2 < DAT_006e0008) {
    local_c = DAT_006e0008 - param_2;
  }
  if (DAT_006a49b8 < (int)(param_2 + uVar6)) {
    local_18 = DAT_006a49b8 - param_2;
  }
  else {
    local_18 = uVar6;
    if (param_2 >= DAT_006e0008) {
      if (param_3 <= param_5) {
        return uVar5;
      }
      param_5 = (byte *)(param_3 - (int)param_5);
      do {
        pbVar15 = (byte *)(*piVar12 + param_1);
        pbVar9 = (byte *)(piVar12[1] + param_1);
        uVar11 = 0;
        if (uVar6 != 0) {
          do {
            pbVar16 = pbVar15 + 1;
            uVar11 = uVar11 + *pbVar15;
            puVar8 = puVar8 + *pbVar15;
            if (uVar6 <= uVar11) break;
            pbVar15 = pbVar15 + 2;
            uVar11 = uVar11 + *pbVar16;
            iVar7 = *pbVar16 - 1;
            *puVar8 = *(ushort *)((int)puVar3 + (uint)*pbVar9 * 2 + iVar1);
            if (iVar7 != 0) {
              pbVar9 = pbVar9 + iVar7;
              puVar8 = puVar8 + iVar7;
              *puVar8 = *(ushort *)((int)puVar3 + (uint)*pbVar9 * 2 + iVar1);
            }
            pbVar9 = pbVar9 + 1;
            puVar8 = puVar8 + 1;
            uVar6 = *(uint *)(param_1 + 4);
          } while (uVar11 < uVar6);
        }
        uVar6 = *(uint *)(param_1 + 4);
        if (uVar11 != uVar6) {
          uVar5 = 3;
        }
        piVar12 = piVar12 + 2;
        puVar8 = puVar8 + iVar10;
        param_5 = (byte *)((int)param_5 + -1);
        if (param_5 == (byte *)0x0) {
          return uVar5;
        }
      } while( true );
    }
  }
  if (param_5 < param_3) {
    local_10 = param_3 - (int)param_5;
    do {
      uVar11 = 0;
      param_5 = (byte *)(*piVar12 + param_1);
      pbVar9 = (byte *)(piVar12[1] + param_1);
      if (uVar6 != 0) {
        do {
          uVar11 = uVar11 + *param_5;
          puVar8 = puVar8 + *param_5;
          if (uVar6 <= uVar11) break;
          uVar6 = (uint)param_5[1];
          param_5 = param_5 + 2;
          if (uVar11 < local_c) {
            uVar13 = local_c - uVar11;
            if (uVar6 < uVar13) {
              pbVar9 = pbVar9 + uVar6;
              puVar8 = puVar8 + uVar6;
            }
            else {
              pbVar9 = pbVar9 + uVar13;
              iVar7 = uVar6 - uVar13;
              puVar8 = puVar8 + uVar13;
              if (local_18 < uVar11 + uVar6) {
                iVar14 = (iVar7 - (uVar11 + uVar6)) + local_18;
                uVar13 = iVar7 - iVar14;
                for (; iVar14 != 0; iVar14 = iVar14 + -1) {
                  *puVar8 = (*(ushort *)((int)puVar3 + (uint)*pbVar9 * 2 + iVar1) >> 1 & uVar4) +
                            (*puVar8 >> 1 & uVar4);
                  pbVar9 = pbVar9 + 1;
                  puVar8 = puVar8 + 1;
                }
                goto LAB_005801f2;
              }
              for (; iVar7 != 0; iVar7 = iVar7 + -1) {
                bVar2 = *pbVar9;
                pbVar9 = pbVar9 + 1;
                *puVar8 = (*(ushort *)((int)puVar3 + (uint)bVar2 * 2 + iVar1) >> 1 & uVar4) +
                          (*puVar8 >> 1 & uVar4);
                puVar8 = puVar8 + 1;
              }
            }
          }
          else {
            uVar13 = uVar6;
            if (uVar11 < local_18) {
              if (uVar11 + uVar6 < local_18) {
                for (; uVar13 != 0; uVar13 = uVar13 - 1) {
                  bVar2 = *pbVar9;
                  pbVar9 = pbVar9 + 1;
                  *puVar8 = (*(ushort *)((int)puVar3 + (uint)bVar2 * 2 + iVar1) >> 1 & uVar4) +
                            (*puVar8 >> 1 & uVar4);
                  puVar8 = puVar8 + 1;
                }
                goto LAB_005801f7;
              }
              iVar7 = (uVar6 - (uVar11 + uVar6)) + local_18;
              uVar13 = uVar6 - iVar7;
              for (; iVar7 != 0; iVar7 = iVar7 + -1) {
                *puVar8 = (*(ushort *)((int)puVar3 + (uint)*pbVar9 * 2 + iVar1) >> 1 & uVar4) +
                          (*puVar8 >> 1 & uVar4);
                pbVar9 = pbVar9 + 1;
                puVar8 = puVar8 + 1;
              }
            }
LAB_005801f2:
            puVar8 = puVar8 + uVar13;
            pbVar9 = pbVar9 + uVar13;
          }
LAB_005801f7:
          uVar11 = uVar11 + uVar6;
          uVar6 = *(uint *)(param_1 + 4);
        } while (uVar11 < uVar6);
      }
      uVar6 = *(uint *)(param_1 + 4);
      if (uVar11 != uVar6) {
        uVar5 = 3;
      }
      piVar12 = piVar12 + 2;
      puVar8 = puVar8 + iVar10;
      local_10 = local_10 + -1;
    } while (local_10 != 0);
  }
  return uVar5;
}

