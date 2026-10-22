
void __fastcall FUN_004db6d0(int param_1)

{
  FUN_0058b3e0();
  if (*(int **)(param_1 + 0x186) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x186) + 4))(1);
    *(undefined4 *)(param_1 + 0x186) = 0;
  }
  return;
}

