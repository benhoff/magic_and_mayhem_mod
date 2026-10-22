
void __fastcall FUN_004d3800(int param_1)

{
  if (*(int *)(param_1 + 0xc) != 0) {
    if (*(int **)(param_1 + 0x1d3) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0x1d3) + 4))(1);
      *(undefined4 *)(param_1 + 0x1d3) = 0;
    }
    FUN_0058b3e0();
    *(undefined4 *)(param_1 + 0x1ce) = 0;
  }
  return;
}

