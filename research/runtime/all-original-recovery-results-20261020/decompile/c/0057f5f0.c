
uint __fastcall FUN_0057f5f0(int param_1,int param_2,uint param_3,uint param_4,byte *param_5)

{
  byte bVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  ushort *puVar6;
  ushort *puVar7;
  ushort uVar8;
  byte *pbVar9;
  int *piVar10;
  byte *pbVar11;
  ushort uVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  uint local_18;
  uint local_14;
  uint local_10;
  int local_c;
  int local_4;
  
  uVar5 = *(uint *)(param_1 + 8);
  uVar13 = (-(uint)(DAT_006e1f88 != 0) & 0x200) + 0xfbef;
  local_18 = 0;
  if ((uVar5 == 0) && (uVar3 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_0057fb00:
    return uVar3 & 0xffff0000;
  }
  puVar2 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (byte *)0x0; param_5 = (byte *)((int)param_5 + -1)) {
    puVar2 = (undefined4 *)*puVar2;
  }
  iVar14 = (int)param_4 >> ((byte)puVar2[1] & 0x1f);
  if ((iVar14 < 0) && (puVar2[1] != 0)) {
    iVar14 = iVar14 + 1;
  }
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  uVar15 = *(uint *)(param_1 + 4);
  iVar14 = (puVar2[2] + iVar14) * 0x200 + 0xc;
  uVar3 = param_3 - *(int *)(param_1 + 0x10);
  if (((((int)(uVar15 + param_2) <= DAT_006e0008) || ((int)(uVar5 + uVar3) <= DAT_006cbb6c)) ||
      (DAT_006a49b8 <= param_2)) || (DAT_00656618 <= (int)uVar3)) goto LAB_0057fb00;
  param_5 = (byte *)0x0;
  if ((int)uVar3 < DAT_006cbb6c) {
    param_5 = (byte *)(DAT_006cbb6c - uVar3);
  }
  param_4 = uVar5;
  if (DAT_00656618 < (int)(uVar5 + uVar3)) {
    param_4 = DAT_00656618 - uVar3;
  }
  if (uVar5 <= param_5) {
    local_18 = 4;
  }
  if (uVar5 < param_4) {
    local_18 = 5;
  }
  iVar4 = DAT_006a2dc8 - uVar15;
  puVar7 = (ushort *)(DAT_00658174 + (((int)param_5 + uVar3) * DAT_006a2dc8 + param_2) * 2);
  piVar10 = (int *)(param_1 + 0x28 + (int)param_5 * 8);
  local_10 = 0;
  if (param_2 < DAT_006e0008) {
    local_10 = DAT_006e0008 - param_2;
  }
  uVar12 = (ushort)uVar13;
  if (DAT_006a49b8 < (int)(uVar15 + param_2)) {
    local_14 = DAT_006a49b8 - param_2;
  }
  else {
    local_14 = uVar15;
    if (param_2 >= DAT_006e0008) {
      if (param_5 < param_4) {
        local_14 = param_4 - (int)param_5;
        do {
          pbVar9 = (byte *)(*piVar10 + param_1);
          pbVar11 = (byte *)(piVar10[1] + param_1);
          uVar5 = 0;
          if (uVar15 != 0) {
            do {
              uVar5 = uVar5 + *pbVar9;
              puVar7 = puVar7 + *pbVar9;
              if (uVar15 <= uVar5) break;
              param_4 = (uint)pbVar9[1];
              pbVar9 = pbVar9 + 2;
              uVar5 = uVar5 + param_4;
              if (param_4 != 0) {
                puVar6 = puVar7;
                do {
                  puVar7 = puVar6 + 1;
                  bVar1 = *pbVar11;
                  uVar8 = *puVar6 >> 1 & uVar12;
                  pbVar11 = pbVar11 + 1;
                  *puVar6 = (uVar8 >> 1 & uVar12) +
                            uVar8 + (*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar14) >> 2 &
                                     uVar12 >> 1 & uVar12);
                  param_4 = param_4 - 1;
                  puVar6 = puVar7;
                } while (param_4 != 0);
              }
              uVar15 = *(uint *)(param_1 + 4);
            } while (uVar5 < uVar15);
          }
          uVar15 = *(uint *)(param_1 + 4);
          if (uVar5 != uVar15) {
            local_18 = 3;
          }
          puVar7 = puVar7 + iVar4;
          piVar10 = piVar10 + 2;
          local_14 = local_14 + -1;
          if (local_14 == 0) {
            return local_18;
          }
        } while( true );
      }
      goto LAB_0057faf1;
    }
  }
  if (param_5 < param_4) {
    local_4 = param_4 - (int)param_5;
    do {
      param_5 = (byte *)(*piVar10 + param_1);
      pbVar9 = (byte *)(piVar10[1] + param_1);
      uVar5 = 0;
      if (uVar15 != 0) {
        do {
          uVar5 = uVar5 + *param_5;
          puVar7 = puVar7 + *param_5;
          if (uVar15 <= uVar5) break;
          uVar3 = (uint)param_5[1];
          param_5 = param_5 + 2;
          if (uVar5 < local_10) {
            uVar15 = local_10 - uVar5;
            if (uVar3 < uVar15) {
              pbVar9 = pbVar9 + uVar3;
              puVar7 = puVar7 + uVar3;
            }
            else {
              pbVar9 = pbVar9 + uVar15;
              param_3 = uVar3 - uVar15;
              puVar7 = puVar7 + uVar15;
              if (local_14 < uVar5 + uVar3) {
                local_c = (param_3 - (uVar5 + uVar3)) + local_14;
                param_3 = param_3 - local_c;
                if (local_c != 0) {
                  puVar6 = puVar7;
                  do {
                    puVar7 = puVar6 + 1;
                    bVar1 = *pbVar9;
                    uVar15 = *puVar6 >> 1 & uVar13;
                    pbVar9 = pbVar9 + 1;
                    *puVar6 = ((ushort)(uVar15 >> 1) & uVar12) +
                              (short)uVar15 +
                              (*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar14) >> 2 &
                               uVar12 >> 1 & uVar12);
                    local_c = local_c + -1;
                    puVar6 = puVar7;
                  } while (local_c != 0);
                }
                goto LAB_0057faa9;
              }
              if (param_3 != 0) {
                puVar6 = puVar7;
                do {
                  puVar7 = puVar6 + 1;
                  uVar15 = *puVar6 >> 1 & uVar13;
                  *puVar6 = ((ushort)(uVar15 >> 1) & uVar12) +
                            (short)uVar15 +
                            (*(ushort *)((int)puVar2 + (uint)*pbVar9 * 2 + iVar14) >> 2 &
                             uVar12 >> 1 & uVar12);
                  pbVar9 = pbVar9 + 1;
                  param_3 = param_3 + -1;
                  puVar6 = puVar7;
                } while (param_3 != 0);
              }
            }
          }
          else {
            param_3 = uVar3;
            if (uVar5 < local_14) {
              if (uVar5 + uVar3 < local_14) {
                if (uVar3 != 0) {
                  puVar6 = puVar7;
                  do {
                    puVar7 = puVar6 + 1;
                    uVar15 = *puVar6 >> 1 & uVar13;
                    *puVar6 = ((ushort)(uVar15 >> 1) & uVar12) +
                              (short)uVar15 +
                              (*(ushort *)((int)puVar2 + (uint)*pbVar9 * 2 + iVar14) >> 2 &
                               uVar12 >> 1 & uVar12);
                    pbVar9 = pbVar9 + 1;
                    param_3 = param_3 - 1;
                    puVar6 = puVar7;
                  } while (param_3 != 0);
                }
                goto LAB_0057faae;
              }
              local_c = (uVar3 - (uVar5 + uVar3)) + local_14;
              param_3 = uVar3 - local_c;
              if (local_c != 0) {
                puVar6 = puVar7;
                do {
                  puVar7 = puVar6 + 1;
                  bVar1 = *pbVar9;
                  uVar15 = *puVar6 >> 1 & uVar13;
                  pbVar9 = pbVar9 + 1;
                  *puVar6 = ((ushort)(uVar15 >> 1) & uVar12) +
                            (short)uVar15 +
                            (*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar14) >> 2 & uVar12 >> 1
                            & uVar12);
                  local_c = local_c + -1;
                  puVar6 = puVar7;
                } while (local_c != 0);
              }
            }
LAB_0057faa9:
            puVar7 = puVar7 + param_3;
            pbVar9 = pbVar9 + param_3;
          }
LAB_0057faae:
          uVar5 = uVar5 + uVar3;
          uVar15 = *(uint *)(param_1 + 4);
        } while (uVar5 < uVar15);
      }
      uVar15 = *(uint *)(param_1 + 4);
      if (uVar5 != uVar15) {
        local_18 = 3;
      }
      piVar10 = piVar10 + 2;
      local_4 = local_4 + -1;
      puVar7 = puVar7 + iVar4;
      param_4 = 0;
    } while (local_4 != 0);
  }
LAB_0057faf1:
  return CONCAT22((short)(param_4 >> 0x10),(undefined2)local_18);
}

