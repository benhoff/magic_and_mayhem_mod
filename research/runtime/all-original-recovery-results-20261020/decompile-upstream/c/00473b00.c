
void __fastcall FUN_00473b00(int *param_1)

{
  int *piVar1;
  int *piVar2;
  int local_14;
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_005bf606;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = (int)&PTR_LAB_005c5ebc;
  local_4 = 1;
  if (param_1[2] != 0) {
    piVar1 = *(int **)((int)param_1 + 0x2a5);
    param_1[2] = 0;
    if (piVar1 != (int *)0x0) {
      if (*piVar1 != 0) {
        FUN_005979b0(*piVar1);
        *piVar1 = 0;
        piVar1[1] = 0;
        piVar1[2] = 0;
        piVar1[3] = 0;
      }
      piVar2 = piVar1 + 4;
      local_14 = 4;
      do {
        if (*piVar2 != 0) {
          FUN_0057d0d0(*piVar2);
          *piVar2 = 0;
        }
        piVar2 = piVar2 + 1;
        local_14 = local_14 + -1;
      } while (local_14 != 0);
      if (piVar1[8] != 0) {
        FUN_005979b0(piVar1[8]);
        piVar1[8] = 0;
      }
      FUN_00597870(piVar1);
      *(undefined4 *)((int)param_1 + 0x2a5) = 0;
    }
    FUN_0058b3e0();
    (**(code **)(*param_1 + 0x20))();
    *(undefined4 *)((int)param_1 + 0x43) = 1;
    *(undefined4 *)((int)param_1 + 0x53) = 0;
  }
  FUN_00473c10();
  FUN_0058a0a0();
  local_4 = local_4 & 0xffffff00;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  FUN_004cab70();
  ExceptionList = local_c;
  return;
}

