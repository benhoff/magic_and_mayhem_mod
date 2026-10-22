
void __thiscall
FUN_0058e830(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int param_7,
            int param_8,int param_9,ushort *param_10,int param_11)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  ushort *puVar11;
  bool bVar12;
  int local_14;
  int local_10;
  int local_8;
  
  iVar9 = (int)param_10;
  iVar4 = param_8;
  iVar7 = param_3;
  iVar3 = param_2;
  if ((param_3 != param_5) && (param_2 != param_4)) {
    iVar10 = param_6;
    if (param_5 < param_3) {
      param_10 = (ushort *)param_7;
      param_3 = param_5;
      param_5 = iVar7;
      param_8 = param_11;
      param_11 = iVar4;
      iVar10 = param_9;
      param_9 = param_6;
      param_7 = iVar9;
    }
    if (param_4 < param_2) {
      param_2 = param_4;
      param_4 = iVar3;
    }
    iVar10 = (int)(iVar10 + (iVar10 >> 0x1f & 7U)) >> 3;
    iVar3 = (int)(param_9 + (param_9 >> 0x1f & 7U)) >> 3;
    iVar7 = (-(uint)(DAT_006e1f88 != 0) & 4) + 4;
    param_6 = param_7 / iVar7;
    iVar7 = (int)param_10 / iVar7;
    iVar4 = (int)(param_11 + (param_11 >> 0x1f & 7U)) >> 3;
    iVar9 = param_5 - param_3;
    local_8 = (int)(param_8 + (param_8 >> 0x1f & 7U)) >> 3;
    if (iVar10 < iVar3) {
      local_10 = iVar3 - iVar10;
    }
    else {
      local_10 = iVar10 - iVar3;
    }
    if (param_6 < iVar7) {
      local_14 = iVar7 - param_6;
    }
    else {
      local_14 = param_6 - iVar7;
    }
    if (local_8 < iVar4) {
      iVar5 = iVar4 - local_8;
    }
    else {
      iVar5 = local_8 - iVar4;
    }
    bVar1 = iVar7 <= param_6;
    bVar2 = iVar4 <= local_8;
    bVar12 = DAT_006e1f88 != 0;
    iVar7 = 0;
    param_11 = 0;
    param_9 = 0;
    iVar4 = FUN_0058b660();
    if (iVar4 != 0) {
      uVar6 = *(uint *)(param_1 + 0x18) >> 1;
      param_10 = (ushort *)(iVar4 + (uVar6 * param_3 + param_2) * 2);
      param_7 = iVar9;
      param_8 = iVar10;
      if (param_3 < param_5) {
        do {
          puVar11 = param_10;
          for (iVar8 = param_4 - param_2; iVar8 != 0; iVar8 = iVar8 + -1) {
            *puVar11 = (ushort)(param_8 << (0xbU - bVar12 & 0x1f)) | (ushort)(param_6 << 5) |
                       (ushort)local_8;
            puVar11 = puVar11 + 1;
          }
          param_10 = param_10 + uVar6;
          param_11 = param_11 + local_14 * 2;
          param_9 = param_9 + iVar5 * 2;
          for (iVar7 = iVar7 + local_10 * 2; iVar9 < iVar7; iVar7 = iVar7 + iVar9 * -2) {
            param_8 = param_8 + (((iVar3 <= iVar10) - 1 & 2) - 1);
          }
          for (; iVar9 < param_11; param_11 = param_11 + iVar9 * -2) {
            param_6 = param_6 + ((bVar1 - 1 & 2) - 1);
          }
          for (; iVar9 < param_9; param_9 = param_9 + iVar9 * -2) {
            local_8 = local_8 + ((bVar2 - 1 & 2) - 1);
          }
          param_7 = param_7 + -1;
        } while (param_7 != 0);
      }
      (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),iVar4);
    }
  }
  return;
}

