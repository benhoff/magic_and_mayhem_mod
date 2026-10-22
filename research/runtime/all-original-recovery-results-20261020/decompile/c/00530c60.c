
int __thiscall FUN_00530c60(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  char cVar4;
  char cVar5;
  char cVar6;
  char cVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c24fb;
  local_c = ExceptionList;
  iVar8 = param_1[0xe68];
  iVar1 = param_1[0xe67];
  ExceptionList = &local_c;
  cVar4 = FUN_005300a0(param_2,param_3);
  param_1[0xe68] = 0;
  param_1[0xe66] = 0xffffffff;
  param_1[0xe67] = 0xffffffff;
  param_1[0xe69] = 0xffffffff;
  param_1[0xe6a] = 0xffffffff;
  param_1[0xe6b] = 0xffffffff;
  param_1[0xe6c] = 0xffffffff;
  param_1[0xe66] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x2c);
  param_1[0xe67] = iVar1;
  param_1[0xe69] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x56);
  param_1[0xe6a] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x5a);
  param_1[0xe6b] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x5e);
  param_1[0xe6c] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x62);
  iVar10 = iVar8;
  if (0 < iVar8) {
    do {
      uVar2 = param_1[0xe6c];
      param_1[0xe6c] = param_1[0xe6b];
      param_1[0xe6b] = param_1[0xe6a];
      uVar3 = param_1[0xe69];
      param_1[0xe69] = uVar2;
      param_1[0xe6a] = uVar3;
      if (param_1[0xe68] == 3) {
        param_1[0xe68] = 0;
      }
      else {
        param_1[0xe68] = param_1[0xe68] + 1;
      }
      iVar10 = iVar10 + -1;
    } while (iVar10 != 0);
  }
  uVar2 = param_1[0xe6c];
  param_1[0xe6c] = param_1[0xe6b];
  uVar3 = param_1[0xe69];
  param_1[0xe69] = uVar2;
  param_1[0xe6b] = param_1[0xe6a];
  param_1[0xe6a] = uVar3;
  if (param_1[0xe68] == 3) {
    param_1[0xe68] = 0;
  }
  else {
    param_1[0xe68] = param_1[0xe68] + 1;
  }
  cVar5 = FUN_005300a0(param_2,param_3);
  param_1[0xe66] = 0xffffffff;
  param_1[0xe67] = 0xffffffff;
  param_1[0xe68] = 0;
  param_1[0xe69] = 0xffffffff;
  param_1[0xe6a] = 0xffffffff;
  param_1[0xe6b] = 0xffffffff;
  param_1[0xe6c] = 0xffffffff;
  param_1[0xe66] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x2c);
  param_1[0xe67] = iVar1;
  param_1[0xe69] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x56);
  param_1[0xe6a] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x5a);
  param_1[0xe6b] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x5e);
  param_1[0xe6c] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x62);
  iVar10 = iVar8;
  if (0 < iVar8) {
    do {
      uVar2 = param_1[0xe6c];
      param_1[0xe6c] = param_1[0xe6b];
      param_1[0xe6b] = param_1[0xe6a];
      uVar3 = param_1[0xe69];
      param_1[0xe69] = uVar2;
      param_1[0xe6a] = uVar3;
      if (param_1[0xe68] == 3) {
        param_1[0xe68] = 0;
      }
      else {
        param_1[0xe68] = param_1[0xe68] + 1;
      }
      iVar10 = iVar10 + -1;
    } while (iVar10 != 0);
  }
  iVar10 = 2;
  do {
    uVar2 = param_1[0xe6c];
    param_1[0xe6c] = param_1[0xe6b];
    param_1[0xe6b] = param_1[0xe6a];
    uVar3 = param_1[0xe69];
    param_1[0xe69] = uVar2;
    param_1[0xe6a] = uVar3;
    if (param_1[0xe68] == 3) {
      param_1[0xe68] = 0;
    }
    else {
      param_1[0xe68] = param_1[0xe68] + 1;
    }
    iVar10 = iVar10 + -1;
  } while (iVar10 != 0);
  cVar6 = FUN_005300a0(param_2,param_3);
  param_1[0xe66] = 0xffffffff;
  param_1[0xe67] = 0xffffffff;
  param_1[0xe68] = 0;
  param_1[0xe69] = 0xffffffff;
  param_1[0xe6a] = 0xffffffff;
  param_1[0xe6b] = 0xffffffff;
  param_1[0xe6c] = 0xffffffff;
  param_1[0xe66] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x2c);
  param_1[0xe67] = iVar1;
  param_1[0xe69] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x56);
  param_1[0xe6a] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x5a);
  param_1[0xe6b] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x5e);
  param_1[0xe6c] = *(undefined4 *)((int)param_1 + iVar1 * 0x3f + 0x62);
  iVar10 = iVar8;
  if (0 < iVar8) {
    do {
      uVar2 = param_1[0xe6c];
      param_1[0xe6c] = param_1[0xe6b];
      param_1[0xe6b] = param_1[0xe6a];
      uVar3 = param_1[0xe69];
      param_1[0xe69] = uVar2;
      param_1[0xe6a] = uVar3;
      if (param_1[0xe68] == 3) {
        param_1[0xe68] = 0;
      }
      else {
        param_1[0xe68] = param_1[0xe68] + 1;
      }
      iVar10 = iVar10 + -1;
    } while (iVar10 != 0);
  }
  FUN_00531880(3);
  cVar7 = FUN_005300a0(param_2,param_3);
  FUN_0052d400();
  FUN_0052f3a0(iVar1);
  FUN_00531880(iVar8);
  FUN_0054e170(0x499602d2);
  local_4 = 0;
  FUN_0054e170(*param_1);
  uVar11 = (uint)(cVar4 == '\x01');
  if (cVar5 == '\x01') {
    uVar11 = uVar11 + 1;
  }
  if (cVar6 == '\x01') {
    uVar11 = uVar11 + 1;
  }
  if (cVar7 == '\x01') {
    uVar11 = uVar11 + 1;
  }
  if (uVar11 == 0) {
    iVar8 = -1;
  }
  else {
    uVar9 = FUN_0054e200();
    iVar8 = (int)((ulonglong)(uVar9 & 0x7fffffff) % (ulonglong)(longlong)(int)uVar11);
    if (cVar4 == '\x01') {
      if (iVar8 == 0) {
        ExceptionList = local_c;
        return 0;
      }
      iVar8 = iVar8 + -1;
    }
    if (cVar5 == '\x01') {
      if (iVar8 == 0) {
        ExceptionList = local_c;
        return 1;
      }
      iVar8 = iVar8 + -1;
    }
    if (cVar6 == '\x01') {
      if (iVar8 == 0) {
        ExceptionList = local_c;
        return 2;
      }
      iVar8 = iVar8 + -1;
    }
    if ((cVar7 == '\x01') && (iVar8 == 0)) {
      iVar8 = 3;
    }
  }
  ExceptionList = local_c;
  return iVar8;
}

