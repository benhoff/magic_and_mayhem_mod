
bool __fastcall FUN_00548860(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  uint uVar4;
  bool bVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  ushort *local_28;
  int local_14;
  int local_10;
  
  bVar5 = false;
  uVar1 = *(undefined4 *)(param_1 + 8);
  uVar2 = *(undefined4 *)(param_1 + 4);
  uVar3 = *(undefined4 *)(param_1 + 0xc);
  iVar6 = FUN_005456c0();
  *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + -1;
  if ((iVar6 == 0) || (uVar4 = *(uint *)(param_1 + 0xc), (int)uVar4 < 1)) {
    return false;
  }
  if ((int)uVar4 < DAT_005eb488) {
    uVar8 = *(uint *)(param_1 + 4) >> 5;
    uVar9 = *(uint *)(param_1 + 8) >> 5;
    uVar7 = uVar4 >> 4;
    if (((uVar8 == *(uint *)(param_1 + 0x32)) && (uVar9 == *(uint *)(param_1 + 0x36))) &&
       (uVar7 == *(uint *)(param_1 + 0x3a))) {
      local_14 = *(int *)(param_1 + 0x46);
      local_28 = *(ushort **)(param_1 + 0x3e);
    }
    else {
      bVar5 = true;
      local_28 = (ushort *)
                 (DAT_006c54dc + ((&DAT_006cb8c2)[uVar7] + (&DAT_006cb942)[uVar9] + uVar8) * 0xc);
      local_14 = (int)*(char *)((&DAT_006cb8c2)[uVar4 >> 5] + (&DAT_006cb942)[uVar9] + DAT_006c5c5c
                               + uVar8);
    }
    if (*local_28 == 0) {
      if ((*(byte *)((int)local_28 + 0xb) & 0x40) != 0) goto LAB_005489bb;
    }
    else {
      local_10 = DAT_0065660c + (uint)*local_28 * 0x164;
      iVar6 = FUN_004e10e0(*(uint *)(param_1 + 4) + uVar8 * -0x20 >> 2,
                           *(uint *)(param_1 + 8) + uVar9 * -0x20 >> 2,uVar4 + uVar7 * -0x10 >> 2);
      if (iVar6 != 0) {
LAB_005489bb:
        iVar10 = *(int *)(param_1 + 0x56) + -1;
        *(undefined4 *)(param_1 + 8) = uVar1;
        *(undefined4 *)(param_1 + 4) = uVar2;
        *(undefined4 *)(param_1 + 0x10) = 0;
        *(undefined4 *)(param_1 + 0x14) = 0;
        *(undefined4 *)(param_1 + 0x18) = 0;
        *(undefined4 *)(param_1 + 0xc) = uVar3;
        *(int *)(param_1 + 0x56) = iVar10;
        iVar6 = 4 - (iVar10 * 4) / DAT_005e1428;
        if (3 < iVar6) {
          iVar6 = 3;
        }
        *(int *)(param_1 + 0x1c) = iVar6;
        return 0 < iVar10;
      }
    }
    if (bVar5) {
      if (*(int *)(param_1 + 0x46) != local_14) {
        *(int *)(param_1 + 0x46) = local_14;
        local_14 = local_14 + 0x7f;
        FUN_00547890((*(int *)(param_1 + 0x4a) * local_14) / 0x7f,
                     (*(int *)(param_1 + 0x4e) * local_14) / 0x7f,
                     (*(int *)(param_1 + 0x52) * local_14) / 0x7f);
      }
      *(int *)(param_1 + 0x42) = local_10;
      *(ushort **)(param_1 + 0x3e) = local_28;
      *(uint *)(param_1 + 0x32) = uVar8;
      *(uint *)(param_1 + 0x36) = uVar9;
      *(uint *)(param_1 + 0x3a) = uVar7;
    }
  }
  return true;
}

