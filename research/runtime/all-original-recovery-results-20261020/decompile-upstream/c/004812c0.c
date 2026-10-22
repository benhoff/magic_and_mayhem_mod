
undefined4 * __fastcall FUN_004812c0(undefined4 *param_1)

{
  DWORD DVar1;
  int iVar2;
  undefined4 *puVar3;
  
  param_1[0xcb] = 0;
  *param_1 = 0;
  param_1[0xca] = 1;
  param_1[0xcc] = 0;
  param_1[0xc9] = 0;
  param_1[0xcd] = 0;
  param_1[0xce] = 0;
  param_1[0xd0] = 0;
  param_1[0xd1] = 0;
  *(undefined2 *)(param_1 + 0xe2) = 0;
  *(undefined1 *)((int)param_1 + 0x38a) = 0;
  puVar3 = param_1;
  for (iVar2 = 10; puVar3 = puVar3 + 1, iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = 0;
  }
  param_1[199] = 0;
  DVar1 = timeGetTime();
  FUN_0054e170(DVar1);
  return param_1;
}

