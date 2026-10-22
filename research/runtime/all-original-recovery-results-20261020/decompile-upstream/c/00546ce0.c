
undefined4 __fastcall FUN_00546ce0(int param_1)

{
  void **ppvVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int local_2c;
  int local_28;
  int local_24;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_c = ExceptionList;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c285b;
  ExceptionList = &local_c;
  iVar2 = *(int *)(param_1 + 0x14 + *(int *)(param_1 + 0x24) * 4);
  iVar2 = ((*(int *)(param_1 + 0x18 + *(int *)(param_1 + 0x24) * 4) - iVar2) *
          (*(int *)(param_1 + 0x10) - *(int *)(param_1 + 0x20))) / *(int *)(param_1 + 0x10) + iVar2;
  iVar8 = *(int *)(param_1 + 0x54);
  ppvVar1 = &local_c;
  if ((iVar8 != 0) && (ppvVar1 = &local_c, *(int *)(iVar8 + 0xe4) != 0)) {
    ppvVar1 = &local_c;
    if (*(int *)(param_1 + 0x58) != 0) {
      iVar9 = *(int *)(*(int *)(iVar8 + 0xac) + 0x198) + *(int *)(iVar8 + 0x14);
      *(int *)(param_1 + 4) = iVar9;
      while (iVar9 < 0) {
        iVar9 = *(int *)(param_1 + 4) + DAT_005eb480;
        *(int *)(param_1 + 4) = iVar9;
      }
      ppvVar1 = ExceptionList;
      if (DAT_005eb480 <= *(int *)(param_1 + 4)) {
        do {
          iVar9 = *(int *)(param_1 + 4) - DAT_005eb480;
          *(int *)(param_1 + 4) = iVar9;
          ppvVar1 = ExceptionList;
        } while (DAT_005eb480 <= iVar9);
      }
    }
    ExceptionList = ppvVar1;
    if (*(int *)(param_1 + 0x5c) != 0) {
      iVar9 = *(int *)(*(int *)(iVar8 + 0xac) + 0x198) + *(int *)(iVar8 + 0x18);
      *(int *)(param_1 + 8) = iVar9;
      while (iVar9 < 0) {
        iVar9 = *(int *)(param_1 + 8) + DAT_005eb484;
        *(int *)(param_1 + 8) = iVar9;
      }
      if (DAT_005eb484 <= *(int *)(param_1 + 8)) {
        do {
          iVar9 = *(int *)(param_1 + 8) - DAT_005eb484;
          *(int *)(param_1 + 8) = iVar9;
        } while (DAT_005eb484 <= iVar9);
      }
    }
    ppvVar1 = ExceptionList;
    if (*(int *)(param_1 + 0x60) != 0) {
      *(int *)(param_1 + 0xc) = *(int *)(iVar8 + 0x1c) + *(int *)(*(int *)(iVar8 + 0xac) + 0x19c);
      ppvVar1 = ExceptionList;
    }
  }
  do {
    ExceptionList = ppvVar1;
    if (iVar2 < 1) {
LAB_005471c0:
      iVar2 = *(int *)(param_1 + 0x20) + -1;
      *(int *)(param_1 + 0x20) = iVar2;
      if (iVar2 < 1) {
        *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_1 + 0x10);
        iVar2 = *(int *)(param_1 + 0x24) + 1;
        *(int *)(param_1 + 0x24) = iVar2;
        if (1 < iVar2) {
          ExceptionList = local_c;
          return 0;
        }
      }
      ExceptionList = local_c;
      return 1;
    }
    iVar8 = *(int *)(param_1 + 0x44);
    local_24 = *(int *)(param_1 + 0x38);
    if (iVar8 != 0) {
      uVar3 = FUN_0054e200();
      local_24 = local_24 +
                 ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar8) -
                 *(int *)(param_1 + 0x44) / 2);
    }
    iVar8 = *(int *)(param_1 + 0x48);
    local_28 = *(int *)(param_1 + 0x3c);
    if (iVar8 != 0) {
      uVar3 = FUN_0054e200();
      local_28 = local_28 +
                 ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar8) -
                 *(int *)(param_1 + 0x48) / 2);
    }
    iVar8 = *(int *)(param_1 + 0x4c);
    local_2c = *(int *)(param_1 + 0x40);
    if (iVar8 != 0) {
      uVar3 = FUN_0054e200();
      local_2c = local_2c +
                 ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)(iVar8 * 2)) -
                 *(int *)(param_1 + 0x4c));
    }
    puVar4 = (undefined4 *)FUN_00597850(100);
    local_4 = 0;
    if (puVar4 == (undefined4 *)0x0) {
      puVar4 = (undefined4 *)0x0;
    }
    else {
      iVar8 = *(int *)(param_1 + 0x50);
      uVar3 = FUN_0054e200();
      iVar9 = *(int *)(param_1 + 0x50);
      uVar5 = FUN_0054e200();
      iVar10 = *(int *)(param_1 + 0x50);
      uVar6 = FUN_0054e200();
      FUN_00545650(((int)(uVar6 & 0x7fffffff) % (iVar10 * 2 + 1) + *(int *)(param_1 + 4)) -
                   *(int *)(param_1 + 0x50),
                   ((int)((longlong)(ulonglong)(uVar5 & 0x7fffffff) % (longlong)(iVar9 * 2 + 1)) +
                   *(int *)(param_1 + 8)) - *(int *)(param_1 + 0x50),
                   ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)(iVar8 * 2 + 1)) +
                   *(int *)(param_1 + 0xc)) - *(int *)(param_1 + 0x50),local_24,local_28,local_2c);
      puVar4[7] = 0x80;
      puVar4[8] = 0x80;
      puVar4[9] = 0x20;
      puVar4[10] = 100;
      puVar4[0xb] = 5;
      puVar4[0xc] = 0;
      FUN_00545b70(2,2,0x80,0x80,0x20);
      puVar4[0x15] = 0;
      puVar4[0x16] = 0;
      puVar4[0x17] = 0;
      puVar4[0x18] = 0;
      *puVar4 = &PTR_FUN_005c7408;
    }
    local_4 = 0xffffffff;
    if (puVar4 == (undefined4 *)0x0) goto LAB_005471c0;
    iVar8 = *(int *)(param_1 + 0x34);
    iVar9 = *(int *)(param_1 + 0x30);
    iVar10 = *(int *)(param_1 + 0x28);
    iVar11 = *(int *)(param_1 + 0x2c);
    if (iVar8 != 0) {
      uVar3 = FUN_0054e200();
      iVar7 = *(int *)(param_1 + 0x34);
      iVar10 = iVar10 + ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar8) -
                        *(int *)(param_1 + 0x34) / 2);
      uVar3 = FUN_0054e200();
      iVar8 = *(int *)(param_1 + 0x34);
      iVar11 = iVar11 + ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar7) -
                        iVar8 / 2);
      uVar3 = FUN_0054e200();
      iVar9 = iVar9 + ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar8) -
                      *(int *)(param_1 + 0x34) / 2);
    }
    puVar4[9] = iVar9;
    iVar8 = puVar4[10];
    puVar4[8] = iVar11;
    puVar4[7] = iVar10;
    iVar7 = (iVar9 * 100) / iVar8;
    iVar9 = (iVar11 * 100) / iVar8;
    iVar8 = (iVar10 * 100) / iVar8;
    if (iVar8 < 0x100) {
      if (iVar8 < 0) {
        iVar8 = 0;
      }
    }
    else {
      iVar8 = 0xff;
    }
    puVar4[0xf] = (int)(iVar8 + (iVar8 >> 0x1f & 7U)) >> 3;
    if (iVar9 < 0x100) {
      if (iVar9 < 0) {
        iVar9 = 0;
      }
    }
    else {
      iVar9 = 0xff;
    }
    puVar4[0x10] = iVar9 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
    if (iVar7 < 0x100) {
      if (iVar7 < 0) {
        iVar7 = 0;
      }
    }
    else {
      iVar7 = 0xff;
    }
    puVar4[0x11] = (int)(iVar7 + (iVar7 >> 0x1f & 7U)) >> 3;
    puVar4[0xc] = -(*(int *)(param_1 + 0xc) / local_2c);
    iVar8 = *(int *)(param_1 + 0x54);
    puVar4[0x18] = iVar8;
    iVar9 = *(int *)(param_1 + 0x58);
    puVar4[0x15] = iVar9;
    if (iVar9 != 0) {
      iVar9 = *(int *)(*(int *)(iVar8 + 0xac) + 0x198);
      iVar8 = *(int *)(iVar8 + 0x14);
      puVar4[4] = 0;
      puVar4[0x12] = (iVar9 - puVar4[1]) + iVar8;
    }
    iVar8 = *(int *)(param_1 + 0x5c);
    puVar4[0x16] = iVar8;
    if (iVar8 != 0) {
      iVar8 = *(int *)(*(int *)(puVar4[0x18] + 0xac) + 0x198);
      iVar9 = *(int *)(puVar4[0x18] + 0x18);
      puVar4[5] = 0;
      puVar4[0x13] = (iVar8 - puVar4[2]) + iVar9;
    }
    iVar8 = *(int *)(param_1 + 0x60);
    puVar4[0x17] = iVar8;
    if (iVar8 != 0) {
      iVar8 = *(int *)(*(int *)(puVar4[0x18] + 0xac) + 0x19c);
      iVar9 = *(int *)(puVar4[0x18] + 0x1c);
      puVar4[6] = 0;
      puVar4[0x14] = (iVar8 - puVar4[3]) + iVar9;
    }
    iVar8 = FUN_005457e0(puVar4);
    if (iVar8 == 0) {
      if (puVar4 != (undefined4 *)0x0) {
        (**(code **)*puVar4)(1);
      }
      goto LAB_005471c0;
    }
    iVar2 = iVar2 + -1;
    ppvVar1 = ExceptionList;
  } while( true );
}

