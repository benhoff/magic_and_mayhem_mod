
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall FUN_005806f0(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  undefined2 *puVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  byte *pbVar8;
  int *piVar9;
  uint uVar10;
  int *local_1c;
  uint local_18;
  undefined2 local_14;
  uint local_10;
  uint local_8;
  
  iVar7 = 0;
  local_14 = 0;
  if (((param_4 != 0) || (param_4 = DAT_005e15f8, DAT_005e15f8 != 0)) && (DAT_005f14d0 == 32000)) {
    piVar9 = &DAT_005f14d0;
    do {
      local_10 = 0;
      do {
        fsin((float10)(int)local_10 * (float10)_DAT_005c7858);
        iVar1 = __ftol();
        local_10 = local_10 + 1;
        *piVar9 = iVar1 + iVar7;
        piVar9 = piVar9 + 1;
      } while ((int)local_10 < 0x10);
      iVar7 = iVar7 + 1;
    } while ((int)piVar9 < 0x5f18d0);
  }
  uVar6 = *(uint *)(param_1 + 8);
  if ((uVar6 == 0) && (uVar10 = 0, *(int *)(param_1 + 4) == 0)) {
LAB_00580a77:
    return uVar10 & 0xffff0000;
  }
  param_2 = param_2 - *(int *)(param_1 + 0xc);
  uVar3 = *(uint *)(param_1 + 4);
  param_3 = param_3 - *(int *)(param_1 + 0x10);
  uVar10 = uVar3 + param_2;
  if ((((int)(uVar3 + param_2) <= DAT_006e0008) ||
      ((uVar10 = DAT_006cbb6c, (int)(param_3 + uVar6) <= (int)DAT_006cbb6c ||
       (DAT_006a49b8 <= param_2)))) || (DAT_00656618 <= param_3)) goto LAB_00580a77;
  uVar10 = 0;
  if (param_3 < (int)DAT_006cbb6c) {
    uVar10 = DAT_006cbb6c - param_3;
  }
  local_18 = uVar6;
  if (DAT_00656618 < (int)(param_3 + uVar6)) {
    local_18 = DAT_00656618 - param_3;
  }
  if (uVar6 <= uVar10) {
    local_14 = 4;
  }
  if (uVar6 < local_18) {
    local_14 = 5;
  }
  iVar7 = DAT_006a2dc8 - uVar3;
  local_1c = (int *)(param_1 + 0x28 + uVar10 * 8);
  local_8 = 0;
  puVar4 = (undefined2 *)(DAT_00658174 + ((uVar10 + param_3) * DAT_006a2dc8 + param_2) * 2);
  if (param_2 < DAT_006e0008) {
    local_8 = DAT_006e0008 - param_2;
  }
  uVar6 = uVar10;
  if (DAT_006a49b8 < (int)(uVar3 + param_2)) {
    local_10 = DAT_006a49b8 - param_2;
  }
  else {
    local_10 = uVar3;
    if (param_2 >= DAT_006e0008) {
      if (uVar10 < local_18) {
        do {
          pbVar8 = (byte *)(*local_1c + param_1);
          uVar6 = 0;
          iVar1 = (&DAT_005f14d0)
                  [(uVar10 + DAT_006c4830 + param_3) % (uint)(param_4 * 2) + param_4 * 0x10];
          if (uVar3 != 0) {
            do {
              uVar6 = uVar6 + *pbVar8;
              puVar4 = puVar4 + *pbVar8;
              if (uVar3 <= uVar6) break;
              uVar3 = (uint)pbVar8[1];
              pbVar8 = pbVar8 + 2;
              uVar6 = uVar6 + uVar3;
              for (; uVar3 != 0; uVar3 = uVar3 - 1) {
                *puVar4 = puVar4[iVar1];
                puVar4 = puVar4 + 1;
              }
              uVar3 = *(uint *)(param_1 + 4);
            } while (uVar6 < uVar3);
          }
          uVar3 = *(uint *)(param_1 + 4);
          if (uVar6 != uVar3) {
            local_14 = 3;
          }
          local_1c = local_1c + 2;
          puVar4 = puVar4 + iVar7;
          uVar10 = uVar10 + 1;
          if (local_18 <= uVar10) {
            return CONCAT22((short)(uVar10 >> 0x10),local_14);
          }
        } while( true );
      }
      goto LAB_00580a4b;
    }
  }
  for (; uVar10 = local_18, uVar6 < local_18; uVar6 = uVar6 + 1) {
    pbVar8 = (byte *)(*local_1c + param_1);
    uVar10 = 0;
    if (uVar3 != 0) {
      do {
        uVar10 = uVar10 + *pbVar8;
        puVar4 = puVar4 + *pbVar8;
        uVar2 = 8 - (DAT_006c4830 + uVar6 + param_3 & 0xf);
        uVar5 = (int)uVar2 >> 0x1f;
        iVar1 = (uVar2 ^ uVar5) - uVar5;
        if (uVar3 <= uVar10) break;
        uVar3 = (uint)pbVar8[1];
        pbVar8 = pbVar8 + 2;
        if (uVar10 < local_8) {
          uVar2 = local_8 - uVar10;
          if (uVar3 < uVar2) {
            puVar4 = puVar4 + uVar3;
          }
          else {
            puVar4 = puVar4 + uVar2;
            uVar2 = uVar3 - uVar2;
            if (local_10 < uVar10 + uVar3) goto LAB_005809fb;
            for (; uVar2 != 0; uVar2 = uVar2 - 1) {
              *puVar4 = puVar4[iVar1];
              puVar4 = puVar4 + 1;
            }
          }
        }
        else {
          uVar2 = uVar3;
          if ((uVar10 < local_10) && (uVar10 + uVar3 < local_10)) {
            for (; uVar2 != 0; uVar2 = uVar2 - 1) {
              *puVar4 = puVar4[iVar1];
              puVar4 = puVar4 + 1;
            }
          }
          else {
LAB_005809fb:
            puVar4 = puVar4 + uVar2;
          }
        }
        uVar10 = uVar10 + uVar3;
        uVar3 = *(uint *)(param_1 + 4);
      } while (uVar10 < uVar3);
    }
    uVar3 = *(uint *)(param_1 + 4);
    if (uVar10 != uVar3) {
      local_14 = 3;
    }
    local_1c = local_1c + 2;
    puVar4 = puVar4 + iVar7;
  }
LAB_00580a4b:
  return CONCAT22((short)(uVar10 >> 0x10),local_14);
}

