
void __fastcall FUN_004bbb60(int param_1)

{
  int *piVar1;
  int iVar2;
  
  FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)(param_1 + 0x13) + 0x20),
               *(undefined4 *)(*(int *)(param_1 + 0x13) + 0x24));
  iVar2 = 0;
  do {
    piVar1 = *(int **)(*(int *)(param_1 + 0x4b) + iVar2);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x10))();
    }
    iVar2 = iVar2 + 4;
  } while (iVar2 < 8);
  if (*(int **)(param_1 + 0x57) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x57) + 0x10))();
  }
  if (**(int **)(param_1 + 0x53) != 0) {
    FUN_004d2800();
    return;
  }
  return;
}

