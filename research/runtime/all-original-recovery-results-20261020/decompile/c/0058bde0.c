
/* WARNING: Removing unreachable block (ram,0x0058be65) */
/* WARNING: Removing unreachable block (ram,0x0058be32) */

undefined4 FUN_0058bde0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c2e48;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  iVar1 = FUN_00533700(param_1,&param_1);
  if (iVar1 == 0) {
    ExceptionList = local_c;
    return 0;
  }
  uVar2 = FUN_0058bd10(param_1);
  ExceptionList = local_c;
  return uVar2;
}

