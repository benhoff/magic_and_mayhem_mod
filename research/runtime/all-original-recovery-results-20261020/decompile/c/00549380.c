
undefined4 __fastcall FUN_00549380(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iStack_34;
  uint uStack_30;
  uint uStack_2c;
  int iStack_28;
  int iStack_24;
  
  iVar2 = FUN_005456c0();
  uVar4 = 0;
  if (iVar2 != 0) {
    iVar2 = *(int *)(param_1 + 0x1c);
    uVar4 = *(undefined4 *)(iVar2 + 0x114);
    iVar6 = *(int *)(*(int *)(iVar2 + 0xac) + 0x198);
    iVar8 = *(int *)(iVar2 + 0x18) + iVar6;
    iVar6 = *(int *)(iVar2 + 0x14) + iVar6;
    iVar9 = *(int *)(*(int *)(iVar2 + 0xac) + 0x19c) + *(int *)(iVar2 + 0x1c);
    iVar2 = DAT_005e14c0 + 1;
    uVar3 = FUN_0054e200();
    *(int *)(param_1 + 0x10) =
         *(int *)(param_1 + 0x10) +
         ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar2) - DAT_005e14c0 / 2);
    iVar2 = DAT_005e14c0 + 1;
    uVar3 = FUN_0054e200();
    *(int *)(param_1 + 0x14) =
         *(int *)(param_1 + 0x14) +
         ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar2) - DAT_005e14c0 / 2);
    iVar2 = DAT_005e14c0 + 1;
    uVar3 = FUN_0054e200();
    iVar7 = *(int *)(param_1 + 0x18) +
            ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar2) - DAT_005e14c0 / 2);
    iVar2 = *(int *)(param_1 + 0x10);
    uVar3 = (*(int *)(param_1 + 4) - iVar6) + iVar2;
    *(int *)(param_1 + 0x18) = iVar7;
    uVar5 = (int)uVar3 >> 0x1f;
    uVar5 = (uVar3 ^ uVar5) - uVar5;
    uStack_30 = DAT_005eb480 - uVar5;
    if ((int)uVar5 < (int)uStack_30) {
      iStack_34 = 1;
      uStack_30 = uVar5;
    }
    else {
      iStack_34 = -1;
    }
    uVar3 = (*(int *)(param_1 + 8) - iVar8) + *(int *)(param_1 + 0x14);
    uVar5 = (int)uVar3 >> 0x1f;
    uVar5 = (uVar3 ^ uVar5) - uVar5;
    uStack_2c = DAT_005eb484 - uVar5;
    if ((int)uVar5 < (int)uStack_2c) {
      iStack_28 = 1;
      uStack_2c = uVar5;
    }
    else {
      iStack_28 = -1;
    }
    iVar1 = *(int *)(param_1 + 0xc);
    if (iVar1 + iVar7 < iVar9) {
      iStack_24 = 1;
      uVar3 = (iVar9 - iVar1) - iVar7;
    }
    else {
      uVar3 = (iVar1 + iVar7) - iVar9;
      iStack_24 = -1;
    }
    iVar10 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
    iVar9 = (uStack_2c ^ (int)uStack_2c >> 0x1f) - ((int)uStack_2c >> 0x1f);
    iVar7 = (uStack_30 ^ (int)uStack_30 >> 0x1f) - ((int)uStack_30 >> 0x1f);
    if (3 < iVar7 + iVar9 + iVar10) {
      if (iVar7 < DAT_005e14b8) {
        iStack_34 = -iStack_34;
      }
      if (iVar9 < DAT_005e14b8) {
        iStack_28 = -iStack_28;
      }
      if (iVar10 < DAT_005e14b8) {
        iStack_24 = -iStack_24;
      }
      iVar7 = uStack_30 + uVar3 + uStack_2c;
      iVar9 = iVar7;
      if (iVar7 < 2) {
        iVar9 = 1;
      }
      iVar9 = (int)(DAT_005e14bc * uStack_30 * iStack_34) / iVar9;
      iVar10 = iVar7;
      if (iVar7 < 2) {
        iVar10 = 1;
      }
      iVar10 = (int)(DAT_005e14bc * uStack_2c * iStack_28) / iVar10;
      if (iVar7 < 2) {
        iVar7 = 1;
      }
      iVar7 = (int)(DAT_005e14bc * iStack_24 * uVar3) / iVar7;
      if (iVar6 < *(int *)(param_1 + 4)) {
        *(int *)(param_1 + 0x10) = iVar2 - iVar9;
      }
      else {
        *(int *)(param_1 + 0x10) = iVar2 + iVar9;
      }
      if (DAT_005e14bc < *(int *)(param_1 + 0x10)) {
        *(int *)(param_1 + 0x10) = DAT_005e14bc;
      }
      if (*(int *)(param_1 + 0x10) < -DAT_005e14bc) {
        *(int *)(param_1 + 0x10) = -DAT_005e14bc;
      }
      if (iVar8 < *(int *)(param_1 + 8)) {
        iVar10 = -iVar10;
      }
      *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + iVar10;
      if (DAT_005e14bc < *(int *)(param_1 + 0x14)) {
        *(int *)(param_1 + 0x14) = DAT_005e14bc;
      }
      if (*(int *)(param_1 + 0x14) < -DAT_005e14bc) {
        *(int *)(param_1 + 0x14) = -DAT_005e14bc;
      }
      if (iVar6 < iVar1) {
        iVar7 = -iVar7;
      }
      iVar7 = *(int *)(param_1 + 0x18) + iVar7;
      *(int *)(param_1 + 0x18) = iVar7;
      if (DAT_005e14bc < iVar7) {
        *(int *)(param_1 + 0x18) = DAT_005e14bc;
      }
      if (*(int *)(param_1 + 0x18) < -DAT_005e14bc) {
        *(int *)(param_1 + 0x18) = -DAT_005e14bc;
      }
    }
  }
  return uVar4;
}

