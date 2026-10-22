
void __fastcall FUN_004d94f0(int param_1)

{
  if (*(int **)(param_1 + 0x61f) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x61f) + 4))(1);
    *(undefined4 *)(param_1 + 0x61f) = 0;
  }
  if (*(int *)(param_1 + 0x5a4) != 0) {
    FUN_0058b3e0();
  }
  return;
}

