
undefined4 __fastcall FUN_00547380(int *param_1)

{
  void **ppvVar1;
  uint uVar2;
  undefined4 *puVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  undefined4 local_30;
  int iStack_2c;
  int iStack_28;
  int local_24;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_005c287b;
  pvStack_c = ExceptionList;
  local_30 = 1;
  iVar7 = param_1[0x11];
  iVar8 = param_1[4];
  ppvVar1 = &pvStack_c;
  if (iVar7 != 0) {
    if (*(int *)(iVar7 + 0xe4) == 0) {
      local_30 = 0;
      ppvVar1 = &pvStack_c;
    }
    else {
      ExceptionList = &pvStack_c;
      if (param_1[0x12] != 0) {
        iVar9 = *(int *)(*(int *)(iVar7 + 0xac) + 0x198) + *(int *)(iVar7 + 0x14);
        ExceptionList = &pvStack_c;
        param_1[1] = iVar9;
        while (iVar9 < 0) {
          iVar9 = param_1[1] + DAT_005eb480;
          param_1[1] = iVar9;
        }
        if (DAT_005eb480 <= param_1[1]) {
          do {
            iVar9 = param_1[1] - DAT_005eb480;
            param_1[1] = iVar9;
          } while (DAT_005eb480 <= iVar9);
        }
      }
      if (param_1[0x13] != 0) {
        iVar9 = *(int *)(*(int *)(iVar7 + 0xac) + 0x198) + *(int *)(iVar7 + 0x18);
        param_1[2] = iVar9;
        while (iVar9 < 0) {
          iVar9 = param_1[2] + DAT_005eb484;
          param_1[2] = iVar9;
        }
        if (DAT_005eb484 <= param_1[2]) {
          do {
            iVar9 = param_1[2] - DAT_005eb484;
            param_1[2] = iVar9;
          } while (DAT_005eb484 <= iVar9);
        }
      }
      if (param_1[0x14] != 0) {
        param_1[3] = *(int *)(iVar7 + 0x1c) + *(int *)(*(int *)(iVar7 + 0xac) + 0x19c);
      }
      local_30 = (**(code **)(*param_1 + 0x1c))();
      ppvVar1 = ExceptionList;
    }
  }
  while( true ) {
    ExceptionList = ppvVar1;
    if (iVar8 < 1) {
      ExceptionList = pvStack_c;
      return local_30;
    }
    iVar7 = param_1[0xc];
    local_24 = param_1[9];
    if (iVar7 != 0) {
      uVar2 = FUN_0054e200();
      local_24 = local_24 +
                 ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar7) -
                 param_1[0xc] / 2);
    }
    iVar7 = param_1[0xd];
    iStack_28 = param_1[10];
    if (iVar7 != 0) {
      uVar2 = FUN_0054e200();
      iStack_28 = iStack_28 +
                  ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar7) -
                  param_1[0xd] / 2);
    }
    iVar7 = param_1[0xe];
    iStack_2c = param_1[0xb];
    if (iVar7 != 0) {
      uVar2 = FUN_0054e200();
      iStack_2c = iStack_2c +
                  ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)(iVar7 * 2)) -
                  param_1[0xe]);
    }
    puVar3 = (undefined4 *)FUN_00597850(100);
    uStack_4 = 0;
    if (puVar3 == (undefined4 *)0x0) {
      puVar3 = (undefined4 *)0x0;
    }
    else {
      iVar7 = param_1[0x10];
      uVar2 = FUN_0054e200();
      iVar9 = param_1[0x10];
      uVar4 = FUN_0054e200();
      iVar10 = param_1[0x10];
      uVar5 = FUN_0054e200();
      FUN_00545650(((int)(uVar5 & 0x7fffffff) % (iVar10 * 2 + 1) + param_1[1]) - param_1[0x10],
                   ((int)((longlong)(ulonglong)(uVar4 & 0x7fffffff) % (longlong)(iVar9 * 2 + 1)) +
                   param_1[2]) - param_1[0x10],
                   ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)(iVar7 * 2 + 1)) +
                   param_1[3]) - param_1[0x10],local_24,iStack_28,iStack_2c);
      puVar3[7] = 0x80;
      puVar3[8] = 0x80;
      puVar3[9] = 0x20;
      puVar3[10] = 100;
      puVar3[0xb] = 5;
      puVar3[0xc] = 0;
      FUN_00545b70(2,2,0x80,0x80,0x20);
      puVar3[0x15] = 0;
      puVar3[0x16] = 0;
      puVar3[0x17] = 0;
      puVar3[0x18] = 0;
      *puVar3 = &PTR_FUN_005c7408;
    }
    uStack_4 = 0xffffffff;
    if (puVar3 == (undefined4 *)0x0) break;
    iVar7 = param_1[8];
    iVar9 = param_1[7];
    iVar10 = param_1[5];
    iVar11 = param_1[6];
    if (iVar7 != 0) {
      uVar2 = FUN_0054e200();
      iVar6 = param_1[8];
      iVar10 = iVar10 + ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar7) -
                        param_1[8] / 2);
      uVar2 = FUN_0054e200();
      iVar7 = param_1[8];
      iVar11 = iVar11 + ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar6) -
                        iVar7 / 2);
      uVar2 = FUN_0054e200();
      iVar9 = iVar9 + ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar7) -
                      param_1[8] / 2);
    }
    uVar2 = ((uint *)param_1[0xf])[1];
    uVar4 = *(uint *)param_1[0xf];
    uVar4 = uVar4 & ((int)uVar4 < 0) - 1;
    if (0x10 < (int)uVar4) {
      uVar4 = 0x10;
    }
    puVar3[0xd] = uVar4;
    uVar2 = uVar2 & ((int)uVar2 < 0) - 1;
    if (0x10 < (int)uVar2) {
      uVar2 = 0x10;
    }
    puVar3[0xe] = uVar2;
    puVar3[9] = iVar9;
    iVar7 = puVar3[10];
    puVar3[8] = iVar11;
    puVar3[7] = iVar10;
    iVar6 = (iVar9 * 100) / iVar7;
    iVar9 = (iVar11 * 100) / iVar7;
    iVar7 = (iVar10 * 100) / iVar7;
    if (iVar7 < 0x100) {
      if (iVar7 < 0) {
        iVar7 = 0;
      }
    }
    else {
      iVar7 = 0xff;
    }
    puVar3[0xf] = (int)(iVar7 + (iVar7 >> 0x1f & 7U)) >> 3;
    if (iVar9 < 0x100) {
      if (iVar9 < 0) {
        iVar9 = 0;
      }
    }
    else {
      iVar9 = 0xff;
    }
    puVar3[0x10] = iVar9 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
    if (iVar6 < 0x100) {
      if (iVar6 < 0) {
        iVar6 = 0;
      }
    }
    else {
      iVar6 = 0xff;
    }
    puVar3[0x11] = (int)(iVar6 + (iVar6 >> 0x1f & 7U)) >> 3;
    puVar3[0xc] = -(param_1[3] / iStack_2c);
    iVar7 = param_1[0x11];
    puVar3[0x18] = iVar7;
    iVar9 = param_1[0x12];
    puVar3[0x15] = iVar9;
    if (iVar9 != 0) {
      iVar9 = *(int *)(*(int *)(iVar7 + 0xac) + 0x198);
      iVar7 = *(int *)(iVar7 + 0x14);
      puVar3[4] = 0;
      puVar3[0x12] = (iVar9 - puVar3[1]) + iVar7;
    }
    iVar7 = param_1[0x13];
    puVar3[0x16] = iVar7;
    if (iVar7 != 0) {
      iVar7 = *(int *)(*(int *)(puVar3[0x18] + 0xac) + 0x198);
      iVar9 = *(int *)(puVar3[0x18] + 0x18);
      puVar3[5] = 0;
      puVar3[0x13] = (iVar7 - puVar3[2]) + iVar9;
    }
    iVar7 = FUN_005457e0(puVar3);
    if (iVar7 == 0) {
      if (puVar3 == (undefined4 *)0x0) {
        ExceptionList = pvStack_c;
        return local_30;
      }
      (**(code **)*puVar3)(1);
      ExceptionList = pvStack_c;
      return local_30;
    }
    iVar8 = iVar8 + -1;
    ppvVar1 = ExceptionList;
  }
  ExceptionList = pvStack_c;
  return local_30;
}

