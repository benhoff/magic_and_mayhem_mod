
void __fastcall FUN_004ac510(int param_1)

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
    do {
      iVar1 = *(int *)(*(int *)(param_1 + 0x53) + iVar3);
      if (iVar1 != 0) {
        FUN_004d27d0();
        FUN_00597870(iVar1);
        *(undefined4 *)(*(int *)(param_1 + 0x53) + iVar3) = 0;
      }
      iVar3 = iVar3 + 4;
    } while (iVar3 < 0xa0);
    iVar3 = 0;
    do {
      iVar1 = *(int *)(*(int *)(param_1 + 0x4f) + iVar3);
      if (iVar1 != 0) {
        FUN_00475c10();
        FUN_00597870(iVar1);
        *(undefined4 *)(*(int *)(param_1 + 0x4f) + iVar3) = 0;
      }
      iVar3 = iVar3 + 4;
    } while (iVar3 < 0xc);
    iVar3 = 0;
    do {
      iVar1 = *(int *)(iVar3 + *(int *)(param_1 + 0x57));
      if (iVar1 != 0) {
        FUN_004cb560();
        FUN_00597870(iVar1);
        *(undefined4 *)(iVar3 + *(int *)(param_1 + 0x57)) = 0;
      }
      iVar3 = iVar3 + 4;
    } while (iVar3 < 0x10);
    iVar3 = 0;
    do {
      iVar1 = *(int *)(*(int *)(param_1 + 0x5f) + iVar3);
      if (iVar1 != 0) {
        FUN_004cdbc0();
        FUN_00597870(iVar1);
        *(undefined4 *)(*(int *)(param_1 + 0x5f) + iVar3) = 0;
      }
      iVar3 = iVar3 + 4;
    } while (iVar3 < 0x44);
    iVar3 = 0;
    do {
      piVar2 = *(int **)(*(int *)(param_1 + 0x4b) + iVar3);
      if (piVar2 != (int *)0x0) {
        (**(code **)(*piVar2 + 4))(1);
        *(undefined4 *)(*(int *)(param_1 + 0x4b) + iVar3) = 0;
      }
      iVar3 = iVar3 + 4;
    } while (iVar3 < 0xc);
    iVar3 = 0;
    do {
      iVar1 = *(int *)(iVar3 + *(int *)(param_1 + 0x5b));
      if (iVar1 != 0) {
        FUN_004d2a30();
        FUN_00597870(iVar1);
        *(undefined4 *)(iVar3 + *(int *)(param_1 + 0x5b)) = 0;
      }
      iVar3 = iVar3 + 4;
    } while (iVar3 < 0x18);
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
    FUN_0058a0a0();
    FUN_004cacb0();
  }
  return;
}

