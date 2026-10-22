
void __thiscall FUN_0058e2c0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  ushort uVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  int *piVar7;
  ushort uVar8;
  byte bVar9;
  ushort uVar10;
  ushort *puVar11;
  bool bVar12;
  bool bVar13;
  
  bVar9 = 0xb - (DAT_006e1f88 != 0);
  bVar12 = DAT_006e1f88 != 0;
  uVar3 = (-(ushort)(DAT_006e1f88 != 0) & 0xffe0) + 0x3f;
  bVar13 = DAT_006e1f88 != 0;
  iVar1 = FUN_0058b660();
  piVar7 = (int *)param_2[1];
  if ((int)piVar7 < param_2[3]) {
    do {
      iVar2 = *param_2;
      piVar4 = piVar7;
      puVar11 = (ushort *)
                (iVar1 + ((*(uint *)(param_1 + 0x18) >> 1 & 0xffff) * (int)piVar7 + iVar2) * 2);
      if (iVar2 < param_2[2]) {
        do {
          uVar8 = *puVar11;
          uVar5 = CONCAT22((short)((uint)piVar4 >> 0x10),uVar8) & 0x1f << (bVar9 & 0x1f);
          iVar6 = CONCAT22((short)(uVar5 >> 0x10),(ushort)((ushort)uVar5 >> (bVar9 & 0x1f)) >> 1) +
                  0x10;
          if (0x1f < (ushort)iVar6) {
            iVar6 = 0x1f;
          }
          uVar10 = ((uVar8 & (-(ushort)bVar12 & 0xfc00) + 0x7e0) >> 6) +
                   (-(ushort)bVar13 & 0xfff0) + 0x20;
          if (uVar3 < uVar10) {
            uVar10 = uVar3;
          }
          uVar8 = (uVar8 >> 1 & 0xf) + 0x10;
          if (0x1f < uVar8) {
            uVar8 = 0x1f;
          }
          iVar2 = iVar2 + 1;
          *puVar11 = (ushort)(iVar6 << (bVar9 & 0x1f)) | uVar10 << 5 | uVar8;
          piVar4 = param_2;
          puVar11 = puVar11 + 1;
        } while (iVar2 < param_2[2]);
      }
      piVar7 = (int *)((int)piVar7 + 1);
    } while ((int)piVar7 < param_2[3]);
  }
  (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),iVar1);
  return;
}

