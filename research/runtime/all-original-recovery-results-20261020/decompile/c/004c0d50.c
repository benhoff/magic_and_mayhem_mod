
undefined1 * __fastcall FUN_004c0d50(undefined1 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  undefined1 local_11;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c1660;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = local_11;
  param_1[1] = local_11;
  param_1[8] = 0;
  local_4 = 0;
  if (DAT_005faf4c == (undefined4 *)0x0) {
    DAT_005faf4c = (undefined4 *)FUN_00597850(0xe68);
    DAT_005faf4c[1] = 0;
    DAT_005faf4c[0x399] = 1;
    *DAT_005faf4c = 0;
    DAT_005faf4c[2] = 0;
  }
  puVar1 = DAT_005faf4c;
  DAT_006e204c = DAT_006e204c + 1;
  iVar2 = FUN_00597850(0xe68);
  *(undefined4 **)(iVar2 + 4) = puVar1;
  *(undefined4 *)(iVar2 + 0xe64) = 0;
  *(int *)(param_1 + 4) = iVar2;
  *(undefined4 *)(param_1 + 0xc) = 0;
  *(int *)iVar2 = iVar2;
  *(int *)(*(int *)(param_1 + 4) + 8) = *(int *)(param_1 + 4);
  local_4 = 1;
  FUN_0059c441(param_1 + 0x10,0x10,2,&LAB_004c1070,FUN_00429760);
  ExceptionList = local_c;
  return param_1;
}

