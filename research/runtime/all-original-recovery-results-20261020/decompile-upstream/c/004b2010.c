
void __fastcall FUN_004b2010(int param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  
  if (*(int *)(param_1 + 8) != 0) {
    *(undefined4 *)(param_1 + 8) = 0;
    if (*(int *)(param_1 + 0x13) != 0) {
      FUN_0058b3e0();
      iVar3 = *(int *)(param_1 + 0x13);
      if (iVar3 != 0) {
        FUN_0058b3e0();
        FUN_00597870(iVar3);
      }
      *(undefined4 *)(param_1 + 0x13) = 0;
    }
    iVar3 = 0;
    if (0 < *(int *)(param_1 + 0x57)) {
      do {
        iVar1 = *(int *)(*(int *)(param_1 + 0x4b) + iVar3 * 4);
        if (iVar1 != 0) {
          FUN_00475c10();
          FUN_00597870(iVar1);
          *(undefined4 *)(*(int *)(param_1 + 0x4b) + iVar3 * 4) = 0;
        }
        iVar3 = iVar3 + 1;
      } while (iVar3 < *(int *)(param_1 + 0x57));
    }
    iVar3 = 0;
    do {
      piVar2 = *(int **)(iVar3 + *(int *)(param_1 + 0x4f));
      if (piVar2 != (int *)0x0) {
        (**(code **)(*piVar2 + 4))(1);
        *(undefined4 *)(iVar3 + *(int *)(param_1 + 0x4f)) = 0;
      }
      iVar3 = iVar3 + 4;
    } while (iVar3 < 8);
    iVar3 = *(int *)(param_1 + 0x3f);
    if (iVar3 != 0) {
      FUN_0059bf07(iVar3,0x141,*(undefined4 *)(iVar3 + -4),FUN_00559880);
      FUN_00597870(iVar3 + -4);
      *(undefined4 *)(param_1 + 0x3f) = 0;
    }
    if (*(int **)(param_1 + 0x47) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0x47) + 4))(1);
      *(undefined4 *)(param_1 + 0x47) = 0;
    }
    iVar3 = *(int *)(param_1 + 0x3b);
    if (iVar3 != 0) {
      FUN_004cefa0();
      FUN_00597870(iVar3);
      *(undefined4 *)(param_1 + 0x3b) = 0;
    }
    FUN_004cacb0();
  }
  return;
}

