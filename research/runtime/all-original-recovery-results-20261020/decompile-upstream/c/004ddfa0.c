
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004ddfa0(undefined4 *param_1)

{
  int iVar1;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c219b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FUN_005c6f68;
  local_4 = 0;
  param_1[1] = 0;
  if (*(int **)((int)param_1 + 0x53) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x53) + 4))(1);
    *(undefined4 *)((int)param_1 + 0x53) = 0;
  }
  if (param_1[3] != 0) {
    FUN_0058b3e0();
  }
  if (DAT_006e2090 != 0) {
    FUN_005979b0(DAT_006e2090);
    DAT_006e2090 = 0;
    _DAT_006e2094 = 0;
    DAT_006e2098 = 0;
    DAT_006e209c = 0;
  }
  iVar1 = 0x10;
  do {
    if (*(int *)((int)&DAT_006e2090 + iVar1) != 0) {
      FUN_0057d0d0(*(int *)((int)&DAT_006e2090 + iVar1));
      *(undefined4 *)((int)&DAT_006e2090 + iVar1) = 0;
    }
    iVar1 = iVar1 + 4;
  } while (iVar1 < 0x20);
  if (DAT_006e20b0 != 0) {
    FUN_005979b0(DAT_006e20b0);
    DAT_006e20b0 = 0;
  }
  if (*(int *)((int)param_1 + 0x4f) != 0) {
    FUN_004de090(0,1);
    *(undefined4 *)((int)param_1 + 0x4f) = 0;
  }
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = pvStack_c;
  return;
}

