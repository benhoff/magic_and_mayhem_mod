
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall
FUN_00581ec0(int param_1,int param_2,uint param_3,undefined4 param_4,undefined4 param_5,int param_6)

{
  float fVar1;
  short sVar2;
  short sVar3;
  ushort uVar4;
  float *pfVar5;
  float *pfVar6;
  int iVar7;
  byte *pbVar8;
  uint uVar9;
  int *piVar10;
  uint uVar11;
  uint uVar12;
  ushort *puVar13;
  bool bVar14;
  undefined2 local_24;
  uint local_20;
  
  bVar14 = DAT_006e1f88 != 0;
  local_24 = 0;
  pfVar6 = (float *)((uint)(ushort)((int)(param_6 + (param_6 >> 0x1f & 7U)) >> 0x13) << 0x10);
  if (_DAT_006f5eac == _DAT_005c5bb0) {
    local_20 = 0;
    pfVar5 = (float *)&DAT_006f5dac;
    do {
      fVar1 = (float)(int)local_20;
      pfVar6 = pfVar5 + 1;
      local_20 = local_20 + 1;
      *pfVar5 = fVar1 * _DAT_005c7860;
      pfVar5 = pfVar6;
    } while ((int)pfVar6 < 0x6f5eac);
  }
  uVar12 = *(uint *)(param_1 + 8);
  if ((uVar12 != 0) || (*(int *)(param_1 + 4) != 0)) {
    param_2 = param_2 - *(int *)(param_1 + 0xc);
    pfVar6 = (float *)(param_3 - *(int *)(param_1 + 0x10));
    uVar9 = *(uint *)(param_1 + 4);
    if ((DAT_006e0008 < (int)(param_2 + uVar9)) &&
       (((DAT_006cbb6c < (int)(uVar12 + (int)pfVar6) && (param_2 < (int)DAT_006a49b8)) &&
        ((int)pfVar6 < DAT_00656618)))) {
      uVar11 = 0;
      if ((int)pfVar6 < DAT_006cbb6c) {
        uVar11 = DAT_006cbb6c - (int)pfVar6;
      }
      param_3 = uVar12;
      if (DAT_00656618 < (int)(uVar12 + (int)pfVar6)) {
        param_3 = DAT_00656618 - (int)pfVar6;
      }
      if (uVar12 <= uVar11) {
        local_24 = 4;
      }
      if (uVar12 < param_3) {
        local_24 = 5;
      }
      iVar7 = DAT_006a2dc8 - uVar9;
      puVar13 = (ushort *)(DAT_00658174 + ((uVar11 + (int)pfVar6) * DAT_006a2dc8 + param_2) * 2);
      piVar10 = (int *)(param_1 + 0x28 + uVar11 * 8);
      local_20 = (uint)(param_2 < DAT_006e0008);
      uVar12 = DAT_006a49b8;
      if ((((int)(param_2 + uVar9) <= (int)DAT_006a49b8) && (uVar12 = local_20, local_20 == 0)) &&
         (uVar12 = param_3, uVar11 < param_3)) {
        local_20 = param_3 - uVar11;
        do {
          pbVar8 = (byte *)(*piVar10 + param_1);
          uVar12 = 0;
          if (uVar9 != 0) {
            do {
              uVar12 = uVar12 + *pbVar8;
              puVar13 = puVar13 + *pbVar8;
              if (uVar9 <= uVar12) break;
              uVar9 = (uint)pbVar8[1];
              pbVar8 = pbVar8 + 2;
              uVar12 = uVar12 + uVar9;
              for (; uVar9 != 0; uVar9 = uVar9 - 1) {
                sVar2 = __ftol();
                sVar3 = __ftol();
                uVar4 = __ftol();
                *puVar13 = sVar2 << (0xbU - bVar14 & 0x1f) | sVar3 << 5 | uVar4;
                puVar13 = puVar13 + 1;
              }
              uVar9 = *(uint *)(param_1 + 4);
            } while (uVar12 < uVar9);
          }
          uVar9 = *(uint *)(param_1 + 4);
          if (uVar12 != uVar9) {
            local_24 = 3;
          }
          piVar10 = piVar10 + 2;
          puVar13 = puVar13 + iVar7;
          local_20 = local_20 + -1;
          uVar12 = 0;
        } while (local_20 != 0);
      }
      return CONCAT22((short)(uVar12 >> 0x10),local_24);
    }
  }
  return (uint)pfVar6 & 0xffff0000;
}

