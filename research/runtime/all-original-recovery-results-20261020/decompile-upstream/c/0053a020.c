
void __fastcall FUN_0053a020(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x12) != 0) {
    FUN_0058b3e0();
    iVar1 = *(int *)(param_1 + 0x12);
    if (iVar1 != 0) {
      FUN_0058b3e0();
      FUN_00597870(iVar1);
    }
    *(undefined4 *)(param_1 + 0x12) = 0;
  }
  if (*(int **)(param_1 + 0x16) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x16) + 4))(1);
    *(undefined4 *)(param_1 + 0x16) = 0;
  }
  return;
}

