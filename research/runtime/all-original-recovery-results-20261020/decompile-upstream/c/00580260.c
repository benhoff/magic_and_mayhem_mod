
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall FUN_00580260(int param_1,int param_2,int param_3,int param_4,int *param_5)

{
  byte bVar1;
  undefined4 *puVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  byte *pbVar12;
  int iVar13;
  uint uVar14;
  byte *pbVar15;
  int *piVar16;
  undefined2 local_20;
  int *local_1c;
  int *local_18;
  uint local_14;
  byte *local_10;
  uint local_c;
  
  uVar6 = DAT_005e15f8;
  uVar14 = 0;
  local_20 = 0;
  if ((DAT_005f1450 == 32000) && ((DAT_005e15f8 & 0x7fffffff) != 0)) {
    piVar16 = &DAT_005f1450;
    do {
      fsin((float10)uVar14 * (float10)_DAT_005c7858);
      iVar4 = __ftol();
      uVar14 = uVar14 + 1;
      *piVar16 = iVar4 + uVar6;
      piVar16 = piVar16 + 1;
    } while (uVar14 < uVar6 * 2);
  }
  piVar16 = *(int **)(param_1 + 8);
  if ((piVar16 == (int *)0x0) && (uVar6 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_005806e0:
    return uVar6 & 0xffff0000;
  }
  puVar2 = *(undefined4 **)(param_1 + 0x1c);
  for (; param_5 != (int *)0x0; param_5 = (int *)((int)param_5 + -1)) {
    puVar2 = (undefined4 *)*puVar2;
  }
  param_4 = param_4 >> ((byte)puVar2[1] & 0x1f);
  if ((param_4 < 0) && (puVar2[1] != 0)) {
    param_4 = param_4 + 1;
  }
  iVar4 = (puVar2[2] + param_4) * 0x200 + 0xc;
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  param_3 = param_3 - *(int *)(param_1 + 0x10);
  uVar14 = *(uint *)(param_1 + 4);
  uVar6 = uVar14 + param_2;
  if (((((int)(uVar14 + param_2) <= DAT_006e0008) ||
       (uVar6 = DAT_006cbb6c, param_3 + (int)piVar16 <= (int)DAT_006cbb6c)) ||
      (DAT_006a49b8 <= param_2)) || (DAT_00656618 <= param_3)) goto LAB_005806e0;
  local_18 = (int *)0x0;
  if (param_3 < (int)DAT_006cbb6c) {
    local_18 = (int *)(DAT_006cbb6c - param_3);
  }
  param_5 = piVar16;
  if (DAT_00656618 < param_3 + (int)piVar16) {
    param_5 = (int *)(DAT_00656618 - param_3);
  }
  if (piVar16 <= local_18) {
    local_20 = 4;
  }
  if (piVar16 < param_5) {
    local_20 = 5;
  }
  iVar5 = DAT_006a2dc8 - uVar14;
  piVar16 = (int *)(param_1 + 0x28 + (int)local_18 * 8);
  iVar8 = DAT_00658174 + (((int)local_18 + param_3) * DAT_006a2dc8 + param_2) * 2;
  local_c = 0;
  if (param_2 < DAT_006e0008) {
    local_c = DAT_006e0008 - param_2;
  }
  piVar3 = piVar16;
  if (DAT_006a49b8 < (int)(param_2 + uVar14)) {
    local_14 = DAT_006a49b8 - param_2;
  }
  else {
    local_14 = uVar14;
    if (param_2 >= DAT_006e0008) {
      local_1c = local_18;
      if (local_18 < param_5) {
        do {
          pbVar12 = (byte *)(*piVar16 + param_1);
          pbVar15 = (byte *)(piVar16[1] + param_1);
          iVar13 = (&DAT_005f1450)
                   [(uint)((int)local_1c + param_3 + DAT_006c4830) % (DAT_005e15f8 * 2)];
          uVar6 = 0;
          if (uVar14 != 0) {
            do {
              uVar6 = uVar6 + *pbVar12;
              iVar8 = iVar8 + (uint)*pbVar12 * 2;
              if (uVar14 <= uVar6) break;
              uVar14 = (uint)pbVar12[1];
              pbVar12 = pbVar12 + 2;
              uVar6 = uVar6 + uVar14;
              for (; uVar14 != 0; uVar14 = uVar14 - 1) {
                bVar1 = *pbVar15;
                pbVar15 = pbVar15 + 1;
                *(undefined2 *)(iVar8 + iVar13 * 2) =
                     *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
                iVar8 = iVar8 + 2;
              }
              uVar14 = *(uint *)(param_1 + 4);
            } while (uVar6 < uVar14);
          }
          uVar14 = *(uint *)(param_1 + 4);
          if (uVar6 != uVar14) {
            local_20 = 3;
          }
          iVar8 = iVar8 + iVar5 * 2;
          piVar16 = piVar16 + 2;
          local_1c = (int *)((int)local_1c + 1);
          if (param_5 <= local_1c) {
            return CONCAT22((short)((uint)piVar16 >> 0x10),local_20);
          }
        } while( true );
      }
      goto LAB_005806d1;
    }
  }
  while (local_18 < param_5) {
    local_10 = (byte *)(*piVar3 + param_1);
    pbVar15 = (byte *)(piVar3[1] + param_1);
    uVar11 = 0;
    uVar6 = DAT_005e15f8 - (uint)((int)local_18 + param_3 + DAT_006c4830) % (DAT_005e15f8 * 2);
    uVar9 = (int)uVar6 >> 0x1f;
    iVar13 = (uVar6 ^ uVar9) - uVar9;
    if (uVar14 != 0) {
      do {
        uVar11 = uVar11 + *local_10;
        iVar8 = iVar8 + (uint)*local_10 * 2;
        if (uVar14 <= uVar11) break;
        uVar6 = (uint)local_10[1];
        local_10 = local_10 + 2;
        if (uVar11 < local_c) {
          uVar14 = local_c - uVar11;
          if (uVar6 < uVar14) {
            pbVar15 = pbVar15 + uVar6;
            iVar8 = iVar8 + uVar6 * 2;
          }
          else {
            pbVar15 = pbVar15 + uVar14;
            iVar10 = uVar6 - uVar14;
            iVar8 = iVar8 + uVar14 * 2;
            if (local_14 < uVar11 + uVar6) {
              iVar7 = (iVar10 - (uVar11 + uVar6)) + local_14;
              uVar14 = iVar10 - iVar7;
              for (; iVar7 != 0; iVar7 = iVar7 + -1) {
                bVar1 = *pbVar15;
                pbVar15 = pbVar15 + 1;
                *(undefined2 *)(iVar8 + iVar13 * 2) =
                     *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
                iVar8 = iVar8 + 2;
              }
              goto LAB_00580683;
            }
            for (; iVar10 != 0; iVar10 = iVar10 + -1) {
              bVar1 = *pbVar15;
              pbVar15 = pbVar15 + 1;
              *(undefined2 *)(iVar8 + iVar13 * 2) =
                   *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
              iVar8 = iVar8 + 2;
            }
          }
        }
        else {
          uVar14 = uVar6;
          if (uVar11 < local_14) {
            if (uVar11 + uVar6 < local_14) {
              for (; uVar14 != 0; uVar14 = uVar14 - 1) {
                bVar1 = *pbVar15;
                pbVar15 = pbVar15 + 1;
                *(undefined2 *)(iVar8 + iVar13 * 2) =
                     *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
                iVar8 = iVar8 + 2;
              }
              goto LAB_00580688;
            }
            iVar10 = (uVar6 - (uVar11 + uVar6)) + local_14;
            uVar14 = uVar6 - iVar10;
            for (; iVar10 != 0; iVar10 = iVar10 + -1) {
              bVar1 = *pbVar15;
              pbVar15 = pbVar15 + 1;
              *(undefined2 *)(iVar8 + iVar13 * 2) =
                   *(undefined2 *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
              iVar8 = iVar8 + 2;
            }
          }
LAB_00580683:
          iVar8 = iVar8 + uVar14 * 2;
          pbVar15 = pbVar15 + uVar14;
        }
LAB_00580688:
        uVar11 = uVar11 + uVar6;
        uVar14 = *(uint *)(param_1 + 4);
      } while (uVar11 < uVar14);
    }
    uVar14 = *(uint *)(param_1 + 4);
    if (uVar11 != uVar14) {
      local_20 = 3;
    }
    iVar8 = iVar8 + iVar5 * 2;
    piVar16 = (int *)((int)local_18 + 1);
    piVar3 = piVar3 + 2;
    local_18 = piVar16;
  }
LAB_005806d1:
  return CONCAT22((short)((uint)piVar16 >> 0x10),local_20);
}

