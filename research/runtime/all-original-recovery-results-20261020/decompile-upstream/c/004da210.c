
void __fastcall FUN_004da210(int param_1)

{
  if (*(int **)(param_1 + 0x1bf) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x1bf) + 4))(1);
    *(undefined4 *)(param_1 + 0x1bf) = 0;
  }
  if (*(int *)(param_1 + 0x16d) != 0) {
    FUN_0058b3e0();
  }
  FUN_0058a0a0();
  FUN_004a4f70();
  *(undefined4 *)(param_1 + 0xc) = 0xffffffff;
  return;
}

