
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall FUN_00580a90(int param_1,int param_2,int param_3,int param_4,uint param_5)

{
  int iVar1;
  byte bVar2;
  ushort uVar3;
  undefined4 *puVar4;
  ushort uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  ushort *puVar9;
  uint uVar10;
  byte *pbVar11;
  uint uVar12;
  int iVar13;
  uint uVar14;
  ushort uVar15;
  uint uVar16;
  byte *pbVar17;
  int *local_24;
  uint local_1c;
  uint local_18;
  byte *local_14;
  uint local_10;
  int local_4;
  
  uVar10 = *(uint *)(param_1 + 8);
  local_1c = 0;
  if ((uVar10 == 0) && (uVar6 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_005811f3:
    return uVar6 & 0xffff0000;
  }
  puVar4 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != 0; param_5 = param_5 + -1) {
    puVar4 = (undefined4 *)*puVar4;
  }
  param_4 = param_4 >> ((byte)puVar4[1] & 0x1f);
  if ((param_4 < 0) && (puVar4[1] != 0)) {
    param_4 = param_4 + 1;
  }
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  uVar12 = *(uint *)(param_1 + 4);
  iVar1 = (puVar4[2] + param_4) * 0x200 + 0xc;
  uVar6 = param_3 - *(int *)(param_1 + 0x10);
  if (((((int)(uVar12 + param_2) <= DAT_006e0008) || ((int)(uVar10 + uVar6) <= DAT_006cbb6c)) ||
      (DAT_006a49b8 <= param_2)) || (DAT_00656618 <= (int)uVar6)) goto LAB_005811f3;
  uVar16 = 0;
  if ((int)uVar6 < DAT_006cbb6c) {
    uVar16 = DAT_006cbb6c - uVar6;
  }
  param_5 = uVar10;
  if (DAT_00656618 < (int)(uVar10 + uVar6)) {
    param_5 = DAT_00656618 - uVar6;
  }
  if (uVar10 <= uVar16) {
    local_1c = 4;
  }
  if (uVar10 < param_5) {
    local_1c = 5;
  }
  iVar7 = DAT_006a2dc8 - uVar12;
  local_24 = (int *)(param_1 + 0x28 + uVar16 * 8);
  local_10 = 0;
  puVar9 = (ushort *)(DAT_00658174 + ((uVar16 + uVar6) * DAT_006a2dc8 + param_2) * 2);
  if (param_2 < DAT_006e0008) {
    local_10 = DAT_006e0008 - param_2;
  }
  if (DAT_006a49b8 < (int)(uVar12 + param_2)) {
    local_18 = DAT_006a49b8 - param_2;
  }
  else {
    local_18 = uVar12;
    if (param_2 >= DAT_006e0008) {
      if (uVar16 < param_5) {
        local_10 = param_5 - uVar16;
        do {
          uVar10 = 0;
          pbVar17 = (byte *)(*local_24 + param_1);
          local_14 = (byte *)(local_24[1] + param_1);
          if (uVar12 != 0) {
            do {
              uVar10 = uVar10 + *pbVar17;
              puVar9 = puVar9 + *pbVar17;
              if (uVar12 <= uVar10) break;
              uVar6 = (uint)pbVar17[1];
              pbVar17 = pbVar17 + 2;
              uVar10 = uVar10 + uVar6;
              for (; uVar6 != 0; uVar6 = uVar6 - 1) {
                uVar5 = *puVar9;
                bVar2 = *local_14;
                local_14 = local_14 + 1;
                uVar3 = *(ushort *)((int)puVar4 + (uint)bVar2 * 2 + iVar1);
                uVar16 = (uint)DAT_006e1f7c;
                uVar12 = (uint)(uVar3 & DAT_006e1f7c) + (uint)(uVar5 & DAT_006e1f7c);
                uVar15 = (DAT_006e1f7e & uVar3) + (DAT_006e1f7e & uVar5);
                uVar5 = (_DAT_006e1f80 & uVar3) + (_DAT_006e1f80 & uVar5);
                if ((uVar16 | uVar12) != uVar16) {
                  uVar12 = uVar16;
                }
                if ((DAT_006e1f7e | uVar15) != DAT_006e1f7e) {
                  uVar15 = DAT_006e1f7e;
                }
                if ((_DAT_006e1f80 | uVar5) != _DAT_006e1f80) {
                  uVar5 = _DAT_006e1f80;
                }
                *puVar9 = (ushort)uVar12 | uVar5 | uVar15;
                puVar9 = puVar9 + 1;
              }
              uVar12 = *(uint *)(param_1 + 4);
            } while (uVar10 < uVar12);
          }
          uVar12 = *(uint *)(param_1 + 4);
          if (uVar10 != uVar12) {
            local_1c = 3;
          }
          local_24 = local_24 + 2;
          puVar9 = puVar9 + iVar7;
          local_10 = local_10 + -1;
          if (local_10 == 0) {
            return local_1c;
          }
        } while( true );
      }
      goto LAB_005811e4;
    }
  }
  if (uVar16 < param_5) {
    local_4 = param_5 - uVar16;
    do {
      uVar10 = 0;
      pbVar11 = (byte *)(*local_24 + param_1);
      pbVar17 = (byte *)(local_24[1] + param_1);
      if (uVar12 != 0) {
        do {
          uVar10 = uVar10 + *pbVar11;
          puVar9 = puVar9 + *pbVar11;
          if (uVar12 <= uVar10) break;
          uVar6 = (uint)pbVar11[1];
          pbVar11 = pbVar11 + 2;
          if (uVar10 < local_10) {
            uVar12 = local_10 - uVar10;
            if (uVar6 < uVar12) {
              pbVar17 = pbVar17 + uVar6;
              puVar9 = puVar9 + uVar6;
            }
            else {
              pbVar17 = pbVar17 + uVar12;
              iVar8 = uVar6 - uVar12;
              puVar9 = puVar9 + uVar12;
              if (local_18 < uVar10 + uVar6) {
                iVar13 = (iVar8 - (uVar10 + uVar6)) + local_18;
                uVar12 = iVar8 - iVar13;
                for (; iVar13 != 0; iVar13 = iVar13 + -1) {
                  uVar5 = *puVar9;
                  uVar3 = *(ushort *)((int)puVar4 + (uint)*pbVar17 * 2 + iVar1);
                  uVar14 = (uint)DAT_006e1f7c;
                  pbVar17 = pbVar17 + 1;
                  uVar16 = (uint)(uVar3 & DAT_006e1f7c) + (uint)(uVar5 & DAT_006e1f7c);
                  uVar15 = (DAT_006e1f7e & uVar3) + (DAT_006e1f7e & uVar5);
                  uVar5 = (_DAT_006e1f80 & uVar3) + (_DAT_006e1f80 & uVar5);
                  if ((uVar14 | uVar16) != uVar14) {
                    uVar16 = uVar14;
                  }
                  if ((DAT_006e1f7e | uVar15) != DAT_006e1f7e) {
                    uVar15 = DAT_006e1f7e;
                  }
                  if ((_DAT_006e1f80 | uVar5) != _DAT_006e1f80) {
                    uVar5 = _DAT_006e1f80;
                  }
                  *puVar9 = (ushort)uVar16 | uVar5 | uVar15;
                  puVar9 = puVar9 + 1;
                }
                goto LAB_0058119e;
              }
              for (; iVar8 != 0; iVar8 = iVar8 + -1) {
                uVar5 = *puVar9;
                uVar3 = *(ushort *)((int)puVar4 + (uint)*pbVar17 * 2 + iVar1);
                uVar16 = (uint)DAT_006e1f7c;
                pbVar17 = pbVar17 + 1;
                uVar12 = (uint)(uVar3 & DAT_006e1f7c) + (uint)(uVar5 & DAT_006e1f7c);
                uVar15 = (DAT_006e1f7e & uVar3) + (DAT_006e1f7e & uVar5);
                uVar5 = (_DAT_006e1f80 & uVar3) + (_DAT_006e1f80 & uVar5);
                if ((uVar16 | uVar12) != uVar16) {
                  uVar12 = uVar16;
                }
                if ((DAT_006e1f7e | uVar15) != DAT_006e1f7e) {
                  uVar15 = DAT_006e1f7e;
                }
                if ((_DAT_006e1f80 | uVar5) != _DAT_006e1f80) {
                  uVar5 = _DAT_006e1f80;
                }
                *puVar9 = (ushort)uVar12 | uVar5 | uVar15;
                puVar9 = puVar9 + 1;
              }
            }
          }
          else {
            uVar12 = uVar6;
            if (uVar10 < local_18) {
              if (uVar10 + uVar6 < local_18) {
                for (; uVar12 != 0; uVar12 = uVar12 - 1) {
                  uVar5 = *puVar9;
                  uVar3 = *(ushort *)((int)puVar4 + (uint)*pbVar17 * 2 + iVar1);
                  uVar14 = (uint)DAT_006e1f7c;
                  pbVar17 = pbVar17 + 1;
                  uVar16 = (uint)(uVar3 & DAT_006e1f7c) + (uint)(uVar5 & DAT_006e1f7c);
                  uVar15 = (DAT_006e1f7e & uVar3) + (DAT_006e1f7e & uVar5);
                  uVar5 = (_DAT_006e1f80 & uVar3) + (_DAT_006e1f80 & uVar5);
                  if ((uVar14 | uVar16) != uVar14) {
                    uVar16 = uVar14;
                  }
                  if ((DAT_006e1f7e | uVar15) != DAT_006e1f7e) {
                    uVar15 = DAT_006e1f7e;
                  }
                  if ((_DAT_006e1f80 | uVar5) != _DAT_006e1f80) {
                    uVar5 = _DAT_006e1f80;
                  }
                  *puVar9 = (ushort)uVar16 | uVar5 | uVar15;
                  puVar9 = puVar9 + 1;
                }
                goto LAB_005811a3;
              }
              iVar8 = (uVar6 - (uVar10 + uVar6)) + local_18;
              uVar12 = uVar6 - iVar8;
              for (; iVar8 != 0; iVar8 = iVar8 + -1) {
                uVar5 = *puVar9;
                bVar2 = *pbVar17;
                pbVar17 = pbVar17 + 1;
                uVar3 = *(ushort *)((int)puVar4 + (uint)bVar2 * 2 + iVar1);
                uVar14 = (uint)DAT_006e1f7c;
                uVar16 = (uint)(uVar3 & DAT_006e1f7c) + (uint)(uVar5 & DAT_006e1f7c);
                uVar15 = (DAT_006e1f7e & uVar3) + (DAT_006e1f7e & uVar5);
                uVar5 = (_DAT_006e1f80 & uVar3) + (_DAT_006e1f80 & uVar5);
                if ((uVar14 | uVar16) != uVar14) {
                  uVar16 = uVar14;
                }
                if ((DAT_006e1f7e | uVar15) != DAT_006e1f7e) {
                  uVar15 = DAT_006e1f7e;
                }
                if ((_DAT_006e1f80 | uVar5) != _DAT_006e1f80) {
                  uVar5 = _DAT_006e1f80;
                }
                *puVar9 = (ushort)uVar16 | uVar5 | uVar15;
                puVar9 = puVar9 + 1;
              }
            }
LAB_0058119e:
            puVar9 = puVar9 + uVar12;
            pbVar17 = pbVar17 + uVar12;
          }
LAB_005811a3:
          uVar10 = uVar10 + uVar6;
          uVar12 = *(uint *)(param_1 + 4);
        } while (uVar10 < uVar12);
      }
      uVar12 = *(uint *)(param_1 + 4);
      if (uVar10 != uVar12) {
        local_1c = 3;
      }
      puVar9 = puVar9 + iVar7;
      local_24 = local_24 + 2;
      local_4 = local_4 + -1;
      param_5 = 0;
    } while (local_4 != 0);
  }
LAB_005811e4:
  return CONCAT22((short)(param_5 >> 0x10),(undefined2)local_1c);
}

