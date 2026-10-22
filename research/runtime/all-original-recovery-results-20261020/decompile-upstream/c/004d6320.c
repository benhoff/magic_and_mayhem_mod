
void __fastcall FUN_004d6320(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x1876) != 0) {
    if (*(int **)(param_1 + 0x1925) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0x1925) + 4))(1);
      *(undefined4 *)(param_1 + 0x1925) = 0;
    }
    FUN_0058b3e0();
    FUN_0058b3e0();
    *(undefined4 *)(param_1 + 0x18ec) = 0;
    if (*(int *)(param_1 + 0x1618) != 0) {
      FUN_0058b3e0();
    }
    if (*(int *)(param_1 + 0x16de) != 0) {
      FUN_0058b3e0();
    }
    if (*(int *)(param_1 + 0x17a4) != 0) {
      FUN_0058b3e0();
    }
    param_1 = param_1 + 0x17a;
    iVar1 = 0x1a;
    do {
      if (*(int *)(param_1 + 4) != 0) {
        FUN_0058b3e0();
      }
      param_1 = param_1 + 0xc6;
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
  }
  return;
}

