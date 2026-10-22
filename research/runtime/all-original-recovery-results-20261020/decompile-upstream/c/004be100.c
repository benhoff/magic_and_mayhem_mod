
void __fastcall FUN_004be100(int param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  int local_14;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c1210;
  local_c = ExceptionList;
  puVar2 = (undefined4 *)0x0;
  local_4 = 2;
  ExceptionList = &local_c;
  if (*(int *)(param_1 + 0xb3) != 0) {
    ExceptionList = &local_c;
    FUN_005979b0(*(int *)(param_1 + 0xb3));
    *(undefined4 *)(param_1 + 0xb3) = 0;
    *(undefined4 *)(param_1 + 0xb7) = 0;
    *(undefined4 *)(param_1 + 0xbb) = 0;
    *(undefined4 *)(param_1 + 0xbf) = 0;
  }
  piVar1 = (int *)(param_1 + 0xc3);
  local_14 = 4;
  do {
    if (*piVar1 != 0) {
      FUN_0057d0d0(*piVar1);
      *piVar1 = 0;
    }
    piVar1 = piVar1 + 1;
    local_14 = local_14 + -1;
  } while (local_14 != 0);
  if (*(int *)(param_1 + 0xd3) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0xd3));
    *(undefined4 *)(param_1 + 0xd3) = 0;
  }
  local_4 = CONCAT31(local_4._1_3_,1);
  FUN_0058b3e0();
  if (*(int *)(param_1 + 0x6d) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x6d));
    *(undefined4 *)(param_1 + 0x6d) = 0;
    *(undefined4 *)(param_1 + 0x71) = 0;
  }
  *(undefined4 *)(param_1 + 0x75) = 0;
  *(undefined4 *)(param_1 + 0x79) = 0;
  *(undefined4 *)(param_1 + 0x7d) = 0;
  if (*(int *)(param_1 + 0x6d) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x6d));
    *(undefined4 *)(param_1 + 0x6d) = 0;
    *(undefined4 *)(param_1 + 0x71) = 0;
  }
  local_4 = 0xffffffff;
  if (param_1 != 0) {
    puVar2 = (undefined4 *)(param_1 + 8);
  }
  *puVar2 = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  puVar2[1] = 0xffffffff;
  ExceptionList = local_c;
  return;
}

