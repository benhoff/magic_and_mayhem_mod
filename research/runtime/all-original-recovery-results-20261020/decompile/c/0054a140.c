
int __fastcall FUN_0054a140(int param_1)

{
  void **ppvVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int *piVar12;
  int iStack_24;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_005c29bb;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  if ((*(int *)(param_1 + 0x56) == DAT_005e1428) &&
     (iStack_24 = 0, ExceptionList = &pvStack_c, ppvVar1 = &pvStack_c, 0 < *(int *)(param_1 + 0x5e))
     ) {
    do {
      ExceptionList = ppvVar1;
      iVar2 = FUN_00597850(0x5a);
      piVar12 = (int *)0x0;
      uStack_4 = 0;
      if (iVar2 != 0) {
        uVar3 = FUN_0054e200();
        uVar4 = FUN_0054e200();
        uVar5 = FUN_0054e200();
        uVar6 = FUN_0054e200();
        uVar7 = FUN_0054e200();
        uVar8 = FUN_0054e200();
        piVar12 = (int *)FUN_00548650(((uVar8 & 0x7fffffff) % 5 - 2) + *(int *)(param_1 + 4),
                                      ((uVar7 & 0x7fffffff) % 5 - 2) + *(int *)(param_1 + 8),
                                      ((uVar6 & 0x7fffffff) % 5 - 2) + *(int *)(param_1 + 0xc),
                                      (uVar5 & 0x7fffffff) % 5 - 2,(uVar4 & 0x7fffffff) % 5 - 2,
                                      (uVar3 & 0x7fffffff) % 5 - 2);
      }
      uStack_4 = 0xffffffff;
      if (piVar12 != (int *)0x0) {
        uVar3 = FUN_0054e200();
        iVar2 = DAT_005e1428;
        if (DAT_005e1424 < (int)((uVar3 & 0x7fffffff) % 100)) {
          uVar3 = FUN_0054e200();
          *(int *)((int)piVar12 + 0x56) =
               (int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar2);
        }
        iVar2 = *(int *)(param_1 + 0x6e);
        if (iVar2 == 0) {
          (**(code **)(*piVar12 + 0x18))
                    (*(undefined4 *)(param_1 + 0x62),*(undefined4 *)(param_1 + 0x66),
                     *(undefined4 *)(param_1 + 0x6a));
        }
        else {
          uVar3 = FUN_0054e200();
          iVar9 = *(int *)(param_1 + 0x6e);
          uVar4 = FUN_0054e200();
          iVar11 = *(int *)(param_1 + 0x6e);
          uVar5 = FUN_0054e200();
          iVar10 = *(int *)(param_1 + 0x6e) / 2;
          (**(code **)(*piVar12 + 0x18))
                    (((int)(uVar5 & 0x7fffffff) % iVar11 + *(int *)(param_1 + 0x62)) - iVar10,
                     ((int)(uVar4 & 0x7fffffff) % iVar9 + *(int *)(param_1 + 0x66)) - iVar10,
                     ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar2) +
                     *(int *)(param_1 + 0x6a)) - iVar10);
        }
        iVar2 = FUN_005457e0(piVar12);
        if (iVar2 == 0) {
          if (piVar12 != (int *)0x0) {
            (**(code **)*piVar12)(1);
          }
          break;
        }
      }
      iStack_24 = iStack_24 + 1;
      ppvVar1 = ExceptionList;
    } while (iStack_24 < *(int *)(param_1 + 0x5e));
  }
  iVar2 = FUN_00548860();
  if (iVar2 != 0) {
    if (*(short *)(*(int *)(param_1 + 0x3e) + 4) != -1) {
      *(undefined4 *)(param_1 + 0x10) = 0;
      *(undefined4 *)(param_1 + 0x14) = 0;
    }
    if (*(int *)(param_1 + 0x56) == DAT_005e1428) {
      uVar3 = *(int *)(param_1 + 0x7e) + 1U & 0x80000007;
      if ((int)uVar3 < 0) {
        uVar3 = (uVar3 - 1 | 0xfffffff8) + 1;
      }
      *(uint *)(param_1 + 0x7e) = uVar3;
    }
    else {
      *(undefined4 *)(param_1 + 0x7e) = 8;
    }
    iVar9 = *(int *)(param_1 + 0x76);
    iVar11 = *(int *)(param_1 + 0x7a);
    iVar10 = *(int *)(&DAT_006f2174 + iVar9 * 4);
    if (iVar11 < iVar10) {
      uVar3 = iVar11 * 9 + *(int *)(param_1 + 0x7e);
      if (*(uint *)((&DAT_006f1d58)[iVar9 * 9] + 0xc) < uVar3) {
        uVar3 = 0;
      }
      iVar9 = *(int *)((&DAT_006f1d60)[iVar9 * 9] + uVar3 * 4) + (&DAT_006f1d64)[iVar9 * 9];
    }
    else {
      if (*(uint *)((int)&DAT_006a5f88 + iVar9 * 0x5c9) < 2) {
        uVar3 = ((iVar11 - iVar10) % DAT_006f2148) * 9 + *(int *)(param_1 + 0x7e);
        if (*(uint *)(DAT_006f2240 + 0xc) < uVar3) {
          uVar3 = 0;
        }
        iVar9 = *(int *)(DAT_006f2248 + uVar3 * 4);
        iVar11 = DAT_006f224c;
      }
      else {
        uVar3 = ((iVar11 - iVar10) % DAT_006f220c) * 9 + *(int *)(param_1 + 0x7e);
        if (*(uint *)(DAT_006f2218 + 0xc) < uVar3) {
          uVar3 = 0;
        }
        iVar9 = *(int *)(DAT_006f2220 + uVar3 * 4);
        iVar11 = DAT_006f2224;
      }
      iVar9 = iVar9 + iVar11;
    }
    *(int *)(param_1 + 0x5a) = iVar9;
  }
  ExceptionList = pvStack_c;
  return iVar2;
}

