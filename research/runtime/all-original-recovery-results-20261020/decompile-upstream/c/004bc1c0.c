
void __fastcall FUN_004bc1c0(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 8) != 0) {
    *(undefined4 *)(param_1 + 8) = 0;
    if (*(int *)(param_1 + 0x13) != 0) {
      FUN_0058b3e0();
      iVar2 = *(int *)(param_1 + 0x13);
      if (iVar2 != 0) {
        FUN_0058b3e0();
        FUN_00597870(iVar2);
      }
      *(undefined4 *)(param_1 + 0x13) = 0;
    }
    iVar2 = 0;
    do {
      iVar1 = *(int *)(*(int *)(param_1 + 0x4b) + iVar2);
      if (iVar1 != 0) {
        FUN_00475c10();
        FUN_00597870(iVar1);
        *(undefined4 *)(*(int *)(param_1 + 0x4b) + iVar2) = 0;
      }
      iVar2 = iVar2 + 4;
    } while (iVar2 < 8);
    iVar2 = **(int **)(param_1 + 0x4f);
    if (iVar2 != 0) {
      FUN_004d27d0();
      FUN_00597870(iVar2);
      **(undefined4 **)(param_1 + 0x4f) = 0;
    }
    if ((int *)**(int **)(param_1 + 0x53) != (int *)0x0) {
      (**(code **)(*(int *)**(int **)(param_1 + 0x53) + 4))(1);
      **(undefined4 **)(param_1 + 0x53) = 0;
    }
    iVar2 = *(int *)(param_1 + 0x3f);
    if (iVar2 != 0) {
      FUN_0059bf07(iVar2,0x141,*(undefined4 *)(iVar2 + -4),FUN_00559880);
      FUN_00597870(iVar2 + -4);
      *(undefined4 *)(param_1 + 0x3f) = 0;
    }
    if (*(int **)(param_1 + 0x47) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0x47) + 4))(1);
      *(undefined4 *)(param_1 + 0x47) = 0;
    }
    FUN_004cacb0();
  }
  return;
}

