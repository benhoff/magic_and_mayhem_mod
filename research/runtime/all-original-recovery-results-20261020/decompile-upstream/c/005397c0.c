
void __fastcall FUN_005397c0(undefined4 *param_1)

{
  int iVar1;
  
  iVar1 = param_1[8];
  *param_1 = &PTR_LAB_005c72f0;
  if (iVar1 != 0) {
    FUN_004d27d0();
    FUN_00597870(iVar1);
    param_1[8] = 0;
  }
  if (*(int *)((int)param_1 + 0x12) != 0) {
    FUN_0058b3e0();
    iVar1 = *(int *)((int)param_1 + 0x12);
    if (iVar1 != 0) {
      FUN_0058b3e0();
      FUN_00597870(iVar1);
    }
    *(undefined4 *)((int)param_1 + 0x12) = 0;
  }
  if (*(int **)((int)param_1 + 0x16) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x16) + 4))(1);
    *(undefined4 *)((int)param_1 + 0x16) = 0;
  }
  return;
}

