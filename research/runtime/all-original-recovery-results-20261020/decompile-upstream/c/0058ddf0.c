
void __thiscall FUN_0058ddf0(int param_1,ushort *param_2,int param_3,int param_4,int param_5)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  int local_40;
  int local_3c;
  int local_34;
  sbyte local_30;
  sbyte local_2c;
  int local_24;
  int local_10 [4];
  
  if (DAT_006e1f88 == 0) {
    local_30 = 8;
    local_2c = 3;
  }
  else {
    local_30 = 7;
    local_2c = 2;
  }
  iVar8 = *(int *)param_2 - *(int *)(param_1 + 0x20);
  iVar2 = *(int *)((int)param_2 + 8) - *(int *)(param_1 + 0x20);
  iVar10 = *(int *)((int)param_2 + 4) - *(int *)(param_1 + 0x24);
  iVar5 = *(int *)((int)param_2 + 0xc) - *(int *)(param_1 + 0x24);
  iVar3 = iVar2 + 1;
  iVar6 = iVar5 + 1;
  iVar4 = FUN_0058b660();
  local_10[1] = iVar10 + 1;
  local_10[3] = iVar5;
  iVar5 = (-(uint)(param_4 != 0) & 2) - 1;
  local_10[0] = iVar10;
  local_10[2] = iVar6;
  local_40 = 0;
  local_3c = 0;
  local_34 = iVar3;
  do {
    if (local_3c == 2) {
      if (param_5 != 0) {
        iVar5 = -iVar5;
      }
      local_40 = local_40 + 2;
      local_34 = local_34 + -2;
    }
    param_2 = (ushort *)
              (iVar4 + (local_10[local_3c] * (*(uint *)(param_1 + 0x18) >> 1 & 0xffff) + local_40 +
                       iVar8) * 2);
    local_24 = local_34 - iVar8;
    if (0 < local_24) {
      do {
        uVar1 = *param_2;
        uVar7 = (uint)(byte)(((byte)uVar1 & DAT_006e1f80) << 3);
        uVar9 = (ushort)((DAT_006e1f7c & uVar1) >> local_30) & 0xff;
        uVar11 = (ushort)((DAT_006e1f7e & uVar1) >> local_2c) & 0xff;
        uVar9 = uVar9 + ((int)(uVar9 * param_3) / 100) * iVar5;
        uVar11 = uVar11 + ((int)(uVar11 * param_3) / 100) * iVar5;
        uVar7 = uVar7 + ((int)(uVar7 * param_3) / 100) * iVar5;
        uVar9 = ((int)uVar9 < 1) - 1 & uVar9;
        if (0xfe < (int)uVar9) {
          uVar9 = 0xff;
        }
        uVar11 = ((int)uVar11 < 1) - 1 & uVar11;
        if (0xfe < (int)uVar11) {
          uVar11 = 0xff;
        }
        uVar7 = ((int)uVar7 < 1) - 1 & uVar7;
        if (0xfe < (int)uVar7) {
          uVar7 = 0xff;
        }
        if (0xff < (ushort)uVar9) {
          uVar9 = 0xff;
        }
        if (0xff < (ushort)uVar11) {
          uVar11 = 0xff;
        }
        if (0xff < (ushort)uVar7) {
          uVar7 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar1 = (ushort)(((uVar9 & 0xfff8) << 5 | uVar11 & 0xfc) << 3);
        }
        else {
          uVar1 = (ushort)(((uVar9 & 0xf8) << 5 | uVar11 & 0xf8) << 2);
        }
        *param_2 = uVar1 | (ushort)(uVar7 >> 3) & 0x1f;
        param_2 = param_2 + 1;
        local_24 = local_24 + -1;
      } while (local_24 != 0);
    }
    local_3c = local_3c + 1;
  } while (local_3c < 4);
  local_10[0] = iVar8;
  local_10[2] = iVar3;
  local_10[1] = iVar8 + 1;
  local_10[3] = iVar2;
  local_40 = 2;
  param_4 = (-(uint)(param_4 != 0) & 2) - 1;
  local_3c = 0;
  do {
    if (local_3c == 2) {
      if (param_5 != 0) {
        param_4 = -param_4;
      }
      local_40 = 0;
    }
    param_2 = (ushort *)
              (iVar4 + ((*(uint *)(param_1 + 0x18) >> 1 & 0xffff) * (local_40 + iVar10) +
                       local_10[local_3c]) * 2);
    local_24 = (iVar6 - local_40) - iVar10;
    if (0 < local_24) {
      do {
        uVar1 = *param_2;
        uVar9 = (ushort)((DAT_006e1f7c & uVar1) >> local_30) & 0xff;
        uVar7 = (uint)(byte)(((byte)uVar1 & DAT_006e1f80) << 3);
        uVar11 = (ushort)((DAT_006e1f7e & uVar1) >> local_2c) & 0xff;
        uVar9 = uVar9 + ((int)(uVar9 * param_3) / 100) * param_4;
        uVar11 = uVar11 + ((int)(uVar11 * param_3) / 100) * param_4;
        uVar7 = uVar7 + ((int)(uVar7 * param_3) / 100) * param_4;
        uVar9 = ((int)uVar9 < 1) - 1 & uVar9;
        if (0xfe < (int)uVar9) {
          uVar9 = 0xff;
        }
        uVar11 = ((int)uVar11 < 1) - 1 & uVar11;
        if (0xfe < (int)uVar11) {
          uVar11 = 0xff;
        }
        uVar7 = ((int)uVar7 < 1) - 1 & uVar7;
        if (0xfe < (int)uVar7) {
          uVar7 = 0xff;
        }
        if (0xff < (ushort)uVar9) {
          uVar9 = 0xff;
        }
        if (0xff < (ushort)uVar11) {
          uVar11 = 0xff;
        }
        if (0xff < (ushort)uVar7) {
          uVar7 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar1 = (ushort)(((uVar9 & 0xfff8) << 5 | uVar11 & 0xfc) << 3);
        }
        else {
          uVar1 = (ushort)(((uVar9 & 0xf8) << 5 | uVar11 & 0xf8) << 2);
        }
        *param_2 = uVar1 | (ushort)(uVar7 >> 3) & 0x1f;
        param_2 = param_2 + (*(uint *)(param_1 + 0x18) >> 1 & 0xffff);
        local_24 = local_24 + -1;
      } while (local_24 != 0);
    }
    local_3c = local_3c + 1;
  } while (local_3c < 4);
  (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),iVar4);
  return;
}

