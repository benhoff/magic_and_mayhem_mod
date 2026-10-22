
void __thiscall FUN_00547ae0(int param_1,int param_2,int param_3)

{
  if ((param_2 != *(int *)(param_1 + 4)) || (param_3 != *(int *)(param_1 + 8))) {
    if (param_2 < 1) {
      param_2 = 1;
    }
    else if (0x10 < param_2) {
      param_2 = 0x10;
    }
    *(int *)(param_1 + 4) = param_2;
    if (param_3 < 1) {
      param_3 = 1;
    }
    else if (0x10 < param_3) {
      param_3 = 0x10;
    }
    *(int *)(param_1 + 8) = param_3;
    if (*(int *)(param_1 + 0x12) != 0) {
      FUN_00597870(*(int *)(param_1 + 0x12));
      *(undefined4 *)(param_1 + 0x12) = 0;
      FUN_00547b70();
    }
  }
  return;
}

