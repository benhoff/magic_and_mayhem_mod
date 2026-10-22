
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0058eac0(int param_1,ushort param_2,ushort param_3,ushort param_4,int *param_5)

{
  ushort uVar1;
  ushort uVar2;
  int iVar3;
  uint uVar4;
  short sVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  ushort *puVar10;
  int local_10 [3];
  undefined4 local_4;
  
  if (0xff < param_2) {
    param_2 = 0xff;
  }
  if (0xff < param_3) {
    param_3 = 0xff;
  }
  if (0xff < param_4) {
    param_4 = 0xff;
  }
  if (DAT_006e1f88 == 0) {
    uVar1 = ((param_2 & 0xfff8) << 5 | param_3 & 0xfc) << 3 | (ushort)(byte)((byte)param_4 >> 3);
  }
  else {
    uVar1 = ((param_2 & 0xf8) << 5 | param_3 & 0xf8) << 2 | (ushort)(byte)((byte)param_4 >> 3);
  }
  local_10[2] = *(undefined4 *)(param_1 + 0x10);
  local_4 = *(undefined4 *)(param_1 + 0x14);
  local_10[0] = 0;
  local_10[1] = 0;
  if (param_5 == (int *)0x0) {
    param_5 = local_10;
  }
  iVar3 = FUN_0058b660();
  iVar6 = param_5[1];
  if (iVar6 < param_5[3]) {
    do {
      iVar9 = *param_5;
      puVar10 = (ushort *)(iVar3 + ((*(uint *)(param_1 + 0x18) >> 1 & 0xffff) * iVar6 + iVar9) * 2);
      if (iVar9 < param_5[2]) {
        do {
          if (*puVar10 != uVar1) {
            uVar4 = __ftol();
            sVar5 = (short)uVar4;
            if (sVar5 == 0) {
              uVar7 = 0;
            }
            else {
              uVar7 = uVar4 & 0xffff;
              if (0xfe < uVar7) {
                uVar7 = 0xff;
              }
            }
            if (sVar5 == 0) {
              uVar8 = 0;
            }
            else {
              uVar8 = uVar4 & 0xffff;
              if (0xfe < uVar8) {
                uVar8 = 0xff;
              }
            }
            if (sVar5 == 0) {
              uVar4 = 0;
            }
            else {
              uVar4 = uVar4 & 0xffff;
              if (0xfe < uVar4) {
                uVar4 = 0xff;
              }
            }
            if (0xff < (ushort)uVar7) {
              uVar7 = 0xff;
            }
            if (0xff < (ushort)uVar8) {
              uVar8 = 0xff;
            }
            if (0xff < (ushort)uVar4) {
              uVar4 = 0xff;
            }
            if (DAT_006e1f88 == 0) {
              uVar2 = (ushort)(((uVar7 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
            }
            else {
              uVar2 = (ushort)(((uVar7 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
            }
            *puVar10 = uVar2 | (ushort)(uVar4 >> 3) & 0x1f;
          }
          puVar10 = puVar10 + 1;
          iVar9 = iVar9 + 1;
        } while (iVar9 < param_5[2]);
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 < param_5[3]);
  }
  (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),iVar3);
  return;
}

