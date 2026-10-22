
void __fastcall FUN_00552f20(int param_1)

{
  undefined2 *puVar1;
  int iVar2;
  undefined2 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  short sVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int *piVar15;
  int local_28;
  int local_20;
  int local_1c;
  int local_18;
  
  *(undefined4 *)(param_1 + 0xfb) = 6;
  *(int *)(param_1 + 0xf7) = (*(int *)(param_1 + 0xeb) - *(int *)(param_1 + 0x107)) + -8;
  FUN_00553a40();
  iVar4 = *(int *)(param_1 + 0x103) / 2;
  iVar13 = *(int *)(param_1 + 0x107) / 2;
  sVar10 = (-(ushort)(DAT_006e1f88 != 0) & 0xfe00) + 0x400;
  uVar3 = FUN_00555e20(0);
  iVar11 = *(int *)(param_1 + 0xf7);
  iVar6 = *(int *)(param_1 + 0xfb) + -1;
  iVar7 = 0;
  if (*(int *)(param_1 + 0x6207) == 0) {
    local_28 = 0;
    if (iVar13 + 1 != 0) {
      do {
        iVar14 = iVar6 + iVar7;
        iVar12 = iVar7 * 2;
        iVar7 = iVar7 + 1;
        puVar1 = (undefined2 *)
                 (*(int *)(param_1 + 0xdf) +
                 (iVar11 + iVar14 * *(int *)(param_1 + 0xeb) + iVar12) * 2);
        *puVar1 = uVar3;
        puVar1[1] = uVar3;
        local_28 = iVar13 + 1;
      } while (iVar7 != iVar13 + 1);
    }
    iVar7 = iVar7 + -1;
    iVar12 = (iVar13 + 1) - (iVar4 + 1);
    if (iVar12 <= iVar7) {
      local_1c = local_28 + iVar6;
      local_28 = (iVar7 - iVar12) + 1 + local_28;
      do {
        puVar1 = (undefined2 *)
                 (*(int *)(param_1 + 0xdf) +
                 (iVar11 + local_1c * *(int *)(param_1 + 0xeb) + iVar7 * 2) * 2);
        *puVar1 = uVar3;
        puVar1[1] = uVar3;
        local_1c = local_1c + 1;
        iVar7 = iVar7 + -1;
      } while (iVar12 <= iVar7);
    }
    local_28 = local_28 + -2;
    iVar12 = -(iVar4 + 1);
    if (iVar12 < iVar7) {
      local_1c = local_28 + iVar6;
      local_28 = local_28 + (iVar12 - iVar7);
      do {
        puVar1 = (undefined2 *)
                 (*(int *)(param_1 + 0xdf) +
                 (iVar11 + local_1c * *(int *)(param_1 + 0xeb) + iVar7 * 2) * 2);
        *puVar1 = uVar3;
        puVar1[1] = uVar3;
        local_1c = local_1c + -1;
        iVar7 = iVar7 + -1;
      } while (iVar12 < iVar7);
    }
    if (iVar7 + 1 < 0) {
      iVar7 = (iVar7 + 1) * 2;
      local_28 = local_28 + iVar6;
      do {
        iVar6 = local_28 * *(int *)(param_1 + 0xeb);
        local_28 = local_28 + -1;
        puVar1 = (undefined2 *)(*(int *)(param_1 + 0xdf) + (iVar6 + iVar7 + iVar11) * 2);
        iVar7 = iVar7 + 2;
        *puVar1 = uVar3;
        puVar1[1] = uVar3;
      } while (iVar7 < 0);
    }
  }
  local_20 = 4;
  iVar11 = DAT_006c482c[2];
  iVar6 = *DAT_006c482c;
  iVar7 = DAT_006c482c[1];
  iVar12 = DAT_006c482c[3];
  iVar13 = (iVar13 + iVar4) / 2;
  piVar15 = (int *)(param_1 + 0x6121);
  do {
    iVar14 = piVar15[1];
    iVar2 = *piVar15;
    iVar9 = iVar13 + iVar2 * (((int)((iVar11 - iVar6) + (iVar11 - iVar6 >> 0x1f & 0x7fU)) >> 7) + 1)
                     * -2;
    iVar8 = (iVar13 + -1) -
            iVar14 * ((int)((iVar12 - iVar7) + (iVar12 - iVar7 >> 0x1f & 0x3fU)) >> 6);
    if (iVar14 < 0) {
      iVar8 = iVar8 + 1;
    }
    local_18 = 4;
    iVar5 = iVar9;
    local_28 = iVar8;
    do {
      *(short *)(*(int *)(param_1 + 0xdf) +
                (((*(int *)(param_1 + 0xfb) + iVar8) * *(int *)(param_1 + 0xeb) - iVar4) + iVar5 +
                *(int *)(param_1 + 0xf7)) * 2) = sVar10;
      *(short *)(*(int *)(param_1 + 0xdf) +
                (((*(int *)(param_1 + 0xfb) + local_28) * *(int *)(param_1 + 0xeb) - iVar4) + iVar9
                + *(int *)(param_1 + 0xf7)) * 2) = sVar10;
      local_28 = local_28 + iVar14;
      iVar5 = iVar5 + iVar2;
      local_18 = local_18 + -1;
    } while (local_18 != 0);
    piVar15 = piVar15 + 2;
    local_20 = local_20 + -1;
  } while (local_20 != 0);
  if (*(int *)(param_1 + 0x6141) == 0) {
    iVar11 = *(int *)(param_1 + 0x6119) + -1;
    iVar4 = -1;
    local_1c = -1;
  }
  else {
    local_1c = *(int *)(param_1 + 0x6119);
    iVar11 = 0;
    iVar4 = 1;
  }
  if (iVar11 != local_1c) {
    piVar15 = (int *)(param_1 + 0x111 + iVar11 * 0x12);
    do {
      if (piVar15[1] == 0) {
        FUN_005536c0(piVar15[-1],*piVar15,
                     *(undefined2 *)(param_1 + 0x12f9 + (*piVar15 + piVar15[-1] * 100) * 2),
                     piVar15[2]);
      }
      else {
        FUN_005536c0(piVar15[-1],*piVar15,8,piVar15[2]);
      }
      iVar11 = iVar11 + iVar4;
      piVar15 = (int *)((int)piVar15 + iVar4 * 0x12);
    } while (iVar11 != local_1c);
  }
  return;
}

