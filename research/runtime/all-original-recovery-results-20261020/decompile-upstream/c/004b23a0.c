
void __fastcall FUN_004b23a0(int param_1)

{
  int *piVar1;
  int iVar2;
  
  FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)(param_1 + 0x13) + 0x20),
               *(undefined4 *)(*(int *)(param_1 + 0x13) + 0x24));
  FUN_004b25e0(0);
  iVar2 = 0;
  do {
    piVar1 = *(int **)(*(int *)(param_1 + 0x4b) + iVar2);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x10))();
    }
    iVar2 = iVar2 + 4;
  } while (iVar2 < 0x14);
  if (*(int *)(param_1 + 0x3b) != 0) {
    FUN_004cfa70();
    return;
  }
  return;
}

