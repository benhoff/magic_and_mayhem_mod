
int __thiscall FUN_00531710(undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  int iVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c251b;
  local_c = ExceptionList;
  iVar5 = 0;
  ExceptionList = &local_c;
  FUN_0054e170(0x499602d2);
  local_4 = 0;
  iVar1 = param_1[2] * param_1[1];
  if (0 < iVar1) {
    piVar4 = param_1 + 0xe1b;
    do {
      if (*piVar4 != -1) {
        iVar5 = iVar5 + 1;
      }
      piVar4 = piVar4 + 3;
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
    if (0 < iVar5) {
      FUN_0054e170(*param_1);
      uVar2 = FUN_0054e200();
      iVar1 = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar5);
      uVar2 = (uint)(*(char *)(param_1 + iVar1 * 3 + 0xe1d) == '\x01');
      if (*(char *)((int)param_1 + iVar1 * 0xc + 0x3875) == '\x01') {
        uVar2 = uVar2 + 1;
      }
      if (*(char *)((int)param_1 + iVar1 * 0xc + 0x3876) == '\x01') {
        uVar2 = uVar2 + 1;
      }
      if (*(char *)((int)param_1 + iVar1 * 0xc + 0x3877) == '\x01') {
        uVar2 = uVar2 + 1;
      }
      uVar3 = FUN_0054e200();
      iVar5 = (int)((ulonglong)(uVar3 & 0x7fffffff) % (ulonglong)(longlong)(int)uVar2);
      if (*(char *)(param_1 + iVar1 * 3 + 0xe1d) == '\x01') {
        if (iVar5 == 0) {
          *param_2 = 0;
          ExceptionList = local_c;
          return iVar1;
        }
        iVar5 = iVar5 + -1;
      }
      if (*(char *)((int)param_1 + iVar1 * 0xc + 0x3875) == '\x01') {
        if (iVar5 == 0) {
          *param_2 = 1;
          ExceptionList = local_c;
          return iVar1;
        }
        iVar5 = iVar5 + -1;
      }
      if (*(char *)((int)param_1 + iVar1 * 0xc + 0x3876) == '\x01') {
        if (iVar5 == 0) {
          *param_2 = 2;
          ExceptionList = local_c;
          return iVar1;
        }
        iVar5 = iVar5 + -1;
      }
      if (*(char *)((int)param_1 + iVar1 * 0xc + 0x3877) != '\x01') {
        ExceptionList = local_c;
        return iVar1;
      }
      if (iVar5 != 0) {
        ExceptionList = local_c;
        return iVar1;
      }
      *param_2 = 3;
      ExceptionList = local_c;
      return iVar1;
    }
  }
  ExceptionList = local_c;
  return -1;
}

