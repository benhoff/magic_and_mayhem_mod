
uint FUN_0057de00(int param_1,uint param_2,byte *param_3,int *param_4,uint param_5)

{
  byte bVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined2 *puVar4;
  int iVar5;
  byte *pbVar6;
  uint uVar7;
  byte *pbVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  undefined2 *puVar13;
  uint local_14;
  int local_10;
  undefined2 local_c;
  
  uVar12 = *(uint *)(param_1 + 8);
  local_c = 0;
  if ((uVar12 == 0) && (uVar3 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_0057e1a5:
    return uVar3 & 0xffff0000;
  }
  puVar2 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != 0; param_5 = param_5 + -1) {
    puVar2 = (undefined4 *)*puVar2;
  }
  iVar9 = (int)param_4 >> ((byte)puVar2[1] & 0x1f);
  if ((iVar9 < 0) && (puVar2[1] != 0)) {
    iVar9 = iVar9 + 1;
  }
  iVar11 = param_2 - *(int *)(param_1 + 0xc);
  uVar7 = *(uint *)(param_1 + 4);
  iVar9 = (puVar2[2] + iVar9) * 0x200 + 0xc;
  uVar3 = (int)param_3 - *(int *)(param_1 + 0x10);
  if (((((int)(uVar7 + iVar11) <= DAT_006e0008) || ((int)(uVar12 + uVar3) <= DAT_006cbb6c)) ||
      (DAT_006a49b8 <= iVar11)) || (DAT_00656618 <= (int)uVar3)) goto LAB_0057e1a5;
  param_5 = 0;
  if ((int)uVar3 < DAT_006cbb6c) {
    param_5 = DAT_006cbb6c - uVar3;
  }
  param_3 = (byte *)uVar12;
  if (DAT_00656618 < (int)(uVar12 + uVar3)) {
    param_3 = (byte *)(DAT_00656618 - uVar3);
  }
  if (uVar12 <= param_5) {
    local_c = 4;
  }
  if (uVar12 < param_3) {
    local_c = 5;
  }
  iVar5 = DAT_006a2dc8 - uVar7;
  puVar4 = (undefined2 *)(DAT_00658174 + ((uVar3 + param_5) * DAT_006a2dc8 + iVar11) * 2);
  param_4 = (int *)(param_1 + 0x28 + param_5 * 8);
  local_14 = 0;
  if (iVar11 < DAT_006e0008) {
    local_14 = DAT_006e0008 - iVar11;
  }
  if (DAT_006a49b8 < (int)(uVar7 + iVar11)) {
    param_2 = DAT_006a49b8 - iVar11;
  }
  else {
    param_2 = uVar7;
    if (iVar11 >= DAT_006e0008) {
      if (param_5 < param_3) {
        local_14 = (int)param_3 - param_5;
        do {
          param_3 = (byte *)(param_4[1] + param_1);
          pbVar6 = (byte *)(param_1 + *param_4);
          uVar12 = 0;
          if (uVar7 != 0) {
            do {
              uVar12 = uVar12 + *pbVar6;
              puVar4 = puVar4 + *pbVar6;
              if (uVar7 <= uVar12) break;
              uVar3 = (uint)pbVar6[1];
              pbVar6 = pbVar6 + 2;
              uVar12 = uVar12 + uVar3;
              puVar13 = puVar4;
              do {
                puVar4 = puVar13 + 1;
                *puVar13 = *(undefined2 *)((int)puVar2 + (uint)*param_3 * 2 + iVar9);
                param_3 = param_3 + 1;
                uVar3 = uVar3 - 1;
                puVar13 = puVar4;
              } while (uVar3 != 0);
              uVar7 = *(uint *)(param_1 + 4);
            } while (uVar12 < uVar7);
          }
          param_4 = param_4 + 2;
          puVar4 = puVar4 + iVar5;
          local_14 = local_14 + -1;
          if (local_14 == 0) {
            return CONCAT22((short)((uint)puVar4 >> 0x10),local_c);
          }
        } while( true );
      }
      goto LAB_0057e19a;
    }
  }
  if (DAT_006e1f68 != 0) {
    return CONCAT22((short)((uint)puVar4 >> 0x10),local_c);
  }
  if (param_5 < param_3) {
    local_10 = (int)param_3 - param_5;
    do {
      pbVar8 = (byte *)(param_1 + *param_4);
      pbVar6 = (byte *)(param_4[1] + param_1);
      uVar12 = 0;
      if (uVar7 != 0) {
        do {
          uVar12 = uVar12 + *pbVar8;
          puVar4 = puVar4 + *pbVar8;
          if (uVar7 <= uVar12) break;
          uVar3 = (uint)pbVar8[1];
          pbVar8 = pbVar8 + 2;
          if (uVar12 < local_14) {
            uVar7 = local_14 - uVar12;
            if (uVar3 < uVar7) {
              pbVar6 = pbVar6 + uVar3;
              puVar4 = puVar4 + uVar3;
            }
            else {
              puVar4 = puVar4 + uVar7;
              pbVar6 = pbVar6 + uVar7;
              iVar10 = uVar3 - uVar7;
              if (param_2 < uVar12 + uVar3) {
                iVar11 = (iVar10 - (uVar12 + uVar3)) + param_2;
                iVar10 = iVar10 - iVar11;
                for (; iVar11 != 0; iVar11 = iVar11 + -1) {
                  bVar1 = *pbVar6;
                  pbVar6 = pbVar6 + 1;
                  *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar9);
                  puVar4 = puVar4 + 1;
                }
LAB_0057e155:
                puVar4 = puVar4 + iVar10;
                pbVar6 = pbVar6 + iVar10;
              }
              else {
                for (; iVar10 != 0; iVar10 = iVar10 + -1) {
                  bVar1 = *pbVar6;
                  pbVar6 = pbVar6 + 1;
                  *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar9);
                  puVar4 = puVar4 + 1;
                }
              }
            }
          }
          else if (uVar12 < param_2) {
            uVar7 = uVar3;
            if (param_2 <= uVar12 + uVar3) {
              iVar11 = (uVar3 - (uVar12 + uVar3)) + param_2;
              iVar10 = uVar3 - iVar11;
              for (; iVar11 != 0; iVar11 = iVar11 + -1) {
                bVar1 = *pbVar6;
                pbVar6 = pbVar6 + 1;
                *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar9);
                puVar4 = puVar4 + 1;
              }
              goto LAB_0057e155;
            }
            for (; uVar7 != 0; uVar7 = uVar7 - 1) {
              bVar1 = *pbVar6;
              pbVar6 = pbVar6 + 1;
              *puVar4 = *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar9);
              puVar4 = puVar4 + 1;
            }
          }
          else {
            puVar4 = puVar4 + uVar3;
            pbVar6 = pbVar6 + uVar3;
          }
          uVar12 = uVar12 + uVar3;
          uVar7 = *(uint *)(param_1 + 4);
        } while (uVar12 < uVar7);
      }
      uVar7 = *(uint *)(param_1 + 4);
      if (uVar12 != uVar7) {
        local_c = 3;
      }
      param_4 = param_4 + 2;
      puVar4 = puVar4 + iVar5;
      local_10 = local_10 + -1;
    } while (local_10 != 0);
  }
LAB_0057e19a:
  return CONCAT22((short)((uint)puVar4 >> 0x10),local_c);
}

