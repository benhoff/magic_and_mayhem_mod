
void __thiscall FUN_0058db80(int param_1,uint param_2,uint param_3,ushort param_4,ushort *param_5)

{
  ushort uVar1;
  int iVar2;
  byte bVar3;
  byte bVar4;
  int iVar5;
  ushort *puVar6;
  ushort uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  bool bVar11;
  bool bVar12;
  ushort local_1c;
  int local_18;
  int local_14;
  
  bVar3 = 0xb - (DAT_006e1f88 != 0);
  iVar2 = (int)param_5 * 3;
  bVar11 = DAT_006e1f88 != 0;
  bVar12 = DAT_006e1f88 != 0;
  uVar7 = (ushort)param_2;
  bVar4 = (byte)param_4;
  if (0xff < uVar7) {
    param_2 = 0xff;
  }
  uVar9 = param_3;
  if (0xff < (ushort)param_3) {
    uVar9 = 0xff;
  }
  if (0xff < param_4) {
    bVar4 = 0xff;
  }
  if (DAT_006e1f88 == 0) {
    local_1c = (ushort)(((param_2 & 0xfff8) << 5 | uVar9 & 0xfc) << 3);
  }
  else {
    local_1c = (ushort)(((param_2 & 0xf8) << 5 | uVar9 & 0xf8) << 2);
  }
  uVar7 = uVar7 >> 3;
  local_1c = local_1c | bVar4 >> 3;
  iVar5 = (int)((ulonglong)(param_3 & 0xffff) /
               (ulonglong)(longlong)(int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4));
  puVar6 = (ushort *)FUN_0058b660();
  local_14 = 0;
  param_5 = puVar6;
  if (0 < *(int *)(param_1 + 0x14)) {
    do {
      local_18 = 0;
      if (0 < *(int *)(param_1 + 0x10)) {
        do {
          uVar1 = *param_5;
          if (uVar1 != local_1c) {
            uVar9 = ((0x1f << (bVar3 & 0x1f) & 0xffffU & (uint)uVar1) >> (bVar3 & 0x1f)) -
                    (uint)uVar7;
            uVar8 = (int)uVar9 >> 0x1f;
            iVar10 = (int)(((uVar9 ^ uVar8) - uVar8) * 100) / 0x1f;
            if (((iVar10 < iVar2) &&
                (uVar9 = (((-(uint)bVar11 & 0xfffffc00) + 0x7e0 & 0xffff) >> 5 & (uint)(uVar1 >> 5))
                         - iVar5, uVar8 = (int)uVar9 >> 0x1f,
                iVar10 = iVar10 + (int)(((uVar9 ^ uVar8) - uVar8) * 100) /
                                  (int)((-(uint)bVar12 & 0xffffffe0) + 0x3f & 0xffff),
                iVar10 < iVar2)) &&
               (uVar9 = (uVar1 & 0x1f) - (uint)(param_4 >> 3), uVar8 = (int)uVar9 >> 0x1f,
               (int)(((uVar9 ^ uVar8) - uVar8) * 100) / 0x1f + iVar10 < iVar2)) {
              *param_5 = uVar7 << (bVar3 & 0x1f) | (ushort)(iVar5 << 5) | param_4 >> 3;
            }
          }
          param_5 = param_5 + 1;
          local_18 = local_18 + 1;
        } while (local_18 < *(int *)(param_1 + 0x10));
      }
      local_14 = local_14 + 1;
    } while (local_14 < *(int *)(param_1 + 0x14));
  }
  (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),puVar6);
  return;
}

