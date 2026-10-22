
/* WARNING: Removing unreachable block (ram,0x0049a41f) */
/* WARNING: Removing unreachable block (ram,0x0049a435) */

void FUN_0049a350(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  undefined4 *puVar11;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  iVar6 = param_1;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c035b;
  local_c = ExceptionList;
  iVar7 = *(int *)(param_1 + 0xac);
  iVar1 = *(int *)(param_1 + 0x14);
  iVar2 = *(int *)(iVar7 + 0xc);
  iVar3 = *(int *)(iVar7 + 0x198);
  iVar4 = *(int *)(param_1 + 0x18);
  iVar5 = *(int *)(param_1 + 0x1c);
  param_1 = ((iVar2 + 1) * *(int *)(iVar7 + 8) + 1) * 6;
  ExceptionList = &local_c;
  if (0 < param_1) {
    do {
      iVar7 = FUN_00597850(0x24);
      local_4 = 0;
      if (iVar7 == 0) {
        puVar11 = (undefined4 *)0x0;
      }
      else {
        uVar8 = FUN_0054e200();
        uVar9 = FUN_0054e200();
        uVar10 = FUN_0054e200();
        puVar11 = (undefined4 *)
                  FUN_00549900(((uVar10 & 0x1f) - 0x10) + iVar1 + iVar3,
                               ((uVar9 & 0x1f) - 0x10) + iVar4 + iVar3,
                               (int)((longlong)(ulonglong)(uVar8 & 0x7fffffff) %
                                    (longlong)(iVar2 << 4)) + iVar5,iVar6,0x19);
      }
      local_4 = 0xffffffff;
      if ((puVar11 != (undefined4 *)0x0) && (iVar7 = FUN_005457e0(puVar11), iVar7 == 0)) {
        (**(code **)*puVar11)(1);
      }
      param_1 = param_1 + -1;
    } while (param_1 != 0);
  }
  ExceptionList = local_c;
  return;
}

