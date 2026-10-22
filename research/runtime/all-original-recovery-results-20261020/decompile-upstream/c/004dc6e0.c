
void __fastcall FUN_004dc6e0(int param_1)

{
  FUN_0058b3e0();
  FUN_0058b3e0();
  if (*(int **)(param_1 + 0x80) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x80) + 4))(1);
    *(undefined4 *)(param_1 + 0x80) = 0;
  }
  return;
}

