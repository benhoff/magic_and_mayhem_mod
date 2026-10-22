
void __fastcall FUN_00463e20(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = 3;
  iVar2 = param_1;
  do {
    FUN_0058b3e0();
    FUN_0058b3e0();
    if (*(int **)(iVar2 + 0x93) != (int *)0x0) {
      (**(code **)(**(int **)(iVar2 + 0x93) + 4))(1);
    }
    *(undefined4 *)(iVar2 + 0x93) = 0;
    FUN_004a4f70();
    *(undefined4 *)(iVar2 + 4) = 0xffffffff;
    if (*(int *)(iVar2 + 0x89) != 0) {
      *(undefined4 *)(*(int *)(iVar2 + 0x89) + 0x967) = 0;
      *(undefined4 *)(iVar2 + 0x89) = 0;
    }
    iVar1 = iVar1 + -1;
    *(undefined4 *)(iVar2 + 0x8e) = 0;
    iVar2 = iVar2 + 0x97;
  } while (iVar1 != 0);
  *(undefined1 *)(param_1 + 0x1c9) = 0;
  *(undefined4 *)(param_1 + 0x1c5) = 0;
  *(undefined1 *)(param_1 + 0x1ca) = 0;
  return;
}

