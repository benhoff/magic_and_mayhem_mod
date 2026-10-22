
void __fastcall FUN_004b40c0(int param_1)

{
  int iVar1;
  int *piVar2;
  int iStack_4;
  
  if (*(int **)(param_1 + 0x6b) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x6b) + 4))(1);
    *(undefined4 *)(param_1 + 0x6b) = 0;
  }
  if (*(int *)(param_1 + 0x13) != 0) {
    FUN_0058b3e0();
    iVar1 = *(int *)(param_1 + 0x13);
    if (iVar1 != 0) {
      FUN_0058b3e0();
      FUN_00597870(iVar1);
    }
    *(undefined4 *)(param_1 + 0x13) = 0;
  }
  if (*(int **)(param_1 + 0x6f) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x6f) + 4))(1);
    *(undefined4 *)(param_1 + 0x6f) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x73);
  if (iVar1 != 0) {
    FUN_0059bf07(iVar1,0x59,*(undefined4 *)(iVar1 + -4),FUN_00475c10);
    FUN_00597870(iVar1 + -4);
    *(undefined4 *)(param_1 + 0x73) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x3f);
  if (iVar1 != 0) {
    FUN_0059bf07(iVar1,0x141,*(undefined4 *)(iVar1 + -4),FUN_00559880);
    FUN_00597870(iVar1 + -4);
    *(undefined4 *)(param_1 + 0x3f) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x77);
  if (iVar1 != 0) {
    FUN_0059bf07(iVar1,0x29,*(undefined4 *)(iVar1 + -4),FUN_004d27d0);
    FUN_00597870(iVar1 + -4);
    *(undefined4 *)(param_1 + 0x77) = 0;
    *(undefined4 *)(param_1 + 0x8b) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x7b);
  if (iVar1 != 0) {
    FUN_0059bf07(iVar1,0x32,*(undefined4 *)(iVar1 + -4),thunk_FUN_0058b3e0);
    FUN_00597870(iVar1 + -4);
    *(undefined4 *)(param_1 + 0x7b) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x7f);
  if (iVar1 != 0) {
    FUN_0059bf07(iVar1,0x2c,*(undefined4 *)(iVar1 + -4),FUN_004dd9a0);
    FUN_00597870(iVar1 + -4);
    *(undefined4 *)(param_1 + 0x7f) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x83);
  if (iVar1 != 0) {
    FUN_0059bf07(iVar1,0x2c,*(undefined4 *)(iVar1 + -4),&DAT_004dd6b0);
    FUN_00597870(iVar1 + -4);
    *(undefined4 *)(param_1 + 0x83) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x87);
  if (iVar1 != 0) {
    FUN_0059bf07(iVar1,0x7a,*(undefined4 *)(iVar1 + -4),FUN_004cb560);
    FUN_00597870(iVar1 + -4);
    *(undefined4 *)(param_1 + 0x87) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x11b);
  if (iVar1 != 0) {
    FUN_00572400();
    FUN_00597870(iVar1);
    *(undefined4 *)(param_1 + 0x11b) = 0;
  }
  if (*(int *)(param_1 + 0x47) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x47));
    *(undefined4 *)(param_1 + 0x47) = 0;
    *(undefined4 *)(param_1 + 0x4b) = 0;
    *(undefined4 *)(param_1 + 0x4f) = 0;
    *(undefined4 *)(param_1 + 0x53) = 0;
  }
  piVar2 = (int *)(param_1 + 0x57);
  iStack_4 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iStack_4 = iStack_4 + -1;
  } while (iStack_4 != 0);
  if (*(int *)(param_1 + 0x67) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x67));
    *(undefined4 *)(param_1 + 0x67) = 0;
  }
  *(undefined4 *)(param_1 + 8) = 0;
  return;
}

