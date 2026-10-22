
/* WARNING: Removing unreachable block (ram,0x00546751) */
/* WARNING: Removing unreachable block (ram,0x00546770) */

undefined4 * __thiscall
FUN_00546660(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  bool bVar4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2818;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00545590(param_2,param_3,param_4);
  *param_1 = &PTR_FUN_005c73d8;
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[9] = 0x20;
  param_1[7] = 0x80;
  param_1[8] = 0x80;
  param_1[10] = 0x4b;
  param_1[0xb] = DAT_005e1464;
  param_1[0xc] = 2;
  param_1[0xd] = 2;
  param_1[0xe] = 0x10;
  bVar4 = DAT_006e1f88 != 0;
  param_1[0x10] = 4;
  local_4 = 0;
  param_1[0xf] = (int)(0x80 / (ulonglong)(longlong)(int)((-(uint)bVar4 & 4) + 4));
  *(undefined1 *)(param_1 + 0x11) = 0;
  *param_1 = &PTR_FUN_005c7420;
  iVar2 = DAT_005e1468;
  uVar1 = FUN_0054e200();
  iVar2 = (int)((longlong)(ulonglong)(uVar1 & 0x7fffffff) % (longlong)iVar2);
  iVar3 = DAT_005e1468 - iVar2;
  uVar1 = FUN_0054e200();
  iVar3 = (int)((longlong)(ulonglong)(uVar1 & 0x7fffffff) % (longlong)iVar3);
  uVar1 = FUN_0054e200();
  if ((uVar1 & 1) != 0) {
    iVar2 = -iVar2;
  }
  uVar1 = FUN_0054e200();
  if ((uVar1 & 1) != 0) {
    iVar3 = -iVar3;
  }
  param_1[5] = iVar3;
  param_1[4] = iVar2;
  param_1[6] = 0;
  ExceptionList = local_c;
  return param_1;
}

