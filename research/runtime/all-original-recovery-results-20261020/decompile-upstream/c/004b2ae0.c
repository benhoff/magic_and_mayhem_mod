
void __fastcall FUN_004b2ae0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 8) != 0) {
    *(undefined4 *)(param_1 + 8) = 0;
    if (*(int *)(param_1 + 0x13) != 0) {
      FUN_0058b3e0();
      iVar1 = *(int *)(param_1 + 0x13);
      if (iVar1 != 0) {
        FUN_0058b3e0();
        FUN_00597870(iVar1);
      }
      *(undefined4 *)(param_1 + 0x13) = 0;
    }
    iVar1 = *(int *)(param_1 + 0x4b);
    if (iVar1 != 0) {
      FUN_0059bf07(iVar1,0x59,*(undefined4 *)(iVar1 + -4),FUN_00475c10);
      FUN_00597870(iVar1 + -4);
      *(undefined4 *)(param_1 + 0x4b) = 0;
    }
    iVar1 = *(int *)(param_1 + 0x4f);
    if (iVar1 != 0) {
      FUN_0059bf07(iVar1,0x7a,*(undefined4 *)(iVar1 + -4),FUN_004cb560);
      FUN_00597870(iVar1 + -4);
      *(undefined4 *)(param_1 + 0x4f) = 0;
    }
    iVar1 = *(int *)(param_1 + 0x53);
    if (iVar1 != 0) {
      FUN_0059bf07(iVar1,0x7e,*(undefined4 *)(iVar1 + -4),FUN_004cea90);
      FUN_00597870(iVar1 + -4);
      *(undefined4 *)(param_1 + 0x53) = 0;
    }
    iVar1 = *(int *)(param_1 + 0x57);
    if (iVar1 != 0) {
      FUN_0059bf07(iVar1,0x29,*(undefined4 *)(iVar1 + -4),FUN_004d27d0);
      FUN_00597870(iVar1 + -4);
      *(undefined4 *)(param_1 + 0x57) = 0;
    }
    if (*(int **)(param_1 + 0x5b) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0x5b) + 4))(1);
      *(undefined4 *)(param_1 + 0x5b) = 0;
    }
    iVar1 = *(int *)(param_1 + 0x3f);
    if (iVar1 != 0) {
      FUN_0059bf07(iVar1,0x141,*(undefined4 *)(iVar1 + -4),FUN_00559880);
      FUN_00597870(iVar1 + -4);
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

