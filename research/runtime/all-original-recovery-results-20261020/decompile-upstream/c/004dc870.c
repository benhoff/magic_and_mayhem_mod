
void __fastcall FUN_004dc870(int param_1)

{
  FUN_0058b3e0();
  FUN_004a4f70();
  *(undefined4 *)(param_1 + 0xc) = 0xffffffff;
  if (*(int **)(param_1 + 0x130) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x130) + 4))(1);
    *(undefined4 *)(param_1 + 0x130) = 0;
  }
  return;
}

