
/* WARNING: Removing unreachable block (ram,0x00545fe7) */
/* WARNING: Removing unreachable block (ram,0x00545f90) */
/* WARNING: Removing unreachable block (ram,0x00545f33) */
/* WARNING: Removing unreachable block (ram,0x00545f53) */
/* WARNING: Removing unreachable block (ram,0x00545fc8) */
/* WARNING: Removing unreachable block (ram,0x00546006) */

undefined4 * __thiscall
FUN_00545e90(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c27f8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00545590(param_2,param_3,param_4);
  *param_1 = &PTR_FUN_005c73c0;
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0x80;
  param_1[8] = 0x80;
  param_1[9] = 0x20;
  param_1[10] = 100;
  param_1[0xb] = 5;
  param_1[0xc] = 0;
  local_4 = 0;
  param_1[0xd] = 2;
  param_1[0xe] = 2;
  FUN_00545ae0(0x80,0x80,0x20);
  *param_1 = &PTR_FUN_005c73f0;
  uVar1 = FUN_0054e200();
  if ((uVar1 & 1) == 0) {
    uVar2 = FUN_0054e200();
    uVar2 = uVar2 & 0xf;
    iVar4 = 0x10 - uVar2;
    uVar1 = FUN_0054e200();
    uVar1 = (uint)((longlong)(ulonglong)(uVar1 & 0x7fffffff) % (longlong)iVar4);
  }
  else {
    uVar1 = FUN_0054e200();
    uVar1 = uVar1 & 0xf;
    uVar2 = FUN_0054e200();
    uVar2 = (uint)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)(int)(0x10 - uVar1));
    iVar4 = 0x10 - uVar2;
  }
  iVar4 = iVar4 - uVar1;
  uVar3 = FUN_0054e200();
  if ((uVar3 & 1) != 0) {
    uVar1 = -uVar1;
  }
  uVar3 = FUN_0054e200();
  if ((uVar3 & 1) != 0) {
    uVar2 = -uVar2;
  }
  uVar3 = FUN_0054e200();
  if ((uVar3 & 1) != 0) {
    iVar4 = -iVar4;
  }
  param_1[4] = uVar1;
  param_1[5] = uVar2;
  param_1[6] = iVar4;
  ExceptionList = local_c;
  return param_1;
}

