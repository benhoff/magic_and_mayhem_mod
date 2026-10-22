
void __fastcall FUN_004de8d0(int param_1)

{
  if (*(int **)(param_1 + 0x42) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x42) + 4))(1);
    *(undefined4 *)(param_1 + 0x42) = 0;
  }
  if (*(int *)(param_1 + 0x14) != 0) {
    FUN_0058b3e0();
  }
  return;
}

