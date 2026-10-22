
uint __thiscall FUN_00545c10(uint *param_1,uint param_2,int param_3)

{
  ushort uVar1;
  ushort uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  ushort uVar9;
  byte bVar10;
  ushort *puVar11;
  ushort *puVar12;
  bool bVar13;
  int local_18;
  
  iVar7 = param_2;
  bVar10 = 0xb - (DAT_006e1f88 != 0);
  bVar13 = DAT_006e1f88 != 0;
  uVar1 = (-(ushort)(DAT_006e1f88 != 0) & 0xffe0) + 0x3f;
  uVar3 = param_2 - DAT_006e0008 & (0 < (int)(param_2 - DAT_006e0008)) - 1;
  iVar4 = -uVar3;
  uVar5 = param_3 - DAT_006cbb6c & (0 < param_3 - DAT_006cbb6c) - 1;
  iVar6 = -uVar5;
  uVar8 = DAT_006a49b8 - param_2;
  param_2 = *param_1;
  if ((int)uVar8 <= (int)*param_1) {
    param_2 = uVar8;
  }
  uVar8 = DAT_00656618 - param_3;
  if ((int)param_1[1] < DAT_00656618 - param_3) {
    uVar8 = param_1[1];
  }
  puVar12 = (ushort *)(DAT_00658174 + ((param_3 + iVar6) * DAT_006a2dc8 + iVar4 + iVar7) * 2);
  if (iVar6 < (int)uVar8) {
    local_18 = uVar8 + uVar5;
    do {
      if (iVar4 < (int)param_2) {
        iVar7 = param_2 + uVar3;
        puVar11 = puVar12;
        do {
          uVar9 = *puVar11;
          uVar5 = CONCAT22((short)((uint)iVar7 >> 0x10),uVar9) & 0x1f << (bVar10 & 0x1f);
          uVar2 = ((ushort)uVar5 >> (bVar10 & 0x1f)) + (short)param_1[2];
          iVar6 = CONCAT22((short)(uVar5 >> 0x10),uVar2);
          if (0x1f < uVar2) {
            iVar6 = 0x1f;
          }
          uVar2 = ((uVar9 & (-(ushort)bVar13 & 0xfc00) + 0x7e0) >> 5) + (short)param_1[3];
          if (uVar1 < uVar2) {
            uVar2 = uVar1;
          }
          uVar9 = (uVar9 & 0x1f) + (short)param_1[4];
          if (0x1f < uVar9) {
            uVar9 = 0x1f;
          }
          puVar12 = puVar11 + 1;
          *puVar11 = (ushort)(iVar6 << (bVar10 & 0x1f)) | uVar2 << 5 | uVar9;
          iVar7 = iVar7 + -1;
          puVar11 = puVar12;
        } while (iVar7 != 0);
      }
      puVar12 = puVar12 + (DAT_006a2dc8 - param_2) + iVar4;
      local_18 = local_18 + -1;
      uVar8 = param_2;
    } while (local_18 != 0);
  }
  return uVar8 & 0xffff0000;
}

