
void __thiscall FUN_0058d8c0(int param_1,int *param_2)

{
  int *piVar1;
  ushort uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  ushort *puVar10;
  byte bVar11;
  ushort uVar12;
  int iVar13;
  int iVar14;
  int *local_34;
  uint local_30;
  int local_2c;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  piVar1 = param_2;
  bVar11 = 0xb - (DAT_006e1f88 != 0);
  uVar7 = 0x1f << (bVar11 & 0x1f);
  iVar13 = (-(uint)(DAT_006e1f88 != 0) & 0xfffffc00) + 0x7e0;
  iVar6 = (-(uint)(DAT_006e1f88 != 0) & 0xffffffe0) + 0x3f;
  iVar3 = (-(uint)(DAT_006e1f88 != 0) & 0xfffffff0) + 0x20;
  iVar4 = FUN_0058b660();
  param_2 = (int *)0x0;
  local_c = piVar1[3];
  local_34 = &local_10;
  do {
    local_2c = 0;
    uVar8 = piVar1[2] - *piVar1;
    uVar12 = (ushort)iVar13;
    if (0 < (int)uVar8) {
      uVar2 = uVar12 >> 6;
      local_30 = CONCAT22(local_30._2_2_,uVar2);
      uVar5 = CONCAT22((short)((uint)iVar4 >> 0x10),uVar2);
      puVar10 = (ushort *)
                (iVar4 + ((*(uint *)(param_1 + 0x18) >> 1 & 0xffff) * *local_34 + *piVar1 +
                         (int)param_2) * 2);
      iVar14 = iVar13;
      do {
        uVar2 = *puVar10;
        uVar8 = CONCAT22((short)(uVar8 >> 0x10),uVar2) & uVar7;
        iVar9 = CONCAT22((short)(uVar8 >> 0x10),(ushort)((ushort)uVar8 >> (bVar11 & 0x1f)) >> 1) +
                0x10;
        if (0x1f < (ushort)iVar9) {
          iVar9 = 0x1f;
        }
        iVar14 = (CONCAT22((short)((uint)iVar14 >> 0x10),uVar2 >> 6) & uVar5) + iVar3;
        if ((ushort)iVar6 < (ushort)iVar14) {
          iVar14 = iVar6;
        }
        uVar8 = ((byte)((byte)uVar2 >> 1) & 0xf) + 0x10;
        if (0x1f < (ushort)uVar8) {
          uVar8 = 0x1f;
        }
        uVar8 = iVar9 << (bVar11 & 0x1f) | iVar14 << 5 | uVar8;
        *puVar10 = (ushort)uVar8;
        iVar14 = *piVar1;
        local_2c = local_2c + 1;
        uVar5 = local_30;
        puVar10 = puVar10 + 1;
      } while (local_2c < piVar1[2] - iVar14);
    }
    param_2 = (int *)((int)param_2 + 1);
    local_34 = local_34 + 1;
  } while ((int)param_2 < 2);
  local_8 = *piVar1;
  local_4 = piVar1[2];
  local_34 = &local_8;
  param_2 = (int *)0x1;
  local_10 = 2;
  do {
    local_2c = 0;
    puVar10 = (ushort *)
              (iVar4 + ((*(uint *)(param_1 + 0x18) >> 1 & 0xffff) * ((int)param_2 + piVar1[1]) +
                       *local_34) * 2);
    param_2 = (int *)((int)param_2 + -1);
    uVar8 = piVar1[3] - piVar1[1];
    if (0 < (int)uVar8) {
      do {
        uVar2 = *puVar10;
        uVar8 = CONCAT22((short)(uVar8 >> 0x10),uVar2) & uVar7;
        iVar13 = CONCAT22((short)(uVar8 >> 0x10),(ushort)((ushort)uVar8 >> (bVar11 & 0x1f)) >> 1) +
                 0x10;
        if (0x1f < (ushort)iVar13) {
          iVar13 = 0x1f;
        }
        iVar14 = (uint)(ushort)((uVar2 & uVar12) >> 6) + iVar3;
        if ((ushort)iVar6 < (ushort)iVar14) {
          iVar14 = iVar6;
        }
        uVar2 = (uVar2 >> 1 & 0xf) + 0x10;
        if (0x1f < uVar2) {
          uVar2 = 0x1f;
        }
        *puVar10 = (ushort)(iVar13 << (bVar11 & 0x1f)) | (ushort)(iVar14 << 5) | uVar2;
        uVar8 = *(uint *)(param_1 + 0x18) >> 1 & 0xffff;
        local_2c = local_2c + 1;
        puVar10 = puVar10 + uVar8;
      } while (local_2c < piVar1[3] - piVar1[1]);
    }
    local_34 = local_34 + 1;
    local_10 = local_10 + -1;
  } while (local_10 != 0);
  (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),iVar4);
  return;
}

