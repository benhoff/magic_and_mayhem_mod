
uint __fastcall FUN_0057f0f0(int param_1,int param_2,int *param_3,uint param_4,byte *param_5)

{
  byte bVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  ushort *puVar6;
  ushort *puVar7;
  byte *pbVar8;
  int iVar9;
  uint uVar10;
  byte *pbVar11;
  ushort uVar12;
  uint uVar13;
  uint uVar14;
  int local_1c;
  uint local_18;
  uint local_14;
  uint local_10;
  uint local_c;
  int local_4;
  
  uVar5 = *(uint *)(param_1 + 8);
  uVar13 = (-(uint)(DAT_006e1f88 != 0) & 0x200) + 0xfbef;
  local_18 = 0;
  if ((uVar5 == 0) && (uVar3 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_0057f5e2:
    return uVar3 & 0xffff0000;
  }
  puVar2 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (byte *)0x0; param_5 = (byte *)((int)param_5 + -1)) {
    puVar2 = (undefined4 *)*puVar2;
  }
  iVar9 = (int)param_4 >> ((byte)puVar2[1] & 0x1f);
  if ((iVar9 < 0) && (puVar2[1] != 0)) {
    iVar9 = iVar9 + 1;
  }
  iVar9 = (puVar2[2] + iVar9) * 0x200 + 0xc;
  uVar14 = *(uint *)(param_1 + 4);
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  uVar3 = (int)param_3 - *(int *)(param_1 + 0x10);
  if (((((int)(uVar14 + param_2) <= DAT_006e0008) || ((int)(uVar3 + uVar5) <= DAT_006cbb6c)) ||
      (DAT_006a49b8 <= param_2)) || (DAT_00656618 <= (int)uVar3)) goto LAB_0057f5e2;
  uVar10 = 0;
  if ((int)uVar3 < DAT_006cbb6c) {
    uVar10 = DAT_006cbb6c - uVar3;
  }
  param_5 = (byte *)uVar5;
  if (DAT_00656618 < (int)(uVar3 + uVar5)) {
    param_5 = (byte *)(DAT_00656618 - uVar3);
  }
  if (uVar5 <= uVar10) {
    local_18 = 4;
  }
  if (uVar5 < param_5) {
    local_18 = 5;
  }
  iVar4 = DAT_006a2dc8 - uVar14;
  param_3 = (int *)(param_1 + 0x28 + uVar10 * 8);
  local_10 = 0;
  puVar7 = (ushort *)(DAT_00658174 + ((uVar10 + uVar3) * DAT_006a2dc8 + param_2) * 2);
  if (param_2 < DAT_006e0008) {
    local_10 = DAT_006e0008 - param_2;
  }
  uVar12 = (ushort)uVar13;
  if (DAT_006a49b8 < (int)(uVar14 + param_2)) {
    local_14 = DAT_006a49b8 - param_2;
  }
  else {
    local_14 = uVar14;
    if (param_2 >= DAT_006e0008) {
      if (uVar10 < param_5) {
        local_14 = (int)param_5 - uVar10;
        do {
          pbVar11 = (byte *)(*param_3 + param_1);
          pbVar8 = (byte *)(param_3[1] + param_1);
          uVar5 = 0;
          if (uVar14 != 0) {
            do {
              uVar5 = uVar5 + *pbVar11;
              puVar7 = puVar7 + *pbVar11;
              if (uVar14 <= uVar5) break;
              param_4 = (uint)pbVar11[1];
              pbVar11 = pbVar11 + 2;
              uVar5 = uVar5 + param_4;
              if (param_4 != 0) {
                puVar6 = puVar7;
                do {
                  bVar1 = *pbVar8;
                  pbVar8 = pbVar8 + 1;
                  puVar7 = puVar6 + 1;
                  uVar3 = *(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar9) >> 1 & uVar13;
                  *puVar6 = ((ushort)(uVar3 >> 1) & uVar12) +
                            (short)uVar3 + (*puVar6 >> 2 & uVar12 >> 1 & uVar12);
                  param_4 = param_4 - 1;
                  puVar6 = puVar7;
                } while (param_4 != 0);
              }
              uVar14 = *(uint *)(param_1 + 4);
            } while (uVar5 < uVar14);
          }
          uVar14 = *(uint *)(param_1 + 4);
          if (uVar5 != uVar14) {
            local_18 = 3;
          }
          puVar7 = puVar7 + iVar4;
          param_3 = param_3 + 2;
          local_14 = local_14 + -1;
          if (local_14 == 0) {
            return local_18;
          }
        } while( true );
      }
      goto LAB_0057f5d3;
    }
  }
  if (uVar10 < param_5) {
    local_4 = (int)param_5 - uVar10;
    do {
      param_5 = (byte *)(*param_3 + param_1);
      pbVar8 = (byte *)(param_3[1] + param_1);
      uVar5 = 0;
      if (uVar14 != 0) {
        do {
          uVar5 = uVar5 + *param_5;
          puVar7 = puVar7 + *param_5;
          if (uVar14 <= uVar5) break;
          uVar3 = (uint)param_5[1];
          param_5 = param_5 + 2;
          if (uVar5 < local_10) {
            uVar14 = local_10 - uVar5;
            if (uVar3 < uVar14) {
              pbVar8 = pbVar8 + uVar3;
              puVar7 = puVar7 + uVar3;
            }
            else {
              pbVar8 = pbVar8 + uVar14;
              local_1c = uVar3 - uVar14;
              puVar7 = puVar7 + uVar14;
              if (local_14 < uVar5 + uVar3) {
                local_c = (local_1c - (uVar5 + uVar3)) + local_14;
                uVar14 = local_1c - local_c;
                if (local_c != 0) {
                  puVar6 = puVar7;
                  do {
                    bVar1 = *pbVar8;
                    pbVar8 = pbVar8 + 1;
                    puVar7 = puVar6 + 1;
                    uVar10 = *(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar9) >> 1 & uVar13;
                    *puVar6 = ((ushort)(uVar10 >> 1) & uVar12) +
                              (short)uVar10 + (*puVar6 >> 2 & uVar12 >> 1 & uVar12);
                    local_c = local_c + -1;
                    puVar6 = puVar7;
                  } while (local_c != 0);
                }
                goto LAB_0057f58b;
              }
              if (local_1c != 0) {
                puVar6 = puVar7;
                do {
                  bVar1 = *pbVar8;
                  pbVar8 = pbVar8 + 1;
                  puVar7 = puVar6 + 1;
                  uVar14 = *(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar9) >> 1 & uVar13;
                  *puVar6 = ((ushort)(uVar14 >> 1) & uVar12) +
                            (short)uVar14 + (*puVar6 >> 2 & uVar12 >> 1 & uVar12);
                  local_1c = local_1c + -1;
                  puVar6 = puVar7;
                } while (local_1c != 0);
              }
            }
          }
          else {
            uVar14 = uVar3;
            if (uVar5 < local_14) {
              if (uVar5 + uVar3 < local_14) {
                if (uVar3 != 0) {
                  puVar6 = puVar7;
                  local_c = uVar3;
                  do {
                    bVar1 = *pbVar8;
                    pbVar8 = pbVar8 + 1;
                    puVar7 = puVar6 + 1;
                    uVar14 = *(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar9) >> 1 & uVar13;
                    *puVar6 = ((ushort)(uVar14 >> 1) & uVar12) +
                              (short)uVar14 + (*puVar6 >> 2 & uVar12 >> 1 & uVar12);
                    local_c = local_c - 1;
                    puVar6 = puVar7;
                  } while (local_c != 0);
                }
                goto LAB_0057f590;
              }
              local_c = (uVar3 - (uVar5 + uVar3)) + local_14;
              uVar14 = uVar3 - local_c;
              if (local_c != 0) {
                puVar6 = puVar7;
                do {
                  bVar1 = *pbVar8;
                  pbVar8 = pbVar8 + 1;
                  puVar7 = puVar6 + 1;
                  uVar10 = *(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar9) >> 1 & uVar13;
                  *puVar6 = ((ushort)(uVar10 >> 1) & uVar12) +
                            (short)uVar10 + (*puVar6 >> 2 & uVar12 >> 1 & uVar12);
                  local_c = local_c + -1;
                  puVar6 = puVar7;
                } while (local_c != 0);
              }
            }
LAB_0057f58b:
            puVar7 = puVar7 + uVar14;
            pbVar8 = pbVar8 + uVar14;
          }
LAB_0057f590:
          uVar5 = uVar5 + uVar3;
          uVar14 = *(uint *)(param_1 + 4);
        } while (uVar5 < uVar14);
      }
      uVar14 = *(uint *)(param_1 + 4);
      if (uVar5 != uVar14) {
        local_18 = 3;
      }
      param_3 = param_3 + 2;
      puVar7 = puVar7 + iVar4;
      local_4 = local_4 + -1;
      param_5 = (byte *)0x0;
    } while (local_4 != 0);
  }
LAB_0057f5d3:
  return CONCAT22((short)((uint)param_5 >> 0x10),(undefined2)local_18);
}

