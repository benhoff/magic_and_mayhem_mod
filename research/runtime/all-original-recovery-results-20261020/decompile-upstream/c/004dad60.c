
void __fastcall FUN_004dad60(int param_1)

{
  if (*(int *)(param_1 + 0xc) != 0) {
    FUN_0058b3e0();
  }
  if (*(int **)(param_1 + 0x1f6) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x1f6) + 4))(1);
    *(undefined4 *)(param_1 + 0x1f6) = 0;
  }
  return;
}

