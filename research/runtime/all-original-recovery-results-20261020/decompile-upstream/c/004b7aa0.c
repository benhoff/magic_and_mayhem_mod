
void __fastcall FUN_004b7aa0(int param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = *(int *)(param_1 + 0x13);
  if (iVar2 != 0) {
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(iVar2 + 0x20),*(undefined4 *)(iVar2 + 0x24));
    if ((int *)**(int **)(param_1 + 0x57) != (int *)0x0) {
      (**(code **)(*(int *)**(int **)(param_1 + 0x57) + 0x10))();
    }
    iVar2 = 0;
    do {
      if (*(int *)(*(int *)(param_1 + 0x5b) + iVar2) != 0) {
        FUN_004d2800();
      }
      iVar2 = iVar2 + 4;
    } while (iVar2 < 0xa0);
    if (*(int **)(param_1 + 0x4f) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0x4f) + 0x10))();
    }
    if (*(int **)(param_1 + 0x53) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0x53) + 0x10))();
    }
    iVar2 = 0;
    piVar3 = &DAT_00658f25;
    do {
      if ((*piVar3 != -1) &&
         (piVar1 = *(int **)(*(int *)(param_1 + 0x5f) + iVar2), piVar1 != (int *)0x0)) {
        (**(code **)(*piVar1 + 0x10))();
      }
      piVar3 = (int *)((int)piVar3 + 0x133);
      iVar2 = iVar2 + 4;
    } while ((int)piVar3 < 0x6593f1);
    iVar2 = 0x10;
    piVar3 = &DAT_00658f2d;
    do {
      if ((*piVar3 != -1) &&
         (piVar1 = *(int **)(*(int *)(param_1 + 0x5f) + iVar2), piVar1 != (int *)0x0)) {
        (**(code **)(*piVar1 + 0x10))();
      }
      piVar3 = (int *)((int)piVar3 + 0x133);
      iVar2 = iVar2 + 4;
    } while ((int)piVar3 < 0x6593f9);
    if (*(int *)(param_1 + 0x3b) != 0) {
      FUN_004cfa70();
    }
    if (*(int **)(param_1 + 99) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 99) + 0x10))();
    }
  }
  return;
}

