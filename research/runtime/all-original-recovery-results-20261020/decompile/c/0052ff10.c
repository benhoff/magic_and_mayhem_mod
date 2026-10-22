
int __thiscall FUN_0052ff10(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4)

{
  int iVar1;
  char *pcVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c24db;
  local_c = ExceptionList;
  iVar6 = 0;
  ExceptionList = &local_c;
  FUN_0054e170(0x499602d2);
  local_4 = 0;
  param_3 = param_3 + param_2 * 5;
  iVar5 = 0x32;
  pcVar2 = (char *)((int)param_1 + param_3 * 0x1c2 + 0xc7a);
  do {
    if (*pcVar2 == '\x01') {
      iVar6 = iVar6 + 1;
    }
    pcVar2 = pcVar2 + 9;
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  if (iVar6 < 1) {
    iVar5 = -1;
  }
  else {
    FUN_0054e170(*param_1);
    uVar3 = FUN_0054e200();
    iVar5 = (int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar6);
    iVar6 = iVar5 + param_3 * 0x32;
    iVar1 = iVar6 * 9;
    uVar3 = (uint)(*(char *)((int)param_1 + iVar6 * 9 + 0xc7f) == '\x01');
    if (*(char *)((int)param_1 + iVar1 + 0xc80) == '\x01') {
      uVar3 = uVar3 + 1;
    }
    if (*(char *)((int)param_1 + iVar1 + 0xc81) == '\x01') {
      uVar3 = uVar3 + 1;
    }
    if (*(char *)((int)param_1 + iVar1 + 0xc82) == '\x01') {
      uVar3 = uVar3 + 1;
    }
    uVar4 = FUN_0054e200();
    iVar6 = (int)((ulonglong)(uVar4 & 0x7fffffff) % (ulonglong)(longlong)(int)uVar3);
    if (*(char *)((int)param_1 + iVar1 + 0xc7f) == '\x01') {
      if (iVar6 == 0) {
        *param_4 = 0;
        ExceptionList = local_c;
        return iVar5;
      }
      iVar6 = iVar6 + -1;
    }
    if (*(char *)((int)param_1 + iVar1 + 0xc80) == '\x01') {
      if (iVar6 == 0) {
        *param_4 = 1;
        ExceptionList = local_c;
        return iVar5;
      }
      iVar6 = iVar6 + -1;
    }
    if (*(char *)((int)param_1 + iVar1 + 0xc81) == '\x01') {
      if (iVar6 == 0) {
        *param_4 = 2;
        ExceptionList = local_c;
        return iVar5;
      }
      iVar6 = iVar6 + -1;
    }
    if ((*(char *)((int)param_1 + iVar1 + 0xc82) == '\x01') && (iVar6 == 0)) {
      *param_4 = 3;
    }
  }
  ExceptionList = local_c;
  return iVar5;
}

